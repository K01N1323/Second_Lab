#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include <stdexcept>
#include <utility>

#include "Sequences/IEnumerator.hpp"

// Шаблонный класс односвязного списка
template <class T> class LinkedList {
private:
    // Внутренняя структура узла списка
    struct Node {
        T data;
        Node* next;

        Node(const T& value): data(value), next(nullptr) {}
    };

    Node* head; // Указатель на начало списка
    Node* tail; // Указатель на конец списка
    int size; // Текущий размер списка

    // Освобождаю память всех узлов
    void Clear() {
        Node* current = this->head;

        while (current != nullptr) {
            Node* local_current = current->next;
            delete current;
            current = local_current;
        }

        this->head = nullptr;
        this->tail = nullptr;
        this->size = 0;
    }

    // Обмениваю содержимое двух списков
    void Swap(LinkedList<T>& list) {
        std::swap(this->head, list.head);
        std::swap(this->tail, list.tail);
        std::swap(this->size, list.size);
    }

public:
    // Конструктор по умолчанию: создаю пустой список
    LinkedList(): head(nullptr), tail(nullptr), size(0) {}

    // Конструктор: создаю список из массива элементов
    LinkedList(T* items, int count) {
        if (count < 0) {
            throw std::invalid_argument("Размер не может быть отрицательным");
        }

        this->head = nullptr;
        this->tail = nullptr;
        this->size = 0;

        try {
            for (int index = 0; index < count; index++) {
                if (items == nullptr) {
                    this->Append(T());
                } else {
                    this->Append(items[index]);
                }
            }
        } catch (...) {
            this->Clear();
            throw;
        }
    }

    // Конструктор копирования
    LinkedList(const LinkedList<T>& list): LinkedList() {
        try {
            Node* current = list.head;

            while (current != nullptr) {
                this->Append(current->data);
                current = current->next;
            }
        } catch (...) {
            this->Clear();
            throw;
        }
    }

    // Копирующее присваивание
    LinkedList<T>& operator=(const LinkedList<T>& list) {
        if (this != &list) {
            LinkedList<T> copy(list);
            this->Swap(copy);
        }

        return *this;
    }

    // Возвращаю первый элемент списка
    const T& GetFirst() const {
        if (head == nullptr) {
            throw std::out_of_range("Первого элемента не существует");
        }

        return head->data;
    }

    // Возвращаю последний элемент списка
    const T& GetLast() const {
        if (tail == nullptr) {
            throw std::out_of_range("Последнего элемента не существует");
        }

        return tail->data;
    }

    // Возвращаю элемент по индексу
    const T& Get(int index) const {
        if (index >= size || index < 0) {
            throw std::out_of_range("Элемента с таким индексом не существует");
        }

        Node* current = head;

        for (int ind = 0; ind < index; ind++) {
            current = current->next;
        }

        return current->data;
    }

    // Возвращаю подсписок по заданным индексам
    LinkedList<T>* GetSubList(int start_index, int end_index) {
        if (start_index < 0 || end_index < 0 || end_index >= size ||
            start_index >= size || end_index < start_index) {
            throw std::out_of_range("Индексы невалидны для данного списка");
        }

        LinkedList<T>* sub_list = new LinkedList<T>();
        Node* current = this->head;

        for (int index = 0; index < start_index; index++) {
            current = current->next;
        }

        try {
            for (int index = start_index; index <= end_index; index++) {
                sub_list->Append(current->data);
                current = current->next;
            }
        } catch (...) {
            delete sub_list;
            throw;
        }

        return sub_list;
    }

    // Возвращаю текущую длину списка
    int GetLength() const {
        return size;
    }

    // Добавляю элемент в конец списка
    void Append(const T& item) {
        Node* new_node = new Node(item);

        if (head == nullptr) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }

        size++;
    }

    // Добавляю элемент в начало списка
    void Prepend(const T& item) {
        Node* new_node = new Node(item);

        if (this->head == nullptr) {
            this->head = new_node;
            this->tail = new_node;
        } else {
            new_node->next = this->head;
            this->head = new_node;
        }

        size++;
    }

    // Вставляю элемент по заданному индексу
    void InsertAt(const T& item, int index) {
        if (index > size || index < 0) {
            throw std::out_of_range("Индекс вне списка");
        }

        if (index == size) {
            Append(item);
            return;
        }

        if (index == 0) {
            Prepend(item);
            return;
        }

        Node* new_node = new Node(item);
        Node* current = head;

        for (int ind = 0; ind < (index - 1); ind++) {
            current = current->next;
        }

        new_node->next = current->next;
        current->next = new_node;
        size++;
    }

    // Возвращаю новый список, являющийся конкатенацией текущего и переданного
    LinkedList<T>* Concat(LinkedList<T>* list) {
        if (list == nullptr) {
            throw std::invalid_argument("Список не задан");
        }

        LinkedList<T>* sub_list = new LinkedList<T>(*this);
        Node* current = list->head;

        try {
            for (int index = 0; index < list->size; index++) {
                sub_list->Append(current->data);
                current = current->next;
            }
        } catch (...) {
            delete sub_list;
            throw;
        }

        return sub_list;
    }

    // Деструктор: освобождаю память всех узлов
    ~LinkedList() {
        this->Clear();
    }

    // Итератор для связного списка
    class LinkedListEnumerator : public IEnumerator<T> {
    private:
        Node* current;

    public:
        // Конструктор итератора
        LinkedListEnumerator(Node* head) {
            current = head;
        }

        // Возвращаю текущий элемент
        const T& GetCurrent() const override {
            if (current == nullptr) {
                throw std::out_of_range("Перечислитель завершен");
            }

            return current->data;
        }

        // Перехожу к следующему элементу
        void MoveNext() override {
            if (current != nullptr) {
                current = current->next;
            }
        }

        // Проверяю, есть ли следующий элемент
        bool HasNext() const override {
            return current != nullptr;
        }
    };

    // Возвращаю итератор для обхода списка
    IEnumerator<T>* GetEnumerator() const {
        return new LinkedListEnumerator(this->head);
    }
};

#endif // LINKEDLIST_HPP
