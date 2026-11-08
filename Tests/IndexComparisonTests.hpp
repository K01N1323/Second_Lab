#ifndef INDEX_COMPARISON_TESTS_HPP
#define INDEX_COMPARISON_TESTS_HPP

#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

#include "TestHelpers.hpp"
#include "Include/IndexMeasurement.hpp"

inline std::string IndexTestWord(int Number) {
    std::string word = "wordaaaa";
    for (int Index = 7; Index >= 4; Index--) {
        word[Index] = static_cast<char>('a' + Number % 26);
        Number /= 26;
    }
    return word;
}

inline std::string IndexCollisionWord(int Number) {
    std::string word;
    for (int Index = 0; Index < 12; Index++) {
        word += Number % 2 == 0 ? "Aa" : "BB";
        Number /= 2;
    }
    return word;
}

inline unsigned int IndexTestHash(const std::string& word) {
    unsigned int HashCode = 0;
    for (unsigned char Symbol : word) {HashCode = HashCode * 31u + Symbol;}
    return HashCode;
}

// Order: 0 — прямой, 1 — обратный, 2 — перемешанный, 3 — коллизии.
inline std::string MakeIndexText(int Count, int Unique, int Order, bool LongWords = false) {
    MutableArraySequence<std::string> words;
    for (int Number = 0; Number < Count; Number++) {
        int Key = Number % Unique;
        if (Order == 1) {Key = Unique - 1 - Key;}
        std::string word = Order == 3 ? IndexCollisionWord(Key) : IndexTestWord(Key);
        if (LongWords) {word += std::string(120, 'x');}
        words.Append(word);
    }

    DynamicArray<int> positions(Count);
    for (int Index = 0; Index < Count; Index++) {positions.Set(Index, Index);}
    if (Order == 2) {
        unsigned int State = 1323;
        for (int Index = Count - 1; Index > 0; Index--) {
            State = State * 1664525u + 1013904223u;
            int OtherIndex = static_cast<int>(State % static_cast<unsigned int>(Index + 1));
            int Position = positions.Get(Index);
            positions.Set(Index, positions.Get(OtherIndex));
            positions.Set(OtherIndex, Position);
        }
    }

    std::string Text;
    for (int Index = 0; Index < Count; Index++) {
        if (Index != 0) {Text += ' ';}
        Text += words.Get(positions.Get(Index));
    }
    return Text;
}

inline bool EqualIndexEntries(const AlphabeticalIndex& first, const AlphabeticalIndex& second) {
    auto FirstEntries = first.GetEntries();
    auto SecondEntries = second.GetEntries();
    if (FirstEntries.GetLength() != SecondEntries.GetLength()) {return false;}

    std::unique_ptr<IEnumerator<AlphabeticalIndex::Entry>> First(FirstEntries.GetEnumerator());
    std::unique_ptr<IEnumerator<AlphabeticalIndex::Entry>> Second(SecondEntries.GetEnumerator());
    std::string PreviousWord;
    bool HasPrevious = false;
    while (First->HasNext() && Second->HasNext()) {
        const auto& Left = First->GetCurrent();
        const auto& Right = Second->GetCurrent();
        if (Left.key != Right.key || Left.value.GetLength() != Right.value.GetLength() ||
            (HasPrevious && !(PreviousWord < Left.key))) {return false;}
        int PreviousPage = 0;
        for (int Index = 0; Index < Left.value.GetLength(); Index++) {
            int Page = Left.value.Get(Index);
            if (Page != Right.value.Get(Index) || Page <= PreviousPage) {return false;}
            PreviousPage = Page;
        }
        PreviousWord = Left.key;
        HasPrevious = true;
        First->MoveNext();
        Second->MoveNext();
    }
    return !First->HasNext() && !Second->HasNext();
}

