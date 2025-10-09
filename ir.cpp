#include "ir.h"

// Map ints to lexemes
array<string, 10> lex_mapping = {
    "load", "store", "loadI", "add", "sub",
    "mult", "lshift", "rshift", "output", "nop"
};

// LAB 1 toString() function
string IRNode::toString() {
    string prefix1 = opcode == LOADI || opcode == OUTPUT ? "val" : "sr";
    string sr_prefix = "sr";

    return lex_mapping[opcode] + "\t" +
            op1.toString(prefix1) + ", " +
            op2.toString() + ", " +
            op3.toString();
}

// LAB 2 toLine() function
string IRNode::toLine() {
    switch (opcode) {
        case LOAD:
        case STORE:
            return lex_mapping[opcode] + " r" + to_string(op1.vr) + " => r" + to_string(op3.vr);
        case LOADI:
            return lex_mapping[opcode] + " " + to_string(op1.sr) + " => r" + to_string(op3.vr);
        case ADD:
        case SUB:
        case MULT:
        case LSHIFT:
        case RSHIFT:
            return lex_mapping[opcode] + " r" + to_string(op1.vr) + ", r" + to_string(op2.vr) + " => r" + to_string(op3.vr);
        case OUTPUT:
            return lex_mapping[opcode] + " " + to_string(op1.sr);
        case NOP:
            return "nop";
        default:
            return "UNKNOWN OPCODE";
    }
}


// Default value for prefix is "sr"
string Operand::toString(string prefix) {
    if (sr == -1) {
        return "[ ]";
    }
    return "[ " + prefix + " " + to_string(sr) + " ]";
}

std::pair<vector<Operand*>, vector<Operand*>> IRNode::getDefsAndUses() {
    vector<Operand*> defs;
    vector<Operand*> uses;
    
    switch (opcode) {
        case LOAD:
            defs.push_back(&op3);
            uses.push_back(&op1);
            break;
        case STORE:
            uses.push_back(&op1);
            uses.push_back(&op3);
            break;
        case LOADI:
            defs.push_back(&op3);
            break;
        case ADD:
        case SUB:
        case MULT:
        case LSHIFT:
        case RSHIFT:
            defs.push_back(&op3);
            uses.push_back(&op1);
            uses.push_back(&op2);
            break;
        case OUTPUT:
            uses.push_back(&op1);
            break;
    }

    return {defs, uses};
}