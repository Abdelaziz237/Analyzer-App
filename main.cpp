#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include "CompilersTask_2_Parser.cpp"

using namespace std;

TreeNode* parse_stmtseq(CompilerInfo& ci, ParseInfo& pi);
TreeNode* parse_math_expr(CompilerInfo& ci, ParseInfo& pi);

// newexpr -> ( mathexpr ) | number | identifier
TreeNode* parse_newexpr(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = new TreeNode();
    node->line_num = 1;

    switch (pi.next_token.type){
        case LEFT_BRACE: {
            node = parse_math_expr(ci, pi);
            break;
        }
        case NUM: {
            node->node_kind = NUM_NODE;
            node->id = strdup(pi.next_token.str);
            break;
        }
        case ID: {
            node->node_kind = ID_NODE;
            node->id = strdup(pi.next_token.str);
            break;
        }
        default: {
            throw "Syntax Error";
            break;
        }
    }
    GetNextToken(&ci, &pi.next_token);
    return node;
}

// factor -> newexpr { ^ newexpr } (right associative)
TreeNode* parse_factor(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = parse_newexpr(ci, pi);

    while (pi.next_token.type == POWER) {
        TreeNode* powerNode = new TreeNode();
        powerNode->node_kind = OPER_NODE;
        powerNode->oper = POWER;
        powerNode->child[0] = node;

        // Move to the next token after ^
        GetNextToken(&ci, &pi.next_token);

        // Parse the right operand
        powerNode->child[1] = parse_newexpr(ci, pi);

        // Update the node to be the new powerNode
        node = powerNode;
    }

    return node;
}


// term -> factor { (*|/) factor } (left associative)
TreeNode* parse_term(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = parse_factor(ci, pi);

    while (pi.next_token.type == TIMES || pi.next_token.type == DIVIDE) {
        TreeNode* termNode = new TreeNode();
        termNode->line_num = 1;
        termNode->node_kind = OPER_NODE;

        switch (pi.next_token.type) {
            case TIMES:
                termNode->oper = TIMES;
                break;
            case DIVIDE:
                termNode->oper = DIVIDE;
                break;
            default:
                // This case should not be reached since you are handling TIMES and DIVIDE
                break;
        }

        // Move to the next token after TIMES or DIVIDE
        GetNextToken(&ci, &pi.next_token);

        // Parse the right operand
        termNode->child[1] = parse_factor(ci, pi);

        // Set the left operand to the result of the previous iteration
        termNode->child[0] = node;

        // Update the node to be the new termNode
        node = termNode;
    }

    return node;
}

