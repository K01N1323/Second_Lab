#ifndef TREE_TESTS_HPP
#define TREE_TESTS_HPP

#include <memory>
#include <utility>

#include "TreeTestHelpers.hpp"

inline void TestTreeBasic(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree(TestTreeCompareInt);
    check_tree(tree.IsEmpty() && tree.GetCount() == 0 && tree.Find(1) == nullptr,
               "Новое дерево пусто", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::invalid_argument>([]() {BinarySearchTree<int> invalid(nullptr);}),
               "Пустой компаратор отклонён", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::out_of_range>([&]() {tree.Remove(1);}) &&
                   TreeTestThrows<std::out_of_range>([&]() {tree.Get(0);}),
               "Пустое дерево: удаление и индекс отклонены", tests_passed, tests_failed);
    tree.Insert(4);
    check_tree(tree.GetCount() == 1 && tree.Contains(4) && *tree.Find(4) == 4,
               "Одно значение вставлено и найдено", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::invalid_argument>([&]() {tree.Insert(4);}) && tree.GetCount() == 1,
               "Повторная вставка сохраняет дерево", tests_passed, tests_failed);
    tree.Remove(4);
    check_tree(tree.IsEmpty(), "Удаление единственного корня", tests_passed, tests_failed);

    tree = MakeTestTree();
    tree.Remove(1);
    check_tree(!tree.Contains(1) && tree.Contains(2) && tree.GetCount() == 6,
               "Удаление листа", tests_passed, tests_failed);
    tree.Remove(2);
    check_tree(tree.Contains(3) && tree.Get(0) == 3 && tree.GetCount() == 5,
               "Удаление узла с одним ребёнком", tests_passed, tests_failed);
    tree.Remove(6);
    check_tree(tree.Contains(5) && tree.Contains(7) && tree.GetCount() == 4,
               "Удаление узла с двумя детьми", tests_passed, tests_failed);
    tree.Remove(4);
    check_tree(!tree.Contains(4) && tree.GetCount() == 3 && tree.Get(0) == 3 && tree.Get(2) == 7,
               "Удаление корня с двумя детьми", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::out_of_range>([&]() {tree.Remove(99);}) && !tree.Contains(99),
               "Отсутствующее значение отклонено", tests_passed, tests_failed);
    tree.Clear();
    tree.Clear();
    check_tree(tree.IsEmpty(), "Повторная очистка", tests_passed, tests_failed);
    tree.Insert(1);
    tree.Insert(2);
    tree.Remove(1);
    check_tree(tree.Get(0) == 2 && tree.GetCount() == 1, "Удаление корня с одним ребёнком", tests_passed, tests_failed);

    tree.Clear();
    int values[] = {8, 4, 12, 10, 11, 14};
    for (int value : values) {tree.Insert(value);}
    tree.Remove(8);
    bool correct = tree.GetCount() == 5;
    int remaining[] = {4, 10, 11, 12, 14};
    for (int index = 0; index < 5; index++) {correct = correct && tree.Get(index) == remaining[index];}
    check_tree(correct, "Преемник корня с правым ребёнком: parent сохранены", tests_passed, tests_failed);
}

