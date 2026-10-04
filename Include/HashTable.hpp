#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <limits>
#include <new>
#include <stdexcept>

#include "KeyValuePair.hpp"
#include "BaseStructures/DynamicArray.hpp"
#include "Sequences/IEnumerator.hpp"

template <typename TKey, typename TValue>
class HashTable {
private:
    struct Node {
        KeyValuePair<TKey, TValue> data;
        unsigned int HashCode;
        Node* next;

        Node(const TKey& key, const TValue& value, unsigned int Code, Node* NextNode = nullptr): data(key, value), HashCode(Code), next(NextNode) {}
    };

    DynamicArray<Node*>* buckets;
    int size;
    int minimumCapacity;
    unsigned int version;

    unsigned int (*hash)(const TKey&);
    bool (*equal)(const TKey&, const TKey&);

    DynamicArray<Node*>* CreateBuckets(int Capacity) {
        if (Capacity < 2) {throw std::invalid_argument("Вместимость должна быть не меньше 2");}

        DynamicArray<Node*>* NewBuckets = new DynamicArray<Node*>(Capacity);

        for (int BucketIndex = 0; BucketIndex < Capacity; BucketIndex++) {
            NewBuckets->Set(BucketIndex, nullptr);
        }

        return NewBuckets;
    }

    int GetBucketIndex(unsigned int HashCode, int Capacity) const {
        if (Capacity <= 0) {throw std::invalid_argument("Вместимость должна быть положительной");}
        return static_cast<int>(HashCode % static_cast<unsigned int>(Capacity));
    }

    Node* FindNode(const TKey& key, unsigned int HashCode) const {
        if (buckets == nullptr || size == 0) {return nullptr;}

        int BucketIndex = GetBucketIndex(HashCode, GetCapacity());
        Node* CurrentNode = buckets->Get(BucketIndex);

        while (CurrentNode != nullptr) {
            if (CurrentNode->HashCode == HashCode && equal(CurrentNode->data.key, key)) {
                return CurrentNode;
            }

            CurrentNode = CurrentNode->next;
        }

        return nullptr;
    }

    void EnsureStorage() {
        if (buckets == nullptr) {
            buckets = CreateBuckets(minimumCapacity);
            version++;
        }
    }

    void Rehash(int NewCapacity) {
        if (NewCapacity < minimumCapacity || NewCapacity < size) {
            throw std::invalid_argument("Недопустимая новая вместимость");
        }

        if (NewCapacity == GetCapacity()) {return;}

        DynamicArray<Node*>* NewBuckets = CreateBuckets(NewCapacity);
        int OldCapacity = GetCapacity();

        for (int BucketIndex = 0; BucketIndex < OldCapacity; BucketIndex++) {
            Node* CurrentNode = buckets->Get(BucketIndex);

            while (CurrentNode != nullptr) {
                Node* NextNode = CurrentNode->next;
                int NewIndex = GetBucketIndex(CurrentNode->HashCode, NewCapacity);

                CurrentNode->next = NewBuckets->Get(NewIndex);
                NewBuckets->Set(NewIndex, CurrentNode);

                CurrentNode = NextNode;
            }
        }

        delete buckets;
        buckets = NewBuckets;
        version++;
    }

    void TryShrink() {
        int Capacity = GetCapacity();

        if (Capacity <= minimumCapacity || size > Capacity / 4) {return;}

        int NewCapacity = Capacity / 2;
        if (NewCapacity < minimumCapacity) {NewCapacity = minimumCapacity;}

        try {
            Rehash(NewCapacity);
        } catch (const std::bad_alloc&) {}
    }

    void DeleteBuckets(DynamicArray<Node*>* Buckets) noexcept {
        if (Buckets == nullptr) {return;}

        for (int BucketIndex = 0; BucketIndex < Buckets->GetSize(); BucketIndex++) {
            Node* CurrentNode = Buckets->Get(BucketIndex);

            while (CurrentNode != nullptr) {
                Node* NextNode = CurrentNode->next;
                delete CurrentNode;
                CurrentNode = NextNode;
            }
        }

        delete Buckets;
    }

