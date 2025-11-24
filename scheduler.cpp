#include "graph.h"
#include "ir.h"
#include "scheduler.h"

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
            dep_graph.addEdge(node, to_node, 0);
        }

        if (root->opcode == LOAD) {
            // Add a conflict edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, 6);
            }

            prevLoads.push_back(node);
        } else if (root->opcode == OUTPUT) {
            // Add a conflict edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, 6);
            }

            // Add a serialization edge to the most recent ouput
            if (prevOutputs.size() != 0) {
                dep_graph.addEdge(node, prevOutputs[prevOutputs.size() - 1], 1);
            }

            prevOutputs.push_back(node);
        } else if (root->opcode == STORE) {
            // Add a serialization edge to the most recent store
            if (lastStore != -1) {
                dep_graph.addEdge(node, lastStore, 1);
            }

            // Add a serialization edge to each previous load and output
            for (int load : prevLoads) {
                dep_graph.addEdge(node, load, 1);
            }
            for (int output : prevOutputs) {
                dep_graph.addEdge(node, output, 1);
            }

            prevLoads.clear();
            prevOutputs.clear();
            lastStore = node;
        }

        root = root->next.get();
    }
}

void Scheduler::computeNodePriorities() {
    return;
}

int Scheduler::schedule(IRNode *root) {
    // Build the dependency graph
    buildGraph(root);

    // Compute the priorities of each node
    computeNodePriorities();

    return 0;
}