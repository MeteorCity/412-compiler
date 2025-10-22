#include "ir.h"
#include "renamer.h"

using std::vector;

const int INF = 1e9;

int Renamer::rename_IR(int operations, int maxSR, IRNode *root) {
    int VRName = 0;
    int SRToVR[maxSR + 1];
    int LU[maxSR + 1];

    for (int i = 0; i <= maxSR; i++) {
        SRToVR[i] = -1; // Set invalid
        LU[i] = INF; // Set infinity
    }

    int index = operations;

    // Live variable tracking
    vector<bool> liveNow(maxSR + 1, false);
    int liveCount = 0;
    int maxlive = 0;

    // Iterate until we reach dummy
    while (root && root->line_number != -1) {
        // Ignore nop
        if (root->opcode == NOP) {
            root=root->prev;
            index--;
            continue;
        }

        auto [defs, uses] = root->getDefsAndUses();

        int unusedDef = 0;
        for (Operand* def : defs) {
            if (liveNow[def->sr]) {
                liveNow[def->sr] = false;
                liveCount--;
            } else {
                unusedDef++;
            }

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

            if (!liveNow[use->sr]) {
                liveNow[use->sr] = true;
                liveCount++;
            }
        }

        for (Operand* use : uses) {
            LU[use->sr] = index;
        }

        if (liveCount + unusedDef > maxlive) {
            maxlive = liveCount + unusedDef;
        }

        index--;
        root = root->prev;
    }

    return maxlive;
}