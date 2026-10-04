#ifndef CONTAINER_TESTS_HPP
#define CONTAINER_TESTS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

#include "BaseStructures/DynamicArray.hpp"
#include "BaseStructures/LinkedList.hpp"
#include "Sequences/ImmutableArraySequence.hpp"
#include "Sequences/ImmutableListSequence.hpp"
#include "Sequences/MutableArraySequence.hpp"
#include "Sequences/MutableListSequence.hpp"

inline void check_container(bool condition, const std::string& test_name, int& tests_passed, int& tests_failed) {
    if (condition) {
        std::cout << "  Успешно: " << test_name << "\n";
        tests_passed++;
    } else {
        std::cout << "  Провалено: " << test_name << "\n";
        tests_failed++;
    }
}

template <class TException, class TAction>
void RequireThrows(TAction action, const std::string& message, int& tests_passed, int& tests_failed) {
    bool exception_caught = false;

    try {
        action();
    } catch (const TException&) {
        exception_caught = true;
    }

    check_container(exception_caught, message, tests_passed, tests_failed);
}

template <class T>
void CheckSelfAssignment(T& value) {
    T* same_value = &value;
    value = *same_value;
}

class ThrowingValue {
public:
    static bool throw_on_copy;

    int value;

    ThrowingValue(int value = 0) {this->value = value;}

    ThrowingValue(const ThrowingValue& other) {
        if (throw_on_copy) {
            throw std::runtime_error("Ошибка копирования");
        }

        this->value = other.value;
    }

    ThrowingValue& operator=(const ThrowingValue& other) {
        if (throw_on_copy) {
            throw std::runtime_error("Ошибка копирования");
        }

        this->value = other.value;
        return *this;
    }
};

inline bool ThrowingValue::throw_on_copy = false;

inline int DoubleValue(const int& value) {return value * 2;}

inline bool IsEven(const int& value) {return value % 2 == 0;}

inline int AddValue(const int& sum, const int& value) {return sum + value;}

inline void TestDynamicArray(int& tests_passed, int& tests_failed) {
    DynamicArray<int> array(2);
    array.Set(0, 10);
    array.Set(1, 20);

    array.Resize(8);
    check_container(array.GetSize() == 8, "DynamicArray: неверный размер после роста", tests_passed, tests_failed);
    check_container(array.Get(0) == 10 && array.Get(1) == 20,
                    "DynamicArray: потеряны элементы при росте", tests_passed, tests_failed);
    check_container(array.Get(7) == 0, "DynamicArray: новый элемент не инициализирован", tests_passed, tests_failed);

    const int* first_address = &array.Get(0);
    array.Resize(2);
    array.Resize(8);
    check_container(&array.Get(0) == first_address,
                    "DynamicArray: буфер перевыделен внутри существующей вместимости", tests_passed, tests_failed);
    check_container(array.Get(7) == 0, "DynamicArray: исключенный элемент не освобожден", tests_passed, tests_failed);

    DynamicArray<int> copy(1);
    copy = array;
    copy.Set(0, 99);
    check_container(array.Get(0) == 10, "DynamicArray: копия зависит от оригинала", tests_passed, tests_failed);

    CheckSelfAssignment(copy);
    check_container(copy.Get(0) == 99, "DynamicArray: нарушено самоприсваивание", tests_passed, tests_failed);

    int old_size = copy.GetSize();
    copy.Reserve(32);
    check_container(copy.GetSize() == old_size, "DynamicArray: Reserve изменил логический размер", tests_passed, tests_failed);

    DynamicArray<std::string> strings(1);
    strings.Set(0, "text");
    strings.Resize(4);
    strings.Resize(1);
    strings.Resize(4);
    check_container(strings.Get(0) == "text" && strings.Get(1).empty(),
                    "DynamicArray: неверное повторное увеличение со std::string", tests_passed, tests_failed);

    RequireThrows<std::invalid_argument>([]() {DynamicArray<int> invalid(-1);},
                                         "DynamicArray: принят отрицательный размер", tests_passed, tests_failed);
    RequireThrows<std::invalid_argument>([&array]() {array.Resize(-1);},
                                         "DynamicArray: Resize принял отрицательный размер", tests_passed, tests_failed);
    RequireThrows<std::invalid_argument>([&array]() {array.Reserve(-1);},
                                         "DynamicArray: Reserve принял отрицательную вместимость", tests_passed, tests_failed);
    RequireThrows<std::out_of_range>([&array]() {array.Get(-1);},
                                     "DynamicArray: принят отрицательный индекс", tests_passed, tests_failed);

    ThrowingValue source_items[] = {ThrowingValue(1), ThrowingValue(2)};
    ThrowingValue target_items[] = {ThrowingValue(9)};
    DynamicArray<ThrowingValue> source(source_items, 2);
    DynamicArray<ThrowingValue> target(target_items, 1);

    ThrowingValue::throw_on_copy = true;
    RequireThrows<std::runtime_error>([&source, &target]() {target = source;},
                                      "DynamicArray: исключение копирования потеряно", tests_passed, tests_failed);
    ThrowingValue::throw_on_copy = false;
    check_container(target.GetSize() == 1 && target.Get(0).value == 9,
                    "DynamicArray: прежнее значение разрушено при ошибке присваивания", tests_passed, tests_failed);
}