    DynamicArray<Node*>* CopyBuckets(const DynamicArray<Node*>* Source) {
        if (Source == nullptr) {return nullptr;}

        DynamicArray<Node*>* NewBuckets = CreateBuckets(Source->GetSize());

        try {
            for (int BucketIndex = 0; BucketIndex < Source->GetSize(); BucketIndex++) {
                const Node* SourceNode = Source->Get(BucketIndex);
                Node* LastNode = nullptr;

                while (SourceNode != nullptr) {
                    Node* NewNode = new Node(SourceNode->data.key, SourceNode->data.value, SourceNode->HashCode);

                    if (LastNode == nullptr) {NewBuckets->Set(BucketIndex, NewNode);} else {LastNode->next = NewNode;}

                    LastNode = NewNode;
                    SourceNode = SourceNode->next;
                }
            }
        } catch (...) {
            DeleteBuckets(NewBuckets);
            throw;
        }

        return NewBuckets;
    }

    class HashTableEnumerator : public IEnumerator<KeyValuePair<TKey, TValue>> {
    private:
        const HashTable<TKey, TValue>& table;
        int CurrentBucket;
        const Node* CurrentNode;
        unsigned int ExpectedVersion;

        void CheckVersion() const {
            if (ExpectedVersion != table.version) {
                throw std::logic_error("Таблица была изменена во время обхода");
            }
        }

        void FindNextBucket() {
            CurrentNode = nullptr;

            while (CurrentBucket < table.GetCapacity()) {
                CurrentNode = table.buckets->Get(CurrentBucket);
                if (CurrentNode != nullptr) {return;}
                CurrentBucket++;
            }
        }

    public:
        explicit HashTableEnumerator(const HashTable<TKey, TValue>& Table): table(Table), CurrentBucket(0), CurrentNode(nullptr), ExpectedVersion(Table.version) {
            FindNextBucket();
        }

        const KeyValuePair<TKey, TValue>& GetCurrent() const override {
            CheckVersion();
            if (CurrentNode == nullptr) {throw std::out_of_range("Перечислитель находится за границами таблицы");}
            return CurrentNode->data;
        }

        void MoveNext() override {
            CheckVersion();
            if (CurrentNode == nullptr) {return;}

            if (CurrentNode->next != nullptr) {
                CurrentNode = CurrentNode->next;
            } else {
                CurrentBucket++;
                FindNextBucket();
            }
        }

        bool HasNext() const override {
            CheckVersion();
            return CurrentNode != nullptr;
        }
    };

public:
    explicit HashTable(unsigned int (*hashFunction)(const TKey&), bool (*keyEqual)(const TKey&, const TKey&), int InitialCapacity = 8): buckets(nullptr), size(0), minimumCapacity(InitialCapacity), version(0), hash(hashFunction), equal(keyEqual) {
        if (hash == nullptr || equal == nullptr) {
            throw std::invalid_argument("Функции хеширования и сравнения должны быть заданы");
        }

        buckets = CreateBuckets(InitialCapacity);
    }

    HashTable(const HashTable<TKey, TValue>& other): buckets(nullptr), size(0), minimumCapacity(other.minimumCapacity), version(0), hash(other.hash), equal(other.equal) {
        buckets = CopyBuckets(other.buckets);
        size = other.size;
    }

    HashTable(HashTable<TKey, TValue>&& other) noexcept: buckets(other.buckets), size(other.size), minimumCapacity(other.minimumCapacity), version(0), hash(other.hash), equal(other.equal) {
        other.buckets = nullptr;
        other.size = 0;
        other.version++;
    }

    HashTable<TKey, TValue>& operator=(const HashTable<TKey, TValue>& other) {
        if (this == &other) {return *this;}

        DynamicArray<Node*>* NewBuckets = CopyBuckets(other.buckets);
        DeleteBuckets(buckets);

        buckets = NewBuckets;
        size = other.size;
        minimumCapacity = other.minimumCapacity;
        hash = other.hash;
        equal = other.equal;
        version++;

        return *this;
    }