inline void TestTreeCopying(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree = MakeTestTree();
    BinarySearchTree<int> copy(tree);
    check_tree(copy == tree, "Копирование сохраняет форму", tests_passed, tests_failed);
    copy.Remove(2);
    check_tree(tree.Contains(2) && copy != tree, "Копия независима", tests_passed, tests_failed);
    copy = tree;
    BinarySearchTree<int>* same_tree = &copy;
    copy = *same_tree;
    check_tree(copy == tree, "Присваивание и самоприсваивание", tests_passed, tests_failed);
    BinarySearchTree<int> moved(std::move(copy));
    copy.Insert(20);
    check_tree(moved == tree && copy.GetCount() == 1, "Перемещение и использование источника", tests_passed, tests_failed);
    copy = std::move(moved);
    moved.Insert(30);
    copy = std::move(*same_tree);
    check_tree(copy == tree && moved.GetCount() == 1, "Перемещающее присваивание и самоперемещение", tests_passed, tests_failed);

    std::unique_ptr<IEnumerator<int>> it(tree.GetEnumerator());
    bool correct = true;
    for (int value = 1; value <= 7; value++) {
        if (!it->HasNext() || it->GetCurrent() != value) {correct = false; break;}
        it->MoveNext();
    }
    it->MoveNext();
    check_tree(correct && !it->HasNext(), "Перечислитель: ЛКП и конец обхода", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::out_of_range>([&]() {it->GetCurrent();}),
               "Завершённый перечислитель нельзя читать", tests_passed, tests_failed);
    int value = -1;
    check_tree(!it->Next() && !it->TryGetCurrentItem(value) && value == -1,
               "Адаптеры IEnumerator после завершения", tests_passed, tests_failed);
    tree.Clear();
    it.reset(tree.GetEnumerator());
    check_tree(!it->HasNext(), "Перечислитель пустого дерева", tests_passed, tests_failed);
    tree.Insert(5);
    it.reset(tree.GetEnumerator());
    check_tree(it->TryGetCurrentItem(value) && value == 5 && it->GetCurrentItem() == 5 && !it->Next(),
               "Адаптеры IEnumerator на одном элементе", tests_passed, tests_failed);
}

inline void TestTreeFunctions(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree = MakeTestTree();
    BinarySearchTree<int> original(tree);
    BinarySearchTree<int> mapped = tree.Map(TestTreeNegate);
    bool correct = mapped.GetCount() == 7;
    for (int index = 0; index < 7; index++) {correct = correct && mapped.Get(index) == index - 7;}
    check_tree(correct && TestTreeHeight(mapped.ToString()) == 3,
               "Map восстанавливает порядок и не строит цепочку", tests_passed, tests_failed);
    mapped.Remove(-1);
    check_tree(tree == original, "Map возвращает независимый результат", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::invalid_argument>([&]() {tree.Map(TestTreeConstant);}) && tree == original,
               "Map отклоняет эквивалентные результаты", tests_passed, tests_failed);
    BinarySearchTree<int> filtered = tree.Where(TestTreeEven);
    check_tree(filtered.GetCount() == 3 && filtered.Get(0) == 2 && filtered.Get(2) == 6,
               "Where отбирает подходящие значения", tests_passed, tests_failed);
    filtered.Clear();
    check_tree(tree == original && tree.Where(TestTreeAll).GetCount() == 7 && tree.Where(TestTreeNone).IsEmpty(),
               "Where: независимость, все и ни одного", tests_passed, tests_failed);
    check_tree(tree.Reduce(TestTreeSum, 10LL) == 38 && tree.Reduce(TestTreeJoin, std::string("ЛКП: ")) == "ЛКП: 1 2 3 4 5 6 7 ",
               "Reduce: другой тип результата и порядок ЛКП", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::invalid_argument>([&]() {tree.Map(nullptr);}) &&
                   TreeTestThrows<std::invalid_argument>([&]() {tree.Where(nullptr);}) &&
                   TreeTestThrows<std::invalid_argument>([&]() {tree.Reduce<long long>(nullptr, 0LL);}),
               "Пустые указатели на функции отклонены", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::runtime_error>([&]() {tree.Map(TestTreeThrowMap);}) &&
                   TreeTestThrows<std::runtime_error>([&]() {tree.Where(TestTreeThrowWhere);}) &&
                   TreeTestThrows<std::runtime_error>([&]() {tree.Reduce(TestTreeThrowReduce, 0LL);}) && tree == original,
               "Исключения функций сохраняют исходное дерево", tests_passed, tests_failed);
    tree.Clear();
    check_tree(tree.Map(TestTreeNegate).IsEmpty() && tree.Where(TestTreeAll).IsEmpty() && tree.Reduce(TestTreeSum, 17LL) == 17,
               "Map, Where и Reduce пустого дерева", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::invalid_argument>([&]() {tree.Map(nullptr);}) &&
                   TreeTestThrows<std::invalid_argument>([&]() {tree.Where(nullptr);}) &&
                   TreeTestThrows<std::invalid_argument>([&]() {tree.Reduce<long long>(nullptr, 0LL);}),
               "Пустые функции отклонены даже на пустом дереве", tests_passed, tests_failed);
}

