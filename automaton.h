#ifndef AUTOMATON_H
#define AUTOMATON_H
#include <set>
#include <cstddef>
#include <iosfwd>
#include <string>
#include "transition.h"

class Automaton
{
    std::size_t states=0;
    std::set<Transition> transitions;
    std::set<char> alpha;
    std::set<std::size_t> finalStates;
    bool deterministic=true;
    Automaton() = default;
    bool traverse(const char*, std::size_t, std::set<std::pair<const char*, std::size_t>>&) const;
    bool traverse(const char*) const;
    bool isFinal(std::size_t) const;
    bool isDeterm() const;
    void addReachableThroughEps(std::set<std::size_t>&) const;
    void removeUnreachableStates();
    bool containsFinalState(const std::set<std::size_t>&) const;
    bool isFinalStateReachable(std::size_t) const;
    bool existPathWithNonzeroLength(std::size_t, std::size_t) const;
    friend class RegularExpression;
public:
    static constexpr char epsilon='E';
    bool isDeterministic() const;
    Automaton(std::istream&);
    bool operator()(const std::string&) const;
    Automaton Union(const Automaton&) const;
    Automaton Concatenation(const Automaton&) const;
    Automaton KleeneStar() const;
    bool acceptsTheEmptyLang() const;
    bool acceptsFiniteLang() const;
    bool save(const std::string&) const;
    Automaton& convertToDFA();
    Automaton& minimize();
    friend std::ostream& operator<<(std::ostream&, const Automaton&);
};

#endif // AUTOMATON_H
