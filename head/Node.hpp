# pragma once
template<typename T>
struct Node {
        T data;
        Node* next;
        Node* prev;

        Node(const T& d, Node* n = nullptr, Node* p = nullptr)
            : data(d), next(n), prev(p) {}
    };