#pragma once
#include <string>
#include <memory>

struct OutputNode {
    std::string operation1;
    std::string operation2;
    std::unique_ptr<OutputNode> next;

    OutputNode(std::string op1, std::string op2)
        : operation1(op1), operation2(op2), next(nullptr) {}
    
    // Custom destructor to avoid recursion causing stack overflow
    ~OutputNode() {
        OutputNode* current = next.release(); // release ownership
        while (current) {
            OutputNode* tmp = current->next.release();
            delete current;
            current = tmp;
        }
    }
    
    std::string toString();
};
