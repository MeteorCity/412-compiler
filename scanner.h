#pragma once
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::runtime_error;
using std::string;
using std::to_string;

// Maps ints to categories
extern array<string, 12> cat_mapping;

struct Token {
    int category;
    string lexeme;
    int line_number;

    string toString() {
        return to_string(line_number) + ": < " + cat_mapping[category] + ", \"" + lexeme + "\" >";
    }
};

class Scanner {
    int line_number = 1;
    ifstream file;

    private:
        // Private helper functions
        Token create_token(int category, string lexeme);
        void skip_to_end();
        bool check_constant(string &number);
        bool check_operation(string operation);

    public:
        explicit Scanner(string filename);
        Token get_next_token();
        void scan_file();
};