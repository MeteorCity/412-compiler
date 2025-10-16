#pragma once
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ir.h"

class Renamer {
    public:
        int rename_IR(int operations, int maxSR, IRNode *root);
};