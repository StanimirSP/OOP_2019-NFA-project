#include "automaton.h"
#include <stdexcept>
#include <iostream>
#include <queue>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <map>
#include <utility>

Automaton::Automaton(std::istream& is)
{
    if(!is) throw std::runtime_error("Could not read from the file");
    std::size_t finalStatesCount, transitionsCount;
    is >> states >> finalStatesCount;
    while(finalStatesCount--)
    {
        std::size_t f;
        is >> f;
        finalStates.insert(f);
    }
    is >> transitionsCount;
    while(transitionsCount--)
    {
        std::size_t from, to;
        char label;
        is >> from >> label >> to;
        if(from==to && label==epsilon) continue;
        if(label!=epsilon) alpha.insert(label);
        transitions.emplace(from, label, to);
    }
    if(!is) throw std::runtime_error("Wrong input");
    deterministic=isDeterm();
}

bool Automaton::isFinal(std::size_t state) const
{
    return finalStates.find(state)!=finalStates.end();
}

bool Automaton::isDeterm() const
{
    if(transitions.size()!=states*alpha.size()) return false;
    if(transitions.empty()) return true;
    auto it1=transitions.begin(), it2=it1;
    for(++it2; it2!=transitions.end(); ++it1, ++it2)
        if(it1->Label()==epsilon || it1->From()==it2->From() && it1->Label()==it2->Label())
            return false;
    return it2->Label()!=epsilon;
}

bool Automaton::isDeterministic() const
{
    return deterministic;
}

bool Automaton::traverse(const char* word, std::size_t state, std::set<std::pair<const char*, std::size_t>>& s) const
{
    if(s.find(std::make_pair(word, state))!=s.end()) return false;
    auto it=transitions.lower_bound(Transition(state, 0, 0)), end=transitions.upper_bound(Transition(state, 127, 0));
    if(!*word)
    {
        for(; it!=end; ++it)
            if(it->Label()==epsilon)
            {
                s.emplace(word, state);
                if(traverse(word, it->To(), s)) return true;
            }
        return isFinal(state);
    }
    if(*word==epsilon) return traverse(word+1, state, s);
    for(; it!=end; ++it)
    {
        if(it->Label()==*word && traverse(word+1, it->To(), s)) return true;
        if(it->Label()==epsilon)
        {
            s.emplace(word, state);
            if(traverse(word, it->To(), s)) return true;
        }
    }
    return false;
}

bool Automaton::traverse(const char* word) const
{
    std::size_t state=0;
    for(std::size_t i=0; word[i]; ++i)
    {
        if(word[i]==epsilon) continue;
        auto it=transitions.lower_bound(Transition(state, word[i], 0));
        if(it==transitions.end() || it->Label()!=word[i] || it->From()!=state) return false;
        state=it->To();
    }
    return isFinal(state);
}

bool Automaton::operator()(const std::string& word) const
{
    if(deterministic) return traverse(word.c_str());
    std::set<std::pair<const char*, std::size_t>> s;
    return traverse(word.c_str(), 0, s);
}

Automaton Automaton::Union(const Automaton& a) const
{
    if(!states) return a;
    if(!a.states) return *this;
    Automaton res;
    std::set_union(alpha.begin(), alpha.end(), a.alpha.begin(), a.alpha.end(), std::inserter(res.alpha, res.alpha.end()));
    res.states=states+a.states+1;
    res.deterministic=false;
    res.transitions.emplace(0, epsilon, 1);
    res.transitions.emplace(0, epsilon, states+1);
    for(auto&& t: transitions)
        res.transitions.emplace(t.From()+1, t.Label(), t.To()+1);
    for(auto&& t: a.transitions)
        res.transitions.emplace(t.From()+states+1, t.Label(), t.To()+states+1);
    for(auto f: finalStates)
        res.finalStates.insert(f+1);
    for(auto f: a.finalStates)
        res.finalStates.insert(f+states+1);
    return res;
}

Automaton Automaton::Concatenation(const Automaton& a) const
{
    if(!states || !a.states || !finalStates.size()) return {};
    Automaton res;
    std::set_union(alpha.begin(), alpha.end(), a.alpha.begin(), a.alpha.end(), std::inserter(res.alpha, res.alpha.end()));
    res.states=states+a.states;
    res.deterministic=false;
    res.transitions=transitions;
    for(auto f: finalStates)
        res.transitions.emplace(f, epsilon, states);
    for(auto&& t: a.transitions)
        res.transitions.emplace(t.From()+states, t.Label(), t.To()+states);
    for(auto f: a.finalStates)
        res.finalStates.insert(f+states);
    return res;
}

