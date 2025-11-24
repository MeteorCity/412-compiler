#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>

#include "graph.h"
#include "ir.h"

class Scheduler {
    public:
        Graph dep_graph;

        void buildGraph(IRNode *root);
        void computeNodePriorities();
        int schedule(IRNode *root);
};