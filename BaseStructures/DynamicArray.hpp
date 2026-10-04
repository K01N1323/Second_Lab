#ifndef DYNAMICARRAY_HPP
#define DYNAMICARRAY_HPP

#include <limits>
#include <stdexcept>
#include <utility>

// Класс для представления динамического массива
template <class T> class DynamicArray {
private:
    T* items; // Указатель на элементы массива
    int size; // Текущий размер массива
    int capacity; // Вместимость массива

    // Обменивает содержимое двух массивов
    void Swap(DynamicArray<T>& dynamic_array) {
        std::swap(this->items, dynamic_array.items);
        std::swap(this->size, dynamic_array.size);
        std::swap(this->capacity, dynamic_array.capacity);
    }

public:
    // Конструктор: создает массив заданного размера
    DynamicArray(int size): DynamicArray(nullptr, size) {}

    // Конструктор: проверяет и инициализирует массив переданными элементами
    DynamicArray(T* items, int count) {
        if (count < 0) {
            throw std::invalid_argument("Размер не может быть отрицательным");
        }

        this->size = count;
        this->capacity = count;
        this->items = (capacity == 0) ? nullptr : new T[capacity];

        try {
            if (items == nullptr) {
                for (int element = 0; element < size; element++) {
                    this->items[element] = T();
                }
            } else {
                for (int element = 0; element < size; element++) {
                    this->items[element] = items[element];
                }
            }
        } catch (...) {
            delete[] this->items;
            throw;
        }
    }

    // Конструктор копирования
    DynamicArray(const DynamicArray<T>& dynamic_array) {
        this->size = dynamic_array.size;
        this->capacity = dynamic_array.capacity;
        this->items = (capacity == 0) ? nullptr : new T[capacity];

        try {
            for (int element = 0; element < size; element++) {
                items[element] = dynamic_array.items[element];
            }
        } catch (...) {
            delete[] this->items;
            throw;
        }
    }

    // Копирующее присваивание
    DynamicArray<T>& operator=(const DynamicArray<T>& dynamic_array) {
        if (this != &dynamic_array) {
            DynamicArray<T> copy(dynamic_array);
            this->Swap(copy);
        }

        return *this;
    }

    // Возвращает размер массива
    int GetSize() const {
        return this->size;
    }

    // Получает элемент по индексу
    const T& Get(int index) const {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Индекс невалиден");
        }

        return items[index];
    }

    // Устанавливает значение элемента по индексу
    void Set(int index, const T& value) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Индекс невалиден");
        }

        items[index] = value;
    }

    // Увеличивает вместимость массива
    void Reserve(int new_capacity) {
        if (new_capacity < 0) {
            throw std::invalid_argument("Вместимость не может быть отрицательной");
        }

        if (new_capacity <= capacity) {
            return;
        }

        T* new_items = new T[new_capacity];

        try {
            for (int element = 0; element < size; element++) {
                new_items[element] = items[element];
            }
        } catch (...) {
            delete[] new_items;
            throw;
        }

        delete[] this->items;
        this->items = new_items;
        this->capacity = new_capacity;
    }

    // Изменяет размер массива
    void Resize(int new_size) {
        if (new_size < 0) {
            throw std::invalid_argument("Размер не может быть отрицательным");
        }

        if (new_size > capacity) {
            int new_capacity = (capacity == 0) ? 1 : capacity;

            while (new_capacity < new_size) {
                if (new_capacity > std::numeric_limits<int>::max() / 2) {
                    new_capacity = new_size;
                    break;
                }

                new_capacity *= 2;
            }

            this->Reserve(new_capacity);
        }

        if (new_size > size) {
            for (int element = size; element < new_size; element++) {
                items[element] = T();
            }
        } else {
            for (int element = new_size; element < size; element++) {
                items[element] = T();
            }
        }

        this->size = new_size;
    }

    // Деструктор: освобождает выделенную память
    ~DynamicArray() {
        delete[] items;
    }
};

#endif // DYNAMICARRAY_HPP
