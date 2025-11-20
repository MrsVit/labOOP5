#include <gtest/gtest.h>
#include "..\head\Memorylist.hpp"  

class PoolTest : public ::testing::Test {
protected:
    ListMemoryResource pool;
};

TEST_F(PoolTest, AllocateDeallocate) {
    void* p = pool.allocate(32, 8);
    EXPECT_NE(p, nullptr);
    pool.deallocate(p, 32, 8);
    EXPECT_EQ(pool.active_nodes(), 0u);
    EXPECT_EQ(pool.node_count(), 1u);
}

TEST_F(PoolTest, ReuseFreedBlock) {
    void* p1 = pool.allocate(32, 8);
    pool.deallocate(p1, 32, 8);

    void* p2 = pool.allocate(32, 8);
    EXPECT_EQ(p1, p2);  
}

TEST_F(PoolTest, DoubleDeallocateThrows) {
    void* p = pool.allocate(16, 8);
    pool.deallocate(p, 16, 8);

    EXPECT_THROW(
        pool.deallocate(p, 16, 8),
        std::logic_error
    );
}

TEST_F(PoolTest, DeallocateUnknownPtrThrows) {
    auto* upstream = std::pmr::new_delete_resource();
    void* foreign = upstream->allocate(16, 8);

    EXPECT_THROW(
        pool.deallocate(foreign, 16, 8),
        std::logic_error
    );

    upstream->deallocate(foreign, 16, 8); 
}

TEST_F(PoolTest, WorksWithPmrList) {
    std::pmr::list<int> lst(&pool);
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);

    EXPECT_EQ(lst.size(), 3u);
    EXPECT_GT(pool.active_nodes(), 0u); 
    lst.clear();
    EXPECT_EQ(pool.active_nodes(), 0u);
}

TEST(PoolDtorTest, CleansUpUpstream) {
    size_t dealloc_count = 0;
    struct CountingResource : std::pmr::memory_resource {
        size_t& count;
        explicit CountingResource(size_t& c) : count(c) {}
        void* do_allocate(size_t bytes, size_t) override {
            return ::operator new(bytes);
        }
        void do_deallocate(void* p, size_t, size_t) override {
            ++count;
            ::operator delete(p);
        }
        bool do_is_equal(const memory_resource&) const noexcept override {
            return true;
        }
    };

    CountingResource counter(dealloc_count);
    {
        ListMemoryResource pool(&counter);
        pool.allocate(10, 1);
        pool.allocate(20, 1);
    }

    EXPECT_EQ(dealloc_count, 2u);
}