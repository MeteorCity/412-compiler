#include "ir.h"

using std::array;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

// Map ints to lexemes
array<string, 10> lex_mapping = {
    "load", "store", "loadI", "add", "sub",
    "mult", "lshift", "rshift", "output", "nop"
};

string IRNode::toString() {
    string prefix1 = opcode == LOADI || opcode == OUTPUT ? "val" : "sr";
    string sr_prefix = "sr";

    return lex_mapping[opcode] + "\t" +
            op1.toString(prefix1) + ", " +
            op2.toString() + ", " +
            op3.toString();
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
    }

    return {defs, uses};
}