inline int run_index_comparison_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nСравнение результатов двух указателей и быстрая сортировка\n";
    try {
        for (int Count : {0, 1, 2, 100, 4096}) {
            for (int Order = 0; Order < 4; Order++) {
                std::string Text = MakeIndexText(Count, Count == 0 ? 1 : Count, Order);
                for (PageSizeMode Mode : {PageSizeMode::Words, PageSizeMode::Characters}) {
                    AlphabeticalIndex tree(DictionaryType::Tree);
                    AlphabeticalIndex hash(DictionaryType::Hash);
                    tree.Build(Text, 100, Mode);
                    hash.Build(Text, 100, Mode);
                    check_dictionary(EqualIndexEntries(tree, hash) && tree.GetCount() == Count,
                                     ("Порядок " + std::to_string(Order) + ", N=" + std::to_string(Count) +
                                     (Mode == PageSizeMode::Words ? ", слова" : ", байты")).c_str(), tests_passed, tests_failed);
                }
            }
        }
        AlphabeticalIndex tree(DictionaryType::Tree);
        AlphabeticalIndex hash(DictionaryType::Hash);
        std::string Text = MakeIndexText(20000, 16, 2);
        tree.Build(Text, 2, PageSizeMode::Words);
        hash.Build(Text, 2, PageSizeMode::Words);
        check_dictionary(EqualIndexEntries(tree, hash) && tree.GetCount() == 16,
                         "Много повторов и страниц: совпадают все списки", tests_passed, tests_failed);
        hash.Build("a a a a", 4, PageSizeMode::Words);
        auto entries = hash.GetEntries();
        check_dictionary(entries.GetLength() == 1 && entries.GetFirst().key == "a" &&
                         entries.GetFirst().value.GetLength() == 2 && entries.GetFirst().value.Get(1) == 2,
                         "Быстрая сортировка единственного ключа с повторами", tests_passed, tests_failed);
        bool Collisions = true;
        for (int Number = 1; Number < 4096; Number++) {
            if (IndexTestHash(IndexCollisionWord(0)) != IndexTestHash(IndexCollisionWord(Number))) {Collisions = false;}
        }
        check_dictionary(Collisions, "Все 4096 разных слов действительно имеют один хеш", tests_passed, tests_failed);
        IndexMeasurement measurements[] = {{2, 3, 1}, {1, 1, 1}, {3, 1, 1}};
        auto Median = MedianMeasurement(measurements, 3);
        check_dictionary(Median.BuildTime == 3 && Median.EntriesTime == 1 && Median.GetTotal() == 4,
                         "Медиана сохраняет этапы одного запуска", tests_passed, tests_failed);
        check_dictionary(TestThrows<std::invalid_argument>([]() {MedianMeasurement(nullptr, 1);}) &&
                             TestThrows<std::invalid_argument>([&]() {MedianMeasurement(measurements, 0);}),
                         "Медиана отклоняет отсутствующие измерения", tests_passed, tests_failed);
    } catch (const std::exception& Error) {
        check_dictionary(false, Error.what(), tests_passed, tests_failed);
    }
    std::cout << "\nСравнение указателей: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

inline void MeasureIndexCase(std::ostream& Csv, const char* Name, const std::string& Text,
                             int Count, int PageSize, PageSizeMode Mode, int OutputCount = 1) {
    IndexMeasurement Measurements[2][5];
    for (int Run = -1; Run < 5; Run++) {
        for (int Order = 0; Order < 2; Order++) {
            int Number = (Order + (Run + 1) % 2) % 2;
            AlphabeticalIndex index(Number == 0 ? DictionaryType::Tree : DictionaryType::Hash);
            auto Measured = MeasureIndex(index, Text, PageSize, Mode, OutputCount);
            if (Run >= 0) {Measurements[Number][Run] = Measured;}
        }
    }
    AlphabeticalIndex tree(DictionaryType::Tree);
    AlphabeticalIndex hash(DictionaryType::Hash);
    tree.Build(Text, PageSize, Mode);
    hash.Build(Text, PageSize, Mode);
    if (!EqualIndexEntries(tree, hash)) {throw std::runtime_error("Результаты словарей различаются");}

    for (int Number = 0; Number < 2; Number++) {
        auto Measured = MedianMeasurement(Measurements[Number], 5);
        const char* Type = Number == 0 ? "tree" : "hash";
        Csv << Name << ',' << Count << ',' << Measured.Count << ',' << PageSize << ','
            << (Mode == PageSizeMode::Words ? "words" : "bytes") << ',' << OutputCount << ',' << Type << ','
            << Measured.BuildTime << ',' << Measured.EntriesTime << ',' << Measured.GetTotal() << '\n';
        std::cout << std::left << std::setw(15) << Name << std::setw(7) << Type << std::right
                  << std::setw(10) << Count << std::setw(9) << Measured.Count
                  << std::setw(13) << Measured.BuildTime << std::setw(13) << Measured.EntriesTime
                  << std::setw(13) << Measured.GetTotal() << '\n';
    }
}