Automaton Automaton::KleeneStar() const
{
    Automaton res;
    res.alpha=alpha;
    res.states=states+1;
    res.deterministic=!states;
    res.transitions.emplace(0, epsilon, 1);
    for(auto&& t: transitions)
        res.transitions.emplace(t.From()+1, t.Label(), t.To()+1);
    res.finalStates.insert(0);
    for(auto f: finalStates)
    {
        res.transitions.emplace(f+1, epsilon, 0);
        res.finalStates.insert(f+1);
    }
    return res;
}

bool Automaton::isFinalStateReachable(std::size_t start) const
{
    if(!states) return false;
    std::vector<bool> f(states);
    std::queue<std::size_t> q;
    q.push(start);
    f[start]=true;
    while(!q.empty())
    {
        auto v=q.front();
        q.pop();
        if(isFinal(v)) return true;
        for(auto it=transitions.lower_bound(Transition(v, 0, 0)); it!=transitions.end() && it->From()==v; ++it)
            if(!f[it->To()])
            {
                f[it->To()]=true;
                q.push(it->To());
            }
    }
    return false;
}

bool Automaton::acceptsTheEmptyLang() const
{
    return !isFinalStateReachable(0);
}

bool Automaton::existPathWithNonzeroLength(std::size_t from, std::size_t to) const
{
    if(!states) return false;
    std::vector<bool> f(states);
    std::queue<std::size_t> q;
    q.push(from);
    f[from]=true;
    while(!q.empty())
    {
        auto v=q.front();
        q.pop();
        for(auto it=transitions.lower_bound(Transition(v, 0, 0)); it!=transitions.end() && it->From()==v; ++it)
        {
            if(it->To()==to) return true;
            if(!f[it->To()])
            {
                f[it->To()]=true;
                q.push(it->To());
            }
        }
    }
    return false;
}

bool Automaton::acceptsFiniteLang() const
{
    Automaton temp(*this);
    temp.minimize();
    for(std::size_t i=0; i<states; ++i)
        if(isFinalStateReachable(i) && existPathWithNonzeroLength(i,i)) return false;
    return true;
}

bool Automaton::save(const std::string& path) const
{
    std::ofstream ofs(path);
    if(ofs)
    {
        ofs << *this;
        return true;
    }
    return false;
}

void Automaton::addReachableThroughEps(std::set<std::size_t>& st) const
{
    std::set<std::size_t> tmp;
    for(auto n: st)
    {
        std::vector<bool> f(states);
        std::queue<std::size_t> q;
        q.push(n);
        f[n]=true;
        while(!q.empty())
        {
            auto v=q.front();
            q.pop();
            tmp.insert(v);
            for(auto it=transitions.lower_bound(Transition(v, epsilon, 0)); it!=transitions.end() && it->From()==v && it->Label()==epsilon; ++it)
                if(!f[it->To()])
                {
                    f[it->To()]=true;
                    q.push(it->To());
                }
        }
    }
    st=std::move(tmp);
}

bool Automaton::containsFinalState(const std::set<std::size_t>& st) const
{
    std::vector<std::size_t> intersection;
    std::set_intersection(st.begin(), st.end(), finalStates.begin(), finalStates.end(), std::back_inserter(intersection));
    return !intersection.empty();
}

Automaton& Automaton::convertToDFA()
{
    if(deterministic) return *this;
    std::queue<std::set<std::size_t>> st;
    st.push(std::set<std::size_t>{0});
    addReachableThroughEps(st.front());
    std::set<std::set<std::size_t>> visited;
    visited.insert(st.front());
    std::map<std::set<std::size_t>, std::size_t> newStates;
    std::size_t c=0;
    newStates[st.front()]=c++;
    std::set<Transition> trans;
    std::set<std::size_t> fin;
    if(containsFinalState(st.front())) fin.insert(0);
    while(!st.empty())
    {
        auto t=std::move(st.front());
        st.pop();
        for(char letter: alpha)
        {
            std::set<std::size_t> n;
            for(auto oldState: t)
            {
                auto it=transitions.lower_bound(Transition(oldState, letter, 0));
                for(; it!=transitions.end(); ++it)
                    if(it->From()==oldState && it->Label()==letter)
                        n.insert(it->To());
                    else break;
                if(it==transitions.end()) break;
            }
            addReachableThroughEps(n);
            auto p=newStates.emplace(n,c);
            if(p.second) ++c;
            if(containsFinalState(n)) fin.insert(p.first->second);
            trans.emplace(newStates[t], letter, p.first->second);
            if(visited.insert(n).second)
                st.push(std::move(n));
        }
    }
    states=newStates.size();
    transitions=std::move(trans);
    finalStates=std::move(fin);
    deterministic=true;
    return *this;
}

