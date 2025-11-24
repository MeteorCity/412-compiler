#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include "ir.h"

struct Node {
    int id; // Unique node id
    int opcode;
    Operand op1;
    Operand op2;
    Operand op3;
    std::string opString;
};

struct Edge {
    int to_node;
    int latency;
};

class Graph {
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> edges;
    std::vector<std::vector<Edge>> revEdges;

    public:
        // Add a new node and return its internal ID
        int addNode(IRNode *operation);

        // Add an edge u → v
        void addEdge(int from, int to, int latency);

        std::vector<Edge>& getSuccessors(int id);
        std::vector<Edge>& getPredecessors(int id);
        std::string toDot();
};