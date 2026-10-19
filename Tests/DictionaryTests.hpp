#ifndef DICTIONARY_TESTS_HPP
#define DICTIONARY_TESTS_HPP

#include <memory>
#include <type_traits>
#include <utility>

#include "TestHelpers.hpp"
#include "Include/TreeDictionary.hpp"
#include "Include/HashDictionary.hpp"

// Использую один набор проверок для обеих реализаций интерфейса.
template <typename TDictionary>
void CheckDictionary(TDictionary& dictionary, int& tests_passed, int& tests_failed) {
    check_dictionary(dictionary.GetCount() == 0 && dictionary.Find(1) == nullptr,
                     "Новый словарь пуст", tests_passed, tests_failed);
    dictionary.Add(2, 20);
    dictionary.Add(1, 10);
    dictionary.Add(3, 30);
    check_dictionary(dictionary.GetCount() == 3 && dictionary.Get(2) == 20,
                     "Добавление и чтение по ключу", tests_passed, tests_failed);
    check_dictionary(TestThrows<std::invalid_argument>([&]() {dictionary.Add(2, 99);}) &&
                         dictionary.Get(2) == 20,
                     "Повторный ключ отклонен, значение сохранено", tests_passed, tests_failed);
    *dictionary.Find(2) = 25;
    const TDictionary& Constant = dictionary;
    check_dictionary(Constant.Find(2) && *Constant.Find(2) == 25,
                     "Find дает доступ к хранимому значению", tests_passed, tests_failed);
    check_dictionary(TestThrows<std::out_of_range>([&]() {dictionary.Get(9);}) &&
                         TestThrows<std::out_of_range>([&]() {dictionary.Remove(9);}),
                     "Несуществующий ключ вызывает исключение", tests_passed, tests_failed);
    {
        std::unique_ptr<IEnumerator<KeyValuePair<int, int>>> it(dictionary.GetEnumerator());
        int Count = 0;
        int Sum = 0;
        while (it->HasNext()) {
            Sum += it->GetCurrent().key;
            Count++;
            it->MoveNext();
        }
        check_dictionary(Count == 3 && Sum == 6, "Обход возвращает все записи", tests_passed, tests_failed);
        check_dictionary(TestThrows<std::out_of_range>([&]() {it->GetCurrent();}),
                         "Завершенный перечислитель нельзя читать", tests_passed, tests_failed);
    }
    {
        std::unique_ptr<IEnumerator<KeyValuePair<int, int>>> it(dictionary.GetEnumerator());
        dictionary.Add(4, 40);
        // Проверяю изменение версии в HashTable. Для дерева такой контракт не задаю.
        if constexpr (std::is_same<TDictionary, HashDictionary<int, int>>::value) {
            check_dictionary(TestThrows<std::logic_error>([&]() {it->HasNext();}),
                             "Изменение словаря обнаружено при обходе", tests_passed, tests_failed);
        }
    }
    TDictionary Copy(dictionary);
    Copy.Remove(2);
    check_dictionary(dictionary.ContainsKey(2) && !Copy.ContainsKey(2),
                     "Копия владеет независимыми записями", tests_passed, tests_failed);
    Copy = dictionary;
    *Copy.Find(1) = 99;
    check_dictionary(dictionary.Get(1) == 10, "Присваивание создает глубокую копию", tests_passed, tests_failed);
    TDictionary Moved(std::move(Copy));
    Copy.Add(7, 70);
    check_dictionary(Moved.Get(1) == 99 && Copy.GetCount() == 1,
                     "Перемещение и повторное использование источника", tests_passed, tests_failed);
    Copy = std::move(Moved);
    Moved.Add(8, 80);
    check_dictionary(Copy.Get(1) == 99 && Moved.Get(8) == 80,
                     "Перемещающее присваивание", tests_passed, tests_failed);
    dictionary.Remove(2);
    dictionary.Remove(1);
    dictionary.Remove(4);
    dictionary.Remove(3);
    check_dictionary(dictionary.GetCount() == 0, "Удаление всех записей", tests_passed, tests_failed);
    dictionary.Add(5, 50);
    dictionary.Clear();
    dictionary.Clear();
    check_dictionary(dictionary.GetCount() == 0 && !dictionary.ContainsKey(5),
                     "Повторная очистка безопасна", tests_passed, tests_failed);
}

inline int run_tree_dictionary_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nТестирование TreeDictionary\n";
    try {
        TreeDictionary<int, int> dictionary(TestCompareEntries, TestCompareKey);
        CheckDictionary(dictionary, tests_passed, tests_failed);
        dictionary.Add(3, 30);
        dictionary.Add(1, 10);
        dictionary.Add(2, 20);
        std::unique_ptr<IEnumerator<KeyValuePair<int, int>>> it(dictionary.GetEnumerator());
        bool Sorted = true;
        for (int Key = 1; Key <= 3; Key++) {
            if (!it->HasNext() || it->GetCurrent().key != Key) {Sorted = false; break;}
            it->MoveNext();
        }
        check_dictionary(Sorted && !it->HasNext(), "Обход дерева отсортирован по ключу", tests_passed, tests_failed);
        check_dictionary(TestThrows<std::invalid_argument>([]() {TreeDictionary<int, int> Invalid(nullptr, TestCompareKey);}) &&
                             TestThrows<std::invalid_argument>([]() {TreeDictionary<int, int> Invalid(TestCompareEntries, nullptr);}),
                         "Пустой компаратор отклонен", tests_passed, tests_failed);
    } catch (...) {
        check_dictionary(false, "Неожиданное исключение: оставшиеся сценарии не выполнены", tests_passed, tests_failed);
    }
    std::cout << "\nTreeDictionary: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

inline int run_hash_dictionary_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nТестирование HashDictionary и HashTable\n";
    try {
        HashDictionary<int, int> dictionary(TestHashInt, TestEqualInt);
        CheckDictionary(dictionary, tests_passed, tests_failed);
        HashTable<int, int> table(TestCollisionHash, TestEqualInt, 2);
        for (int Key = 0; Key < 128; Key++) {table.Add(Key, Key * 10);}
        bool Correct = table.GetCapacity() > 2 && table.GetCount() == 128;
        for (int Key = 0; Key < 128; Key++) {if (table.Get(Key) != Key * 10) { Correct = false; }}
        check_dictionary(Correct, "Коллизии и рост сохраняют все значения", tests_passed, tests_failed);
        int OldCapacity = table.GetCapacity();
        for (int Key = 0; Key < 127; Key++) {table.Remove(Key);}
        check_dictionary(table.GetCount() == 1 && table.Get(127) == 1270 && table.GetCapacity() < OldCapacity,
                         "Удаление из цепочки и уменьшение вместимости", tests_passed, tests_failed);
        check_dictionary(TestThrows<std::invalid_argument>([]() {HashTable<int, int> Invalid(nullptr, TestEqualInt);}) &&
                             TestThrows<std::invalid_argument>([]() {HashTable<int, int> Invalid(TestHashInt, nullptr);}) &&
                             TestThrows<std::invalid_argument>([]() {HashTable<int, int> Invalid(TestHashInt, TestEqualInt, 1);}),
                         "Некорректные параметры таблицы отклонены", tests_passed, tests_failed);
    } catch (...) {
        check_dictionary(false, "Неожиданное исключение: оставшиеся сценарии не выполнены", tests_passed, tests_failed);
    }
    std::cout << "\nHashDictionary: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

#endif // DICTIONARY_TESTS_HPP
