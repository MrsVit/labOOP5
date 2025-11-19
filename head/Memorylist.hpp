#pragma once

#include <list>
#include <memory_resource>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstddef>

class ListMemoryResource : public std::pmr::memory_resource {
private:
    struct NodeAllocation {
        void* ptr;
        size_t requested_bytes;   
        size_t alignment;
        size_t element_size;     
        bool in_use;

        NodeAllocation(void* p, size_t bytes, size_t align, size_t elem_sz = 0)
            : ptr(p), requested_bytes(bytes), alignment(align), element_size(elem_sz), in_use(true) {}
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
    //запрет копирования и перемещений
    ListMemoryResource(const ListMemoryResource&) = delete;
    ListMemoryResource& operator=(const ListMemoryResource&) = delete;

    size_t node_count() const { return _nodes.size(); }
    size_t active_nodes() const;
    size_t total_allocated_bytes() const;
    void stats() const;
};