inline void TestTreeSubtrees(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree = MakeTestTree();
    BinarySearchTree<int> subtree = tree.GetSubtree(2);
    check_tree(subtree.GetCount() == 3 && subtree.ToString() == "{1:2}({1:1}(#)[#])[{1:3}(#)[#]]" && tree.ContainsSubtree(subtree),
               "Извлечение сохраняет форму и размер", tests_passed, tests_failed);
    check_tree(tree.GetSubtree(4) == tree && tree.GetSubtree(1).GetCount() == 1,
               "Извлечение корня и листа", tests_passed, tests_failed);
    subtree.Remove(1);
    check_tree(tree.Contains(1) && !tree.ContainsSubtree(subtree),
               "Извлечённая копия независима; отсутствующий ребёнок значим", tests_passed, tests_failed);
    BinarySearchTree<int> other(TestTreeCompareInt);
    other.FromString("{1:3}({1:1}(#)[{1:2}(#)[#]])[{1:4}(#)[{1:5}(#)[#]]]");
    subtree.FromString("{1:3}({1:2}({1:1}(#)[#])[#])[{1:5}({1:4}(#)[#])[#]]");
    check_tree(other != subtree && !other.ContainsSubtree(subtree),
               "Те же значения и корень при иной форме не являются поддеревом", tests_passed, tests_failed);
    subtree.Clear();
    check_tree(tree.ContainsSubtree(subtree) && subtree.ContainsSubtree(subtree) && !subtree.ContainsSubtree(tree),
               "Вхождение пустого дерева", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::out_of_range>([&]() {tree.GetSubtree(99);}) &&
                   TreeTestThrows<std::out_of_range>([&]() {subtree.GetSubtree(1);}),
               "Извлечение отсутствующего узла отклонено", tests_passed, tests_failed);
    subtree.Insert(99);
    check_tree(!tree.ContainsSubtree(subtree), "Отсутствующий корень поддерева", tests_passed, tests_failed);
}

