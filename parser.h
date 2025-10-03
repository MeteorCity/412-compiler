#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <string>

#include "ir.h"
#include "scanner.h"

using std::array;
using std::cout;
using std::endl;
using std::make_unique;
using std::string;
using std::stoi;
using std::to_string;

class Parser {
    int operations = 0;
    Scanner &scanner;

    private:
        void insert_new_node(int line, int opcode, int r1, int r2, int r3);

    public:
        IRNode *root;
        int maxSR = -1;
        
        explicit Parser(Scanner &scanner, IRNode *root);
        int parse_file();
};