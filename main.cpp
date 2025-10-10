#include "allocator.h"
#include "ir.h"
#include "parser.h"
#include "renamer.h"
#include "scanner.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;

void print_help() {
    cout << "Usage: 412alloc [option]\n"
         << "Options:\n"
         << "  -h               Show this help message and exit\n"
         << "  -x <filepath>    Scan, parse file, perform renaming, and print renaming to stdout\n"
         << "   k <filepath>    3 <= k <= 64 denotes the number of physical registers, allocates code and prints it to stdout"
         << endl;
}

void print_IR(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toString() << endl;
        root = root->next.get();
    }
}

void print_block1(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toLine1() << endl;
        root = root->next.get();
    }
}

void print_block2(IRNode *root) {
    root = root->next.get(); // Skip the root dummy node
    while (root) {
        cout << root->toLine2() << endl;
        root = root->next.get();
    }
}

bool parseInteger(const std::string& s, int& value) {
    if (s.empty()) return false;

    // Handle optional plus or minus signs
    size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (start == s.size()) return false;

    // Ensure all remaining characters are digits
    for (size_t i = start; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;

    // Try to convert
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    return ec == std::errc() && ptr == s.data() + s.size();
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
            return 1;
        }
    }

    // Ensure there are only two arguments besides the program name
    if (argc != 3) {
        cerr << "ERROR: please specify a valid number of input arguments" << endl;
        print_help();
        return 1;
    }

    string flag = "";
    string filename = argv[2];
    int maxlive;

    if (std::string(argv[1]) == "-x") {
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
                renamer.rename_IR(operations, parser.maxSR, parser.root);
                print_block1(root.get());
            }
        } catch (runtime_error &e) {
            return 1;
        }
    } else if (parseInteger(argv[1], maxlive) && 3 <= maxlive && maxlive <= 64) {
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
                Allocator allocator(maxlive);
                renamer.rename_IR(operations, parser.maxSR, parser.root);
                allocator.allocate(root.get());
                print_block2(root.get());
            }
        } catch (runtime_error &e) {
            return 1;
        }
    } else {
        cerr << "ERROR: please specify a valid set of input arguments" << endl;
        print_help();
        return 1;
    }

    return 0;
}