inline void TestTreeSerialization(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree(TestTreeCompareInt);
    tree.Insert(2);
    tree.Insert(1);
    tree.Insert(3);
    const char* formats[] = {"{К}(Л)[П]", "{К}[П](Л)", "(Л){К}[П]", "(Л)[П]{К}", "[П]{К}(Л)", "[П](Л){К}"};
    const char* expected[] = {
        "{1:2}({1:1}(#)[#])[{1:3}(#)[#]]",
        "{1:2}[{1:3}[#](#)]({1:1}[#](#))",
        "((#){1:1}[#]){1:2}[(#){1:3}[#]]",
        "((#)[#]{1:1})[(#)[#]{1:3}]{1:2}",
        "[[#]{1:3}(#)]{1:2}([#]{1:1}(#))",
        "[[#](#){1:3}]([#](#){1:1}){1:2}"
    };
    for (int index = 0; index < 6; index++) {
        check_tree(tree.ToString(formats[index]) == expected[index], formats[index], tests_passed, tests_failed);
    }
    BinarySearchTree<int> restored(TestTreeCompareInt);
    restored.FromString(tree.ToString());
    check_tree(restored == tree, "Чтение восстанавливает форму и значения", tests_passed, tests_failed);
    BinarySearchTree<int> single(TestTreeCompareInt);
    single.Insert(2);
    check_tree(single.ToString("{Л]К[П}") == "{#]1:2[#}",
               "Ограничители могут различаться, корень может быть без скобок", tests_passed, tests_failed);
    const char* invalid_formats[] = {"", "КЛ", "ККП", "КЛХ", "{К}(Л)[П", "{{К}}ЛП", "КЛПx", "К)ЛП", "{КЛ}П"};
    for (const char* format : invalid_formats) {
        check_tree(TreeTestThrows<std::invalid_argument>([&]() {tree.ToString(format);}),
                   "Некорректный формат отклонён", tests_passed, tests_failed);
    }
    const char* invalid_texts[] = {
        "", "##", "{1:2}(#)[#]tail", "{1:2}(#)[", "{1:2}(#){#}", "{2:2}(#)[#]", "{1:x}(#)[#]",
        "{1:2}({1:3}(#)[#])[#]", "{1:2}(#)[{1:2}(#)[#]]",
        "{1:4}({1:2}(#)[{1:5}(#)[#]])[#]", "{999999999999999999999999999999:2}(#)[#]",
        "{0:}(#)[#]", "{2: 2}(#)[#]", "{11:99999999999}(#)[#]",
        "{1:1}(#)[{1:2}(#)[{1:3}(#)[#]]]"
    };
    for (const char* text : invalid_texts) {
        check_tree(TreeTestThrows<std::invalid_argument>([&]() {restored.FromString(text);}) && restored == tree,
                   "Повреждённая строка или неверный BST: прежнее дерево сохранено", tests_passed, tests_failed);
    }
    restored.FromString("#");
    check_tree(restored.IsEmpty() && restored.ToString() == "#", "Сохранение и чтение пустого дерева", tests_passed, tests_failed);
    restored.Insert(-7);
    BinarySearchTree<int> copy(TestTreeCompareInt);
    copy.FromString(restored.ToString());
    check_tree(copy == restored, "Отрицательное значение и один элемент", tests_passed, tests_failed);

    BinarySearchTree<std::string> strings(TestTreeCompareString);
    std::string values[] = {"a b", "\"\\{}()[]#:КЛП", "", "line\n\tend", std::string("x\0y", 3)};
    for (const auto& value : values) {strings.Insert(value);}
    BinarySearchTree<std::string> string_copy(TestTreeCompareString);
    string_copy.FromString(strings.ToString());
    check_tree(strings == string_copy, "Строки: пробелы, кавычки, ограничители, UTF-8, NUL", tests_passed, tests_failed);
    BinarySearchTree<double> numbers(TestTreeCompareDouble);
    double number_values[] = {0.1, -3.141592653589793, 123456.789, 1e-12};
    for (double value : number_values) {numbers.Insert(value);}
    BinarySearchTree<double> number_copy(TestTreeCompareDouble);
    number_copy.FromString(numbers.ToString());
    check_tree(number_copy == numbers, "Double: точность обратного чтения", tests_passed, tests_failed);

}

inline void TestTreeRotations(int& tests_passed, int& tests_failed) {
    int values[][3] = {{3, 2, 1}, {1, 2, 3}, {3, 1, 2}, {1, 3, 2}};
    const char* names[] = {"LL: правый поворот", "RR: левый поворот", "LR: двойной поворот", "RL: двойной поворот"};
    const char* expected = "{1:2}({1:1}(#)[#])[{1:3}(#)[#]]";

    for (int number = 0; number < 4; number++) {
        BinarySearchTree<int> tree(TestTreeCompareInt);
        tree.Insert(values[number][0]);
        tree.Insert(values[number][1]);
        const int* first = tree.Find(values[number][0]);
        const int* second = tree.Find(values[number][1]);
        tree.Insert(values[number][2]);
        check_tree(tree.ToString() == expected && CheckTreeAVL(tree), names[number], tests_passed, tests_failed);
        check_tree(tree.Find(values[number][0]) == first && tree.Find(values[number][1]) == second,
                   "Поворот сохраняет сами узлы", tests_passed, tests_failed);
    }
}

