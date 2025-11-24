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

            // Determine the edge kind by latency
            std::string kind;
            if (e.latency == 0) kind = "Data";
            else if (e.latency == 1) kind = "Serial";
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

    // Add the node with currently uncalculated priority (-1)
    nodes.push_back({id, opcode, op1, op2, op3, opString, -1});
    edges.emplace_back();
    revEdges.emplace_back();
    return id;
}

void Graph::addEdge(int from, int to, int latency) {
    // Look for existing edge
    for (auto &e : edges[from]) {
        if (e.to_node == to) {
            // Keep the edge with smaller latency
            if (latency < e.latency) {
                e.latency = latency;

                // Update reverse edge as well
                for (auto &re : revEdges[to]) {
                    if (re.to_node == from) {
                        re.latency = latency;
                        break;
                    }
                }
            }
            return;
        }
    }

    // There's no existing edge so add normally
    edges[from].push_back({to, latency});
    revEdges[to].push_back({from, latency});
}

std::vector<Edge>& Graph::getDependencies(int id) {
    return edges[id];
}

 std::vector<Edge>& Graph::getUsers(int id) {
    return revEdges[id];
}