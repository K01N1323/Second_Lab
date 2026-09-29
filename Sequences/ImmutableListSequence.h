#ifndef IMMUTABLELISTSEQUENCE_H
#define IMMUTABLELISTSEQUENCE_H

#include "Sequences/ListSequence.h"

// Неизменяемая последовательность на основе связного списка
template <class T> class ImmutableListSequence : public ListSequence<T> {
public:
    // Конструктор по умолчанию
    ImmutableListSequence() : ListSequence<T>() {}
    
    // Конструктор копирования элементов из массива
    ImmutableListSequence(T *items, int count) : ListSequence<T>(items, count) {}
    
    // Конструктор копирования
    ImmutableListSequence(const ImmutableListSequence<T> &other) : ListSequence<T>(other) {}

    // Копирующее присваивание
    ImmutableListSequence<T> &operator=(const ImmutableListSequence<T> &other) {
        ListSequence<T>::operator=(other);
        return *this;
    }
    
    // Конструктор на основе существующего связного списка
    ImmutableListSequence(const LinkedList<T> &list) : ListSequence<T>(list) {}

    // Возвращает копию текущего объекта для безопасного изменения
    ListSequence<T> *Instance() override {
        return new ImmutableListSequence<T>(*this);
    }

    // Создает новую пустую неизменяемую последовательность
    ListSequence<T> *CreateEmpty() const override {
        return new ImmutableListSequence<T>();
    }
};

#endif // IMMUTABLELISTSEQUENCE_H
