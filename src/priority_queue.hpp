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
        int refCount;

        Node(const T &val) : data(val), left(nullptr), right(nullptr), npl(0), refCount(1) {}
    };

    Node *root;
    Compare cmp;
    size_t sz;

    int getNpl(Node *node) const {
        return node ? node->npl : -1;
    }

    void retain(Node *node) {
        if (node) node->refCount++;
    }

    void release(Node *node) {
        if (!node) return;
        node->refCount--;
        if (node->refCount == 0) {
            release(node->left);
            release(node->right);
            delete node;
        }
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

        Node *newNode = new Node(a->data);
        newNode->left = a->left;
        retain(newNode->left);

        try {
            newNode->right = merge(a->right, b);
        } catch (...) {
            release(newNode->left);
            delete newNode;
            throw;
        }

        if (getNpl(newNode->left) < getNpl(newNode->right)) {
            std::swap(newNode->left, newNode->right);
        }

        newNode->npl = getNpl(newNode->right) + 1;

        a->left = nullptr;
        a->right = nullptr;
        release(a);
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

        Node *left = root->left;
        Node *right = root->right;
        Node *oldRoot = root;

        try {
            root = merge(left, right);
            sz--;
            delete oldRoot;
        } catch (...) {
            root = oldRoot;
            throw runtime_error();
        }
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    void merge(priority_queue &other) {
        if (this == &other) return;

        Node *oldRoot1 = root;
        Node *oldRoot2 = other.root;
        size_t oldSz1 = sz;
        size_t oldSz2 = other.sz;

        try {
            root = merge(root, other.root);
            sz += other.sz;
            other.root = nullptr;
            other.sz = 0;
        } catch (...) {
            root = oldRoot1;
            other.root = oldRoot2;
            sz = oldSz1;
            other.sz = oldSz2;
            throw runtime_error();
        }
    }
};

}

#endif
