# pragma once
#include "iterator.hpp"
#include <memory_resource>
#include <cstddef>
#include <stdexcept>
#include <iostream>

template<typename T>
class MyList {
private:
    using Allocator = std::pmr::polymorphic_allocator<Node>;
    Node* head;
    Node* tail;
    size_t _size;
    Allocator _alloc;

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;


    explicit MyList(std::pmr::memory_resource* upstream = nullptr)
        : head(nullptr), tail(nullptr), _size(0),
          _alloc(upstream ? upstream : std::pmr::new_delete_resource()) {}

    ~MyList() {
        clear();
    }

    MyList(const MyList& other)
        : MyList(other._alloc.resource()) {
        for (const auto& item : other) {
            push_back(item);
        }
    }

    MyList& operator=(const MyList& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    void push_back(const T& value);
    void push_front(const T& value);
    void pop_back();
    void pop_front();

    size_type size() const { return _size; }
    bool empty() const { return _size == 0; }

    void clear();

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }
    void print() const;
};

template<typename T>
void MyList<T>::push_back(const T& value) {
    Node* new_node = _alloc.allocate(1);
    try {
        _alloc.construct(new_node, value, nullptr, tail);
        if (tail) {
            tail->next = new_node;
        } else {
            head = new_node;
        }
        tail = new_node;
        ++_size;
    } catch (...) {
        _alloc.deallocate(new_node, 1);
        throw;
    }
}

template<typename T>
void MyList<T>::push_front(const T& value) {
    Node* new_node = _alloc.allocate(1);
    try {
        _alloc.construct(new_node, value, head, nullptr);
        if (head) {
            head->prev = new_node;
        } else {
            tail = new_node;
        }
        head = new_node;
        ++_size;
    } catch (...) {
        _alloc.deallocate(new_node, 1);
        throw;
    }
}

template<typename T>
void MyList<T>::pop_back() {
    if (empty()) throw std::out_of_range("empty list");
    Node* to_delete = tail;
    tail = tail->prev;
    if (tail) {
        tail->next = nullptr;
    } else {
        head = nullptr;
    }
    _alloc.destroy(to_delete);
    _alloc.deallocate(to_delete, 1);
    --_size;
}

template<typename T>
void MyList<T>::pop_front() {
    if (empty()) throw std::out_of_range("empty list");
    Node* to_delete = head;
    head = head->next;
    if (head) {
        head->prev = nullptr;
    } else {
        tail = nullptr;
    }
    _alloc.destroy(to_delete);
    _alloc.deallocate(to_delete, 1);
    --_size;
}

template<typename T>
void MyList<T>::clear() {
    while (!empty()) {
        pop_front();
    }
}

template<typename T>
void MyList<T>::print() const {
    for (const auto& x : *this) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}