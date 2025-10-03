#pragma once
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ir.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::to_string;
using std::tuple;
using std::vector;

const int INF = 1e9;

class Renamer {
    public:
        void rename_IR(int operations, int maxSR, IRNode *root);
};