    HashTable<TKey, TValue>& operator=(HashTable<TKey, TValue>&& other) noexcept {
        if (this == &other) {return *this;}

        DeleteBuckets(buckets);

        buckets = other.buckets;
        size = other.size;
        minimumCapacity = other.minimumCapacity;
        hash = other.hash;
        equal = other.equal;
        version++;

        other.buckets = nullptr;
        other.size = 0;
        other.version++;

        return *this;
    }

    ~HashTable() {
        DeleteBuckets(buckets);
    }

    int GetCount() const {return size;}
    int GetCapacity() const {return buckets == nullptr ? 0 : buckets->GetSize();}
    bool IsEmpty() const {return size == 0;}

    bool ContainsKey(const TKey& key) const {
        return Find(key) != nullptr;
    }

    const TValue* Find(const TKey& key) const {
        if (size == 0) {return nullptr;}

        Node* CurrentNode = FindNode(key, hash(key));
        return CurrentNode == nullptr ? nullptr : &(CurrentNode->data.value);
    }

    TValue* Find(const TKey& key) {
        if (size == 0) {return nullptr;}

        Node* CurrentNode = FindNode(key, hash(key));
        return CurrentNode == nullptr ? nullptr : &(CurrentNode->data.value);
    }

    TValue Get(const TKey& key) const {
        const TValue* Value = Find(key);
        if (Value == nullptr) {throw std::out_of_range("Ключ не найден");}
        return *Value;
    }

    void Add(const TKey& key, const TValue& value) {
        unsigned int HashCode = hash(key);
        if (FindNode(key, HashCode) != nullptr) {throw std::invalid_argument("Ключ уже содержится в таблице");}
        if (size == std::numeric_limits<int>::max()) {throw std::length_error("Превышен максимальный размер таблицы");}

        Node* NewNode = new Node(key, value, HashCode);

        try {
            EnsureStorage();
            int Capacity = GetCapacity();

            if (size + 1 >= Capacity) {
                if (Capacity > std::numeric_limits<int>::max() / 2) {
                    throw std::length_error("Невозможно увеличить вместимость таблицы вдвое");
                }

                Rehash(Capacity * 2);
            }
        } catch (...) {
            delete NewNode;
            throw;
        }

        int BucketIndex = GetBucketIndex(HashCode, GetCapacity());
        NewNode->next = buckets->Get(BucketIndex);
        buckets->Set(BucketIndex, NewNode);

        size++;
        version++;
    }

    void Remove(const TKey& key) {
        if (size == 0) {throw std::out_of_range("Ключ не найден");}

        unsigned int HashCode = hash(key);
        int BucketIndex = GetBucketIndex(HashCode, GetCapacity());
        Node* CurrentNode = buckets->Get(BucketIndex);
        Node* PreviousNode = nullptr;

        while (CurrentNode != nullptr) {
            if (CurrentNode->HashCode == HashCode && equal(CurrentNode->data.key, key)) {break;}
            PreviousNode = CurrentNode;
            CurrentNode = CurrentNode->next;
        }

        if (CurrentNode == nullptr) {throw std::out_of_range("Ключ не найден");}

        if (PreviousNode == nullptr) {buckets->Set(BucketIndex, CurrentNode->next);} else {PreviousNode->next = CurrentNode->next;}

        delete CurrentNode;
        size--;
        version++;

        TryShrink();
    }

    void Clear() noexcept {
        if (buckets != nullptr) {
            for (int BucketIndex = 0; BucketIndex < GetCapacity(); BucketIndex++) {
                Node* CurrentNode = buckets->Get(BucketIndex);

                while (CurrentNode != nullptr) {
                    Node* NextNode = CurrentNode->next;
                    delete CurrentNode;
                    CurrentNode = NextNode;
                }

                buckets->Set(BucketIndex, nullptr);
            }
        }

        size = 0;
        version++;
    }

    IEnumerator<KeyValuePair<TKey, TValue>>* GetEnumerator() const {
        return new HashTableEnumerator(*this);
    }
};

#endif // HASH_TABLE_HPP
