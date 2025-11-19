#include "..\head\Memorylist.hpp" 

#include <iostream>
#include <stdexcept>

ListMemoryResource::ListMemoryResource(std::pmr::memory_resource* upstream)
    : _upstream(upstream ? upstream : std::pmr::new_delete_resource()) {
    std::cout << "ListMemoryResource created (upstream: " << _upstream << ")\n";
}

ListMemoryResource::~ListMemoryResource() {
    for (const auto& node : _nodes) {
         _upstream->deallocate(node.ptr, node.requested_bytes, node.alignment);
    }
}

void* ListMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    for (auto& node : _nodes) {
        if (!node.in_use
            && node.requested_bytes >= bytes
            && node.alignment >= alignment) {
            node.in_use = true;
            return node.ptr;
        }
    }

    void* ptr = _upstream->allocate(bytes, alignment);
    _nodes.emplace_back(ptr, bytes, alignment);
    return ptr;
}

void ListMemoryResource::do_deallocate(void* p, size_t bytes, size_t alignment) {
    for (auto& node : _nodes) {
        if (node.ptr == p) {
            if (!node.in_use) {
                throw std::logic_error("Double deallocation detected for node at " + std::to_string(reinterpret_cast<uintptr_t>(p)));
            }
            if (node.requested_bytes < bytes || node.alignment < alignment) {
                std::cout << "WARNING: deallocate"<< std::endl;
            }
            node.in_use = false;
            return;
        }
    }
    throw std::logic_error("Deallocation of unknown node at " + std::to_string(reinterpret_cast<uintptr_t>(p)));
}

bool ListMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

// для анализа
size_t ListMemoryResource::active_nodes() const {
    return std::count_if(_nodes.begin(), _nodes.end(),
                         [](const NodeAllocation& n) { return n.in_use; });
}

size_t ListMemoryResource::total_allocated_bytes() const {
    size_t total = 0;
    for (const auto& node : _nodes) {
        total += node.requested_bytes;
    }
    return total;
}

void ListMemoryResource::stats() const {
    auto active = active_nodes();
    auto total = _nodes.size();
    auto total_bytes = total_allocated_bytes();
    std::cout << "[ListMemoryResource] Stats:\n"
              << "  Total nodes:      " << total << "\n"
              << "  Active nodes:     " << active << "\n"
              << "  Free nodes:       " << (total - active) << "\n"
              << "  Total allocated:  " << total_bytes << " bytes\n";
}
/* было тут ради проверки
int main() {
    ListMemoryResource pool;
    std::pmr::list<int> myList(&pool);

    myList.push_back(42);
    myList.push_back(100);

    std::cout << "List size: " << myList.size() << "\n";
    pool.stats();

    return 0;
} *\