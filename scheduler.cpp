#include "graph.h"
#include "ir.h"
#include "output.h"
#include "scheduler.h"

const int NUM_UNITS = 2;

static int getLatency(int opcode) {
    switch (opcode) {
        case LOAD:
        case STORE:
            return 6;
        case MULT:
            return 3;
        case LOADI:
        case ADD:
        case SUB:
        case LSHIFT:
        case RSHIFT:
        case OUTPUT:
        case NOP:
            return 1;
        default:
            throw std::runtime_error("Invalid opcode encountered: " + std::to_string(opcode));
    }
}

// Print all elements in priority_queue without modifying original
void printHeap(std::priority_queue<std::pair<int,int>> heap) {
    std::cout << "Heap contents (priority_queue):\n";
    while (!heap.empty()) {
        auto [first, second] = heap.top();
        heap.pop();
        std::cout << "(" << first << ", " << second << ")\n";
    }
}

// Print all elements in unordered_map<int, vector<int>>
void printDictionary(const std::unordered_map<int, std::vector<int>>& dict) {
    std::cout << "Dictionary contents (unordered_map<int, vector<int>>):\n";
    for (const auto& [key, vec] : dict) {
        std::cout << key << ": [";
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << vec[i];
            if (i + 1 < vec.size())
                std::cout << ", ";
        }
        std::cout << "]\n";
    }
}

void Scheduler::buildGraph(IRNode *root) {
    std::unordered_map<int, int> map; // Maps VRs to node IDs
    int lastStore = -1;
    std::vector<int> prevLoads;
    std::vector<int> prevOutputs;

    root = root->next.get(); // Skip the root dummy node
    
    // Iterate until we reach end of linked list
    while (root != nullptr) {
        int node = dep_graph.addNode(root);

        auto [defs, uses] = root->getDefsAndUses();
        for (Operand* def : defs) {
            map[def->vr] = node;
        }
        for (Operand* use : uses) {
            int to_node = map[use->vr];
            int to_opcode = dep_graph.nodes[to_node].opcode;
            int latency = getLatency(to_opcode);
            dep_graph.addEdge(node, to_node, NORMAL, latency);
        }

        if (root->opcode == LOAD) {
            // Add a conflict edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, CONFLICT, 6);
            }

            prevLoads.push_back(node);
        } else if (root->opcode == OUTPUT) {
            // Add a conflict edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, CONFLICT, 6);
            }

            // Add a serialization edge to the most recent ouput
            if (prevOutputs.size() != 0) {
                dep_graph.addEdge(node, prevOutputs[prevOutputs.size() - 1], SERIAL, 1);
            }

            prevOutputs.push_back(node);
        } else if (root->opcode == STORE) {
            // Add a serialization edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, SERIAL, 1);
            }

            // Add a serialization edge to each previous load and output
            for (int load : prevLoads) {
                dep_graph.addEdge(node, load, SERIAL, 1);
            }
            for (int output : prevOutputs) {
                dep_graph.addEdge(node, output, SERIAL, 1);
            }

            prevLoads.clear();
            prevOutputs.clear();
            lastStore = node;
        }

        root = root->next.get();
    }
}

void Scheduler::computeNodePriorities() {
    // If no nodes return early
    const size_t n = dep_graph.nodes.size();
    if (n == 0) return;

    // Store the number of dependencies for each node
    std::vector<int> indeg(n, 0);
    for (size_t i = 0; i < n; ++i) {
        indeg[i] = static_cast<int>(dep_graph.edges[i].size());
    }

    // Kahn's algorithm: start with nodes that have indeg of 0
    std::queue<int> queue;
    for (size_t i = 0; i < n; ++i) {
        if (indeg[i] == 0) queue.push(static_cast<int>(i));
    }

    // Get valid topographical ordering
    std::vector<int> topo;
    topo.reserve(n);
    while (!queue.empty()) {
        int u = queue.front(); queue.pop();
        topo.push_back(u);

        // Uses are stored in revEdges[u]
        for (const Edge &e : dep_graph.revEdges[u]) {
            int v = e.to_node; // Use
            --indeg[v];
            if (indeg[v] == 0) queue.push(v);
        }
    }

    // Populate node priorities
    for (int i = (int)topo.size() - 1; i >= 0; --i) {
        int u = topo[i];

        int best = 0;
        for (const Edge &e : dep_graph.revEdges[u]) {
            int v = e.to_node; // Use
            int cand = dep_graph.nodes[v].priority + e.latency;
            if (cand > best) best = cand;
        }

        dep_graph.nodes[u].priority = best;
    }
}

