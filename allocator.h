#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ir.h"

class Allocator {
    int maxlive;

    public:
        explicit Allocator(int maxlive);
        void insert_pr(int pr, std::vector<int> &freePRs);
        void allocate(IRNode *root);
};