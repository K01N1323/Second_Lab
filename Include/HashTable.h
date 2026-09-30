#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdexcept>
#include "KeyValuePair.h"
#include "BaseStructures/DynamicArray.h"
#include "Sequences/IEnumerator.h"

template <class TKey, class TValue>
class HashTable {
private:
    struct Node {
        KeyValuePair<TKey, TValue> data;
        std::size_t HashCode;
        Node* next;

        Node(const TKey& key, const TValue& value, std::size_t Code, Node* NextNode = nullptr);
    };

    DynamicArray<Node*>* buckets;
    int size;
    int minimumCapacity;

    unsigned_int (*hash)(const TKey&);
    bool (*equal)(const TKey&, const TKey&);

    // Вычислить индекс корзины по хешу и вместимости.
    int GetBucketIndex(std::size_t HashCode, int Capacity) const;

    // Найти узел в соответствующей цепочке.
    Node* FindNode(const TKey& key, std::size_t HashCode) const;

    // Создать хранилище при повторном использовании после перемещения.
    void EnsureStorage();

    // Перераспределить существующие узлы по новым корзинам.
    void Rehash(int NewCapacity);

    // Проверить порог сжатия и при необходимости уменьшить таблицу.
    void TryShrink();

    // Создать независимую копию массива и всех цепочек.
    DynamicArray<Node*>* CopyBuckets(const DynamicArray<Node*>* Source);

    // Удалить все цепочки и сам переданный массив.
    void DeleteBuckets(DynamicArray<Node*>* Buckets) noexcept;

    class HashTableEnumerator : public IEnumerator<KeyValuePair<TKey, TValue>> {
    private:
        const HashTable<TKey, TValue>& table;
        int CurrentBucket;
        const Node* CurrentNode;

        // Найти непустую корзину, начиная с CurrentBucket.
        void FindNextBucket();

    public:
        explicit HashTableEnumerator(const HashTable<TKey, TValue>& Table);

        const KeyValuePair<TKey, TValue>& GetCurrent() const override;
        void MoveNext() override;
        bool HasNext() const override;
    };

public:
    explicit HashTable(std::size_t (*hashFunction)(const TKey&), bool (*keyEqual)(const TKey&, const TKey&), int InitialCapacity = 8);

    HashTable(const HashTable<TKey, TValue>& other);
    HashTable(HashTable<TKey, TValue>&& other) noexcept;

    HashTable<TKey, TValue>& operator=(const HashTable<TKey, TValue>& other);
    HashTable<TKey, TValue>& operator=(HashTable<TKey, TValue>&& other) noexcept;

    ~HashTable();

    int GetCount() const;
    int GetCapacity() const;
    bool IsEmpty() const;

    bool ContainsKey(const TKey& key) const;

    // Получить копию значения; отсутствие ключа — исключение.
    TValue Get(const TKey& key) const;

    // Получить адрес значения; отсутствие ключа — nullptr.
    const TValue* Find(const TKey& key) const;
    TValue* Find(const TKey& key);

    void Add(const TKey& key, const TValue& value);
    void Remove(const TKey& key);

    // Удалить записи, сохранив массив корзин.
    void Clear() noexcept;

    IEnumerator<KeyValuePair<TKey, TValue>>* GetEnumerator() const;
};

#endif // HASH_TABLE_H