inline void TestLinkedList(int& tests_passed, int& tests_failed) {
    int values[] = {1, 2, 3};
    LinkedList<int> list(values, 3);
    LinkedList<int> copy;

    copy = list;
    copy.Prepend(0);
    check_container(list.GetLength() == 3 && list.GetFirst() == 1,
                    "LinkedList: копия зависит от оригинала", tests_passed, tests_failed);

    CheckSelfAssignment(copy);
    check_container(copy.GetLength() == 4 && copy.GetFirst() == 0,
                    "LinkedList: нарушено самоприсваивание", tests_passed, tests_failed);

    list.InsertAt(4, list.GetLength());
    check_container(list.GetLast() == 4, "LinkedList: вставка в конец не выполнена", tests_passed, tests_failed);

    IEnumerator<int>* enumerator = list.GetEnumerator();
    while (enumerator->HasNext()) {
        enumerator->MoveNext();
    }
    RequireThrows<std::out_of_range>([enumerator]() {enumerator->GetCurrent();},
                                     "LinkedList: чтение завершенного перечислителя разрешено", tests_passed, tests_failed);
    delete enumerator;

    RequireThrows<std::invalid_argument>([]() {LinkedList<int> invalid(nullptr, -1);},
                                         "LinkedList: принят отрицательный размер", tests_passed, tests_failed);
    RequireThrows<std::out_of_range>([&list]() {list.Get(list.GetLength());},
                                     "LinkedList: принят индекс за границей", tests_passed, tests_failed);

    ThrowingValue source_items[] = {ThrowingValue(1), ThrowingValue(2)};
    ThrowingValue target_items[] = {ThrowingValue(9)};
    LinkedList<ThrowingValue> source(source_items, 2);
    LinkedList<ThrowingValue> target(target_items, 1);

    ThrowingValue::throw_on_copy = true;
    RequireThrows<std::runtime_error>([&source, &target]() {target = source;},
                                      "LinkedList: исключение копирования потеряно", tests_passed, tests_failed);
    ThrowingValue::throw_on_copy = false;
    check_container(target.GetLength() == 1 && target.GetFirst().value == 9,
                    "LinkedList: прежнее значение разрушено при ошибке присваивания", tests_passed, tests_failed);
}

