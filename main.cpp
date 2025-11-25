#include "ir.h"
#include "output.h"
#include "parser.h"
#include "renamer.h"
#include "scanner.h"
#include "scheduler.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::to_string;

void print_help() {
    cout << "Usage: schedule [option]\n"
         << "Options:\n"
         << "  -h               Show this help message and exit\n"
         << "  -g               Output a .dot file for dependency graph visualization"
         << "  <filename>       Invoke schedule on the ILOC block in filename and output the scheduled block to stdout"
         << endl;
}

void print_IR(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toString() << endl;
        root = root->next.get();
    }
}

void print_output(OutputNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toString() << endl;
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

    // Check for the -h flag
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-h") {
            print_help();
            return 0;
        }
    }

    // Ensure there is a valid number of command-line arguments
    if (argc > 3) {
        cerr << "ERROR: please specify a valid number of input arguments" << endl;
        print_help();
        return 1;
    }

    if (string(argv[1]) == "-g") {
        try {
            string filename = argv[2];
            Scanner scanner(filename);
            auto root = make_unique<IRNode>(-1, -1, -1, -1, -1, nullptr); // Dummy root node
            Parser parser(scanner, root.get());
            int operations = parser.parse_file();
            if (operations == -1) {
                cerr << "Due to syntax errors, run terminates." << endl;
                return 1;
            } else {
                Renamer renamer;
                Scheduler scheduler;
                renamer.rename_IR(operations, parser.maxSR, parser.root);
                scheduler.buildGraph(root.get());
                string dot = scheduler.dep_graph.toDot();
                ofstream fout("dep_graph.dot");
                fout << dot;
                fout.close();
            }
        } catch (runtime_error &e) {
            return 1;
        }
    } else {
        try {
            string filename = argv[1];
            Scanner scanner(filename);
            auto root = make_unique<IRNode>(-1, -1, -1, -1, -1, nullptr); // Dummy root node
            Parser parser(scanner, root.get());
            int operations = parser.parse_file();
            if (operations == -1) {
                cerr << "Due to syntax errors, run terminates." << endl;
                return 1;
            } else {
                Renamer renamer;
                renamer.rename_IR(operations, parser.maxSR, parser.root);
                Scheduler scheduler;
                auto outputRoot = make_unique<OutputNode>("", ""); // Dummy root node
                scheduler.schedule(root.get(), outputRoot.get());
                print_output(outputRoot.get());
            }
        } catch (runtime_error &e) {
            return 1;
        }
    }

    return 0;
}