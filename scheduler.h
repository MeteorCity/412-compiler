#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

#include "graph.h"
#include "ir.h"

class Scheduler {
    public:
        Graph dep_graph;

        void build_graph(IRNode *root);
        int schedule(/*IRNode *root*/);
};