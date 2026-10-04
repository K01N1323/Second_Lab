#include "Menu.hpp"

#include <iostream>
#include <limits>
#include <exception>
#include <string>

#include "Include/AlphabeticalIndex.hpp"
#include "Tests/Tests.hpp"
#include "Tests/PerformanceTests.hpp"

// Чтение одного целого числа.
// false означает конец ввода или ошибку потока.
static bool ReadInt(const char* Prompt, int& Value) {
    while (true) {
        std::cout << Prompt << std::flush;

        int NewValue = 0;

        if (!(std::cin >> NewValue)) {
            if (std::cin.eof() || std::cin.bad()) {return false;}

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Ошибка: введите одно целое число в диапазоне int.\n";
            continue;
        }

        bool ExtraCharacters = false;
        char Symbol = '\0';

        while (std::cin.get(Symbol)) {
            if (Symbol == '\n') {break;}

            if (Symbol != ' ' && Symbol != '\t' && Symbol != '\r') {
                ExtraCharacters = true;
            }
        }

        if (std::cin.bad()) {return false;}

        if (ExtraCharacters) {
            std::cout << "Ошибка: в строке должно быть только одно целое число.\n";

            if (std::cin.eof()) {return false;}

            continue;
        }

        Value = NewValue;
        return true;
    }
}

static bool ReadChoice(const char* Prompt, int MinValue, int MaxValue, int& Choice) {
    while (ReadInt(Prompt, Choice)) {
        if (Choice >= MinValue && Choice <= MaxValue) {return true;}

        std::cout << "Выберите число от " << MinValue << " до " << MaxValue << ".\n";
    }

    return false;
}

static void ShowIndex(const AlphabeticalIndex& index) {
    auto entries = index.GetEntries();
    std::cout << "Слов в указателе: " << index.GetCount() << "\n";

    for (int Number = 0; Number < entries.GetLength(); Number++) {
        const auto& entry = entries.Get(Number);
        std::cout << entry.key << ": ";

        for (int Page = 0; Page < entry.value.GetLength(); Page++) {
            if (Page != 0) {std::cout << ", ";}
            std::cout << entry.value.Get(Page);
        }

        std::cout << "\n";
    }
}

// Указатель локальный: при выходе из демонстрации освобождается.
// false означает конец ввода.
static bool RunIndexDemo(DictionaryType Type) {
    AlphabeticalIndex index(Type);
    std::cout << "\nАлфавитный указатель: слово и номера страниц, на которых оно встречается.\n";
    std::cout << "Первая страница вмещает половину размера, каждая десятая — три четверти.\n";

    while (true) {
        std::cout << "\n--- Состояние указателя ---\n";
        ShowIndex(index);

        std::cout << "\n1. Построить или заменить указатель\n"
                  << "2. Показать состояние\n"
                  << "3. Найти страницы слова\n"
                  << "4. Очистить указатель\n"
                  << "0. Вернуться в главное меню\n";

        int Choice = 0;
        if (!ReadChoice("Ваш выбор: ", 0, 4, Choice)) {return false;}
        if (Choice == 0) {return true;}
        if (Choice == 2) {continue;}

        try {
            if (Choice == 1) {
                int Mode = 0;
                int PageSize = 0;
                if (!ReadChoice("Размер в словах (1) или символах (2): ", 1, 2, Mode)) {return false;}
                if (!ReadChoice("Размер полной страницы (не меньше 2): ", 2, std::numeric_limits<int>::max(), PageSize)) {return false;}

                std::string Text;
                std::cout << "Текст одной строкой (слова разделены пробелами): " << std::flush;
                if (!std::getline(std::cin, Text)) {return false;}

                index.Build(Text, PageSize, Mode == 1 ? PageSizeMode::Words : PageSizeMode::Characters);
                std::cout << "Указатель построен.\n";
            } else if (Choice == 3) {
                std::string Word;
                std::cout << "Слово: " << std::flush;
                if (!std::getline(std::cin, Word)) {return false;}

                const auto& pages = index.GetPages(Word);
                std::cout << Word << ": ";
                for (int Page = 0; Page < pages.GetLength(); Page++) {
                    if (Page != 0) {std::cout << ", ";}
                    std::cout << pages.Get(Page);
                }
                std::cout << "\n";
            } else if (Choice == 4) {
                index.Clear();
                std::cout << "Указатель очищен.\n";
            }
        } catch (const std::exception& Error) {
            std::cout << "\nОперация прервана: " << Error.what() << "\n";
        }
    }
}

static int RunSelectedTests(int Choice) {
    switch (Choice) {
        case 3: return run_tree_dictionary_tests();
        case 4: return run_hash_dictionary_tests();
        case 5: return run_alphabetical_index_tests();
        case 6: return run_container_tests();
    }
    return 1;
}

static void ShowTestResult(int Failed) {
    if (Failed == 0) {
        std::cout << "\nВсе выполненные проверки прошли.\n";
    } else {
        std::cout << "\nЗарегистрировано ошибок: " << Failed << ". Подробности выше.\n";
    }
}

int RunMenu() {
    bool HadFailures = false;
    bool ContinueInput = true;

    while (ContinueInput) {
        std::cout << "\n АЛФАВИТНЫЙ УКАЗАТЕЛЬ \n"
                  << "1. Демонстрация словаря на дереве\n"
                  << "2. Демонстрация хеш-словаря\n"
                  << "3. Тесты TreeDictionary\n"
                  << "4. Тесты HashDictionary и HashTable\n"
                  << "5. Тесты AlphabeticalIndex\n"
                  << "6. Тесты контейнеров и последовательностей\n"
                  << "7. Сравнение производительности\n"
                  << "8. Запустить все тесты корректности\n"
                  << "0. Выход\n";

        int Choice = 0;
        if (!ReadChoice("Ваш выбор: ", 0, 8, Choice)) {break;}
        if (Choice == 0) {break;}

        try {
            if (Choice == 1 || Choice == 2) {
                ContinueInput = RunIndexDemo(Choice == 1 ? DictionaryType::Tree : DictionaryType::Hash);
            } else if (Choice == 7) {
                if (run_performance_tests() != 0) {HadFailures = true;}
            } else {
                int Failed = 0;
                if (Choice == 8) {
                    int Completed = 0;
                    for (int TestNumber = 3; TestNumber <= 6; TestNumber++) {
                        Failed += RunSelectedTests(TestNumber);
                        Completed++;
                    }
                    std::cout << "\nЗавершено наборов: " << Completed << " из 4.\n";
                } else {
                    Failed = RunSelectedTests(Choice);
                }
                ShowTestResult(Failed);
                if (Failed != 0) {HadFailures = true;}
            }
        } catch (const std::exception& Error) {
            std::cout << "\nОперация прервана: " << Error.what() << "\n";
            HadFailures = true;
        } catch (...) {
            std::cout << "\nОперация прервана неизвестным исключением.\n";
            HadFailures = true;
        }
    }

    std::cout << "\nПрограмма завершена.\n";
    return HadFailures ? 1 : 0;
}
