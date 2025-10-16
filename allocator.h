#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ir.h"

class Allocator {
    int nextSpillLoc = 32768;
    int k;
    int maxlive;
    IRNode *root;
    // TODO: Evaluate list vs map, list is more time efficient, map is more memory efficient
    std::unordered_map<int, int> VRToPR;
    std::vector<int> PRToVR;
    std::unordered_map<int, int> VRToSpillLoc;
    std::vector<int> PRToNU;
    std::vector<bool> PRMarked;
    std::vector<int> freePRs;

    public:
        explicit Allocator(int k, int maxlive, IRNode *root);
        void insert(int opcode, int s1, int p1, int p3);
        void spill(int pr);
        void restore(int vr, int pr);
        int getPR(int vr, int nu);
        void freePR(int pr);
        void allocate();
};