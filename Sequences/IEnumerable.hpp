#ifndef IENUMERABLE_HPP
#define IENUMERABLE_HPP

#include "Sequences/IEnumerator.hpp"

template <typename T>
class IEnumerable {
public:
    virtual ~IEnumerable() {}

    virtual IEnumerator<T>* GetEnumerator() const = 0;
};

#endif // IENUMERABLE_HPP
