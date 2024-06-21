#include "regularExpression.h"
#include <cstddef>
#include <stdexcept>
#include <stack>
#include <utility>

RegularExpression::RegularExpression(const std::string& r): regex(r), RPN(produceRPN()) {}

bool RegularExpression::isLetter(char c)
{
    return c>='a' && c<='z' || c>='0' && c<='9';
}

bool RegularExpression::isOperator(char c)
{
    return c=='|' || c=='&' || c=='*';
}

int RegularExpression::precedence(char c)
{
    switch(c)
    {
    case '|': return 1;
    case '&': return 2;
    case '*': return 3;
    default: throw std::logic_error("Bad regular expression");
    }
}

std::string RegularExpression::produceRPN() const
{
    if(regex.empty()) return {};
    std::string expr, res;
    expr.push_back(regex[0]);
    for(std::size_t i=1; i<regex.size(); ++i)
    {
        if((isLetter(regex[i]) || regex[i]==Automaton::epsilon || regex[i]=='(') && regex[i-1]!='(' && regex[i-1]!='|')
            expr.push_back('&'); /// add concatenation operator
        expr.push_back(regex[i]);
    }
    std::stack<char> op;
    for(char c: expr)
    {
        if(isLetter(c) || c==Automaton::epsilon) res.push_back(c);
        else if(isOperator(c))
        {
            /*if(c=='*')
            {
                res.push_back(c);
                continue;
            }*/
            while(!op.empty() && op.top()!='(' && precedence(op.top())>=precedence(c))
            {
                res.push_back(op.top());
                op.pop();
            }
            op.push(c);
        }
        else if(c=='(') op.push(c);
        else if(c==')')
        {
            while(!op.empty() && op.top()!='(')
            {
                res.push_back(op.top());
                op.pop();
            }
            if(op.empty()) throw std::logic_error("Bad regular expression: mismatched parentheses");
            op.pop();
        }

    }
    while(!op.empty())
    {
        if(op.top()=='(') throw std::logic_error("Bad regular expression: mismatched parentheses");
        res.push_back(op.top());
        op.pop();
    }
    return res;
}

const std::string& RegularExpression::expression() const
{
    return regex;
}

const std::string& RegularExpression::reversePolishNotation() const
{
    return RPN;
}

Automaton RegularExpression::NFA() const
{
    std::stack<Automaton> s;
    for(char c: RPN)
    {
        switch(c)
        {
        case '*':
            {
                if(s.empty()) throw std::runtime_error("Bad regular expression");
                auto result=s.top().KleeneStar();
                s.pop();
                s.push(std::move(result));
                break;
            }
        case '&':
            {
                if(s.size()<2) throw std::runtime_error("Bad regular expression");
                auto op2=std::move(s.top());
                s.pop();
                auto op1=std::move(s.top());
                s.pop();
                s.push(op1.Concatenation(op2));
                break;
            }
        case '|':
            {
                if(s.size()<2) throw std::runtime_error("Bad regular expression");
                auto op2=std::move(s.top());
                s.pop();
                auto op1=std::move(s.top());
                s.pop();
                s.push(op1.Union(op2));
                break;
            }
        default:
            if(isLetter(c) || c==Automaton::epsilon)
            {
                Automaton tmp;
                tmp.states=2;
                if(c!=Automaton::epsilon) tmp.alpha.insert(c);
                tmp.transitions.emplace(0,c,1);
                tmp.finalStates.insert(1);
                tmp.deterministic=c!=Automaton::epsilon;
                s.push(std::move(tmp));
            }
        }
    }
    if(s.size()!=1) throw std::runtime_error("Bad regular expression");
    return s.top();
}

