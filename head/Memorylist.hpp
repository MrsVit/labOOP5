#pragma once

#include <list>
#include <memory_resource>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <sstream>

class ListMemoryResource : public std::pmr::memory_resource {
private:
    struct NodeAllocation {
        void* ptr;
        size_t byt;        size_t alignment;
        size_t el_size;
        bool use;

        NodeAllocation(void* p, size_t bytes, size_t align, size_t elem_sz = 0)
            : ptr(p), byt(bytes), alignment(align), el_size(elem_sz), use(true) {}
    };

    std::vector<NodeAllocation> _nodes;
    std::pmr::memory_resource* _upstream;

protected:
    void* do_allocate(size_t bytes, size_t alignment) override;
        void do_deallocate(void* p, size_t bytes, size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

public:
    virtual ~ListMemoryResource();
    explicit ListMemoryResource(std::pmr::memory_resource* upstream = nullptr);
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
        _upstream->deallocate(node.ptr, node.byt, node.alignment);
    }
}

void* ListMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    for (auto& node : _nodes) {
        if (!node.use  && node.byt >= bytes && node.alignment >= alignment) {
            node.use = true;
            return node.ptr;}
    }
    void* ptr = _upstream->allocate(bytes, alignment);
    _nodes.emplace_back(ptr, bytes, alignment);
    return ptr;
}

void ListMemoryResource::do_deallocate(void* p, size_t bytes, size_t alignment) {
    for (auto& node : _nodes) {
        if (node.ptr == p) {
            if (!node.use) {
                std::ostringstream os;
                os << "Double deallocation";
                throw std::logic_error(os.str());
            }
            node.use = false;
            return;
        }
    }

    std::ostringstream os;
    os << "Deallocation of unknown";
    throw std::logic_error(os.str());
}

bool ListMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

size_t ListMemoryResource::active_nodes() const {
    size_t cnt = 0;
    for (const auto& n : _nodes) {
        if (n.use) ++cnt;
    }
    return cnt;
}

size_t ListMemoryResource::total_allocated_bytes() const {
    size_t total = 0;
    for (const auto& node : _nodes) {
        total += node.byt;
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