#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <utility>
#include <queue>

#include "ir.h"

enum EdgeTypes {
    NORMAL,
    SERIAL,
    CONFLICT
};

struct Node {
    int id; // Unique node id
    int opcode;
    Operand op1;
    Operand op2;
    Operand op3;
    std::string opString;
    int priority = 0;
    bool issued = false;
    bool retired = false;
};

struct Edge {
    int to_node;
    int edgeType;
    int latency;
};

class Graph {
    public:
        std::vector<Node> nodes;
        std::vector<std::vector<Edge>> edges;
        std::vector<std::vector<Edge>> revEdges;

        // Add a new node and return its internal ID
        int addNode(IRNode *operation);

        // Add an edge u → v
        void addEdge(int from, int to, int edgeType, int latency);

        std::vector<int> getDependencies(int id);
        std::vector<int> getUsers(int id);
        std::priority_queue<std::pair<int,int>> getLeafHeap();
        std::string toDot();
};