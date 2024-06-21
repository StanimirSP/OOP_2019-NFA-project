#ifndef TRANSITION_H
#define TRANSITION_H

#include <cstddef>

class Transition
{
    std::size_t from, to;
    char label;
public:
    Transition(std::size_t, char, std::size_t) noexcept;
    std::size_t From() const noexcept;
    std::size_t To() const noexcept;
    char Label() const noexcept;
    bool operator<(const Transition&) const noexcept;
};

#endif // TRANSITION_H
