#include "ir.h"
#include "parser.h"
#include "renamer.h"
#include "scanner.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::runtime_error;
using std::string;
using std::unordered_map;

void print_help() {
    cout << "Usage: 412alloc [option]\n"
         << "Options:\n"
         << "  -h               Show this help message and exit\n"
         << "  -x <filepath>    Scan, parse file, perform renaming, and print renaming to stdout\n"
         << "   k <filepath>    Not implemented for code check 1"
         << endl;
}

void print_old_IR(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toString() << endl;
        root = root->next.get();
    }
}

void print_IR(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toLine() << endl;
        root = root->next.get();
    }
}

int main(int argc, char* argv[]) {
    // Debug statements
    // cout << "Program name: " << argv[0] << endl;
    // for (int i = 1; i < argc; i++) {
    //     cout << "Argument " << i << ": " << argv[i] << endl;
    // }

    // Ensure there are command line arguments
    if (argc == 1) {
        cerr << "ERROR: please specify a set of input arguments" << endl;
        print_help();
        return 1;
    }

    unordered_map<string, int> priorities;
    priorities["-h"] = 3;
    priorities["-x"] = 2;
    priorities[""] = 1;

    string flag = "";
    string filename = "";
    bool multiple_flags = false;

    // Extract the flag and filename to execute
    for (int i = 1; i < argc; i++) {
        if (priorities.find(argv[i]) != priorities.end()) { // Arg is a flag
            if (flag != "") {
                multiple_flags = true;
            }
            if (priorities[argv[i]] > priorities[flag]) {
                flag = argv[i];
            }
            if (flag == "-h") {
                break;
            }
        } else { // Arg is a filename
            if (filename != "") {
                cerr << "ERROR: multiple files are not allowed" << endl;
                print_help();
                return 1;
            }
            filename = argv[i];
        }
    }

    // Log an error if no flag is found
    if (flag == "") {
        cerr << "ERROR: no flag found, use -h flag for help" << endl;
        return 1;
    }

    // Log the multiple flags error
    if (multiple_flags) {
        cerr << "ERROR: multiple flags, choosing highest priority flag" << endl;
    }

    // cout << "Flag: " << flag << ", Filename: " << filename << endl;

    if (flag == "-h") {
        print_help();
    } else if (flag == "-x") {
        try {
            Scanner scanner(filename);
            auto root = make_unique<IRNode>(-1, -1, -1, -1, -1, nullptr); // Dummy root node
            Parser parser(scanner, root.get());
            int operations = parser.parse_file();
            if (operations == -1) {
                cerr << "Due to syntax errors, run terminates." << endl;
                return 1;
            } else {
                Renamer renamer;
                // print_old_IR(root.get());
                renamer.rename_IR(operations, parser.maxSR, parser.root);
                print_IR(root.get());
            }
        } catch (runtime_error &e) {
            return 1;
        }
    } else { // Unsupported flag
        cerr << "How did you get here..." << endl;
        return 1;
    }

    return 0;
}