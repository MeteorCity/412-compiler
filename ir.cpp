#include "ir.h"

// Map ints to lexemes
array<string, 10> lex_mapping = {
    "load", "store", "loadI", "add", "sub",
    "mult", "lshift", "rshift", "output", "nop"
};

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