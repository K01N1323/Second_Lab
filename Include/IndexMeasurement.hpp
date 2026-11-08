#ifndef INDEX_MEASUREMENT_HPP
#define INDEX_MEASUREMENT_HPP

#include <chrono>

#include "AlphabeticalIndex.hpp"

struct IndexMeasurement {
    double BuildTime;
    double EntriesTime;
    int Count;

    double GetTotal() const {return BuildTime + EntriesTime;}
};

inline IndexMeasurement MeasureIndex(AlphabeticalIndex& index, const std::string& Text, int PageSize, PageSizeMode Mode, int OutputCount = 1) {
    if (OutputCount <= 0) {throw std::invalid_argument("Число выдач должно быть положительным");}
    auto Start = std::chrono::steady_clock::now();
    index.Build(Text, PageSize, Mode);
    auto Built = std::chrono::steady_clock::now();
    double EntriesTime = 0;
    int Count = 0;
    for (int Number = 0; Number < OutputCount; Number++) {
        auto BeforeEntries = std::chrono::steady_clock::now();
        auto entries = index.GetEntries();
        auto Finished = std::chrono::steady_clock::now();
        EntriesTime += std::chrono::duration<double, std::milli>(Finished - BeforeEntries).count();
        Count = entries.GetLength();
    }

    return {std::chrono::duration<double, std::milli>(Built - Start).count(),
            EntriesTime, Count};
}

// Выбираю целиком средний по общему времени запуск, сохраняя сумму его этапов.
inline IndexMeasurement MedianMeasurement(IndexMeasurement* Measurements, int Count) {
    if (Count <= 0) {throw std::invalid_argument("Нет результатов измерения");}
    if (Measurements == nullptr) {throw std::invalid_argument("Результаты измерения не заданы");}

    for (int Index = 1; Index < Count; Index++) {
        IndexMeasurement Current = Measurements[Index];
        int Position = Index;

        while (Position > 0 && Current.GetTotal() < Measurements[Position - 1].GetTotal()) {
            Measurements[Position] = Measurements[Position - 1];
            Position--;
        }

        Measurements[Position] = Current;
    }

    return Measurements[Count / 2];
}

#endif // INDEX_MEASUREMENT_HPP
