#include "..\head\Memorylist.hpp" 

#include <iostream>
#include <stdexcept>

int main() {
    ListMemoryResource pool;
    std::pmr::list<int> myList(&pool);

    myList.push_back(42);
    myList.push_back(100);

    std::cout << "List size: " << myList.size() << "\n";
    pool.stats();

    return 0;
} 