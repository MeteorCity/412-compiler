#include "ir.h"
#include "allocator.h"

using std::cout;
using std::cerr;
using std::endl;
using std::greater;
using std::make_unique;
using std::runtime_error;
using std::string;
using std::unordered_map;
using std::upper_bound;
using std::vector;

const int INF = 1e9;

Allocator::Allocator(int k, int maxlive, IRNode *root) : k(k), maxlive(maxlive), root(root) {}

void Allocator::insert(int opcode, int s1, int p1, int p3) {
    if (!root || !root->prev) {
        // Can't insert before if root is null or has no previous node
        throw runtime_error("Insert not possible due to null root or null prev node");
    }

    IRNode* prev = root->prev;

    // Move ownership of the previous node's next pointer (which currently points to root)
    std::unique_ptr<IRNode> old_next = std::move(prev->next);

    // Create the new node that goes between prev and root
    auto new_node = std::make_unique<IRNode>(opcode, s1, p1, p3, prev);

    // Link up the new node with root
    new_node->next = std::move(old_next);
    root->prev = new_node.get();

    // Finish linking by updating the previous node
    prev->next = std::move(new_node);
}

void Allocator::spill(int pr) {
    // cout << "spill input, pr: " << pr << endl;
    int vr = PRToVR[pr];

    // If the VR doesn’t have a spill slot assign one
    if (VRToSpillLoc.find(vr) == VRToSpillLoc.end()) {
        VRToSpillLoc[vr] = nextSpillLoc;
        nextSpillLoc += 4;
    }

    // int spillLoc = VRToSpillLoc[vr];
    // Emit store instruction to save the PR’s value to its spill location
    // This is typically IR generation logic:
    // cout << "store PR" << pr << " -> [spill" << spillLoc << "]" << endl;

    VRToPR.erase(vr);
    PRToVR[pr] = -1;
    PRToNU[pr] = INF;

    int spillLoc = VRToSpillLoc[vr];

    // Insert new loadI operation
    insert(LOADI, spillLoc, spillLoc, k);

    // Insert new store operation
    insert(STORE, -1, pr, k);
}

void Allocator::restore(int vr, int pr) {
    // cout << "restore input, vr: " << vr << " pr: " << pr << endl;
    // int spillLoc = VRToSpillLoc[vr];
    // cout << "load [spill" << spillLoc << "] -> PR" << pr << endl;

    VRToPR[vr] = pr;
    PRToVR[pr] = vr;

    // If the vr can be rematerialized, rematerialize it now and return early
    if (rematVRs.find(vr) != rematVRs.end()) {
        // Insert new loadI operation
        insert(LOADI, rematVRs[vr], rematVRs[vr], pr);
        return;
    }

    int spillLoc = VRToSpillLoc[vr];

    // Insert new loadI operation
    insert(LOADI, spillLoc, spillLoc, k);

    // Insert new load operation
    insert(LOAD, -1, k, pr);
}

int Allocator::getPR(int vr, int nu) {
    int pr = -1;
    int maxNU = -1;

    if (freePRs.empty()) { // No free PR
        // Choose rematerializable value first if it exists
        if (!rematVRs.empty()) {
            for (auto it = rematVRs.begin(); it != rematVRs.end(); ++it) {
                int rematVR = it->first;

                // Check whether the VR has a corresponding PR and whether that PR is marked
                if (VRToPR.find(rematVR) != VRToPR.end() && !PRMarked[VRToPR[rematVR]]) {
                    pr = VRToPR[rematVR];
                    freePR(pr); // Free the rematerializable PR
                    freePRs.pop_back(); // Pop the newly added PR, we're about to use it
                    break;
                }
            }
        }

        // If no valid rematerializable value was found, choose farthest next use
        if (pr == -1) {
            for (int i = 0; i < k; i++) {
                if (!PRMarked[i]) { // Not marked
                    if (PRToNU[i] > maxNU) {
                        maxNU = PRToNU[i];
                        pr = i;
                    }
                }
            }
            spill(pr);
        }
    } else { // A free PR exists
        pr = freePRs.back();
        freePRs.pop_back();
    }

    VRToPR[vr] = pr;
    PRToVR[pr] = vr;
    PRToNU[pr] = nu;
    return pr;
}

void Allocator::freePR(int pr) {
    VRToPR.erase(PRToVR[pr]);
    PRToVR[pr] = -1;
    PRToNU[pr] = INF;
    freePRs.push_back(pr);
}

void Allocator::allocate() {
    if (maxlive > k) {
        k--; // Reserve a register for spilling code
    }

    for (int i = 0; i < k; i++) {
        PRToVR.push_back(-1);
        PRToNU.push_back(INF);
        PRMarked.push_back(false);
        freePRs.push_back(k-i-1);
        // freePRs.push_back(i);
    }

    root = root->next.get(); // Skip the root dummy node

    // int counter = 0;

    // Iterate until we reach end of linked list
    while (root != nullptr) {
        // cout << endl;
        // cout << "Iteration: " << counter << endl;

        auto [defs, uses] = root->getDefsAndUses();

        // 1. Pre-mark all PRs that are already allocated for uses
        for (Operand* use : uses) {
            if (VRToPR.find(use->vr) != VRToPR.end()) {
                PRMarked[VRToPR[use->vr]] = true;
            }
        }

        for (Operand* use : uses) {
            if (VRToPR.find(use->vr) == VRToPR.end()) {
                use->pr = getPR(use->vr, use->nu);
                restore(use->vr, use->pr);
            } else {
                use->pr = VRToPR[use->vr];
            }
            PRMarked[use->pr] = true;
        }

        for (Operand* use : uses) {
            // Free the PR if this is the last use
            if (use->nu == INF && PRToVR[use->pr] != -1) {
                freePR(use->pr);
            }
        }

        fill(PRMarked.begin(), PRMarked.end(), false);

        for (Operand* def : defs) {
            def->pr = getPR(def->vr, def->nu);

            // If the definition isn't used again, free its PR
            if (def->nu == INF && PRToVR[def->pr] != -1) {
                freePR(def->pr);
            } else {
                // If the operation is a loadI, it can be rematerialized
                if (root->opcode == LOADI) {
                    // Assign the vr to its loadI value
                    rematVRs[def->vr] = root->op1.sr;
                } else {
                    // vr is redefined, can't be rematerialized anymore
                    rematVRs.erase(def->vr);
                }
            }
        }

        root = root->next.get();
        // counter++;
    }
}