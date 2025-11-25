#include "graph.h"

// Helper to escape double quotes and backslashes
static std::string escapeForDot(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '"') {
            out.push_back('\\');
            out.push_back(c);
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::string Graph::toDot() {
    std::ostringstream out;
    out << "digraph DG {\n";

    // Get nodes
    for (const Node &n : nodes) {
        std::string label = std::to_string(n.id) + ": " + n.opString;
        out << n.id << " [label=\"" << escapeForDot(label) << "\" ];\n";
    }

    out << "\n";

    // Get edges
    for (size_t u = 0; u < edges.size(); ++u) {
        const Node &fromNode = nodes[u];
        for (const Edge &e : edges[u]) {
            int toIndex = e.to_node;
            if (toIndex < 0 || static_cast<size_t>(toIndex) >= nodes.size()) continue;

            const Node &toNode = nodes[toIndex];

            // Determine the edge kind
            std::string kind;
            if (e.edgeType == NORMAL) kind = "Data";
            else if (e.edgeType == SERIAL) kind = "Serial";
            else kind = "Conflict";

            // Get the toNode's vr (the previously defined vr we're using in this operation)
            std::string vr = "vr" + std::to_string(toNode.op3.vr);
            std::string label = " " + kind;
            if (kind == "Data") {
                label += ", " + vr;
            }

            // Output the edge in DOT format
            out << fromNode.id << " -> " << toNode.id
                << " [ label=\"" << escapeForDot(label) << "\" ];\n";
        }
    }

    out << "}\n";
    return out.str();
}

int Graph::addNode(IRNode *operation) {
    int id = nodes.size();
    int opcode = operation->opcode;
    Operand op1 = operation->op1;
    Operand op2 = operation->op2;
    Operand op3 = operation->op3;
    std::string opString = operation->toString();

    // Add the node with 0 priority and false retired flag
    nodes.push_back({id, opcode, op1, op2, op3, opString});
    edges.emplace_back();
    revEdges.emplace_back();
    return id;
}

void Graph::addEdge(int from, int to, int edgeType, int latency) {
    // Look for existing edge
    for (auto &e : edges[from]) {
        if (e.to_node == to) {
            // Keep the edge with larger latency
            if (latency > e.latency) {
                e.latency = latency;
                e.edgeType = edgeType;

                // Update reverse edge as well
                for (auto &re : revEdges[to]) {
                    if (re.to_node == from) {
                        re.latency = latency;
                        re.edgeType = edgeType;
                        break;
                    }
                }
            }
            return;
        }
    }

    // There's no existing edge so add normally
    edges[from].push_back({to, edgeType, latency});
    revEdges[to].push_back({from, edgeType, latency});
}

std::vector<int> Graph::getDependencies(int id) {
    std::vector<int> out;
    for (const Edge& e : edges[id]) out.push_back(e.to_node);
    return out;
}

std::vector<int> Graph::getUsers(int id) {
    std::vector<int> out;
    for (const Edge& e : revEdges[id]) out.push_back(e.to_node);
    return out;
}

std::priority_queue<std::pair<int,int>> Graph::getLeafHeap() {
    std::priority_queue<std::pair<int,int>> pq;
    for (int i = 0, n = (int)nodes.size(); i < n; ++i) {
        if (edges[i].empty()) {
            pq.push({ nodes[i].priority, nodes[i].id });
        }
    }
    return pq;
}