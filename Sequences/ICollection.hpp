#ifndef ICOLLECTION_HPP
#define ICOLLECTION_HPP

#include <cstddef>

#include "Sequences/IEnumerable.hpp"

template <typename T>
class ICollection : public IEnumerable<T> {
public:
    virtual ~ICollection() override {}

    virtual const T& Get(std::size_t index) const = 0;
    virtual int GetCount() const = 0;
};

#endif // ICOLLECTION_HPP
