#ifndef BINARY_SEARCH_TREE_HPP
#define BINARY_SEARCH_TREE_HPP

#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <utility>

#include "Sequences/ICollection.hpp"

template <typename T>
class BinarySearchTree : public ICollection<T> {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        Node* parent;
        int height;

        explicit Node(const T& value, Node* ParentNode = nullptr): data(value), left(nullptr), right(nullptr), parent(ParentNode), height(1) {}
    };

    Node* root;
    int size;
    bool (*compare)(const T&, const T&);

    bool Less(const T& first, const T& second) const {
        return compare(first, second);
    }

    int GetHeight(const Node* node) const {return node == nullptr ? 0 : node->height;}

    void UpdateHeight(Node* node) {
        int LeftHeight = GetHeight(node->left);
        int RightHeight = GetHeight(node->right);
        node->height = (LeftHeight > RightHeight ? LeftHeight : RightHeight) + 1;
    }

    Node* RotateLeft(Node* node) {
        Node* NewRoot = node->right;
        Node* ParentNode = node->parent;

        node->right = NewRoot->left;
        if (node->right != nullptr) {node->right->parent = node;}

        NewRoot->left = node;
        NewRoot->parent = ParentNode;
        node->parent = NewRoot;

        if (ParentNode == nullptr) {root = NewRoot;}
        else if (ParentNode->left == node) {ParentNode->left = NewRoot;}
        else {ParentNode->right = NewRoot;}

        UpdateHeight(node);
        UpdateHeight(NewRoot);
        return NewRoot;
    }

    Node* RotateRight(Node* node) {
        Node* NewRoot = node->left;
        Node* ParentNode = node->parent;

        node->left = NewRoot->right;
        if (node->left != nullptr) {node->left->parent = node;}

        NewRoot->right = node;
        NewRoot->parent = ParentNode;
        node->parent = NewRoot;

        if (ParentNode == nullptr) {root = NewRoot;}
        else if (ParentNode->left == node) {ParentNode->left = NewRoot;}
        else {ParentNode->right = NewRoot;}

        UpdateHeight(node);
        UpdateHeight(NewRoot);
        return NewRoot;
    }

    Node* BalanceNode(Node* node) {
        UpdateHeight(node);
        int BalanceValue = GetHeight(node->left) - GetHeight(node->right);

        if (BalanceValue > 1) {
            if (GetHeight(node->left->left) < GetHeight(node->left->right)) {RotateLeft(node->left);}
            return RotateRight(node);
        }
        if (BalanceValue < -1) {
            if (GetHeight(node->right->right) < GetHeight(node->right->left)) {RotateRight(node->right);}
            return RotateLeft(node);
        }

        return node;
    }

    void BalanceToRoot(Node* node) {
        while (node != nullptr) {
            node = BalanceNode(node)->parent;
        }
    }

    Node** FindNode(const T& item) {
        Node** CurrentNode = &root;

        while (*CurrentNode != nullptr) {
            Node* node = *CurrentNode;

            if (Less(item, node->data)) {
                CurrentNode = &(node->left);
            }
            else if (Less(node->data, item)) {
                CurrentNode = &(node->right);
            }
            else {
                return CurrentNode;
            }
        }

        return nullptr;
    }

    Node** FindMin(Node** node) {
        if (node == nullptr || *node == nullptr) {return nullptr;}

        while ((*node)->left != nullptr) {
            node = &((*node)->left);
        }

        return node;
    }

    void DeleteSubtree(Node* node) noexcept {
        Node* CurrentNode = node;

        while (CurrentNode != nullptr) {
            if (CurrentNode->left != nullptr) {
                CurrentNode = CurrentNode->left;
            }
            else if (CurrentNode->right != nullptr) {
                CurrentNode = CurrentNode->right;
            }
            else {
                Node* ParentNode = CurrentNode == node ? nullptr : CurrentNode->parent;

                if (ParentNode != nullptr) {
                    if (ParentNode->left == CurrentNode) {ParentNode->left = nullptr;}
                    else {ParentNode->right = nullptr;}
                }

                delete CurrentNode;
                CurrentNode = ParentNode;
            }
        }
    }

    Node* CopySubtree(const Node* node) {
        if (node == nullptr) {return nullptr;}

        Node* NewRoot = new Node(node->data);
        NewRoot->height = node->height;
        const Node* SourceNode = node;
        Node* TargetNode = NewRoot;

        try {
            while (true) {
                if (SourceNode->left != nullptr && TargetNode->left == nullptr) {
                    TargetNode->left = new Node(SourceNode->left->data, TargetNode);

                    SourceNode = SourceNode->left;
                    TargetNode = TargetNode->left;
                    TargetNode->height = SourceNode->height;
                }
                else if (SourceNode->right != nullptr && TargetNode->right == nullptr) {
                    TargetNode->right = new Node(SourceNode->right->data, TargetNode);

                    SourceNode = SourceNode->right;
                    TargetNode = TargetNode->right;
                    TargetNode->height = SourceNode->height;
                }
                else {
                    if (SourceNode == node) {break;}

                    SourceNode = SourceNode->parent;
                    TargetNode = TargetNode->parent;
                }
            }
        }
        catch (...) {
            DeleteSubtree(NewRoot);
            throw;
        }

        return NewRoot;
    }

    bool EqualSubtrees(const Node* first, const Node* second) const {
        if (first == nullptr || second == nullptr) {return first == second;}
        if (Less(first->data, second->data) || Less(second->data, first->data)) {return false;}

        return EqualSubtrees(first->left, second->left) && EqualSubtrees(first->right, second->right);
    }

    struct FormatPart {
        int order;
        char left;
        char right;

        FormatPart(): order(0), left('\0'), right('\0') {}
    };

    void ReadFormat(const std::string& format, FormatPart* parts) const {
        std::size_t position = 0;
        bool Used[] = {false, false, false};
        const char* Names[] = {"К", "Л", "П"};

        for (int index = 0; index < 3; index++) {
            if (position < format.size() && std::string("{([<").find(format[position]) != std::string::npos) {
                parts[index].left = format[position++];
            }

            int order = -1;
            for (int number = 0; number < 3; number++) {
                std::string name = Names[number];
                if (format.compare(position, name.size(), name) == 0) {
                    order = number;
                    position += name.size();
                    break;
                }
            }

            if (order == -1 || Used[order]) {throw std::invalid_argument("Неверная строка форматирования");}
            Used[order] = true;
            parts[index].order = order;

            if (parts[index].left != '\0') {
                if (position == format.size() || std::string("})]>").find(format[position]) == std::string::npos) {
                    throw std::invalid_argument("Не задан правый ограничитель");
                }
                parts[index].right = format[position++];
            }
        }

        if (position != format.size()) {throw std::invalid_argument("Лишние символы в формате");}
    }

    static std::string WriteValue(const T& item) {
        if constexpr (std::is_same<T, std::string>::value) {
            return item;
        }
        else {
            std::ostringstream stream;
            stream << std::setprecision(std::numeric_limits<T>::max_digits10) << item;
            return stream.str();
        }
    }

    static T ReadValue(const std::string& text, std::size_t& position) {
        std::size_t end = text.find(':', position);
        if (end == std::string::npos) {throw std::invalid_argument("Не указана длина значения");}

        std::size_t length = 0;
        std::istringstream length_stream(text.substr(position, end - position));
        if (!(length_stream >> std::noskipws >> length) || length_stream.peek() != std::char_traits<char>::eof()) {
            throw std::invalid_argument("Неверная длина значения");
        }

        position = end + 1;
        if (length > text.size() - position) {throw std::invalid_argument("Повреждена запись значения");}
        std::string value = text.substr(position, length);
        position += length;

        if constexpr (std::is_same<T, std::string>::value) {
            return value;
        }
        else {
            std::istringstream stream(value);
            T item{};
            if (!(stream >> std::noskipws >> item) || stream.peek() != std::char_traits<char>::eof()) {
                throw std::invalid_argument("Неверное значение элемента");
            }
            return item;
        }
    }

    static void ExpectSymbol(const std::string& text, std::size_t& position, char symbol) {
        if (position == text.size() || text[position++] != symbol) {
            throw std::invalid_argument("Повреждена структура дерева");
        }
    }

    Node* ReadNode(const std::string& text, std::size_t& position, Node* ParentNode) {
        if (position < text.size() && text[position] == '#') {position++; return nullptr;}

        ExpectSymbol(text, position, '{');
        T item = ReadValue(text, position);
        ExpectSymbol(text, position, '}');
        if (size == std::numeric_limits<int>::max()) {throw std::length_error("Дерево слишком большое");}

        Node* node = new Node(item, ParentNode);
        size++;
        return node;
    }

    void WriteSubtree(const Node* node, const FormatPart* parts, std::ostream& stream) const {
        if (node == nullptr) {stream << '#'; return;}

        for (int index = 0; index < 3; index++) {
            const FormatPart& part = parts[index];
            if (part.left != '\0') {stream << part.left;}

            if (part.order == 0) {
                std::string value = WriteValue(node->data);
                stream << value.size() << ':' << value;
            }
            else {
                WriteSubtree(part.order == 1 ? node->left : node->right, parts, stream);
            }

            if (part.right != '\0') {stream << part.right;}
        }
    }

    void PrintSubtree(const Node* node, std::ostream& stream, int depth, char name, int& remaining) const {
        if (remaining <= 0) {return;}
        remaining--;

        for (int level = 0; level < depth; level++) {stream << "  ";}
        stream << (name == 'K' ? "К" : name == 'L' ? "Л" : "П") << ": ";
        if (node == nullptr) {stream << "#\n"; return;}

        stream << std::quoted(WriteValue(node->data)) << "\n";
        PrintSubtree(node->left, stream, depth + 1, 'L', remaining);
        PrintSubtree(node->right, stream, depth + 1, 'R', remaining);
    }

    class TreeEnumerator : public IEnumerator<T> {
    private:
        const Node* CurrentNode;

    public:
        explicit TreeEnumerator(const BinarySearchTree<T>& tree): CurrentNode(tree.root) {
            if (CurrentNode != nullptr) {
                while (CurrentNode->left != nullptr) {
                    CurrentNode = CurrentNode->left;
                }
            }
        }

        const T& GetCurrent() const override {
            if (!HasNext()) {
                throw std::out_of_range("Перечислитель находится за границами дерева");
            }

            return CurrentNode->data;
        }

        void MoveNext() override {
            if (!HasNext()) {return;}

            if (CurrentNode->right != nullptr) {
                CurrentNode = CurrentNode->right;

                while (CurrentNode->left != nullptr) {
                    CurrentNode = CurrentNode->left;
                }
            }
            else {
                const Node* ParentNode = CurrentNode->parent;

                while (ParentNode != nullptr && CurrentNode == ParentNode->right) {
                    CurrentNode = ParentNode;
                    ParentNode = ParentNode->parent;
                }

                CurrentNode = ParentNode;
            }
        }

        bool HasNext() const override {
            return CurrentNode != nullptr;
        }
    };

