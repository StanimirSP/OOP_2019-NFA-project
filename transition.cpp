#include "transition.h"

Transition::Transition(std::size_t from, char label, std::size_t to) noexcept: from(from), to(to), label(label) {}

std::size_t Transition::From() const noexcept
{
    return from;
}

std::size_t Transition::To() const noexcept
{
    return to;
}

char Transition::Label() const noexcept
{
    return label;
}

bool Transition::operator<(const Transition& tr) const noexcept
{
    if(from!=tr.from) return from<tr.from;
    if(label!=tr.label) return label<tr.label;
    return to<tr.to;
}
