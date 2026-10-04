#ifndef IMMUTABLEARRAYSEQUENCE_HPP
#define IMMUTABLEARRAYSEQUENCE_HPP

#include "Sequences/ArraySequence.hpp"

// Неизменяемая последовательность на основе массива
template <class T> class ImmutableArraySequence : public ArraySequence<T> {
public:
    // Конструктор по умолчанию
    ImmutableArraySequence(): ArraySequence<T>() {}

    // Конструктор копирования элементов
    ImmutableArraySequence(T* items, int count): ArraySequence<T>(items, count) {}

    // Конструктор копирования
    ImmutableArraySequence(const ImmutableArraySequence<T>& other): ArraySequence<T>(other) {}

    // Копирующее присваивание
    ImmutableArraySequence<T>& operator=(const ImmutableArraySequence<T>& other) {
        ArraySequence<T>::operator=(other);
        return *this;
    }

    // Возвращает копию текущего объекта для избежания изменения оригинала
    ImmutableArraySequence<T>* Instance() override {
        return new ImmutableArraySequence<T>(*this);
    }

    // Создает новую пустую неизменяемую последовательность
    ImmutableArraySequence<T>* CreateEmpty() const override {
        return new ImmutableArraySequence<T>();
    }
};

#endif // IMMUTABLEARRAYSEQUENCE_HPP