void Automaton::removeUnreachableStates()
{
    if(!states) return;
    std::vector<bool> f(states);
    std::queue<std::size_t> q;
    q.push(0);
    f[0]=true;
    while(!q.empty())
    {
        auto v=q.front();
        q.pop();
        for(auto it=transitions.lower_bound(Transition(v, 0, 0)); it!=transitions.end() && it->From()==v; ++it)
            if(!f[it->To()])
            {
                f[it->To()]=true;
                q.push(it->To());
            }
    }
    states=0;
    for(bool b: f)
        if(b) ++states;
    for(auto it=transitions.begin(); it!=transitions.end();)
        if(!f[it->From()] || !f[it->To()]) it=transitions.erase(it);
        else ++it;
    for(auto it=finalStates.begin(); it!=finalStates.end();)
        if(!f[*it]) it=finalStates.erase(it);
        else ++it;
}

Automaton& Automaton::minimize()
{
    convertToDFA();
    removeUnreachableStates();
    if(finalStates.empty()) return *this=Automaton();
    std::vector<int> belongTo(states);
    for(auto f: finalStates)
        belongTo[f]=1;
    std::set<std::size_t> tmp;
    for(std::size_t i=0; i<belongTo.size(); ++i)
        if(!belongTo[i]) tmp.insert(tmp.end(), i);
    std::set<std::set<std::size_t>> eqClasses{tmp, finalStates}, newEqClasses;
    for(;;)
    {
        for(const auto& cl: eqClasses)
        {
            if(cl.empty()) continue;
            if(cl.size()==1)
            {
                newEqClasses.insert(cl);
                continue;
            }
            std::set<std::pair<std::vector<int>, std::size_t>> t;
            for(auto st: cl)
            {
                std::vector<int> tr;
                for(char letter: alpha)
                    tr.push_back(belongTo[transitions.lower_bound(Transition(st,letter,0))->To()]);
                t.emplace(std::move(tr), st);
            }
            auto it2=t.begin();
            std::set<std::size_t> eqClass{it2->second};
            for(auto it1=it2++; it2!=t.end(); ++it1, ++it2)
            {
                if(it1->first!=it2->first)
                {
                    newEqClasses.insert(std::move(eqClass));
                    eqClass.clear();
                }
                eqClass.insert(it2->second);
            }
            newEqClasses.insert(std::move(eqClass));
        }
        if(eqClasses==newEqClasses) break;
        eqClasses=std::move(newEqClasses);
        newEqClasses.clear();
        int c=0;
        for(const auto& cl: eqClasses)
        {
            for(auto st: cl)
                belongTo[st]=c;
            ++c;
        }
    }
    std::size_t c=1;
    std::map<std::set<std::size_t>, std::size_t> newStates;
    for(auto&& cl: eqClasses)
        if(cl.count(0)) newStates[std::move(cl)]=0;
        else newStates[std::move(cl)]=c++;
    std::set<Transition> trans;
    std::set<std::size_t> fin;
    for(const auto& cl: newStates)
    {
        if(containsFinalState(cl.first))
            fin.insert(cl.second);
        for(auto st: cl.first)
            belongTo[st]=cl.second;
    }
    for(const auto& cl: newStates)
        for(char letter: alpha)
            trans.emplace(cl.second, letter, belongTo[transitions.lower_bound(Transition(*cl.first.begin(),letter,0))->To()]);
    states=newStates.size();
    transitions=std::move(trans);
    finalStates=std::move(fin);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Automaton& a)
{
    os << a.states << ' ' << a.finalStates.size() << std::endl;
    auto it=a.finalStates.begin();
    if(it!=a.finalStates.end())
    {
        os << *it;
        while(++it!=a.finalStates.end())
            os << ' ' << *it;
        os << std::endl;
    }
    os << a.transitions.size() << std::endl;
    for(auto&& x: a.transitions)
        os << x.From() << ' ' << x.Label() << ' ' << x.To() << std::endl;
    return os;
}
