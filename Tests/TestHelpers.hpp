#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <iostream>
#include <exception>

#include "Include/KeyValuePair.hpp"

inline void check_dictionary(bool condition, const char* test_name, int& tests_passed, int& tests_failed) {
    if (condition) {
        std::cout << "  Успешно: " << test_name << "\n";
        tests_passed++;
    } else {
        std::cout << "  Провалено: " << test_name << "\n";
        tests_failed++;
    }
}

template <typename TException, typename TAction>
bool TestThrows(TAction action) {
    try {
        action();
    } catch (const TException&) {
        return true;
    }
    return false;
}

inline bool TestCompareEntries(const KeyValuePair<int, int>& first, const KeyValuePair<int, int>& second) {
    return first.key < second.key;
}

inline int TestCompareKey(const int& key, const KeyValuePair<int, int>& entry) {
    if (key < entry.key) {return -1;}
    if (entry.key < key) {return 1;}
    return 0;
}
inline bool TestEqualInt(const int& first, const int& second) {return first == second;}
inline unsigned int TestHashInt(const int& value) {return static_cast<unsigned int>(value);}
inline unsigned int TestCollisionHash(const int&) {return 0;}

#endif // TEST_HELPERS_HPP
