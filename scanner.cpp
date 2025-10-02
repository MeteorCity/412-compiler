#include "scanner.h"

array<string, 12> cat_mapping = {
    "MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "CONSTANT",
    "REGISTER", "COMMA", "INTO", "ENDFILE", "NEWLINE", "ERROR"
};

// Private helpers
Token Scanner::create_token(int category, string lexeme) {
    return Token{category, lexeme, line_number};
}

void Scanner::skip_to_end() {
    char c;
    while (file.peek() != '\n' && file.peek() != EOF) {
        file.get(c);
    }
}

bool Scanner::check_constant(string &number) {
    char c;
    while (file.peek() != EOF) {
        char next = file.peek();
        if (isdigit(next)) {
            file.get(c);
            number += c;
        } else {
            break;
        }
    }
    return true;
}

bool Scanner::check_operation(string operation) {
    for (size_t i = 0; i < operation.size(); i++) {
        int next = file.peek();
        if (next == EOF || next != operation[i]) return false;
        file.get(); // Move forward 1 character
    }
    char next = file.peek();
    return next == '\t' || next == ' ';
}


// Public methods
Scanner::Scanner(string filename) : file(filename) {
    if (!file) {
        // Should line number be added even though there's none...
        cerr << "ERROR: Failed to open " << filename << endl;
        throw runtime_error("Failed to open file: " + filename);
    }
}

Token Scanner::get_next_token() {
    char c;
    while (file.get(c)) {
        // Ignore spaces
        if (isspace(c) && c != '\n') {
            continue;
        }

        // Ignore comments
        else if (c == '/') {
            char next = file.peek();
            if (next != '/') {
                cerr << "ERROR " << line_number << ": Invalid character in comment check" << endl;
                skip_to_end();
                return create_token(11, "");
            }
            skip_to_end();
        }

        // CONSTANT
        else if (isdigit(c)) {
            string number(1, c);
            bool ok = check_constant(number);
            if (!ok) {
                cerr << "ERROR " << line_number << ": Invalid character in constant check" << endl;
                skip_to_end();
                return create_token(11, "");
            } else {
                return create_token(5, number);
            }
        }

        // REGISTER or ARITHOP (rshift)
        else if (c == 'r') {
            char next = file.peek();

            // REGISTER
            if (isdigit(next)) {
                file.get(c);
                string number(1, c);
                bool ok = check_constant(number);
                if (!ok) {
                    cerr << "ERROR " << line_number << ": Invalid character in register check" << endl;
                    skip_to_end();
                    return create_token(11, "");
                } else {
                    return create_token(6, "r" + number);
                }
            }
            
            // ARITHOP (rshift)
            else {
                bool ok = check_operation("shift");
                if (!ok) {
                    cerr << "ERROR " << line_number << ": Invalid character in rshift check" << endl;
                    skip_to_end();
                    return create_token(11, "");
                } else {
                    return create_token(2, "rshift");
                }
            }
        }

        // EOL
        else if (c == '\n') {
            Token tkn = create_token(10, "\\n");
            line_number++;
            return tkn;
        }

        // COMMA
        else if (c == ',') {
            return create_token(7, ",");
        }

        // INTO
        else if (c == '=') {
            char next = file.peek();
            if (next == '>') {
                file.get(c);
                return create_token(8, "=>");
            } else {
                cerr << "ERROR " << line_number << ": Invalid character in into check" << endl;
                skip_to_end();
                return create_token(11, "");
            }
        }

        // NOP
        else if (c == 'n') {
            bool ok = check_operation("op");
            if (!ok) {
                cerr << "ERROR " << line_number << ": Invalid character in nop check" << endl;
                skip_to_end();
                return create_token(11, "");
            } else {
                return create_token(4, "nop");
            }
        }

        // OUTPUT
        else if (c == 'o') {
            bool ok = check_operation("utput");
            if (!ok) {
                cerr << "ERROR " << line_number << ": Invalid character in output check" << endl;
                skip_to_end();
                return create_token(11, "");
            } else {
                return create_token(3, "output");
            }
        }

        // MEMOP (store) and ARITHOP (sub)
        else if (c == 's') {
            char next = file.peek();

            if (next == 't') {
                file.get(c);
                bool ok = check_operation("ore");
                if (!ok) {
                    cerr << "ERROR " << line_number << ": Invalid character in store check" << endl;
                    skip_to_end();
                    return create_token(11, "");
                } else {
                    return create_token(0, "store");
                }
            } else if (next == 'u') {
                file.get(c);
                bool ok = check_operation("b");
                if (!ok) {
                    cerr << "ERROR " << line_number << ": Invalid character in sub check" << endl;
                    skip_to_end();
                    return create_token(11, "");
                } else {
                    return create_token(2, "sub");
                }
            } else {
                cerr << "ERROR " << line_number << ": Invalid character after s" << endl;
                skip_to_end();
                return create_token(11, "");
            }
        }

        // ARITHOP (add)
        else if (c == 'a') {
            bool ok = check_operation("dd");
            if (!ok) {
                cerr << "ERROR " << line_number << ": Invalid character in add check" << endl;
                skip_to_end();
                return create_token(11, "");
            } else {
                return create_token(2, "add");
            }
        }

        // ARITHOP (mult)
        else if (c == 'm') {
            bool ok = check_operation("ult");
            if (!ok) {
                cerr << "ERROR " << line_number << ": Invalid character in mult check" << endl;
                skip_to_end();
                return create_token(11, "");
            } else {
                return create_token(2, "mult");
            }
        }

        // MEMOP (load), LOADI, and ARITHOP (lshift)
        else if (c == 'l') {
            char next = file.peek();

            // ARITHOP (lshift)
            if (next == 's') {
                file.get(c);
                bool ok = check_operation("hift");
                if (!ok) {
                    cerr << "ERROR " << line_number << ": Invalid character in lshift check" << endl;
                    skip_to_end();
                    return create_token(11, "");
                } else {
                    return create_token(2, "lshift");
                }
            }

            // MEMOP (load) or LOADI
            else if (next == 'o') {
                file.get(c);
                next = file.peek();
                if (next == 'a') {
                    file.get(c);
                    next = file.peek();
                    if (next == 'd') {
                        file.get(c);
                        next = file.peek();
                        
                        // MEMOP (load) - must require a blank space after
                        if (next == '\t' or next == ' ') {
                            return create_token(0, "load");
                        }
                        
                        // LOADI
                        else if (next == 'I') {
                            file.get(c);
                            next = file.peek();

                            // Must require a blank space after
                            if (next == '\t' or next == ' ') {
                                return create_token(1, "loadI");
                            }
                        }
                    }
                }
            }

            cerr << "ERROR " << line_number << ": Invalid character in word starting with l" << endl;
            skip_to_end();
            return create_token(11, "");
        }
        
        else {
            cerr << "ERROR " << line_number << ": Invalid character: \"" << c << "\"" << endl;
            skip_to_end();
            return create_token(11, "");
        }
    }

    // EOF
    file.close();
    return create_token(9, "");
}

void Scanner::scan_file() {
    while (true) {
        Token next_token = get_next_token();
        if (next_token.category == 11) { // Custom error token
            continue;
        }

        cout << next_token.toString() << endl;
        if (next_token.category == 9) { // ENDFILE
            break;
        }
    }
}