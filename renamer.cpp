#include "ir.h"
#include "renamer.h"

const int INF = 1e9;

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
    while (root && root->line_number != -1) {
        // Ignore nop
        if (root->opcode == NOP) {
            root=root->prev;
            continue;
        }

        auto [defs, uses] = root->getDefsAndUses();
        for (Operand* def : defs) {
            // cout << "Def " + to_string(def) << endl;
            if (SRToVR[def->sr] == -1) { // Unused def
                SRToVR[def->sr] = VRName++;
            }
            def->vr = SRToVR[def->sr];
            def->nu = LU[def->sr];
            SRToVR[def->sr] = -1; // Kill OP3
            LU[def->sr] = INF;
        }

        for (Operand* use : uses) {
            // cout << "Use " + to_string(use) << endl;
            if (SRToVR[use->sr] == -1) { // Last use
                SRToVR[use->sr] = VRName++;
            }
            use->vr = SRToVR[use->sr];
            use->nu = LU[use->sr];
        }

        for (Operand* use : uses) {
            LU[use->sr] = index;
        }

        index--;
        root = root->prev;
    }
}