inline int run_index_performance_tests() {
    try {
        std::ofstream Csv("index_performance.csv");
        if (!Csv) {throw std::runtime_error("Не удалось открыть index_performance.csv");}
        Csv << "scenario,n,unique,page_size,mode,outputs,dictionary,build_ms,entries_ms,total_ms\n";
        Csv << std::fixed << std::setprecision(6);
        std::cout << "\nГипотеза до замеров: Tests/IndexHypothesis.md\n"
                  << "5 запусков после прогрева, медиана по Build + GetEntries, миллисекунды.\n"
                  << "scenario       type            N        U        Build   GetEntries        Total\n"
                  << std::fixed << std::setprecision(3);

        std::ifstream File("index_text.txt");
        if (!File) {throw std::runtime_error("Не найден index_text.txt. Запускаю из корня проекта.");}
        std::string Text;
        std::string word;
        int Loaded = 0;
        for (int Count : {100, 1000, 10000, 100000, 1000000}) {
            while (Loaded < Count && File >> word) {
                if (Loaded != 0) {Text += ' ';}
                Text += word;
                Loaded++;
            }
            if (Loaded != Count) {throw std::runtime_error("В index_text.txt меньше миллиона слов");}
            MeasureIndexCase(Csv, "file", Text, Count, 1000, PageSizeMode::Characters);
        }

        for (int Count : {100, 1000, 10000, 100000}) {
            for (int Order = 0; Order < 3; Order++) {
                const char* Names[] = {"sorted", "reverse", "shuffled"};
                MeasureIndexCase(Csv, Names[Order], MakeIndexText(Count, Count, Order), Count, 1000, PageSizeMode::Characters);
            }
        }
        for (int Count : {10000, 100000, 1000000}) {
            for (int Unique : {16, 128}) {
                MeasureIndexCase(Csv, Unique == 16 ? "repeat16" : "repeat128",
                                 MakeIndexText(Count, Unique, 2), Count, 1000, PageSizeMode::Characters);
            }
        }
        for (int Count : {100, 1000, 4096}) {
            MeasureIndexCase(Csv, "collisions", MakeIndexText(Count, Count, 3), Count, 1000, PageSizeMode::Characters);
        }
        Text = MakeIndexText(10000, 10000, 2, true);
        MeasureIndexCase(Csv, "long_words", Text, 10000, 1000, PageSizeMode::Characters);
        Text = MakeIndexText(100000, 100000, 2);
        MeasureIndexCase(Csv, "outputs10", Text, 100000, 1000, PageSizeMode::Characters, 10);
        Text = MakeIndexText(100000, 128, 2);
        for (int PageSize : {2, 100, 1000}) {
            MeasureIndexCase(Csv, "word_pages", Text, 100000, PageSize, PageSizeMode::Words);
        }
        Csv.flush();
        if (!Csv) {throw std::runtime_error("Не удалось сохранить результаты");}
        std::cout << "\nРезультаты совпали во всех сценариях. CSV: index_performance.csv\n";
        return 0;
    } catch (const std::exception& Error) {
        std::cerr << "Измерения прерваны: " << Error.what() << '\n';
        return 1;
    }
}

#endif // INDEX_COMPARISON_TESTS_HPP