// mathexpr -> term { (+|-) term } (left associative)
TreeNode* parse_math_expr(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = parse_term(ci, pi);

    while (pi.next_token.type == PLUS || pi.next_token.type == MINUS) {
        TreeNode* mathExprNode = new TreeNode();
        mathExprNode->line_num = 1;
        mathExprNode->node_kind = OPER_NODE;

        switch (pi.next_token.type) {
            case PLUS:
                mathExprNode->oper = PLUS;
                break;
            case MINUS:
                mathExprNode->oper = MINUS;
                break;
            default:
                // This case should not be reached since PLUS and MINUS are handled
                break;
        }

        // Move to the next token after PLUS or MINUS
        GetNextToken(&ci, &pi.next_token);

        // Parse the right operand
        mathExprNode->child[1] = parse_term(ci, pi);

        // Set the left operand to the result of the previous iteration
        mathExprNode->child[0] = node;

        // Update the node to be the new mathExprNode
        node = mathExprNode;
    }

    return node;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode* parse_expr(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = parse_math_expr(ci, pi);

    if (pi.next_token.type == LESS_THAN || pi.next_token.type == EQUAL) {
        TreeNode* exprNode = new TreeNode();
        exprNode->line_num = 1;
        exprNode->node_kind = OPER_NODE;

        switch (pi.next_token.type) {
            case LESS_THAN:
                exprNode->oper = LESS_THAN;
                break;
            case EQUAL:
                exprNode->oper = EQUAL;
                break;
            default:
                // This case should not be reached since LESS_THAN and EQUAL are handled
                break;
        }

        // Move to the next token after LESS_THAN or EQUAL
        GetNextToken(&ci, &pi.next_token);

        // Parse the right operand
        exprNode->child[1] = parse_math_expr(ci, pi);

        // Set the left operand to the result of the previous iteration
        exprNode->child[0] = node;

        // Update the node to be the new exprNode
        node = exprNode;
    }

    return node;
}


// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
TreeNode* parse_stmt(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = new TreeNode();
    node->line_num = 1;

    switch (pi.next_token.type) {

        case ASSIGN: GetNextToken(&ci, &pi.next_token);
        // ifstmt -> if expr then stmtseq [ else stmtseq ] end
        case IF: {
            node->node_kind = IF_NODE;
            node->id = strdup(pi.next_token.str);

            GetNextToken(&ci, &pi.next_token);

            // Parse the expression
            node->child[0] = parse_expr(ci, pi);

            // Check for 'THEN' keyword
            if (pi.next_token.type != THEN) {
                throw "Syntax Error: 'THEN' expected.";
            }

            GetNextToken(&ci, &pi.next_token);

            // Parse the 'THEN' branch
            node->child[1] = parse_stmtseq(ci, pi);

            // Check for 'ELSE' keyword
            if (pi.next_token.type == ELSE) {
                GetNextToken(&ci, &pi.next_token);

                // Parse the 'ELSE' branch
                node->child[2] = parse_stmt(ci, pi);
            }            
            // Check for 'END' keyword
            if (pi.next_token.type != END) {
                throw "Syntax Error: 'END' expected.";
            }

            GetNextToken(&ci, &pi.next_token);

            break;
        }

        //  repeatstmt -> repeat stmtseq until expr
        case REPEAT: {
            node->node_kind = REPEAT_NODE;
            node->id = strdup(pi.next_token.str);

            // Consume the "repeat" keyword
            GetNextToken(&ci, &pi.next_token);

            // Parse the statement sequence
            node->child[0] = parse_stmtseq(ci, pi);

            // Check for "until" keyword
            if (pi.next_token.type != UNTIL) {
                throw "Syntax Error: 'until' expected.";
            }

            // Consume the "until" keyword
            GetNextToken(&ci, &pi.next_token);

            // Parse the expression
            node->child[1] = parse_expr(ci, pi);
            break;
        }

        // readstmt -> read identifier
        case READ: {
            node->node_kind = READ_NODE;
            node->id = strdup(pi.next_token.str);

            // Consume the "read" keyword
            GetNextToken(&ci, &pi.next_token);

            // Parse the identifier
            if (pi.next_token.type == ID) {
                node->child[0] = new TreeNode();
                node->child[0]->line_num = 1;
                node->child[0]->node_kind = ID_NODE;
                node->child[0]->id = strdup(pi.next_token.str); // Assuming TreeNode has a char* id member

                // Consume the identifier
                GetNextToken(&ci, &pi.next_token);
            } else {
                throw "Syntax Error: Identifier expected after 'read'.";
            }

            break;
        }

        // writestmt -> write expr
        case WRITE: {
            node->node_kind = WRITE_NODE;
            node->id = strdup(pi.next_token.str);

            // Consume the "write" keyword
            GetNextToken(&ci, &pi.next_token);

            // Parse the expression
            node->child[0] = parse_expr(ci, pi);
            break;
        }
        case ID: {
            node->line_num = 1;
            node->node_kind = ID_NODE;
            node->id = strdup(pi.next_token.str); 
            GetNextToken(&ci, &pi.next_token);
            GetNextToken(&ci, &pi.next_token);
            node->child[0] = parse_expr(ci, pi);
            break;
        }
        default: {
            throw "Syntax Error: Unexpected token.";
            break;
        }
    }

    return node;
}

// stmtseq -> stmt { ; stmt }
TreeNode* parse_stmtseq(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* node = parse_stmt(ci, pi);

    // Parse additional statements separated by semicolons
    while (pi.next_token.type == SEMI_COLON) {
        GetNextToken(&ci, &pi.next_token); // Consume the semicolon

        // Create a new node for the next statement
        TreeNode* nextStmt = parse_stmt(ci, pi);

        // If there are multiple statements, link them as siblings
        if (node->sibling == nullptr) {
            node->sibling = nextStmt;
        } else {
            TreeNode* temp = node->sibling;
            while (temp->sibling != nullptr) {
                temp = temp->sibling;
            }
            temp->sibling = nextStmt;
        }
    }

    return node;
}

// program -> stmtseq
TreeNode* parse_program(CompilerInfo& ci, ParseInfo& pi) {
    TreeNode* programNode = parse_stmtseq(ci, pi);

    // Check if the end of the file has been reached
    if (pi.next_token.type != ENDFILE) {
        throw "Syntax Error: Unexpected tokens after the program.";
    }

    return programNode;
}



TreeNode* analyze(CompilerInfo& ci) {
    ParseInfo pi;
    GetNextToken(&ci, &pi.next_token);
    return parse_program(ci, pi);
}

void freeTree(TreeNode* node) {
    if (node) {
        for (int i = 0; i < MAX_CHILDREN; i++) {
            freeTree(node->child[i]);
        }
        freeTree(node->sibling);

        // Free node-specific resources
        if (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE) {
            delete[] node->id;
        }

        delete node;
    }
}

int main() {

    CompilerInfo compilerInfo("input.txt", nullptr, nullptr);
    ParseInfo parseInfo;

    TreeNode* parseTreeRoot = analyze(compilerInfo);
    
    FILE* out = fopen("output.txt", "w");
    PrintTreeToFile(parseTreeRoot, out);
    // Print the parse tree for debugging
    PrintTree(parseTreeRoot);
    return 0;
}