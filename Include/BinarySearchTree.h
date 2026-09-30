#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <limits>
#include <stdexcept>
#include "Sequences/IEnumerator.h"

template <class T>
class BinarySearchTree {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        Node* parent;

        explicit Node(const T& value, Node* ParentNode = nullptr): data(value), left(nullptr), right(nullptr), parent(ParentNode) {}
    };

    Node* root;
    int size;
    bool (*compare)(const T&, const T&);

    Node** FindNode(const T& item) {
        Node** CurrentNode = &root;

        while (*CurrentNode != nullptr) {
            Node* node = *CurrentNode;

            if (compare(item, node->data)) {
                CurrentNode = &(node->left);
            }
            else if (compare(node->data, item)) {
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
        const Node* SourceNode = node;
        Node* TargetNode = NewRoot;

        try {
            while (true) {
                if (SourceNode->left != nullptr && TargetNode->left == nullptr) {
                    TargetNode->left = new Node(SourceNode->left->data, TargetNode);

                    SourceNode = SourceNode->left;
                    TargetNode = TargetNode->left;
                }
                else if (SourceNode->right != nullptr && TargetNode->right == nullptr) {
                    TargetNode->right = new Node(SourceNode->right->data, TargetNode);

                    SourceNode = SourceNode->right;
                    TargetNode = TargetNode->right;
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

    ~BinarySearchTree() {
        this->Clear();
    }

    int GetCount() const {return size;}
    bool IsEmpty() const {return size == 0;}
    bool Contains(const T& item) const {return this->Find(item) != nullptr;}

    const T* Find(const T& item) const {
        const Node* CurrentNode = root;

        while (CurrentNode != nullptr) {
            if (compare(item, CurrentNode->data)) {
                CurrentNode = CurrentNode->left;
            }
            else if (compare(CurrentNode->data, item)) {
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
            if (compare(item, CurrentNode->data)) {
                CurrentNode = CurrentNode->left;
            }
            else if (compare(CurrentNode->data, item)) {
                CurrentNode = CurrentNode->right;
            }
            else {
                return &(CurrentNode->data);
            }
        }

        return nullptr;
    }

    void Insert(const T& item) {
        Node** CurrentNode = &root;
        Node* ParentNode = nullptr;

        while (*CurrentNode != nullptr) {
            ParentNode = *CurrentNode;

            if (compare(item, ParentNode->data)) {
                CurrentNode = &(ParentNode->left);
            }
            else if (compare(ParentNode->data, item)) {
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
    }

    void Remove(const T& item) {
        Node** CurrentNode = FindNode(item);

        if (CurrentNode == nullptr) {
            throw std::out_of_range("Элемент не найден");
        }

        Node* NodeToDelete = *CurrentNode;
        Node* ReplacementNode = nullptr;

        if (NodeToDelete->left == nullptr) {
            ReplacementNode = NodeToDelete->right;
        }
        else if (NodeToDelete->right == nullptr) {
            ReplacementNode = NodeToDelete->left;
        }
        else {
            Node** MinNodePtr = FindMin(&(NodeToDelete->right));
            Node* MinNode = *MinNodePtr;

            // Отсоединяем следующий узел от прежнего места.
            *MinNodePtr = MinNode->right;

            if (*MinNodePtr != nullptr) {
                (*MinNodePtr)->parent = MinNode->parent;
            }

            // Передаём ему детей удаляемого узла.
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
    }

    void Clear() noexcept {
        DeleteSubtree(root);

        root = nullptr;
        size = 0;
    }

    IEnumerator<T>* GetEnumerator() const {
        return new TreeEnumerator(*this);
    }
};

#endif // BINARY_SEARCH_TREE_H