#ifndef TESTS_HPP
#define TESTS_HPP

#include "ContainerTests.hpp"
#include "TreeTests.hpp"
#include "DictionaryTests.hpp"
#include "AlphabeticalIndexTests.hpp"
#include "IndexComparisonTests.hpp"

inline int RunTests() {
    int Failed = 0;
    Failed += run_tree_dictionary_tests();
    Failed += run_hash_dictionary_tests();
    Failed += run_alphabetical_index_tests();
    Failed += run_index_comparison_tests();
    Failed += run_container_tests();
    Failed += run_tree_tests();
    std::cout << "\nВсего проваленных проверок: " << Failed << "\n";
    return Failed;
}

#endif // TESTS_HPP
