#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <stdexcept>

#include "Sequences/IEnumerator.hpp"

// Абстрактный базовый класс для всех последовательностей
template <class T> class Sequence {
protected:
    // Добавляю элемент при внутреннем построении результата
    virtual void AppendInternal(const T& item) = 0;

public:
    // Виртуальный деструктор
    virtual ~Sequence() {}

    // Возвращаю первый элемент последовательности
    virtual const T& GetFirst() const = 0;

    // Возвращаю последний элемент последовательности
    virtual const T& GetLast() const = 0;

    // Возвращаю элемент по заданному индексу
    virtual const T& Get(int index) const = 0;

    // Возвращаю текущую длину последовательности
    virtual int GetLength() const = 0;

    // Возвращаю подпоследовательность по заданным индексам
    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) const = 0;

    // Добавляю элемент в конец последовательности
    virtual Sequence<T>* Append(const T& item) = 0;

    // Добавляю элемент в начало последовательности
    virtual Sequence<T>* Prepend(const T& item) = 0;

    // Вставляю элемент по заданному индексу
    virtual Sequence<T>* InsertAt(const T& item, int index) = 0;

    // Возвращаю новую последовательность, являющуюся конкатенацией текущей и переданной
    virtual Sequence<T>* Concat(Sequence<T>* list) = 0;

    // Перегрузка оператора индексации для удобного доступа к элементам
    const T& operator[](int index) const {
        return this->Get(index);
    }

    // Возвращаю экземпляр текущего типа
    virtual Sequence<T>* Instance() = 0;

    // Создаю пустую последовательность текущего типа
    virtual Sequence<T>* CreateEmpty() const = 0;

    // Возвращаю итератор для обхода последовательности
    virtual IEnumerator<T>* GetEnumerator() const = 0;

    // Создаю новую последовательность путем применения функции к каждому элементу
    Sequence<T>* Map(T (*mapper)(const T&)) const {
        if (mapper == nullptr) {throw std::invalid_argument("Функция Map не может быть пустой");}
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

    // Создаю новую последовательность, состоящую только из элементов, удовлетворяющих условию
    Sequence<T>* Where(bool (*where)(const T&)) const {
        if (where == nullptr) {throw std::invalid_argument("Функция Where не может быть пустой");}
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

    // Сворачиваю последовательность в одно значение, используя переданную функцию
    template <typename T2>
    T2 Reduce(T2 (*reduce_func)(const T2&, const T&), const T2& start_value) const {
        if (reduce_func == nullptr) {throw std::invalid_argument("Функция Reduce не может быть пустой");}
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
