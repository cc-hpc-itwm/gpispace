#pragma once

#include <ostream>

namespace jpn {

template<class Iterator>
void printRange(std::ostream &out, Iterator begin, Iterator end) {
    out << '{';
    for (Iterator i = begin; i != end; ++i) {
        if (i != begin) {
            out << ',';
        }
        out << *i;
    }
    out << '}';
}

template<class T>
void printRange(std::ostream &out, const T &container)
{
    printRange(out, container.begin(), container.end());
}

} // namespace jpn

/* vim:set et sts=4 sw=4: */