public:
    explicit BinarySearchTree(bool (*comparator)(const T&, const T&)): root(nullptr), size(0), compare(comparator) {
        if (compare == nullptr) {
            throw std::invalid_argument("Компаратор не может быть пустым");
        }
    }

    BinarySearchTree(const BinarySearchTree<T>& other): root(nullptr), size(0), compare(other.compare) {
        root = CopySubtree(other.root);
        size = other.size;
    }

    BinarySearchTree(BinarySearchTree<T>&& other) noexcept: root(other.root), size(other.size), compare(other.compare) {
        other.root = nullptr;
        other.size = 0;
    }

    BinarySearchTree<T>& operator=(const BinarySearchTree<T>& other) {
        if (this == &other) {return *this;}

        Node* NewRoot = CopySubtree(other.root);

        DeleteSubtree(root);

        root = NewRoot;
        size = other.size;
        compare = other.compare;

        return *this;
    }

    BinarySearchTree<T>& operator=(BinarySearchTree<T>&& other) noexcept {
        if (this == &other) {return *this;}

        DeleteSubtree(root);

        root = other.root;
        size = other.size;
        compare = other.compare;

        other.root = nullptr;
        other.size = 0;

        return *this;
    }

    ~BinarySearchTree() override {
        this->Clear();
    }

    int GetCount() const override {return size;}
    bool IsEmpty() const {return size == 0;}
    bool Contains(const T& item) const {return this->Find(item) != nullptr;}

    const T* Find(const T& item) const {
        const Node* CurrentNode = root;

        while (CurrentNode != nullptr) {
            if (Less(item, CurrentNode->data)) {
                CurrentNode = CurrentNode->left;
            }
            else if (Less(CurrentNode->data, item)) {
                CurrentNode = CurrentNode->right;
            }
            else {
                return &(CurrentNode->data);
            }
        }

        return nullptr;
    }

    T* Find(const T& item) {
        Node* CurrentNode = root;

        while (CurrentNode != nullptr) {
            if (Less(item, CurrentNode->data)) {
                CurrentNode = CurrentNode->left;
            }
            else if (Less(CurrentNode->data, item)) {
                CurrentNode = CurrentNode->right;
            }
            else {
                return &(CurrentNode->data);
            }
        }

        return nullptr;
    }

    // Ищу по ключу без создания промежуточной пары ключ-значение.
    template <typename TKey>
    const T* FindByKey(const TKey& key, int (*keyComparator)(const TKey&, const T&)) const {
        const Node* CurrentNode = root;

        while (CurrentNode != nullptr) {
            int Order = keyComparator(key, CurrentNode->data);

            if (Order < 0) {CurrentNode = CurrentNode->left;}
            else if (Order > 0) {CurrentNode = CurrentNode->right;}
            else {return &(CurrentNode->data);}
        }

        return nullptr;
    }

    template <typename TKey>
    T* FindByKey(const TKey& key, int (*keyComparator)(const TKey&, const T&)) {
        const BinarySearchTree<T>& tree = *this;
        return const_cast<T*>(tree.FindByKey(key, keyComparator));
    }

    void Insert(const T& item) {
        Node** CurrentNode = &root;
        Node* ParentNode = nullptr;

        while (*CurrentNode != nullptr) {
            ParentNode = *CurrentNode;

            if (Less(item, ParentNode->data)) {
                CurrentNode = &(ParentNode->left);
            }
            else if (Less(ParentNode->data, item)) {
                CurrentNode = &(ParentNode->right);
            }
            else {
                throw std::invalid_argument("Данные с таким ключом уже содержатся в дереве");
            }
        }

        if (size == std::numeric_limits<int>::max()) {
            throw std::length_error("Превышен максимальный размер дерева");
        }

        *CurrentNode = new Node(item, ParentNode);
        size++;
        BalanceToRoot(ParentNode);
    }

    void Remove(const T& item) {
        Node** CurrentNode = FindNode(item);

        if (CurrentNode == nullptr) {
            throw std::out_of_range("Элемент не найден");
        }

        Node* NodeToDelete = *CurrentNode;
        Node* ReplacementNode = nullptr;
        Node* BalanceStart = NodeToDelete->parent;

        if (NodeToDelete->left == nullptr) {
            ReplacementNode = NodeToDelete->right;
        }
        else if (NodeToDelete->right == nullptr) {
            ReplacementNode = NodeToDelete->left;
        }
        else {
            Node** MinNodePtr = FindMin(&(NodeToDelete->right));
            Node* MinNode = *MinNodePtr;

            // Начинаю с места фактического отсоединения следующего узла.
            BalanceStart = MinNode->parent == NodeToDelete ? MinNode : MinNode->parent;

            // Отсоединяю следующий узел от прежнего места.
            *MinNodePtr = MinNode->right;

            if (*MinNodePtr != nullptr) {
                (*MinNodePtr)->parent = MinNode->parent;
            }

            // Передаю ему детей удаляемого узла.
            MinNode->left = NodeToDelete->left;
            MinNode->left->parent = MinNode;

            MinNode->right = NodeToDelete->right;

            if (MinNode->right != nullptr) {
                MinNode->right->parent = MinNode;
            }

            ReplacementNode = MinNode;
        }

        *CurrentNode = ReplacementNode;

        if (ReplacementNode != nullptr) {
            ReplacementNode->parent = NodeToDelete->parent;
        }

        delete NodeToDelete;
        size--;
        BalanceToRoot(BalanceStart == nullptr ? ReplacementNode : BalanceStart);
    }

    void Clear() noexcept {
        DeleteSubtree(root);

        root = nullptr;
        size = 0;
    }

    const T& Get(std::size_t index) const override {
        if (index >= static_cast<std::size_t>(size)) {throw std::out_of_range("Индекс вне дерева");}

        TreeEnumerator it(*this);
        for (std::size_t number = 0; number < index; number++) {it.MoveNext();}
        return it.GetCurrent();
    }

    BinarySearchTree<T> Map(T (*mapper)(const T&)) const {
        if (mapper == nullptr) {throw std::invalid_argument("Функция Map не может быть пустой");}

        BinarySearchTree<T> result(compare);
        TreeEnumerator it(*this);

        while (it.HasNext()) {
            result.Insert(mapper(it.GetCurrent()));
            it.MoveNext();
        }

        return result;
    }

    BinarySearchTree<T> Where(bool (*where)(const T&)) const {
        if (where == nullptr) {throw std::invalid_argument("Функция Where не может быть пустой");}

        BinarySearchTree<T> result(compare);
        TreeEnumerator it(*this);

        while (it.HasNext()) {
            if (where(it.GetCurrent())) {result.Insert(it.GetCurrent());}
            it.MoveNext();
        }

        return result;
    }

    template <typename T2>
    T2 Reduce(T2 (*reduce_func)(const T2&, const T&), const T2& start_value) const {
        if (reduce_func == nullptr) {throw std::invalid_argument("Функция Reduce не может быть пустой");}

        T2 result = start_value;
        TreeEnumerator it(*this);

        while (it.HasNext()) {
            result = reduce_func(result, it.GetCurrent());
            it.MoveNext();
        }

        return result;
    }

    BinarySearchTree<T> GetSubtree(const T& item) const {
        const Node* CurrentNode = root;
        while (CurrentNode != nullptr) {
            if (Less(item, CurrentNode->data)) {CurrentNode = CurrentNode->left;}
            else if (Less(CurrentNode->data, item)) {CurrentNode = CurrentNode->right;}
            else {break;}
        }

        if (CurrentNode == nullptr) {throw std::out_of_range("Элемент не найден");}

        BinarySearchTree<T> result(compare);
        result.root = result.CopySubtree(CurrentNode);
        TreeEnumerator it(result);
        while (it.HasNext()) {result.size++; it.MoveNext();}
        return result;
    }

    bool ContainsSubtree(const BinarySearchTree<T>& other) const {
        if (other.IsEmpty()) {return true;}

        const Node* CurrentNode = root;
        while (CurrentNode != nullptr) {
            if (Less(other.root->data, CurrentNode->data)) {CurrentNode = CurrentNode->left;}
            else if (Less(CurrentNode->data, other.root->data)) {CurrentNode = CurrentNode->right;}
            else {return EqualSubtrees(CurrentNode, other.root);}
        }

        return false;
    }

    bool operator==(const BinarySearchTree<T>& other) const {
        return size == other.size && EqualSubtrees(root, other.root);
    }

    bool operator!=(const BinarySearchTree<T>& other) const {return !(*this == other);}

    BinarySearchTree<T> operator+(const BinarySearchTree<T>& other) const {
        BinarySearchTree<T> result(compare);
        TreeEnumerator first(*this);
        TreeEnumerator second(other);

        while (first.HasNext()) {
            result.Insert(first.GetCurrent());
            first.MoveNext();
        }
        while (second.HasNext()) {
            if (!result.Contains(second.GetCurrent())) {result.Insert(second.GetCurrent());}
            second.MoveNext();
        }

        return result;
    }

    std::string ToString(const std::string& format = "{К}(Л)[П]") const {
        FormatPart parts[3];
        ReadFormat(format, parts);
        std::ostringstream stream;
        WriteSubtree(root, parts, stream);
        return stream.str();
    }

    void Print(std::ostream& stream, int MaxNodes = 60) const {
        int remaining = MaxNodes;
        PrintSubtree(root, stream, 0, 'K', remaining);
        if (remaining == 0) {stream << "... ограничение объёма вывода\n";}
    }

    void FromString(const std::string& text) {
        BinarySearchTree<T> result(compare);
        std::size_t position = 0;
        result.root = result.ReadNode(text, position, nullptr);
        Node* CurrentNode = result.root;
        bool ReadLeft = true;

        while (CurrentNode != nullptr) {
            if (ReadLeft) {
                ExpectSymbol(text, position, '(');
                CurrentNode->left = result.ReadNode(text, position, CurrentNode);
                if (CurrentNode->left != nullptr) {CurrentNode = CurrentNode->left; continue;}
                ExpectSymbol(text, position, ')');
            }

            ExpectSymbol(text, position, '[');
            CurrentNode->right = result.ReadNode(text, position, CurrentNode);
            if (CurrentNode->right != nullptr) {CurrentNode = CurrentNode->right; ReadLeft = true; continue;}
            ExpectSymbol(text, position, ']');

            // После обработки обоих детей вычисляю высоту и возвращаюсь к родителю.
            while (CurrentNode != nullptr) {
                result.UpdateHeight(CurrentNode);
                int BalanceValue = result.GetHeight(CurrentNode->left) - result.GetHeight(CurrentNode->right);
                if (BalanceValue > 1 || BalanceValue < -1) {throw std::invalid_argument("Дерево не сбалансировано по AVL");}

                Node* ParentNode = CurrentNode->parent;
                if (ParentNode == nullptr) {CurrentNode = nullptr; break;}

                bool IsLeftChild = CurrentNode == ParentNode->left;
                ExpectSymbol(text, position, IsLeftChild ? ')' : ']');
                CurrentNode = ParentNode;
                if (IsLeftChild) {ReadLeft = false; break;}
            }
        }

        if (position != text.size()) {throw std::invalid_argument("Лишние данные после дерева");}
        TreeEnumerator it(result);
        const T* PreviousValue = nullptr;
        while (it.HasNext()) {
            if (PreviousValue != nullptr && !Less(*PreviousValue, it.GetCurrent())) {
                throw std::invalid_argument("Нарушен порядок дерева поиска или повторяются элементы");
            }
            PreviousValue = &it.GetCurrent();
            it.MoveNext();
        }

        *this = std::move(result);
    }

    IEnumerator<T>* GetEnumerator() const override {
        return new TreeEnumerator(*this);
    }
};

#endif // BINARY_SEARCH_TREE_HPP
