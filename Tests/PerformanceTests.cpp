#include <stdexcept>

#include "Sequences/MutableArraySequence.h"
#include "Tests/PerformanceTests.h"

namespace {
class CopyCounter {
public:
    static long long copy_operations;

    int value;

    CopyCounter(int value = 0) { this->value = value; }

    CopyCounter(const CopyCounter &other) {
        this->value = other.value;
        copy_operations++;
    }

    CopyCounter &operator=(const CopyCounter &other) {
        this->value = other.value;
        copy_operations++;
        return *this;
    }
};

long long CopyCounter::copy_operations = 0;
} // namespace

void RunPerformanceTests() {
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
