#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <cctype>
#include <stdexcept>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Class to handle the parsing and evaluation of Boolean expressions
class BooleanEvaluator {
private:
    std::map<char, int> precedence; // Stores the precedence for each operator

    // Determine if a character is an operator
    bool isOperator(char c) {
        return precedence.find(c) != precedence.end();
    }

    // Evaluate a simple expression with two operands and one operator
    bool applyOp(bool a, bool b, char op) {
        switch (op) {
            case '&': return a && b; // AND
            case '|': return a || b; // OR
            case '@': return !(a && b); // NAND
            case '$': return a != b; // XOR
            default: throw std::invalid_argument("Invalid operator: " + std::string(1, op));
        }
    }

    // Convert infix expression to postfix using the Shunting-yard algorithm
    std::string infixToPostfix(const std::string& infix) {
        std::stack<char> operators;
        std::string postfix;
        for (char c : infix) {
            if (isspace(c))
                continue; // Ignore spaces

            if (c == 'T' || c == 'F') {
                postfix += c; // Append operands directly to the output
                postfix += ' '; // Append space to separate tokens
            } else if (c == '(') {
                operators.push(c);
            } else if (c == ')') {
                while (!operators.empty() && operators.top() != '(') {
                    postfix += operators.top();
                    postfix += ' '; // Append space to separate tokens
                    operators.pop();
                }
                operators.pop(); // Remove the '(' from the stack
            } else if (isOperator(c)) {
                while (!operators.empty() && precedence[operators.top()] >= precedence[c] && operators.top() != '(') {
                    postfix += operators.top();
                    postfix += ' '; // Append space to separate tokens
                    operators.pop();
                }
                operators.push(c);
            } else {
                throw std::runtime_error("Invalid character encountered: " + std::string(1, c));
            }
        }

        // Pop all the operators in the stack at the end of the input
        while (!operators.empty()) {
            if (operators.top() == '(') throw std::runtime_error("Mismatched parentheses");
            postfix += operators.top();
            postfix += ' '; // Append space to separate tokens
            operators.pop();
        }
        return postfix;
    }

    // Evaluate postfix expression
    bool evaluatePostfix(const std::string& postfix) {
        std::stack<bool> s;
        std::istringstream tokens(postfix);
        char token;
        while (tokens >> token) {
            if (token == 'T') s.push(true);
            else if (token == 'F') s.push(false);
            else {
                if (token == '!') {
                    if (s.empty()) throw std::runtime_error("Invalid syntax for NOT operation");
                    bool operand = s.top(); s.pop();
                    s.push(!operand);
                } else {
                    if (s.size() < 2) throw std::runtime_error("Invalid syntax for binary operation");
                    bool right = s.top(); s.pop();
                    bool left = s.top(); s.pop();
                    s.push(applyOp(left, right, token));
                }
            }
        }
        if (s.size() != 1) throw std::runtime_error("Invalid expression");
        return s.top();
    }

public:
    // Constructor to define the precedence of each operator
    BooleanEvaluator() {
        precedence['|'] = 1;  // OR
        precedence['&'] = 2;  // AND
        precedence['@'] = 2;  // NAND
        precedence['$'] = 2;  // XOR
        precedence['!'] = 3;  // NOT (highest precedence)
    }

    // Public method to evaluate a boolean expression from infix notation
    bool evaluate(const std::string& expression) {
        std::string postfix = infixToPostfix(expression);
        return evaluatePostfix(postfix);
    }
};

// Function to get the terminal width
int getTerminalWidth() {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return columns;
    #else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
    #endif
}

int main() {
    int terminalWidth = getTerminalWidth();

    // Print a bold header
    std::cout << "\033[1m" << R"(
 ______      ___      ___   _____          ______       _       _____       ______  
|_   _ \   .'   `.  .'   `.|_   _|       .' ___  |     / \     |_   _|    .' ___  | 
  | |_) | /  .-.  \/  .-.  \ | |        / .'   \_|    / _ \      | |     / .'   \_| 
  |  __'. | |   | || |   | | | |   _    | |          / ___ \     | |   _ | |        
 _| |__) |\  `-'  /\  `-'  /_| |__/ |   \ `.___.'\ _/ /   \ \_  _| |__/ |\ `.___.'\ 
|_______/  `.___.'  `.___.'|________|    `.____ .'|____| |____||________| `.____ .' 
                                                                                  
)" << "\033[0m"
              << "This program allows you to input boolean expressions using logical operators "
              << "and evaluates them to return a boolean result. Supported operators include "
              << "Expressions are handled in infix notation and converted to postfix for evaluation."
              << "\nEnter a boolean expression using T (True) and F (False), and operators & (AND), | (OR), ! (NOT), @ (NAND), $ (XOR)."
              << "\nEnter 'q' to quit.\n\n";

    BooleanEvaluator evaluator;
    std::string expression;

    while (true) {
        std::cout << "\nInput: ";
        std::getline(std::cin, expression);

        if (expression == "q" || expression == "Q") {
            std::cout << "Exiting program...\n";
            break;
        }

        try {
            bool result = evaluator.evaluate(expression);
            std::cout << std::string(terminalWidth, '-') << "\n"; // Dynamic line based on terminal width
            std::cout << "The result of the expression is " << (result ? "\033[32mTrue\033[0m" : "\033[31mFalse\033[0m") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << " Please try again.\n";
        }
    }

    return 0;
}