#include "ir.h"
#include "renamer.h"

tuple<int&, int&, int&> getSRVRNU(IRNode *node, int operand) {
    if (operand == 1) return {node->sr1, node->vr1, node->nu1};
    if (operand == 2) return {node->sr2, node->vr2, node->nu2};
    if (operand == 3) return {node->sr3, node->vr3, node->nu3};
    throw std::runtime_error("Operand not in valid range 1-3");
}

void Renamer::rename_IR(int operations, int maxSR, IRNode *root) {
    int VRName = 0;
    int SRToVR[maxSR + 1];
    int LU[maxSR + 1];

    for (int i = 0; i <= maxSR; i++) {
        SRToVR[i] = -1; // Set invalid
        LU[i] = INF; // Set infinity
    }

    int index = operations;

    // Iterate until we reach dummy
    while (root && root->sr1 != -1) {
        // cout << "BEFORE " + root->toLine() << endl;
        // Ignore nop
        if (root->opcode == 9) {
            root=root->prev;
            continue;
        }

        auto [defs, uses] = root->getDefsAndUses();
        for (int def : defs) {
            // cout << "Def " + to_string(def) << endl;
            auto [sr, vr, nu] = getSRVRNU(root, def);
            if (SRToVR[sr] == -1) { // Unused def
                SRToVR[sr] = VRName++;
            }
            vr = SRToVR[sr];
            nu = LU[sr];
            SRToVR[sr] = -1; // Kill OP3
            LU[sr] = INF;
        }

        for (int use : uses) {
            // cout << "Use " + to_string(use) << endl;
            auto [sr, vr, nu] = getSRVRNU(root, use);
            if (SRToVR[sr] == -1) { // Last use
                SRToVR[sr] = VRName++;
            }
            vr = SRToVR[sr];
            nu = LU[sr];
        }

        for (int use : uses) {
            auto [sr, vr, nu] = getSRVRNU(root, use);
            LU[sr] = index;
        }

        index--;
        // cout << "AFTER " + root->toLine() << endl;
        root = root->prev;
    }
}