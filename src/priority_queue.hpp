#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *left;
        Node *right;
        int npl;

        Node(const T &val) : data(val), left(nullptr), right(nullptr), npl(0) {}
    };

    Node *root;
    Compare cmp;
    size_t sz;

    int getNpl(Node *node) const {
        return node ? node->npl : -1;
    }

    Node *copyNode(Node *node) {
        if (!node) return nullptr;
        Node *newNode = new Node(node->data);
        newNode->left = copyNode(node->left);
        newNode->right = copyNode(node->right);
        newNode->npl = node->npl;
        return newNode;
    }

    void clear(Node *node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    Node *merge(Node *a, Node *b) {
        if (!a) return b;
        if (!b) return a;

        if (cmp(a->data, b->data)) {
            std::swap(a, b);
        }

        a->right = merge(a->right, b);

        if (getNpl(a->left) < getNpl(a->right)) {
            std::swap(a->left, a->right);
        }

        a->npl = getNpl(a->right) + 1;
        return a;
    }

    Node *mergeCopy(Node *a, Node *b) {
        if (!a) return copyNode(b);
        if (!b) return copyNode(a);

        Node *newNode;
        if (cmp(a->data, b->data)) {
            newNode = new Node(b->data);
            newNode->left = copyNode(b->left);
            try {
                newNode->right = mergeCopy(a, b->right);
            } catch (...) {
                clear(newNode->left);
                delete newNode;
                throw;
            }
        } else {
            newNode = new Node(a->data);
            newNode->left = copyNode(a->left);
            try {
                newNode->right = mergeCopy(a->right, b);
            } catch (...) {
                clear(newNode->left);
                delete newNode;
                throw;
            }
        }

        if (getNpl(newNode->left) < getNpl(newNode->right)) {
            std::swap(newNode->left, newNode->right);
        }

        newNode->npl = getNpl(newNode->right) + 1;
        return newNode;
    }

public:
    priority_queue() : root(nullptr), sz(0) {}

    priority_queue(const priority_queue &other) : root(copyNode(other.root)), sz(other.sz) {}

    ~priority_queue() {
        clear(root);
    }

    priority_queue &operator=(const priority_queue &other) {
        if (this == &other) return *this;

        Node *newRoot = copyNode(other.root);
        clear(root);
        root = newRoot;
        sz = other.sz;
        return *this;
    }

    const T &top() const {
        if (empty()) throw container_is_empty();
        return root->data;
    }

    void push(const T &e) {
        Node *newNode = new Node(e);
        root = merge(root, newNode);
        sz++;
    }

    void pop() {
        if (empty()) throw container_is_empty();

        Node *newRoot = merge(root->left, root->right);
        delete root;
        root = newRoot;
        sz--;
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    void merge(priority_queue &other) {
        if (this == &other) return;

        Node *newRoot = mergeCopy(root, other.root);
        size_t newSz = sz + other.sz;

        clear(root);
        clear(other.root);
        root = newRoot;
        sz = newSz;
        other.root = nullptr;
        other.sz = 0;
    }
};

}

#endif
