#ifndef IDICTIONARY_HPP
#define IDICTIONARY_HPP

#include "KeyValuePair.hpp"
#include "Sequences/IEnumerator.hpp"

template <typename TKey, typename TValue>
class IDictionary {
public:
    virtual ~IDictionary() = default;

    virtual int GetCount() const = 0;
    virtual int GetCapacity() const = 0;
    virtual bool ContainsKey(const TKey& key) const = 0;

    virtual TValue Get(const TKey& key) const = 0;
    virtual const TValue* Find(const TKey& key) const = 0;
    virtual TValue* Find(const TKey& key) = 0;

    virtual void Add(const TKey& key, const TValue& value) = 0;
    virtual void Remove(const TKey& key) = 0;
    virtual void Clear() noexcept = 0;

    virtual IEnumerator<KeyValuePair<TKey, TValue>>* GetEnumerator() const = 0;
};

#endif // IDICTIONARY_HPP
