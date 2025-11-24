#include "graph.h"
#include "ir.h"
#include "scheduler.h"

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

int Scheduler::schedule(IRNode *root) {
    // Build the dependency graph
    buildGraph(root);

    // Compute the priorities of each node
    computeNodePriorities();

    for (Node n : dep_graph.nodes) {
        std::string priority = std::to_string(n.priority);
        std::cout << "Priority: " << priority << std::endl;
    }

    return 0;
}