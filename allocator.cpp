#include "ir.h"
#include "allocator.h"

using std::cout;
using std::endl;
using std::greater;
using std::string;
using std::unordered_map;
using std::upper_bound;
using std::vector;

const int INF = 1e9;

Allocator::Allocator(int maxlive) : maxlive(maxlive) {}

void Allocator::insert_pr(int pr, vector<int> &freePRs) {
    auto it = upper_bound(freePRs.begin(), freePRs.end(), pr, greater<int>());
    freePRs.insert(it, pr);
}

void Allocator::allocate(IRNode *root) {
    unordered_map<int, int> VRToPR;
    vector<int> PRToVR;
    unordered_map<int, int> VRToSpillLoc;
    vector<int> PRToNU;
    vector<int> freePRs;

    for (int i = 0; i < maxlive; i++) {
        PRToVR.push_back(-1);
        PRToNU.push_back(-1);
        freePRs.push_back(i);
    }

    root = root->next.get(); // Skip the root dummy node

    // Iterate until we reach end of linked list
    while (root != nullptr) {
        auto [defs, uses] = root->getDefsAndUses();
        for (Operand* use : uses) {
            // VR has no associated PR
            if (VRToPR.find(use->vr) == VRToPR.end()) {
                if (freePRs.empty()) { // No free PR
                    // Handle logic
                } else { // A free PR exists
                    int pr = freePRs.back();
                    freePRs.pop_back();
                    use->pr = pr;
                    VRToPR[use->vr] = pr;
                    PRToVR[pr] = use->vr;
                    PRToNU[pr] = use->nu;
                }
            }
        }

        for (Operand* use : uses) {
            // Free the PR if this is the last use
            if (use->nu == INF) {
                int vr = use->vr;
                int pr = use->pr;
                VRToPR.erase(vr);
                PRToVR[pr] = -1;
                PRToNU[pr] = -1;
                insert_pr(pr, freePRs);
            }
        }

        for (Operand* def : defs) {
            if (freePRs.empty()) { // No free PR
                // Handle logic
            } else { // A free PR exists
                int pr = freePRs.back();
                freePRs.pop_back();
                def->pr = pr;
                VRToPR[def->vr] = pr;
                PRToVR[pr] = def->vr;
                PRToNU[pr] = def->nu;
            }
        }

        root = root->next.get();
    }
}