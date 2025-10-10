#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>

enum Opcode {
    LOAD,
    STORE,
    LOADI,
    ADD,
    SUB,
    MULT,
    LSHIFT,
    RSHIFT,
    OUTPUT,
    NOP
};

struct Operand {
    int sr, vr, pr, nu;

    Operand(int sr, int vr, int pr, int nu)
        : sr(sr), vr(vr), pr(pr), nu(nu) {}

    std::string toString(std::string prefix = "sr");
};

struct IRNode {
    int line_number;
    int opcode;
    Operand op1;
    Operand op2;
    Operand op3;
    IRNode* prev;
    std::unique_ptr<IRNode> next;

    IRNode(int line, int op, int s1, int s2, int s3, IRNode *previous)
        : line_number(line), opcode(op),
          op1(Operand(s1, -1, -1, -1)),
          op2(Operand(s2, -1, -1, -1)),
          op3(Operand(s3, -1, -1, -1)),
          prev(previous), next(nullptr) {}

    std::string toString();
    std::string toLine1();
    std::string toLine2();
    std::pair<std::vector<Operand*>, std::vector<Operand*>> getDefsAndUses();
};