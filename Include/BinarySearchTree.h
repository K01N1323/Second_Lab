#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdexcept>
#include "Sequences/IEnumerator.h"

template <class T>
class BinarySearchTree {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;

        explicit Node(const T& value): data(value) {
            left = nullptr;
            right = nullptr;
        }
    };

    Node* root;
    int size;
    bool (*compare)(const T&, const T&);


    Node** FindNode(const T& item) const{
        Node** CurrentNode = &root;

        while (*CurrentNode != nullptr) {
            Node* node = *CurrentNode;
            if (Compare(item, node->data)) {
                CurrentNode = &(node->left);
            } 
            else if (Compare(node->data, item)) {
                CurrentNode = &(node->right);
            } 
            else {
                return &(node);
        }

    }

        return nullptr;
    }
    Node** FindMin(Node* node) const{
        if (root = nullptr) {return nullptr;}

        Node* CurrentNode = root;
        while (CurrentNode->left != nullptr){
            CurrentNode = CurrentNode->left;
        }

        return CurrentNode;
    }

    Node* CopySubtree(const Node* node){
        if (node == nullptr) {return nullptr;}

        Node* NewNode = new Node(node->value);

        NewNode->left = CopySubtree(node->left);

        NewNode->right = CopySubtree(node->right);

        return NewNode;
    }
    void DeleteSubtree(Node* node) noexcept{
        if (node == nullptr) {return;}

        DeleteSubtree(node->left);

        DeleteSubtree(node->right);

        delete node;
    }

    class TreeEnumerator;

public:
    explicit BinarySearchTree(bool (*comparator)(const T&, const T&)): root(nullptr), size(0), compare(comparator){
        if (compare == nullptr) {throw std::invalid_argument("Компаратор не может быть пустым");}
    }

    BinarySearchTree(const BinarySearchTree<T>& other){
        compare = other.compare;
        root = CopySubtree(other.root);
        size = other.size;
    }
    BinarySearchTree(BinarySearchTree<T>&& other) noexcept: root(other.root), size(other.size), compare(other.compare){
        other.root = nullptr;
        other.size = 0;
    }

    BinarySearchTree<T>& operator=(const BinarySearchTree<T>& other){
        if (this == &other) {return *this;}

        Node* NewRoot = CopySubtree;
        
        DeleteSubtree(root);

        root = NewRoot;
        size = other.size;
        compare = other.compare;

        return *this;
    }

    BinarySearchTree<T>& operator=(BinarySearchTree<T>&& other) noexcept{
        if (this == &other) {return *this;}

        DeleteSubtree(root);

        root = other.root;
        size = other.size
        compare = other.compare;

        other.root = nullptr;
        other.size = 0;
    }

    ~BinarySearchTree() {this->Clear();}

    int GetCount() const {return size;}
    bool IsEmpty() const {return size==0;}

    bool Contains(const T& item) const { return (this->Find(item) != nullptr)}

    const T* Find(const T& item) const{
        Node** CurrentNode = &root;

        while (*CurrentNode != nullptr) {
            const Node* node = *CurrentNode;

            if (Compare(item, node->data)) {
                CurrentNode = &(node->left);
            } 
            else if (Compare(node->data, item)) {
                CurrentNode = &(node->right);
            } 
            else {
                return &(node->data);
        }
    }

        return nullptr;
    }

    T* Find(const T& item){
        Node** CurrentNode = &root;

        while (*CurrentNode != nullptr) {
            Node* node = *CurrentNode;

            if (Compare(item, node->data)) {
                CurrentNode = &(node->left);
            } 
            else if (Compare(node->data, item)) {
                CurrentNode = &(node->right);
            } 
            else {
                return &(node->data);
        }
    }

        return nullptr;
    }
    

    void Insert(const T& item){
        Node** CurrentNode = root;

        while (*CurrentNode != nullptr;){
            Node* node = *CurrentNode;

            if (compare(item, node->data)) {
                CurrentNode = &(node->left);
            }
            else if (compare(node->data, item)) {
                CurrentNode = &(node->right;)
            }
            else {
                throw std::invalid_argument("Данные с таким ключом уже содержатся в дереве";)
            }
        }

        (*CurrentNode) = new Node(item);
        size++;
    }

    void Remove(const T& item){
        Node** CurrentNode = FindNode(item);

        if (*CurrentNode = nullptr) {return;}

        Node* NodeToDelete = (*CurrentNode);

        if (NodeToDelete->left = nullptr){
            (*CurrentNode) = NodeToDelete->right;
            delete NodeToDelete;
            size--;
        }

        else if (NodeTodelete->right = nullptr){
            (*CurrentNode) = NodeToDelete->left;
            delete NodeToDelete;
            size--
        }

        else{
            Node** MinNodePtr = FindMin(NodeToDelete);

            Node* MinNode = *MinNodePtr;

            NodeToDelete->data = MinNode->data;

            delete MinNode;
            size--;
        }

        
    }

    void Clear() noexcept{
        DeleteSubtree(root);

        rooot = nullptr;
        size = 0;
    }

    IEnumerator<T>* GetEnumerator() const;
};

#endif // BINARY_SEARCH_TREE_H
