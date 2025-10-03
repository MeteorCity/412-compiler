#include "ir.h"

// Map ints to lexemes
array<string, 10> lex_mapping = {
    "load", "store", "loadI", "add", "sub",
    "mult", "lshift", "rshift", "output", "nop"
};

// LAB 1 toString() function
string IRNode::toString() {
    // MEMOP
    if (opcode < 2) {
        return lex_mapping[opcode] + "\t" + "[ sr" + to_string(sr1)
                + " ], [ ], [ sr" + to_string(sr3) + " ]";
    }

    // LOADI
    else if (opcode == 2) {
        return lex_mapping[opcode] + "\t" + "[ val " + to_string(sr1)
                + " ], [ ], [ sr" + to_string(sr3) + " ]";
    }

    // ARITHOP
    else if (opcode < 8) {
        return lex_mapping[opcode] + "\t" + "[ sr" + to_string(sr1) + " ], [ sr"
                + to_string(sr2) + " ], [ sr" + to_string(sr3) + " ]";
    }

    // OUTPUT
    else if (opcode == 8) {
        return lex_mapping[opcode] + "\t" + "[ val " + to_string(sr1) + " ], [ ], [ ]";
    }

    // NOP
    else if (opcode == 9) {
        return lex_mapping[opcode] + "\t" + "[ ], [ ], [ ]";
    }

    return "UNKNOWN OPCODE";
}

// LAB 2 toLine() function
string IRNode::toLine() {
    if (opcode < 2) {
        return lex_mapping[opcode] + " r" + to_string(vr1) + " => r" + to_string(vr3);
    }

    // LOADI
    else if (opcode == 2) {
        return lex_mapping[opcode] + " " + to_string(sr1) + " => r" + to_string(vr3);
    }

    // ARITHOP
    else if (opcode < 8) {
        return lex_mapping[opcode] + " r" + to_string(vr1) + ", r" + to_string(vr2) + " => r" + to_string(vr3);
    }

    // OUTPUT
    else if (opcode == 8) {
        return lex_mapping[opcode] + " " + to_string(sr1);
    }

    // NOP
    else if (opcode == 9) {
        return "nop";
    }

    return "UNKNOWN OPCODE";
}

std::pair<vector<int>, vector<int>> IRNode::getDefsAndUses() {
    vector<int> defs;
    vector<int> uses;
    
    // LOAD
    if (opcode == 0) {
        defs.push_back(3);
        uses.push_back(1);
    }

    // STORE
    else if (opcode == 1) {
        uses.push_back(1);
        uses.push_back(3);
    }

    // LOADI
    else if (opcode == 2) {
        defs.push_back(3);
    }

    // ARITHOP
    else if (opcode < 8) {
        defs.push_back(3);
        uses.push_back(1);
        uses.push_back(2);
    }

    return {defs, uses};
}