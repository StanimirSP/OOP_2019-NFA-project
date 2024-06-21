#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <cctype>
#include <iomanip>
#include "Automaton.h"
#include "regularExpression.h"
using namespace std;

std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return std::tolower(c);});
    return s;
}

int main()
{
    std::string command, text;
    std::vector<Automaton> v;
    std::size_t id, id2;
    std::cout << std::boolalpha;
    while(std::cin) try
    {
        std::cin >> command;
        command=toLower(command);
        if(command=="open")
        {
            std::cin >> text;
            std::ifstream is(text);
            if(!is) std::cout << "Could not open file " << std::quoted(text) << std::endl;
            else
            {
                v.emplace_back(is);
                std::cout << "File " << std::quoted(text) << " loaded successfully\n";
                std::cout << "Automaton #" << v.size()-1 << " created successfully\n";
            }
        }
        else if(command=="list")
        {
            if(!v.empty()) std::cout << 0;
            for(std::size_t i=1; i<v.size(); ++i)
                std::cout << ' ' << i;
            std::cout << std::endl;
        }
        else if(command=="print")
        {
            std::cin >> id;
            std::cout << v.at(id) << std::endl;
        }
        else if(command=="save")
        {
            std::cin >> id >> text;
            if(v.at(id).save(text)) std::cout << "Success\n";
            else std::cout << "Could not open file " << std::quoted(text) << std::endl;
        }
        else if(command=="empty")
        {
            std::cin >> id;
            std::cout << v.at(id).acceptsTheEmptyLang() << std::endl;
        }
        else if(command=="deterministic")
        {
            std::cin >> id;
            std::cout << v.at(id).isDeterministic() << std::endl;
        }
        else if(command=="recognize")
        {
            std::cin >> id >> text;
            std::cout << v.at(id)(text) << std::endl;
        }
        else if(command=="union")
        {
            std::cin >> id >> id2;
            v.push_back(v.at(id).Union(v.at(id2)));
            std::cout << "Automaton #" << v.size()-1 << " created successfully\n";
        }
        else if(command=="concat")
        {
            std::cin >> id >> id2;
            v.push_back(v.at(id).Concatenation(v.at(id2)));
            std::cout << "Automaton #" << v.size()-1 << " created successfully\n";
        }
        else if(command=="un")
        {
            std::cin >> id;
            v.push_back(v.at(id).KleeneStar());
            std::cout << "Automaton #" << v.size()-1 << " created successfully\n";
        }
        else if(command=="reg")
        {
            std::cin >> text;
            RegularExpression reg(text);
            v.push_back(reg.NFA());
            std::cout << "Automaton #" << v.size()-1 << " created successfully\n";
        }
        else if(command=="dfa")
        {
            std::cin >> id;
            v.at(id).convertToDFA();
            std::cout << "Success\n";
        }
        else if(command=="finite")
        {
            std::cin >> id;
            std::cout << v.at(id).acceptsFiniteLang() << std::endl;
        }
        else if(command=="min")
        {
            std::cin >> id;
            v.at(id).minimize();
            std::cout << "Success\n";
        }
        else if(command=="exit")
            return 0;
        else
            std::cout << "Invalid command\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 0;
    }
}
