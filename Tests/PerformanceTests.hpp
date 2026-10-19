#ifndef PERFORMANCE_TESTS_HPP
#define PERFORMANCE_TESTS_HPP

#include <stdexcept>
#include <chrono>
#include <iomanip>

#include "DictionaryTests.hpp"

#include "Sequences/MutableArraySequence.hpp"

class CopyCounter {
public:
    static long long copy_operations;

    int value;

    CopyCounter(int value = 0) {this->value = value;}

    CopyCounter(const CopyCounter& other) {
        this->value = other.value;
        copy_operations++;
    }

    CopyCounter& operator=(const CopyCounter& other) {
        this->value = other.value;
        copy_operations++;
        return *this;
    }
};

inline long long CopyCounter::copy_operations = 0;

inline void RunPerformanceTests() {
    const int count = 1024;
    MutableArraySequence<CopyCounter> sequence;

    CopyCounter::copy_operations = 0;

    for (int index = 0; index < count; index++) {
        CopyCounter value(index);
        sequence.Append(value);
    }

    if (sequence.GetLength() != count || sequence.GetLast().value != count - 1) {
        throw std::runtime_error("MutableArraySequence: неверный результат серии Append");
    }

    if (CopyCounter::copy_operations >= count * 6LL) {
        throw std::runtime_error("MutableArraySequence: число копирований растет квадратично");
    }
}

inline double PerformanceElapsed(std::chrono::steady_clock::time_point Start) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - Start).count();
}

template <typename TDictionary>
void MeasureDictionary(TDictionary& dictionary, const char* Name, int Count) {
    auto Start = std::chrono::steady_clock::now();
    for (int Key = 0; Key < Count; Key++) {dictionary.Add(Key, Key);}
    double AddTime = PerformanceElapsed(Start);

    long long Sum = 0;
    Start = std::chrono::steady_clock::now();
    for (int Key = 0; Key < Count; Key++) {Sum += dictionary.Get(Key);}
    double FindTime = PerformanceElapsed(Start);
    if (Sum != static_cast<long long>(Count) * (Count - 1) / 2) {
        throw std::runtime_error("Измерение поиска дало неверный результат");
    }

    Start = std::chrono::steady_clock::now();
    for (int Key = 0; Key < Count; Key++) {dictionary.Remove(Key);}
    double RemoveTime = PerformanceElapsed(Start);
    if (dictionary.GetCount() != 0) {throw std::runtime_error("Не все записи удалены");}

    std::cout << std::left << std::setw(18) << Name << std::right << std::setw(8) << Count
              << std::setw(16) << AddTime << std::setw(16) << FindTime << std::setw(16) << RemoveTime << "\n";
}

inline int run_performance_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nСравнение производительности\n";
    try {
        RunPerformanceTests();
        check_dictionary(true, "Серия Append имеет линейное число копирований", tests_passed, tests_failed);
        std::cout << "\nПоследовательные ключи, время в миллисекундах.\n";
        std::cout << "Дерево автоматически балансируется поворотами AVL.\n";
        std::cout << std::left << std::setw(18) << "Реализация" << std::right << std::setw(8) << "N"
                  << std::setw(16) << "Add, ms" << std::setw(16) << "Get, ms" << std::setw(16) << "Remove, ms" << "\n";
        for (int Count : {1024, 4096}) {
            TreeDictionary<int, int> tree(TestCompareEntries, TestCompareKey);
            HashDictionary<int, int> hash(TestHashInt, TestEqualInt);
            MeasureDictionary(tree, "TreeDictionary", Count);
            MeasureDictionary(hash, "HashDictionary", Count);
        }
    } catch (const std::exception& Error) {
        std::cout << "\nИзмерения прерваны: " << Error.what() << "\n";
        check_dictionary(false, "Измерения завершились без ошибок", tests_passed, tests_failed);
    } catch (...) {
        check_dictionary(false, "Неожиданное исключение", tests_passed, tests_failed);
    }
    std::cout << "\nПроизводительность: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed == 0 ? 0 : 1;
}

#endif // PERFORMANCE_TESTS_HPP
