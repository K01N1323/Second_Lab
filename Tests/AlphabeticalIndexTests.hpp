#ifndef ALPHABETICAL_INDEX_TESTS_HPP
#define ALPHABETICAL_INDEX_TESTS_HPP

#include <utility>

#include "TestHelpers.hpp"
#include "Include/AlphabeticalIndex.hpp"

inline void CheckAlphabeticalIndex(DictionaryType Type, int& tests_passed, int& tests_failed) {
    AlphabeticalIndex index(Type);
    check_dictionary(index.GetCount() == 0, "Новый указатель пуст", tests_passed, tests_failed);
    index.Build("beta alpha beta alpha gamma", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 3 && index.GetPages("alpha").GetLength() == 2 &&
                         index.GetPages("alpha").Get(0) == 1 && index.GetPages("alpha").Get(1) == 2,
                     "Слова распределены по страницам", tests_passed, tests_failed);
    check_dictionary(index.GetPages("beta").GetLength() == 2 && index.GetPages("gamma").GetFirst() == 2,
                     "Первая страница имеет половинный размер", tests_passed, tests_failed);
    auto entries = index.GetEntries();
    check_dictionary(entries.GetLength() == 3 && entries.Get(0).key == "alpha" &&
                         entries.Get(1).key == "beta" && entries.Get(2).key == "gamma",
                     "Записи отсортированы по алфавиту", tests_passed, tests_failed);
    index.Build("a a a a a", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 1 && index.GetPages("a").GetLength() == 2,
                     "Повторное слово на странице не дублирует номер", tests_passed, tests_failed);
    index.Build("a b c d e f g h i j k l m n o p q r s t", 2, PageSizeMode::Words);
    check_dictionary(index.GetPages("r").GetFirst() == 10 && index.GetPages("s").GetFirst() == 11,
                     "Каждая десятая страница имеет три четверти размера", tests_passed, tests_failed);
    index.Build("aa b cc", 6, PageSizeMode::Characters);
    check_dictionary(index.GetPages("aa").GetFirst() == 1 && index.GetPages("b").GetFirst() == 2 &&
                         index.GetPages("cc").GetFirst() == 2,
                     "Символьный режим учитывает пробелы", tests_passed, tests_failed);
    check_dictionary(TestThrows<std::invalid_argument>([&]() {index.Build("toolong", 6);}) &&
                         index.GetCount() == 3 && index.GetPages("aa").GetFirst() == 1,
                     "Ошибка построения сохраняет прежний указатель", tests_passed, tests_failed);
    check_dictionary(TestThrows<std::invalid_argument>([&]() {index.Build("a", 0);}) &&
                         TestThrows<std::invalid_argument>([&]() {index.Build("a", 1);}) &&
                         TestThrows<std::invalid_argument>([&]() {index.Build("a", 4, static_cast<PageSizeMode>(99));}),
                     "Некорректный размер и режим отклонены", tests_passed, tests_failed);
    check_dictionary(TestThrows<std::out_of_range>([&]() {index.GetPages("missing");}),
                     "Отсутствующее слово вызывает исключение", tests_passed, tests_failed);
    index.Build("  alpha   beta  ", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 2, "Лишние пробелы пропускаются", tests_passed, tests_failed);
    auto Snapshot = index.GetEntries();
    AlphabeticalIndex Moved(std::move(index));
    index.Build("new", 4, PageSizeMode::Words);
    check_dictionary(Moved.GetCount() == 2 && index.GetCount() == 1,
                     "Перемещение и повторное построение источника", tests_passed, tests_failed);
    index = std::move(Moved);
    Moved.Build("next", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 2 && Moved.GetCount() == 1,
                     "Перемещающее присваивание указателя", tests_passed, tests_failed);
    index.Clear();
    index.Clear();
    check_dictionary(index.GetCount() == 0 && Snapshot.GetLength() == 2 && Snapshot.GetFirst().value.GetFirst() == 1,
                     "Очистка не изменяет независимый список записей", tests_passed, tests_failed);
    index.Build("   ", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 0 && index.GetEntries().GetLength() == 0,
                     "Текст из пробелов дает пустой указатель", tests_passed, tests_failed);
    index.Build("", 4, PageSizeMode::Words);
    check_dictionary(index.GetCount() == 0, "Пустой текст дает пустой указатель", tests_passed, tests_failed);
}

inline int run_alphabetical_index_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nТестирование AlphabeticalIndex\n";
    for (DictionaryType Type : {DictionaryType::Tree, DictionaryType::Hash}) {
        std::cout << (Type == DictionaryType::Tree ? "\nСловарь на дереве\n" : "\nХеш-словарь\n");
        try {
            CheckAlphabeticalIndex(Type, tests_passed, tests_failed);
        } catch (...) {
            check_dictionary(false, "Неожиданное исключение: оставшиеся сценарии не выполнены", tests_passed, tests_failed);
        }
    }
    check_dictionary(TestThrows<std::invalid_argument>([]() {AlphabeticalIndex Invalid(static_cast<DictionaryType>(99));}),
                     "Неизвестный тип словаря отклонен", tests_passed, tests_failed);
    std::cout << "\nAlphabeticalIndex: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

#endif // ALPHABETICAL_INDEX_TESTS_HPP
