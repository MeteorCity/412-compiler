#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>

using std::array;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

extern array<string, 10> lex_mapping;

struct IRNode {
    int line_number;
    int opcode;
    int sr1, vr1, pr1, nu1;
    int sr2, vr2, pr2, nu2;
    int sr3, vr3, pr3, nu3;
    IRNode* prev;
    unique_ptr<IRNode> next;

    IRNode(int line, int op, int s1, int s2, int s3, IRNode *previous)
        : line_number(line), opcode(op),
          sr1(s1), vr1(-1), pr1(-1), nu1(-1),
          sr2(s2), vr2(-1), pr2(-1), nu2(-1),
          sr3(s3), vr3(-1), pr3(-1), nu3(-1),
          prev(previous), next(nullptr) {}

    string toString();
    string toLine();
    std::pair<vector<int>, vector<int>> getDefsAndUses();
};