inline void TestSequenceCopying(int& tests_passed, int& tests_failed) {
    int values[] = {1, 2, 3};

    MutableArraySequence<int> mutable_array(values, 3);
    MutableArraySequence<int> mutable_array_copy;
    mutable_array_copy = mutable_array;
    mutable_array_copy.Append(4);
    check_container(mutable_array.GetLength() == 3 && mutable_array_copy.GetLength() == 4,
                    "MutableArraySequence: копия зависит от оригинала", tests_passed, tests_failed);
    CheckSelfAssignment(mutable_array_copy);

    MutableListSequence<int> mutable_list(values, 3);
    MutableListSequence<int> mutable_list_copy;
    mutable_list_copy = mutable_list;
    mutable_list_copy.Prepend(0);
    check_container(mutable_list.GetFirst() == 1 && mutable_list_copy.GetFirst() == 0,
                    "MutableListSequence: копия зависит от оригинала", tests_passed, tests_failed);
    CheckSelfAssignment(mutable_list_copy);

    ImmutableArraySequence<int> immutable_array_copy;
    {
        ImmutableArraySequence<int> source(values, 3);
        immutable_array_copy = source;
    }
    check_container(immutable_array_copy.GetLast() == 3,
                    "ImmutableArraySequence: присваивание не владеет копией", tests_passed, tests_failed);
    CheckSelfAssignment(immutable_array_copy);

    ImmutableListSequence<int> immutable_list_copy;
    {
        ImmutableListSequence<int> source(values, 3);
        immutable_list_copy = source;
    }
    check_container(immutable_list_copy.GetLast() == 3,
                    "ImmutableListSequence: присваивание не владеет копией", tests_passed, tests_failed);
    CheckSelfAssignment(immutable_list_copy);

    MutableArraySequence<int> inner(values, 3);
    DynamicArray<MutableArraySequence<int>> nested(1);
    nested.Set(0, inner);

    DynamicArray<MutableArraySequence<int>> nested_copy(1);
    nested_copy = nested;

    int replacement_values[] = {9};
    MutableArraySequence<int> replacement(replacement_values, 1);
    nested_copy.Set(0, replacement);
    check_container(nested.Get(0).GetLength() == 3 && nested.Get(0).GetFirst() == 1,
                    "Вложенный контейнер скопирован поверхностно", tests_passed, tests_failed);
}

inline void TestSelfReferences(int& tests_passed, int& tests_failed) {
    std::string values[] = {"alpha", "beta"};
    MutableArraySequence<std::string> sequence(values, 2);

    sequence.Append(sequence.Get(0));
    check_container(sequence.GetLast() == "alpha", "Append повредил ссылку на собственный элемент", tests_passed, tests_failed);

    sequence.Prepend(sequence.Get(1));
    check_container(sequence.GetFirst() == "beta", "Prepend повредил ссылку на собственный элемент", tests_passed, tests_failed);

    sequence.InsertAt(sequence.Get(3), 2);
    check_container(sequence.Get(2) == "alpha", "InsertAt повредил ссылку на собственный элемент", tests_passed, tests_failed);

    MutableListSequence<std::string> list(values, 2);
    list.Append(list.Get(0));
    list.Prepend(list.Get(1));
    list.InsertAt(list.Get(3), list.GetLength());
    check_container(list.GetLast() == "alpha", "LinkedList-вставка повредила собственный элемент", tests_passed, tests_failed);
}

inline void CheckOperations(Sequence<int>& sequence, int& tests_passed, int& tests_failed) {
    int original_length = sequence.GetLength();
    Sequence<int>* mapped = sequence.Map(DoubleValue);
    check_container(mapped->GetLength() == 4 && mapped->Get(0) == 2 && mapped->Get(3) == 8,
                    "Map вернул неверный результат", tests_passed, tests_failed);

    Sequence<int>* filtered = sequence.Where(IsEven);
    check_container(filtered->GetLength() == 2 && filtered->Get(0) == 2 && filtered->Get(1) == 4,
                    "Where вернул неверный результат", tests_passed, tests_failed);

    check_container(sequence.Reduce(AddValue, 0) == 10, "Reduce вернул неверный результат", tests_passed, tests_failed);

    Sequence<int>* subsequence = sequence.GetSubsequence(1, 2);
    check_container(subsequence->GetLength() == 2 && subsequence->Get(0) == 2 &&
                        subsequence->Get(1) == 3,
                    "GetSubsequence вернул неверный результат", tests_passed, tests_failed);

    int other_values[] = {5, 6};
    MutableListSequence<int> other(other_values, 2);
    Sequence<int>* concatenated = sequence.Concat(&other);
    check_container(concatenated->GetLength() == 6 && concatenated->Get(4) == 5 &&
                        concatenated->Get(5) == 6,
                    "Concat вернул неверный результат", tests_passed, tests_failed);
    check_container(sequence.GetLength() == original_length,
                    "Операция построения результата изменила исходную последовательность", tests_passed, tests_failed);

    delete mapped;
    delete filtered;
    delete subsequence;
    delete concatenated;
}

