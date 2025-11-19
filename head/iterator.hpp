#pragma once

#include <memory>
#include <iterator>


template<typename T>
class ListIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    explicit ListIterator(typename MyList<T>::Node* ptr = nullptr) : current(ptr) {}

    reference operator*() const { return current->data; }
    pointer operator->() const { return &current->data; }

    ListIterator& operator++() {
        current = current->next;
        return *this;
    }

    ListIterator operator++(int) {
        ListIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    ListIterator& operator--() {
        current = current->prev;
        return *this;
    }

    ListIterator operator--(int) {
        ListIterator tmp = *this;
        --(*this);
        return tmp;
    }

    bool operator==(const ListIterator& other) const {
        return current == other.current;
    }

    bool operator!=(const ListIterator& other) const {
        return !(*this == other);
    }

private:
    typename MyList<T>::Node* current;
};