inline void TestTreeDeletion(int& tests_passed, int& tests_failed) {
    int values[][7] = {{4, 2, 5, 1, 3, 0, 0}, {2, 1, 4, 3, 5, 0, 0},
                      {5, 2, 8, 1, 4, 7, 3}, {5, 2, 8, 3, 6, 9, 7}};
    int counts[] = {5, 5, 7, 7};
    int removed[] = {5, 1, 7, 3};
    int roots[] = {2, 4, 4, 6};
    const char* names[] = {"Удаление: правый поворот при нулевом балансе ребёнка",
                          "Удаление: левый поворот при нулевом балансе ребёнка",
                          "Удаление: поворот LR", "Удаление: поворот RL"};
    for (int number = 0; number < 4; number++) {
        BinarySearchTree<int> tree(TestTreeCompareInt);
        for (int index = 0; index < counts[number]; index++) {tree.Insert(values[number][index]);}
        tree.Remove(removed[number]);
        std::string prefix = "{1:" + std::to_string(roots[number]) + '}';
        check_tree(tree.ToString().find(prefix) == 0 && CheckTreeAVL(tree) && !tree.Contains(removed[number]),
                   names[number], tests_passed, tests_failed);
    }

    BinarySearchTree<int> tree(TestTreeCompareInt);
    tree.FromString("{1:8}({1:4}({1:2}(#)[#])[{1:6}(#)[#]])[{2:12}({2:10}(#)[{2:11}(#)[#]])[{2:14}({2:13}(#)[#])[{2:15}(#)[#]]]]");
    const int* successor = tree.Find(10);
    const int* child = tree.Find(11);
    tree.Remove(8);
    check_tree(CheckTreeAVL(tree) && tree.GetCount() == 9 && tree.Find(10) == successor && tree.Find(11) == child,
               "Удаление корня: глубокий преемник с правым ребёнком", tests_passed, tests_failed);
    tree.Remove(2);
    tree.Remove(6);
    tree.Remove(4);
    check_tree(CheckTreeAVL(tree), "Подъём от прежнего родителя преемника до нового корня", tests_passed, tests_failed);

    tree.FromString("{1:2}({1:1}(#)[#])[{1:3}(#)[{1:4}(#)[#]]]");
    tree.Remove(2);
    check_tree(tree.ToString() == "{1:3}({1:1}(#)[#])[{1:4}(#)[#]]" && CheckTreeAVL(tree),
               "Удаление корня: преемник является правым ребёнком", tests_passed, tests_failed);
    tree.Remove(3);
    tree.Remove(4);
    tree.Remove(1);
    check_tree(tree.IsEmpty() && CheckTreeAVL(tree), "Удаление до пустого дерева", tests_passed, tests_failed);
}

inline void TestTreeAVL(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree(TestTreeCompareInt);
    check_tree(CheckTreeAVL(tree) && TestTreeHeight(tree.ToString()) == 0,
               "AVL: пустое дерево имеет высоту 0", tests_passed, tests_failed);
    tree.Insert(0);
    check_tree(CheckTreeAVL(tree) && TestTreeHeight(tree.ToString()) == 1,
               "AVL: лист имеет высоту 1", tests_passed, tests_failed);

    const int count = 30000;
    bool correct = true;
    const int* address = tree.Find(0);
    for (int value = 1; value < count; value++) {
        tree.Insert(value);
        if (value % 1000 == 0 && !CheckTreeAVL(tree)) {correct = false; break;}
    }
    check_tree(correct && tree.GetCount() == count && CheckTreeAVL(tree) && TestTreeHeight(tree.ToString()) <= 22 && tree.Find(0) == address,
               "Возрастающая вставка: небольшая высота, AVL и сохранение узлов", tests_passed, tests_failed);
    if (!correct) {return;}

    std::ostringstream output;
    tree.Print(output, 10);
    check_tree(output.str().size() < 1000 && output.str().find("ограничение") != std::string::npos,
               "Вывод большого AVL-дерева ограничен по числу узлов", tests_passed, tests_failed);

    std::unique_ptr<IEnumerator<int>> it(tree.GetEnumerator());
    for (int value = 0; value < count; value++) {
        if (!it->HasNext() || it->GetCurrent() != value) {correct = false; break;}
        it->MoveNext();
    }
    check_tree(correct && !it->HasNext(), "Возрастающая вставка: порядок перечислителя и parent", tests_passed, tests_failed);
    it.reset();

    BinarySearchTree<int> copy(tree);
    BinarySearchTree<int> restored(TestTreeCompareInt);
    restored.FromString(tree.ToString());
    check_tree(copy == tree && restored == tree && CheckTreeAVL(restored),
               "Копирование и чтение сохраняют форму большого AVL-дерева", tests_passed, tests_failed);
    for (int value = count; value < count + 100; value++) {copy.Insert(value); restored.Insert(value);}
    for (int value = 0; value < 100; value++) {copy.Remove(value); restored.Remove(value);}
    check_tree(copy == restored && CheckTreeAVL(copy) && tree.Contains(0) && !tree.Contains(count),
               "Высоты копии: последующие вставки и удаления", tests_passed, tests_failed);

    BinarySearchTree<int> subtree = tree.GetSubtree(count / 2);
    for (int value = count; value < count + 100; value++) {subtree.Insert(value);}
    check_tree(CheckTreeAVL(subtree) && !tree.Contains(count),
               "Высоты извлечённого поддерева и независимость его узлов", tests_passed, tests_failed);
    BinarySearchTree<int> moved(std::move(subtree));
    moved.Insert(count + 100);
    check_tree(subtree.IsEmpty() && CheckTreeAVL(moved), "Перемещение передаёт готовые AVL-высоты", tests_passed, tests_failed);

    for (int value = 0; value < count; value++) {
        tree.Remove(value);
        if (value % 1000 == 0 && !CheckTreeAVL(tree)) {correct = false; break;}
    }
    check_tree(correct && tree.IsEmpty(), "Удаление возрастающей последовательности с автоматической балансировкой", tests_passed, tests_failed);
}

