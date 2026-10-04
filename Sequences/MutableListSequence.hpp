#ifndef MUTABLELISTSEQUENCE_HPP
#define MUTABLELISTSEQUENCE_HPP

#include "Sequences/ListSequence.hpp"

// Изменяемая последовательность на основе связного списка
template <class T> class MutableListSequence : public ListSequence<T> {
public:
    // Конструктор по умолчанию
    MutableListSequence(): ListSequence<T>() {}

    // Конструктор: создает последовательность из массива элементов
    MutableListSequence(T* items, int count): ListSequence<T>(items, count) {}

    // Конструктор копирования
    MutableListSequence(const MutableListSequence<T>& other): ListSequence<T>(other) {}

    // Копирующее присваивание
    MutableListSequence<T>& operator=(const MutableListSequence<T>& other) {
        ListSequence<T>::operator=(other);
        return *this;
    }

    // Конструктор на основе существующего связного списка
    MutableListSequence(const LinkedList<T>& list): ListSequence<T>(list) {}

    // Возвращает текущий объект для изменения
    ListSequence<T>* Instance() override {
        return this;
    }

    // Создает новую пустую изменяемую последовательность
    ListSequence<T>* CreateEmpty() const override {
        return new MutableListSequence<T>();
    }
};

#endif // MUTABLELISTSEQUENCE_HPP
