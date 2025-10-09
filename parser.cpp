#include "ir.h"
#include "parser.h"
#include "scanner.h"

// Parser stuff
Parser::Parser(Scanner &scanner, IRNode *root) : scanner(scanner), root(root) {}

void Parser::insert_new_node(int line, int opcode, int r1, int r2, int r3) {
    if (opcode == 2) { // LOADI
        maxSR = std::max(maxSR, r3);
    } else if (opcode == 8) { // OUTPUT
        maxSR = std::max(maxSR, r1);
    } else if (opcode < 8) { // Everything else except NOP
        maxSR = std::max(maxSR, std::max(r1, std::max(r2, r3)));
    }

    auto new_node = make_unique<IRNode>(line, opcode, r1, r2, r3, root);
    root->next = std::move(new_node); // Transfer ownership of new_node to previous node
    root = root->next.get(); // Set root to new_node
}

int Parser::parse_file() {
    bool success = true;
    Token next_token;

    while (true) {
        // Check previous token for ENDFILE
        if (next_token.category == 9) {
            if (success) {
                return operations;
            } else {
                return -1;
            }
        }

        next_token = scanner.get_next_token();
        int line = next_token.line_number;

        // Check next token for ENDFILE
        if (next_token.category == 9) {
            if (success) {
                return operations;
            } else {
                return -1;
            }
        }

        // Custom error token (error found in scanner)
        if (next_token.category == 11) {
            success = false;
            continue;
        }

        // NEWLINE (ignore empty lines)
        else if (next_token.category == 10) {
            continue;
        }

        // MEMOP
        else if (next_token.category == 0) {
            int opcode = (next_token.lexeme[0] == 'l') ? 0 : 1; // Either load or store
            next_token = scanner.get_next_token();
            if (next_token.category == 6) {
                int r1 = stoi(next_token.lexeme.c_str() + 1);
                next_token = scanner.get_next_token();
                if (next_token.category == 8) {
                    next_token = scanner.get_next_token();
                    if (next_token.category == 6) {
                        int r3 = stoi(next_token.lexeme.c_str() + 1);
                        next_token = scanner.get_next_token();
                        if (next_token.category == 10 || next_token.category == 9) {
                            operations += 1;
                            insert_new_node(line, opcode, r1, -1, r3);
                            continue;
                        }
                    }
                }
            }
            cerr << "ERROR " << line << ": Invalid MEMOP instruction format" << endl;
        }

        // LOADI
        else if (next_token.category == 1) {
            next_token = scanner.get_next_token();
            if (next_token.category == 5) {
                int r1 = stoi(next_token.lexeme);
                next_token = scanner.get_next_token();
                if (next_token.category == 8) {
                    next_token = scanner.get_next_token();
                    if (next_token.category == 6) {
                        int r3 = stoi(next_token.lexeme.c_str() + 1);
                        next_token = scanner.get_next_token();
                        if (next_token.category == 10 || next_token.category == 9) {
                            operations += 1;
                            insert_new_node(line, 2, r1, -1, r3);
                            continue;
                        }
                    }
                }
            }
            cerr << "ERROR " << line << ": Invalid LOADI instruction format" << endl;
        }

        // ARITHOP
        else if (next_token.category == 2) {
            int opcode;
            switch (next_token.lexeme[0]) {
                case 'a': // add
                    opcode = 3;
                    break;
                case 's': // sub
                    opcode = 4;
                    break;
                case 'm': // mult
                    opcode = 5;
                    break;
                case 'l': // lshift
                    opcode = 6;
                    break;
                case 'r': // rshift
                    opcode = 7;
                    break;
            }
            next_token = scanner.get_next_token();
            if (next_token.category == 6) {
                int r1 = stoi(next_token.lexeme.c_str() + 1);
                next_token = scanner.get_next_token();
                if (next_token.category == 7) {
                    next_token = scanner.get_next_token();
                    if (next_token.category == 6) {
                        int r2 = stoi(next_token.lexeme.c_str() + 1);
                        next_token = scanner.get_next_token();
                        if (next_token.category == 8) {
                            next_token = scanner.get_next_token();
                            if (next_token.category == 6) {
                                int r3 = stoi(next_token.lexeme.c_str() + 1);
                                next_token = scanner.get_next_token();
                                if (next_token.category == 10 || next_token.category == 9) {
                                    operations += 1;
                                    insert_new_node(line, opcode, r1, r2, r3);
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
            cerr << "ERROR " << line << ": Invalid ARITHOP instruction format" << endl;
        }

        // OUTPUT
        else if (next_token.category == 3) {
            next_token = scanner.get_next_token();
            if (next_token.category == 5) {
                int r1 = stoi(next_token.lexeme);
                next_token = scanner.get_next_token();
                if (next_token.category == 10 || next_token.category == 9) {
                    operations += 1;
                    insert_new_node(line, 8, r1, -1, -1);
                    continue;
                }
            }
            cerr << "ERROR " << line << ": Invalid OUTPUT instruction format" << endl;
        }

        // NOP
        else if (next_token.category == 4) {
            next_token = scanner.get_next_token();
            if (next_token.category == 10 || next_token.category == 9) {
                operations += 1;
                insert_new_node(line, 9, -1, -1, -1);
                continue;
            }
            cerr << "ERROR " << line << ": Invalid NOP instruction format" << endl;
        }

        // The iloc code doesn't follow the proper format (error found in parser)
        success = false;
    }
}