inline void TestTreeMixedOperations(int& tests_passed, int& tests_failed) {
    BinarySearchTree<int> tree(TestTreeCompareInt);
    bool present[128] = {};
    unsigned int state = 1323;
    int count = 0;
    bool correct = true;

    for (int step = 0; step < 3000; step++) {
        state = state * 1664525u + 1013904223u;
        int value = static_cast<int>((state >> 8) % 128);
        if (state % 2 == 0) {
            if (!present[value]) {tree.Insert(value); present[value] = true; count++;}
            else {correct = correct && TreeTestThrows<std::invalid_argument>([&]() {tree.Insert(value);});}
        } else {
            if (present[value]) {tree.Remove(value); present[value] = false; count--;}
            else {correct = correct && TreeTestThrows<std::out_of_range>([&]() {tree.Remove(value);});}
        }
        if (tree.GetCount() != count || !CheckTreeAVL(tree)) {correct = false; break;}

        std::unique_ptr<IEnumerator<int>> it(tree.GetEnumerator());
        for (int item = 0; item < 128; item++) {
            if (present[item]) {
                if (!it->HasNext() || it->GetCurrent() != item) {correct = false; break;}
                it->MoveNext();
            }
        }
        if (it->HasNext() || !correct) {correct = false; break;}
    }
    check_tree(correct, "Смешанные операции: AVL, размер и полный порядок после каждого изменения", tests_passed, tests_failed);
}

class TreeTestValue {
public:
    static int alive;
    static int copies_left;
    int key;

    explicit TreeTestValue(int key): key(key) {alive++;}
    TreeTestValue(const TreeTestValue& other): key(other.key) {
        if (copies_left == 0) {throw std::runtime_error("Ошибка копирования");}
        if (copies_left > 0) {copies_left--;}
        alive++;
    }
    ~TreeTestValue() {alive--;}
};

inline int TreeTestValue::alive = 0;
inline int TreeTestValue::copies_left = -1;

inline bool TestTreeCompareValue(const TreeTestValue& first, const TreeTestValue& second) {return first.key < second.key;}
inline TreeTestValue TestTreeMapValue(const TreeTestValue& value) {return TreeTestValue(-value.key);}
inline bool TestTreeWhereValue(const TreeTestValue&) {return true;}
inline int TestTreeReduceValue(const int& sum, const TreeTestValue& value) {return sum + value.key;}
inline int TestTreeComparisonsLeft = -1;
inline bool TestTreeThrowCompare(const int& first, const int& second) {
    if (TestTreeComparisonsLeft == 0) {throw std::runtime_error("Ошибка компаратора");}
    if (TestTreeComparisonsLeft > 0) {TestTreeComparisonsLeft--;}
    return first < second;
}

