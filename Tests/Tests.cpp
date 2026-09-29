#include <iostream>
#include <stdexcept>
#include <string>

#include "BaseStructures/DynamicArray.h"
#include "BaseStructures/LinkedList.h"
#include "Sequences/ImmutableArraySequence.h"
#include "Sequences/ImmutableListSequence.h"
#include "Sequences/MutableArraySequence.h"
#include "Sequences/MutableListSequence.h"
#include "Tests/PerformanceTests.h"
#include "Tests/Tests.h"

namespace {
void Require(bool condition, const std::string &message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <class TException, class TAction>
void RequireThrows(TAction action, const std::string &message) {
    bool exception_caught = false;

    try {
        action();
    } catch (const TException &) {
        exception_caught = true;
    }

    Require(exception_caught, message);
}

template <class T>
void CheckSelfAssignment(T &value) {
    T *same_value = &value;
    value = *same_value;
}

class ThrowingValue {
public:
    static bool throw_on_copy;

    int value;

    ThrowingValue(int value = 0) { this->value = value; }

    ThrowingValue(const ThrowingValue &other) {
        if (throw_on_copy) {
            throw std::runtime_error("Ошибка копирования");
        }

        this->value = other.value;
    }

    ThrowingValue &operator=(const ThrowingValue &other) {
        if (throw_on_copy) {
            throw std::runtime_error("Ошибка копирования");
        }

        this->value = other.value;
        return *this;
    }
};

bool ThrowingValue::throw_on_copy = false;

int DoubleValue(const int &value) { return value * 2; }

bool IsEven(const int &value) { return value % 2 == 0; }

int AddValue(const int &sum, const int &value) { return sum + value; }

void TestDynamicArray() {
    DynamicArray<int> array(2);
    array.Set(0, 10);
    array.Set(1, 20);

    array.Resize(8);
    Require(array.GetSize() == 8, "DynamicArray: неверный размер после роста");
    Require(array.Get(0) == 10 && array.Get(1) == 20,
            "DynamicArray: потеряны элементы при росте");
    Require(array.Get(7) == 0, "DynamicArray: новый элемент не инициализирован");

    const int *first_address = &array.Get(0);
    array.Resize(2);
    array.Resize(8);
    Require(&array.Get(0) == first_address,
            "DynamicArray: буфер перевыделен внутри существующей вместимости");
    Require(array.Get(7) == 0, "DynamicArray: исключенный элемент не освобожден");

    DynamicArray<int> copy(1);
    copy = array;
    copy.Set(0, 99);
    Require(array.Get(0) == 10, "DynamicArray: копия зависит от оригинала");

    CheckSelfAssignment(copy);
    Require(copy.Get(0) == 99, "DynamicArray: нарушено самоприсваивание");

    int old_size = copy.GetSize();
    copy.Reserve(32);
    Require(copy.GetSize() == old_size, "DynamicArray: Reserve изменил логический размер");

    DynamicArray<std::string> strings(1);
    strings.Set(0, "text");
    strings.Resize(4);
    strings.Resize(1);
    strings.Resize(4);
    Require(strings.Get(0) == "text" && strings.Get(1).empty(),
            "DynamicArray: неверное повторное увеличение со std::string");

    RequireThrows<std::invalid_argument>([]() { DynamicArray<int> invalid(-1); },
                                         "DynamicArray: принят отрицательный размер");
    RequireThrows<std::invalid_argument>([&array]() { array.Resize(-1); },
                                         "DynamicArray: Resize принял отрицательный размер");
    RequireThrows<std::invalid_argument>([&array]() { array.Reserve(-1); },
                                         "DynamicArray: Reserve принял отрицательную вместимость");
    RequireThrows<std::out_of_range>([&array]() { array.Get(-1); },
                                     "DynamicArray: принят отрицательный индекс");

    ThrowingValue source_items[] = {ThrowingValue(1), ThrowingValue(2)};
    ThrowingValue target_items[] = {ThrowingValue(9)};
    DynamicArray<ThrowingValue> source(source_items, 2);
    DynamicArray<ThrowingValue> target(target_items, 1);

    ThrowingValue::throw_on_copy = true;
    RequireThrows<std::runtime_error>([&source, &target]() { target = source; },
                                      "DynamicArray: исключение копирования потеряно");
    ThrowingValue::throw_on_copy = false;
    Require(target.GetSize() == 1 && target.Get(0).value == 9,
            "DynamicArray: прежнее значение разрушено при ошибке присваивания");
}

void TestLinkedList() {
    int values[] = {1, 2, 3};
    LinkedList<int> list(values, 3);
    LinkedList<int> copy;

    copy = list;
    copy.Prepend(0);
    Require(list.GetLength() == 3 && list.GetFirst() == 1,
            "LinkedList: копия зависит от оригинала");

    CheckSelfAssignment(copy);
    Require(copy.GetLength() == 4 && copy.GetFirst() == 0,
            "LinkedList: нарушено самоприсваивание");

    list.InsertAt(4, list.GetLength());
    Require(list.GetLast() == 4, "LinkedList: вставка в конец не выполнена");

    IEnumerator<int> *enumerator = list.GetEnumerator();
    while (enumerator->HasNext()) {
        enumerator->MoveNext();
    }
    RequireThrows<std::out_of_range>([enumerator]() { enumerator->GetCurrent(); },
                                     "LinkedList: чтение завершенного перечислителя разрешено");
    delete enumerator;

    RequireThrows<std::invalid_argument>([]() { LinkedList<int> invalid(nullptr, -1); },
                                         "LinkedList: принят отрицательный размер");
    RequireThrows<std::out_of_range>([&list]() { list.Get(list.GetLength()); },
                                     "LinkedList: принят индекс за границей");

    ThrowingValue source_items[] = {ThrowingValue(1), ThrowingValue(2)};
    ThrowingValue target_items[] = {ThrowingValue(9)};
    LinkedList<ThrowingValue> source(source_items, 2);
    LinkedList<ThrowingValue> target(target_items, 1);

    ThrowingValue::throw_on_copy = true;
    RequireThrows<std::runtime_error>([&source, &target]() { target = source; },
                                      "LinkedList: исключение копирования потеряно");
    ThrowingValue::throw_on_copy = false;
    Require(target.GetLength() == 1 && target.GetFirst().value == 9,
            "LinkedList: прежнее значение разрушено при ошибке присваивания");
}

void TestSequenceCopying() {
    int values[] = {1, 2, 3};

    MutableArraySequence<int> mutable_array(values, 3);
    MutableArraySequence<int> mutable_array_copy;
    mutable_array_copy = mutable_array;
    mutable_array_copy.Append(4);
    Require(mutable_array.GetLength() == 3 && mutable_array_copy.GetLength() == 4,
            "MutableArraySequence: копия зависит от оригинала");
    CheckSelfAssignment(mutable_array_copy);

    MutableListSequence<int> mutable_list(values, 3);
    MutableListSequence<int> mutable_list_copy;
    mutable_list_copy = mutable_list;
    mutable_list_copy.Prepend(0);
    Require(mutable_list.GetFirst() == 1 && mutable_list_copy.GetFirst() == 0,
            "MutableListSequence: копия зависит от оригинала");
    CheckSelfAssignment(mutable_list_copy);

    ImmutableArraySequence<int> immutable_array_copy;
    {
        ImmutableArraySequence<int> source(values, 3);
        immutable_array_copy = source;
    }
    Require(immutable_array_copy.GetLast() == 3,
            "ImmutableArraySequence: присваивание не владеет копией");
    CheckSelfAssignment(immutable_array_copy);

    ImmutableListSequence<int> immutable_list_copy;
    {
        ImmutableListSequence<int> source(values, 3);
        immutable_list_copy = source;
    }
    Require(immutable_list_copy.GetLast() == 3,
            "ImmutableListSequence: присваивание не владеет копией");
    CheckSelfAssignment(immutable_list_copy);

    MutableArraySequence<int> inner(values, 3);
    DynamicArray<MutableArraySequence<int>> nested(1);
    nested.Set(0, inner);

    DynamicArray<MutableArraySequence<int>> nested_copy(1);
    nested_copy = nested;

    int replacement_values[] = {9};
    MutableArraySequence<int> replacement(replacement_values, 1);
    nested_copy.Set(0, replacement);
    Require(nested.Get(0).GetLength() == 3 && nested.Get(0).GetFirst() == 1,
            "Вложенный контейнер скопирован поверхностно");
}

void TestSelfReferences() {
    std::string values[] = {"alpha", "beta"};
    MutableArraySequence<std::string> sequence(values, 2);

    sequence.Append(sequence.Get(0));
    Require(sequence.GetLast() == "alpha", "Append повредил ссылку на собственный элемент");

    sequence.Prepend(sequence.Get(1));
    Require(sequence.GetFirst() == "beta", "Prepend повредил ссылку на собственный элемент");

    sequence.InsertAt(sequence.Get(3), 2);
    Require(sequence.Get(2) == "alpha", "InsertAt повредил ссылку на собственный элемент");

    MutableListSequence<std::string> list(values, 2);
    list.Append(list.Get(0));
    list.Prepend(list.Get(1));
    list.InsertAt(list.Get(3), list.GetLength());
    Require(list.GetLast() == "alpha", "LinkedList-вставка повредила собственный элемент");
}

void CheckOperations(Sequence<int> &sequence) {
    int original_length = sequence.GetLength();
    Sequence<int> *mapped = sequence.Map(DoubleValue);
    Require(mapped->GetLength() == 4 && mapped->Get(0) == 2 && mapped->Get(3) == 8,
            "Map вернул неверный результат");

    Sequence<int> *filtered = sequence.Where(IsEven);
    Require(filtered->GetLength() == 2 && filtered->Get(0) == 2 && filtered->Get(1) == 4,
            "Where вернул неверный результат");

    Require(sequence.Reduce(AddValue, 0) == 10, "Reduce вернул неверный результат");

    Sequence<int> *subsequence = sequence.GetSubsequence(1, 2);
    Require(subsequence->GetLength() == 2 && subsequence->Get(0) == 2 &&
                subsequence->Get(1) == 3,
            "GetSubsequence вернул неверный результат");

    int other_values[] = {5, 6};
    MutableListSequence<int> other(other_values, 2);
    Sequence<int> *concatenated = sequence.Concat(&other);
    Require(concatenated->GetLength() == 6 && concatenated->Get(4) == 5 &&
                concatenated->Get(5) == 6,
            "Concat вернул неверный результат");
    Require(sequence.GetLength() == original_length,
            "Операция построения результата изменила исходную последовательность");

    delete mapped;
    delete filtered;
    delete subsequence;
    delete concatenated;
}

void TestSequenceOperations() {
    int values[] = {1, 2, 3, 4};

    MutableArraySequence<int> mutable_array(values, 4);
    Sequence<int> *mutable_result = mutable_array.Append(5);
    Require(mutable_result == &mutable_array && mutable_array.GetLength() == 5,
            "MutableArraySequence перестала изменять себя");

    mutable_array = MutableArraySequence<int>(values, 4);
    CheckOperations(mutable_array);

    ImmutableArraySequence<int> immutable_array(values, 4);
    Sequence<int> *immutable_result = immutable_array.Append(5);
    Require(immutable_result != &immutable_array && immutable_array.GetLength() == 4 &&
                immutable_result->GetLength() == 5,
            "ImmutableArraySequence изменила исходный объект");
    delete immutable_result;
    CheckOperations(immutable_array);

    MutableListSequence<int> mutable_list(values, 4);
    mutable_result = mutable_list.InsertAt(5, mutable_list.GetLength());
    Require(mutable_result == &mutable_list && mutable_list.GetLast() == 5,
            "MutableListSequence не вставила элемент в конец");

    mutable_list = MutableListSequence<int>(values, 4);
    CheckOperations(mutable_list);

    ImmutableListSequence<int> immutable_list(values, 4);
    immutable_result = immutable_list.Prepend(0);
    Require(immutable_result != &immutable_list && immutable_list.GetFirst() == 1 &&
                immutable_result->GetFirst() == 0,
            "ImmutableListSequence изменила исходный объект");
    delete immutable_result;
    CheckOperations(immutable_list);

    RequireThrows<std::out_of_range>([&mutable_array]() { mutable_array.InsertAt(0, -1); },
                                     "ArraySequence: принят отрицательный индекс вставки");
    RequireThrows<std::out_of_range>([&mutable_list]() { mutable_list.InsertAt(0, 10); },
                                     "ListSequence: принята позиция за границей");
}
} // namespace

void RunTests() {
    TestDynamicArray();
    TestLinkedList();
    TestSequenceCopying();
    TestSelfReferences();
    TestSequenceOperations();
}

int main() {
    try {
        RunTests();
        RunPerformanceTests();
        std::cout << "Все тесты пройдены" << std::endl;
        return 0;
    } catch (const std::exception &exception) {
        std::cerr << "Ошибка теста: " << exception.what() << std::endl;
        return 1;
    }
}
