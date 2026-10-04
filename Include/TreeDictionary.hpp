#ifndef TREE_DICTIONARY_HPP
#define TREE_DICTIONARY_HPP

#include <utility>

#include "IDictionary.hpp"
#include "BinarySearchTree.hpp"

template <typename TKey, typename TValue>
class TreeDictionary : public IDictionary<TKey, TValue> {
private:
    int (*compareKey)(const TKey&, const KeyValuePair<TKey, TValue>&);
    BinarySearchTree<KeyValuePair<TKey, TValue>> tree;

public:
    // Обе функции задают одинаковый порядок ключей и не учитывают значения.
    explicit TreeDictionary(
        bool (*entryComparator)(const KeyValuePair<TKey, TValue>&, const KeyValuePair<TKey, TValue>&),
        int (*keyComparator)(const TKey&, const KeyValuePair<TKey, TValue>&)): compareKey(keyComparator), tree(entryComparator) {
        if (entryComparator == nullptr || compareKey == nullptr) {
            throw std::invalid_argument("Компаратор не может быть пустым");
        }
    }

    TreeDictionary(const TreeDictionary<TKey, TValue>& other): compareKey(other.compareKey), tree(other.tree) {}

    TreeDictionary(TreeDictionary<TKey, TValue>&& other) noexcept: compareKey(other.compareKey), tree(std::move(other.tree)) {}

    TreeDictionary<TKey, TValue>& operator=(const TreeDictionary<TKey, TValue>& other) {
        if (this == &other) {return *this;}

        tree = other.tree;
        compareKey = other.compareKey;

        return *this;
    }

    TreeDictionary<TKey, TValue>& operator=(TreeDictionary<TKey, TValue>&& other) noexcept {
        if (this == &other) {return *this;}

        tree = std::move(other.tree);
        compareKey = other.compareKey;

        return *this;
    }

    ~TreeDictionary() override = default;

    int GetCount() const override {return tree.GetCount();}

    // У дерева нет резервных корзин
    int GetCapacity() const override {return tree.GetCount();}

    bool IsEmpty() const {return tree.IsEmpty();}
    bool ContainsKey(const TKey& key) const override {return Find(key) != nullptr;}

    const TValue* Find(const TKey& key) const override {
        const KeyValuePair<TKey, TValue>* entry = tree.FindByKey(key, compareKey);

        return entry == nullptr ? nullptr : &(entry->value);
    }

    TValue* Find(const TKey& key) override {
        KeyValuePair<TKey, TValue>* entry = tree.FindByKey(key, compareKey);

        return entry == nullptr ? nullptr : &(entry->value);
    }

    TValue Get(const TKey& key) const override {
        const TValue* Value = Find(key);
        if (Value == nullptr) {throw std::out_of_range("Ключ не найден");}
        return *Value;
    }

    void Add(const TKey& key, const TValue& value) override {
        tree.Insert(KeyValuePair<TKey, TValue>(key, value));
    }

    void Remove(const TKey& key) override {
        const KeyValuePair<TKey, TValue>* entry = tree.FindByKey(key, compareKey);

        if (entry == nullptr) {throw std::out_of_range("Ключ не найден");}
        tree.Remove(*entry);
    }

    void Clear() noexcept override {tree.Clear();}

    IEnumerator<KeyValuePair<TKey, TValue>>* GetEnumerator() const override {
        return tree.GetEnumerator();
    }
};

#endif // TREE_DICTIONARY_HPP
