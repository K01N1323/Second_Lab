#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <stdexcept>

#include "Sequences/IEnumerator.hpp"

// Абстрактный базовый класс для всех последовательностей
template <class T> class Sequence {
protected:
    // Добавляет элемент при внутреннем построении результата
    virtual void AppendInternal(const T& item) = 0;

public:
    // Виртуальный деструктор
    virtual ~Sequence() {}

    // Возвращает первый элемент последовательности
    virtual const T& GetFirst() const = 0;

    // Возвращает последний элемент последовательности
    virtual const T& GetLast() const = 0;

    // Возвращает элемент по заданному индексу
    virtual const T& Get(int index) const = 0;

    // Возвращает текущую длину последовательности
    virtual int GetLength() const = 0;

    // Возвращает подпоследовательность по заданным индексам
    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) const = 0;

    // Добавляет элемент в конец последовательности
    virtual Sequence<T>* Append(const T& item) = 0;

    // Добавляет элемент в начало последовательности
    virtual Sequence<T>* Prepend(const T& item) = 0;

    // Вставляет элемент по заданному индексу
    virtual Sequence<T>* InsertAt(const T& item, int index) = 0;

    // Возвращает новую последовательность, являющуюся конкатенацией текущей и переданной
    virtual Sequence<T>* Concat(Sequence<T>* list) = 0;

    // Перегрузка оператора индексации для удобного доступа к элементам
    const T& operator[](int index) const {
        return this->Get(index);
    }

    // Возвращает экземпляр текущего типа
    virtual Sequence<T>* Instance() = 0;

    // Создает пустую последовательность текущего типа
    virtual Sequence<T>* CreateEmpty() const = 0;

    // Возвращает итератор для обхода последовательности
    virtual IEnumerator<T>* GetEnumerator() const = 0;

    // Создает новую последовательность путем применения функции к каждому элементу
    Sequence<T>* Map(T (*mapper)(const T&)) const {
        Sequence<T>* result = this->CreateEmpty();

        IEnumerator<T>* it = nullptr;
        try {
            it = this->GetEnumerator();

            if (it != nullptr) {
                while (it->HasNext()) {
                    result->AppendInternal(mapper(it->GetCurrent()));
                    it->MoveNext();
                }
            } else {
                for (int index = 0; index < this->GetLength(); index++) {
                    result->AppendInternal(mapper(this->Get(index)));
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

    // Создает новую последовательность, состоящую только из элементов, удовлетворяющих условию
    Sequence<T>* Where(bool (*where)(const T&)) const {
        Sequence<T>* result = this->CreateEmpty();

        IEnumerator<T>* it = nullptr;
        try {
            it = this->GetEnumerator();

            if (it != nullptr) {
                while (it->HasNext()) {
                    if (where(it->GetCurrent())) {
                        result->AppendInternal(it->GetCurrent());
                    }
                    it->MoveNext();
                }
            } else {
                for (int index = 0; index < this->GetLength(); index++) {
                    if (where(this->Get(index))) {
                        result->AppendInternal(this->Get(index));
                    }
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

    // Сворачивает последовательность в одно значение, используя переданную функцию
    template <typename T2>
    T2 Reduce(T2 (*reduce_func)(const T2&, const T&), const T2& start_value) const {
        T2 result = start_value;

        IEnumerator<T>* it = this->GetEnumerator();
        try {
            if (it != nullptr) {
                while (it->HasNext()) {
                    result = reduce_func(result, it->GetCurrent());
                    it->MoveNext();
                }
            } else {
                for (int index = 0; index < this->GetLength(); index++) {
                    result = reduce_func(result, this->Get(index));
                }
            }
        } catch (...) {
            delete it;
            throw;
        }

        delete it;

        return result;
    }
};

#endif // SEQUENCE_HPP