inline void TestSequenceOperations(int& tests_passed, int& tests_failed) {
    int values[] = {1, 2, 3, 4};

    MutableArraySequence<int> mutable_array(values, 4);
    Sequence<int>* mutable_result = mutable_array.Append(5);
    check_container(mutable_result == &mutable_array && mutable_array.GetLength() == 5,
                    "MutableArraySequence перестала изменять себя", tests_passed, tests_failed);

    mutable_array = MutableArraySequence<int>(values, 4);
    CheckOperations(mutable_array, tests_passed, tests_failed);

    ImmutableArraySequence<int> immutable_array(values, 4);
    Sequence<int>* immutable_result = immutable_array.Append(5);
    check_container(immutable_result != &immutable_array && immutable_array.GetLength() == 4 &&
                        immutable_result->GetLength() == 5,
                    "ImmutableArraySequence изменила исходный объект", tests_passed, tests_failed);
    delete immutable_result;
    CheckOperations(immutable_array, tests_passed, tests_failed);

    MutableListSequence<int> mutable_list(values, 4);
    mutable_result = mutable_list.InsertAt(5, mutable_list.GetLength());
    check_container(mutable_result == &mutable_list && mutable_list.GetLast() == 5,
                    "MutableListSequence не вставила элемент в конец", tests_passed, tests_failed);

    mutable_list = MutableListSequence<int>(values, 4);
    CheckOperations(mutable_list, tests_passed, tests_failed);

    ImmutableListSequence<int> immutable_list(values, 4);
    immutable_result = immutable_list.Prepend(0);
    check_container(immutable_result != &immutable_list && immutable_list.GetFirst() == 1 &&
                        immutable_result->GetFirst() == 0,
                    "ImmutableListSequence изменила исходный объект", tests_passed, tests_failed);
    delete immutable_result;
    CheckOperations(immutable_list, tests_passed, tests_failed);

    RequireThrows<std::out_of_range>([&mutable_array]() {mutable_array.InsertAt(0, -1);},
                                     "ArraySequence: принят отрицательный индекс вставки", tests_passed, tests_failed);
    RequireThrows<std::out_of_range>([&mutable_list]() {mutable_list.InsertAt(0, 10);},
                                     "ListSequence: принята позиция за границей", tests_passed, tests_failed);
}

inline int run_container_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nТестирование контейнеров и последовательностей\n";
    try {
        TestDynamicArray(tests_passed, tests_failed);
    } catch (...) {
        check_container(false, "TestDynamicArray: неожиданное исключение", tests_passed, tests_failed);
    }
    try {
        TestLinkedList(tests_passed, tests_failed);
    } catch (...) {
        check_container(false, "TestLinkedList: неожиданное исключение", tests_passed, tests_failed);
    }
    try {
        TestSequenceCopying(tests_passed, tests_failed);
    } catch (...) {
        check_container(false, "TestSequenceCopying: неожиданное исключение", tests_passed, tests_failed);
    }
    try {
        TestSelfReferences(tests_passed, tests_failed);
    } catch (...) {
        check_container(false, "TestSelfReferences: неожиданное исключение", tests_passed, tests_failed);
    }
    try {
        TestSequenceOperations(tests_passed, tests_failed);
    } catch (...) {
        check_container(false, "TestSequenceOperations: неожиданное исключение", tests_passed, tests_failed);
    }
    std::cout << "\nКонтейнеры: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

#endif // CONTAINER_TESTS_HPP
