#pragma once

#include <list>
#include <memory_resource>
#include <iostream>
#include <algorithm>
#include <sstream>

class ListMemoryResource : public std::pmr::memory_resource {
private:
    struct NodeAllocation {
        void* ptr;
        size_t bytes;
        size_t alignment;
        bool used;

        NodeAllocation(void* p, size_t b, size_t align)
            : ptr(p), bytes(b), alignment(align), used(true) {}
    };

    std::list<NodeAllocation> _nodes;  
    std::pmr::memory_resource* _upstream;

protected:
    void* do_allocate(size_t bytes, size_t alignment) override;
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

public:
    explicit ListMemoryResource(std::pmr::memory_resource* upstream = nullptr);
    virtual ~ListMemoryResource();

    ListMemoryResource(const ListMemoryResource&) = delete;
    ListMemoryResource& operator=(const ListMemoryResource&) = delete;

    size_t node_count() const { return _nodes.size(); }
    size_t active_nodes() const;
    size_t total_allocated_bytes() const;
    void stats() const;
};

ListMemoryResource::ListMemoryResource(std::pmr::memory_resource* upstream)
    : _upstream(upstream ? upstream : std::pmr::new_delete_resource()) {
    std::cout << "ListMemoryResource created (upstream: " << _upstream << ")\n";
}

ListMemoryResource::~ListMemoryResource() {
    for (const auto& node : _nodes) {
        _upstream->deallocate(node.ptr, node.bytes, node.alignment);
    }
}

void* ListMemoryResource::do_allocate(size_t bytes, size_t alignment) { for (auto& node : _nodes) {
        if (!node.used && node.bytes >= bytes && node.alignment >= alignment) {
            node.used = true;
            return node.ptr;
        }
    }
    void* ptr = _upstream->allocate(bytes, alignment);
    _nodes.emplace_back(ptr, bytes, alignment);
    return ptr;
}

void ListMemoryResource::do_deallocate(void* p, size_t, size_t) {
    for (auto& node : _nodes) {
        if (node.ptr == p) {
            if (!node.used) {
                throw std::logic_error("Double deallocation");
            }
            node.used = false;
            return;
        }
    }
    throw std::logic_error("Deallocation of unknown pointer");
}

bool ListMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

size_t ListMemoryResource::active_nodes() const {
    return std::count_if(_nodes.begin(), _nodes.end(),
                         [](const NodeAllocation& n) { return n.used; });
}

size_t ListMemoryResource::total_allocated_bytes() const {
    size_t total = 0;
    for (const auto& node : _nodes) {
        total += node.bytes;
    }
    return total;
}

void ListMemoryResource::stats() const {
    size_t act = active_nodes();
    size_t total = _nodes.size();
    size_t total_bytes = total_allocated_bytes();
    std::cout << "Stats:\n"
              << "  Total nodes:      " << total << "\n"
              << "  Active nodes:     " << act << "\n"
              << "  Free nodes:       " << (total - act) << "\n"
              << "  Total allocated:  " << total_bytes << " bytes\n";
}