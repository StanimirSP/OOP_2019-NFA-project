#ifndef REGULAREXPRESSION_H
#define REGULAREXPRESSION_H

#include <string>
#include "Automaton.h"

class RegularExpression
{
    std::string regex, RPN;
    std::string produceRPN() const;
    static bool isLetter(char);
    static bool isOperator(char);
    static int precedence(char);
public:
    RegularExpression() = default;
    RegularExpression(const std::string&);
    const std::string& expression() const;
    const std::string& reversePolishNotation() const;
    Automaton NFA() const;
};

#endif // REGULAREXPRESSION_H
