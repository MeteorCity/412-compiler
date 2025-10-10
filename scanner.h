#pragma once
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

// Maps ints to categories
extern std::array<std::string, 12> cat_mapping;

struct Token {
    int category;
    std::string lexeme;
    int line_number;

    std::string toString() {
        return std::to_string(line_number) + ": < " + cat_mapping[category] + ", \"" + lexeme + "\" >";
    }
};

class Scanner {
    int line_number = 1;
    std::ifstream file;

    private:
        // Private helper functions
        Token create_token(int category, std::string lexeme);
        void skip_to_end();
        bool check_constant(std::string &number);
        bool check_operation(std::string operation);

    public:
        explicit Scanner(std::string filename);
        Token get_next_token();
        void scan_file();
};