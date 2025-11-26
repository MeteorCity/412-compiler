#pragma once
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "graph.h"
#include "ir.h"
#include "output.h"

class Scheduler {
    public:
        Graph dep_graph;

        bool isValidOp(int opcode, int unit, bool seenOutput);
        void buildGraph(IRNode *root);
        void computeNodePriorities();
        int schedule(IRNode *root, OutputNode *outputRoot);
};