int Scheduler::schedule(IRNode *root, OutputNode *outputRoot) {
    // Build the dependency graph
    buildGraph(root);

    // Compute the priorities of each node
    computeNodePriorities();

    // for (Node n : dep_graph.nodes) {
    //     std::string priority = std::to_string(n.priority);
    //     std::cout << "Priority: " << priority << std::endl;
    // }

    int cycle = 1;
    std::priority_queue<std::pair<int,int>> ready = dep_graph.getLeafHeap();
    std::unordered_map<int, std::vector<int>> active;

    while (ready.size() != 0 || active.size() != 0) {
        std::vector<int> movedOps;
        auto new_node = std::make_unique<OutputNode>("", "");
        for (int i = 0; i < NUM_UNITS; ++i) {
            if (ready.size() != 0) {
                // Get the operation with the highest priority
                int op = ready.top().second;
                ready.pop();

                // Add the operation to the functional unit
                std::string opString = dep_graph.nodes[op].opString;
                if (i == 0) { // Terrible practice but whatever
                    new_node->operation1 = opString;
                } else if (i == 1) {
                    new_node->operation2 = opString;
                }

                // Move the operation from ready to active
                int finish_cycle = cycle + getLatency(dep_graph.nodes[op].opcode);
                active[finish_cycle].push_back(op);
                movedOps.push_back(op);
            } else {
                if (i == 0) { // Terrible practice but whatever
                    new_node->operation1 = "nop";
                } else if (i == 1) {
                    new_node->operation2 = "nop";
                }
            }
        }

        // std::cout << "new_node: " << new_node->toString() << std::endl;

        // Add the new node to the output
        outputRoot->next = std::move(new_node);
        outputRoot = outputRoot->next.get();

        ++cycle;

        // Find each op in active that retires
        if (active.count(cycle)) {
            for (int op : active[cycle]) {
                // Retire the operation
                dep_graph.nodes[op].retired = true;

                for (int user : dep_graph.getUsers(op)) {
                    // Ensure user hasn't already been issued
                    if (dep_graph.nodes[user].issued) continue;

                    // Calculate whether all the user's dependencies have retired
                    bool allRetired = true;
                    for (int dependency : dep_graph.getDependencies(user)) {
                        if (!dep_graph.nodes[dependency].retired) {
                            allRetired = false;
                            break;
                        }
                    }

                    // If all the dependencies have retired, add user to ready
                    if (allRetired) {
                        ready.emplace(dep_graph.nodes[user].priority, user);
                        dep_graph.nodes[user].issued = true;
                    }
                }
            }
            active.erase(cycle); // Delete the key from active
        }

        // Check whether any of the movedOps has any users that can be early released
        for (int op : movedOps) {
            int opcode = dep_graph.nodes[op].opcode;
            if (opcode == LOAD || opcode == STORE || opcode == OUTPUT) {
                for (const Edge &e : dep_graph.revEdges[op]) {
                    if (e.edgeType == SERIAL) {
                        int early = e.to_node;
                        if (dep_graph.nodes[early].issued) continue;
                        ready.emplace(dep_graph.nodes[early].priority, early);
                        dep_graph.nodes[early].issued = true;
                    }
                }
            }
        }

        // printHeap(ready);
        // printDictionary(active);
    }
    
    return 0;
}