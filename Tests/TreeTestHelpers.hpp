#ifndef TREE_TEST_HELPERS_HPP
#define TREE_TEST_HELPERS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

#include "Include/BinarySearchTree.hpp"

inline void check_tree(bool condition, const char* test_name, int& tests_passed, int& tests_failed) {
    if (condition) {
        std::cout << "  Успешно: " << test_name << "\n";
        tests_passed++;
    } else {
        std::cout << "  Провалено: " << test_name << "\n";
        tests_failed++;
    }
}

template <typename TException, typename TAction>
bool TreeTestThrows(TAction action) {
    try {
        action();
    } catch (const TException&) {
        return true;
    }
    return false;
}

inline bool TestTreeCompareInt(const int& first, const int& second) {return first < second;}
inline bool TestTreeCompareDouble(const double& first, const double& second) {return first < second;}
inline bool TestTreeCompareString(const std::string& first, const std::string& second) {return first < second;}
inline bool TestTreeCompareReverse(const int& first, const int& second) {return first > second;}
inline int TestTreeNegate(const int& value) {return -value;}
inline int TestTreeConstant(const int&) {return 0;}
inline bool TestTreeEven(const int& value) {return value % 2 == 0;}
inline bool TestTreeAll(const int&) {return true;}
inline bool TestTreeNone(const int&) {return false;}
inline long long TestTreeSum(const long long& sum, const int& value) {return sum + value;}
inline std::string TestTreeJoin(const std::string& text, const int& value) {return text + std::to_string(value) + ' ';}
inline int TestTreeThrowMap(const int& value) {
    if (value == 2) {throw std::runtime_error("Ошибка функции");}
    return value;
}
inline bool TestTreeThrowWhere(const int& value) {
    if (value == 2) {throw std::runtime_error("Ошибка функции");}
    return true;
}
inline long long TestTreeThrowReduce(const long long& sum, const int& value) {
    if (value == 2) {throw std::runtime_error("Ошибка функции");}
    return sum + value;
}

inline BinarySearchTree<int> MakeTestTree() {
    BinarySearchTree<int> tree(TestTreeCompareInt);
    int values[] = {4, 2, 6, 1, 3, 5, 7};
    for (int value : values) {tree.Insert(value);}
    return tree;
}

// Проверяю высоту по фиксированной записи без рекурсивного обхода.
inline int TestTreeHeight(const std::string& text) {
    int depth = 0;
    int height = 0;
    for (std::size_t index = 0; index < text.size(); index++) {
        char symbol = text[index];
        if (symbol == '{') {
            height = height < depth + 1 ? depth + 1 : height;
            std::size_t length = 0;
            index++;
            while (text[index] != ':') {length = length * 10 + static_cast<unsigned int>(text[index++] - '0');}
            index += length + 1;
        } else if (symbol == '(' || symbol == '[') {depth++;}
        else if (symbol == ')' || symbol == ']') {depth--;}
    }
    return height;
}

// Высоты проверяю по форме дерева, независимо от поля height.
inline int ReadTestTreeHeight(std::istream& stream, bool& balanced, int& count) {
    char symbol = '\0';
    stream.get(symbol);
    if (symbol == '#') {return 0;}

    std::size_t length = 0;
    stream >> length;
    stream.get(symbol);
    stream.ignore(static_cast<std::streamsize>(length));
    stream.get(symbol);
    stream.get(symbol);
    int LeftHeight = ReadTestTreeHeight(stream, balanced, count);
    stream.get(symbol);
    stream.get(symbol);
    int RightHeight = ReadTestTreeHeight(stream, balanced, count);
    stream.get(symbol);

    if (LeftHeight - RightHeight > 1 || RightHeight - LeftHeight > 1) {balanced = false;}
    count++;
    return (LeftHeight > RightHeight ? LeftHeight : RightHeight) + 1;
}

template <typename T>
bool CheckTreeAVL(const BinarySearchTree<T>& tree) {
    std::istringstream stream(tree.ToString());
    bool balanced = true;
    int count = 0;
    ReadTestTreeHeight(stream, balanced, count);
    return balanced && count == tree.GetCount() && stream.peek() == std::char_traits<char>::eof();
}

#endif // TREE_TEST_HELPERS_HPP