inline void TestTreeInterfaces(int& tests_passed, int& tests_failed) {
    std::unique_ptr<ICollection<int>> collection(new BinarySearchTree<int>(MakeTestTree()));
    check_tree(collection->GetCount() == 7 && collection->Get(0) == 1 && collection->Get(6) == 7,
               "ICollection: индекс и размер", tests_passed, tests_failed);
    check_tree(TreeTestThrows<std::out_of_range>([&]() {collection->Get(7);}) &&
                   TreeTestThrows<std::out_of_range>([&]() {collection->Get(static_cast<std::size_t>(-1));}),
               "ICollection: неверные индексы", tests_passed, tests_failed);
    const IEnumerable<int>* enumerable = collection.get();
    std::unique_ptr<IEnumerator<int>> it(enumerable->GetEnumerator());
    check_tree(it->HasNext() && it->GetCurrent() == 1, "IEnumerable: контракт ЛКП", tests_passed, tests_failed);
    BinarySearchTree<int> first = MakeTestTree();
    BinarySearchTree<int> second(TestTreeCompareInt);
    second.FromString("{1:3}({1:2}({1:1}(#)[#])[#])[{1:5}({1:4}(#)[#])[{1:6}(#)[{1:7}(#)[#]]]]");
    check_tree(first != second && !(first == second), "Равные значения иной формы не равны", tests_passed, tests_failed);
    second.Insert(8);
    BinarySearchTree<int> united = first + second;
    check_tree(united.GetCount() == 8 && united.Contains(8) && first.GetCount() == 7 && second.GetCount() == 8 && TestTreeHeight(united.ToString()) == 4,
               "Объединение: уникальные элементы, независимость и баланс", tests_passed, tests_failed);
    united.Clear();
    check_tree(first.GetCount() == 7 && (first + first).GetCount() == 7,
               "Объединение с собой и независимость результата", tests_passed, tests_failed);
    second.Clear();
    check_tree((first + second).GetCount() == 7 && (second + first).GetCount() == 7 && (second + second).IsEmpty(),
               "Операторы с пустыми деревьями", tests_passed, tests_failed);
    BinarySearchTree<int> reverse(TestTreeCompareReverse);
    reverse.Insert(2);
    reverse.Insert(1);
    reverse.Insert(3);
    BinarySearchTree<int> reverse_copy(TestTreeCompareReverse);
    reverse_copy.FromString(reverse.ToString());
    check_tree(reverse.Get(0) == 3 && reverse_copy == reverse && reverse.Map(TestTreeNegate).Get(0) == -1 &&
                   reverse.Where(TestTreeAll).Get(0) == 3 && (reverse + reverse_copy).GetCount() == 3,
               "Обратный компаратор: балансировка, чтение, функции и объединение", tests_passed, tests_failed);

    {
        BinarySearchTree<TreeTestValue> custom(TestTreeCompareValue);
        custom.Insert(TreeTestValue(2));
        custom.Insert(TreeTestValue(1));
        custom.Insert(TreeTestValue(3));
        BinarySearchTree<TreeTestValue> mapped = custom.Map(TestTreeMapValue);
        check_tree(mapped.Get(0).key == -3 && custom.Reduce(TestTreeReduceValue, 0) == 6 && custom.GetSubtree(TreeTestValue(2)).GetCount() == 3,
                   "Тип без конструктора по умолчанию и текстовых операторов", tests_passed, tests_failed);
        BinarySearchTree<TreeTestValue> copy(custom);
        int before = TreeTestValue::alive;
        TreeTestValue::copies_left = 1;
        bool caught = TreeTestThrows<std::runtime_error>([&]() {copy = custom;});
        TreeTestValue::copies_left = -1;
        check_tree(caught && copy == custom && TreeTestValue::alive == before,
                   "Ошибка копирования: прежнее значение и освобождение частичной копии", tests_passed, tests_failed);
        TreeTestValue::copies_left = 1;
        caught = TreeTestThrows<std::runtime_error>([&]() {custom.Map(TestTreeMapValue);});
        TreeTestValue::copies_left = -1;
        check_tree(caught && TreeTestValue::alive == before, "Ошибка Map освобождает созданные узлы", tests_passed, tests_failed);
        TreeTestValue::copies_left = 1;
        caught = TreeTestThrows<std::runtime_error>([&]() {custom.Where(TestTreeWhereValue);});
        TreeTestValue::copies_left = -1;
        check_tree(caught && TreeTestValue::alive == before, "Ошибка Where освобождает созданные узлы", tests_passed, tests_failed);
        TreeTestValue::copies_left = 1;
        caught = TreeTestThrows<std::runtime_error>([&]() {custom + copy;});
        TreeTestValue::copies_left = -1;
        check_tree(caught && TreeTestValue::alive == before, "Ошибка объединения освобождает созданные узлы", tests_passed, tests_failed);
    }
    check_tree(TreeTestValue::alive == 0, "Уничтожение всех пользовательских значений", tests_passed, tests_failed);
    {
        std::unique_ptr<IEnumerable<TreeTestValue>> interface(new BinarySearchTree<TreeTestValue>(TestTreeCompareValue));
        static_cast<BinarySearchTree<TreeTestValue>*>(interface.get())->Insert(TreeTestValue(1));
    }
    check_tree(TreeTestValue::alive == 0, "Виртуальное уничтожение через IEnumerable", tests_passed, tests_failed);
    BinarySearchTree<int> throwing(TestTreeThrowCompare);
    throwing.Insert(2);
    throwing.Insert(1);
    throwing.Insert(3);
    std::string saved = throwing.ToString();
    TestTreeComparisonsLeft = 1;
    bool caught = TreeTestThrows<std::runtime_error>([&]() {throwing.Map(TestTreeNegate);});
    TestTreeComparisonsLeft = -1;
    check_tree(caught && throwing.ToString() == saved, "Ошибка компаратора при вставке результата Map", tests_passed, tests_failed);
    TestTreeComparisonsLeft = 0;
    caught = TreeTestThrows<std::runtime_error>([&]() {throwing.FromString(saved);});
    TestTreeComparisonsLeft = -1;
    check_tree(caught && throwing.ToString() == saved, "Ошибка проверки порядка при чтении сохраняет дерево", tests_passed, tests_failed);
}

