#ifndef ARRAYSEQUENCE_HPP
#define ARRAYSEQUENCE_HPP

#include <stdexcept>

#include "BaseStructures/DynamicArray.hpp"
#include "Sequences/Sequence.hpp"

// Абстрактный базовый класс для последовательностей на основе массива
template <class T> class ArraySequence : public Sequence<T> {
protected:
    DynamicArray<T>* items; // Указатель на внутренний динамический массив

    // Добавляю элемент без создания отдельного результата
    void AppendInternal(const T& item) override {
        T item_copy = item;

        this->items->Resize(this->items->GetSize() + 1);
        this->items->Set(this->items->GetSize() - 1, item_copy);
    }

public:
    // Конструктор по умолчанию: создаю пустую последовательность
    ArraySequence() {
        this->items = new DynamicArray<T>(0);
    }

    // Конструктор копирования элементов из переданного массива
    ArraySequence(T* items, int count) {
        if (count < 0) {
            throw std::invalid_argument("Размер не может быть отрицательным");
        }

        if (items == nullptr || count == 0) {
            this->items = new DynamicArray<T>(0);
        } else {
            this->items = new DynamicArray<T>(items, count);
        }
    }

    // Конструктор копирования
    ArraySequence(const ArraySequence<T>& other) {
        this->items = new DynamicArray<T>(*other.items);
    }

    // Копирующее присваивание
    ArraySequence<T>& operator=(const ArraySequence<T>& other) {
        if (this != &other) {
            DynamicArray<T>* new_items = new DynamicArray<T>(*other.items);
            delete this->items;
            this->items = new_items;
        }

        return *this;
    }

    // Деструктор: освобождаю выделенную память
    virtual ~ArraySequence() override {
        delete this->items;
    }

    // Создаю экземпляр текущего типа
    virtual ArraySequence<T>* Instance() override = 0;

    // Создаю пустую последовательность текущего типа
    virtual ArraySequence<T>* CreateEmpty() const override = 0;

    // Возвращаю первый элемент последовательности
    const T& GetFirst() const override {
        return this->items->Get(0);
    }

    // Возвращаю последний элемент последовательности
    const T& GetLast() const override {
        return this->items->Get(this->items->GetSize() - 1);
    }

    // Получаю элемент по индексу
    const T& Get(int index) const override {
        return this->items->Get(index);
    }

    // Возвращаю длину последовательности
    int GetLength() const override {
        return this->items->GetSize();
    }

    // Возвращаю итератор (по умолчанию недоступно)
    IEnumerator<T>* GetEnumerator() const override {
        return nullptr;
    }

    // Добавляю элемент в конец последовательности
    Sequence<T>* Append(const T& item) override {
        ArraySequence<T>* target = this->Instance();

        try {
            target->AppendInternal(item);
        } catch (...) {
            if (target != this) {
                delete target;
            }

            throw;
        }

        return target;
    }

    // Добавляю элемент в начало последовательности
    Sequence<T>* Prepend(const T& item) override {
        T item_copy = item;
        ArraySequence<T>* target = this->Instance();
        int size = target->items->GetSize();

        try {
            target->items->Resize(size + 1);

            for (int i = size; i > 0; i--) {
                target->items->Set(i, target->items->Get(i - 1));
            }

            target->items->Set(0, item_copy);
        } catch (...) {
            if (target != this) {
                delete target;
            }

            throw;
        }

        return target;
    }

    // Вставляю элемент по заданному индексу
    Sequence<T>* InsertAt(const T& item, int index) override {
        if (index < 0 || index > this->GetLength()) {
            throw std::out_of_range("Индекс невалиден");
        }

        T item_copy = item;
        ArraySequence<T>* target = this->Instance();
        int size = target->items->GetSize();

        try {
            target->items->Resize(size + 1);

            for (int i = size; i > index; i--) {
                target->items->Set(i, target->items->Get(i - 1));
            }

            target->items->Set(index, item_copy);
        } catch (...) {
            if (target != this) {
                delete target;
            }

            throw;
        }

        return target;
    }

    // Возвращаю новую последовательность, являющуюся конкатенацией текущей и переданной
    Sequence<T>* Concat(Sequence<T>* list) override {
        if (list == nullptr) {
            throw std::invalid_argument("Последовательность не задана");
        }

        ArraySequence<T>* result = this->CreateEmpty();
        int first_size = this->GetLength();
        int second_size = list->GetLength();
        IEnumerator<T>* it = nullptr;

        try {
            result->items->Resize(first_size + second_size);

            for (int index = 0; index < first_size; index++) {
                result->items->Set(index, this->Get(index));
            }

            it = list->GetEnumerator();
            if (it != nullptr) {
                int index = first_size;

                while (it->HasNext()) {
                    result->items->Set(index, it->GetCurrent());
                    it->MoveNext();
                    index++;
                }
            } else {
                for (int index = 0; index < second_size; index++) {
                    result->items->Set(first_size + index, list->Get(index));
                }
            }
        } catch (...) {
            delete it;
            delete result;
            throw;
        }

        delete it;

        return result;
    }

    // Возвращаю подпоследовательность по заданным индексам
    Sequence<T>* GetSubsequence(int start_index, int end_index) const override {
        if (start_index < 0 || start_index >= this->GetLength() || end_index < 0 ||
            end_index >= this->GetLength() || start_index > end_index) {
            throw std::out_of_range("Индексы невалидны");
        }

        ArraySequence<T>* result = this->CreateEmpty();
        int result_size = end_index - start_index + 1;

        try {
            result->items->Resize(result_size);

            for (int index = 0; index < result_size; index++) {
                result->items->Set(index, this->Get(start_index + index));
            }
        } catch (...) {
            delete result;
            throw;
        }

        return result;
    }
};

#endif // ARRAYSEQUENCE_HPP
