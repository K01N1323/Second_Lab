#ifndef HASH_DICTIONARY_HPP
#define HASH_DICTIONARY_HPP

#include "IDictionary.hpp"
#include "HashTable.hpp"

template <typename TKey, typename TValue>
class HashDictionary : public IDictionary<TKey, TValue> {
private:
    HashTable<TKey, TValue> table;

public:
    explicit HashDictionary(unsigned int (*hash)(const TKey&), bool (*equal)(const TKey&, const TKey&), int InitialCapacity = 8): table(hash, equal, InitialCapacity) {}

    int GetCount() const override {return table.GetCount();}
    int GetCapacity() const override {return table.GetCapacity();}
    bool IsEmpty() const {return table.IsEmpty();}
    bool ContainsKey(const TKey& key) const override {return table.ContainsKey(key);}
    TValue Get(const TKey& key) const override {return table.Get(key);}
    const TValue* Find(const TKey& key) const override {return table.Find(key);}
    TValue* Find(const TKey& key) override {return table.Find(key);}
    void Add(const TKey& key, const TValue& value) override {table.Add(key, value);}
    void Remove(const TKey& key) override {table.Remove(key);}
    void Clear() noexcept override {table.Clear();}
    IEnumerator<KeyValuePair<TKey, TValue>>* GetEnumerator() const override {return table.GetEnumerator();}
};

#endif // HASH_DICTIONARY_HPP