inline int run_tree_tests() {
    int tests_passed = 0;
    int tests_failed = 0;
    std::cout << "\nТестирование BinarySearchTree\n";
    void (*tests[])(int&, int&) = {TestTreeBasic, TestTreeCopying, TestTreeFunctions, TestTreeSubtrees,
                                 TestTreeSerialization, TestTreeRotations, TestTreeDeletion, TestTreeAVL, TestTreeMixedOperations, TestTreeInterfaces};
    const char* names[] = {"Базовые операции", "Копирование и перечислитель", "Map, Where, Reduce", "Поддеревья",
                           "Сохранение и чтение", "Повороты", "AVL-удаление", "AVL на возрастающих данных",
                           "Смешанные операции", "Интерфейсы, операторы и пользовательский тип"};
    for (int number = 0; number < 10; number++) {
        int before = tests_failed;
        std::cout << "\n" << names[number] << "\n";
        try {
            tests[number](tests_passed, tests_failed);
        } catch (const std::exception& Error) {
            check_tree(false, Error.what(), tests_passed, tests_failed);
        } catch (...) {
            check_tree(false, "Неожиданное исключение", tests_passed, tests_failed);
        }
        TreeTestValue::copies_left = -1;
        TestTreeComparisonsLeft = -1;
        std::cout << names[number] << ": провалено " << tests_failed - before << "\n";
    }
    std::cout << "\nBinarySearchTree: успешно " << tests_passed << ", провалено " << tests_failed << "\n";
    return tests_failed;
}

#endif // TREE_TESTS_HPP
