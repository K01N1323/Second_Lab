#ifndef LIST_SEQUENCE_H
#define LIST_SEQUENCE_H

#include <stdexcept>

#include "BaseStructures/LinkedList.h"
#include "Sequences/Sequence.h"

// Абстрактный базовый класс для последовательностей на основе связного списка
template <class T> class ListSequence : public Sequence<T> {
protected:
  LinkedList<T> *items; // Указатель на внутренний связный список

  // Добавляет элемент без создания отдельного результата
  void AppendInternal(const T &item) override { this->items->Append(item); }

public:
  // Конструктор по умолчанию: создает пустую последовательность
  ListSequence() { this->items = new LinkedList<T>(); }

  // Конструктор: создает последовательность из массива элементов
  ListSequence(T *items, int count) {
    if (count < 0) {
      throw std::invalid_argument("Размер не может быть отрицательным");
    }

    if (items == nullptr) {
      this->items = new LinkedList<T>();
    } else {
      this->items = new LinkedList<T>(items, count);
    }
  }

  // Конструктор копирования
  ListSequence(const ListSequence<T> &other) {
    this->items = new LinkedList<T>(*other.items);
  }

  // Копирующее присваивание
  ListSequence<T> &operator=(const ListSequence<T> &other) {
    if (this != &other) {
      LinkedList<T> *new_items = new LinkedList<T>(*other.items);
      delete this->items;
      this->items = new_items;
    }

    return *this;
  }

  // Конструктор: создает последовательность на основе переданного списка
  ListSequence(const LinkedList<T> &list) {
    this->items = new LinkedList<T>(list);
  }

  // Деструктор: освобождает выделенную память
  virtual ~ListSequence() override { delete this->items; }

  // Создает экземпляр текущего типа
  virtual ListSequence<T> *Instance() override = 0;

  // Создает пустую последовательность текущего типа
  virtual ListSequence<T> *CreateEmpty() const override = 0;

  // Возвращает первый элемент последовательности
  const T &GetFirst() const override { return this->items->GetFirst(); }

  // Возвращает последний элемент последовательности
  const T &GetLast() const override { return this->items->GetLast(); }

  // Получает элемент по индексу
  const T &Get(int index) const override { return this->items->Get(index); }

  // Возвращает длину последовательности
  int GetLength() const override { return this->items->GetLength(); }

  // Возвращает итератор для обхода последовательности
  IEnumerator<T> *GetEnumerator() const override {
    return this->items->GetEnumerator();
  }

  // Добавляет элемент в конец последовательности
  Sequence<T> *Append(const T &item) override {
    ListSequence<T> *target = this->Instance();

    try {
      target->items->Append(item);
    } catch (...) {
      if (target != this) {
        delete target;
      }

      throw;
    }

    return target;
  }

  // Добавляет элемент в начало последовательности
  Sequence<T> *Prepend(const T &item) override {
    ListSequence<T> *target = this->Instance();

    try {
      target->items->Prepend(item);
    } catch (...) {
      if (target != this) {
        delete target;
      }

      throw;
    }

    return target;
  }

  // Вставляет элемент по заданному индексу
  Sequence<T> *InsertAt(const T &item, int index) override {
    if (index < 0 || index > this->GetLength()) {
      throw std::out_of_range("Индекс вне списка");
    }

    ListSequence<T> *target = this->Instance();

    try {
      target->items->InsertAt(item, index);
    } catch (...) {
      if (target != this) {
        delete target;
      }

      throw;
    }

    return target;
  }

  // Возвращает новую последовательность, являющуюся конкатенацией текущей и
  // переданной
  Sequence<T> *Concat(Sequence<T> *list) override {
    if (list == nullptr) {
      throw std::invalid_argument("Последовательность не задана");
    }

    ListSequence<T> *new_list = this->CreateEmpty();
    IEnumerator<T> *it_self = nullptr;
    IEnumerator<T> *it = nullptr;

    try {
      // Обход текущего списка через итератор (O(N) вместо O(N^2))
      it_self = this->GetEnumerator();
      if (it_self != nullptr) {
        while (it_self->HasNext()) {
          new_list->items->Append(it_self->GetCurrent());
          it_self->MoveNext();
        }
      }
      delete it_self;
      it_self = nullptr;

      // Обход переданного списка через итератор
      it = list->GetEnumerator();
      if (it != nullptr) {
        while (it->HasNext()) {
          new_list->items->Append(it->GetCurrent());
          it->MoveNext();
        }
      } else {
        for (int i = 0; i < list->GetLength(); i++) {
          new_list->items->Append(list->Get(i));
        }
      }
    } catch (...) {
      delete it_self;
      delete it;
      delete new_list;
      throw;
    }

    delete it;

    return new_list;
  }

  // Возвращает подпоследовательность по заданным индексам
  Sequence<T> *GetSubsequence(int start_index, int end_index) const override {
    LinkedList<T> *raw_list = this->items->GetSubList(start_index, end_index);
    ListSequence<T> *new_list = nullptr;

    try {
      new_list = this->CreateEmpty();
      delete new_list->items;
      new_list->items = raw_list;
    } catch (...) {
      delete raw_list;
      delete new_list;
      throw;
    }

    return new_list;
  }
};

#endif // LISTSEQUENCE_H
