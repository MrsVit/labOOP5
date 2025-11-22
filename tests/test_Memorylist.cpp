#include <gtest/gtest.h>
#include "..\head\Memorylist.hpp"
#include "..\head\list.hpp"

class PoolTest : public ::testing::Test {
protected:
    ListMemoryResource pool;
};

TEST_F(PoolTest, AllocateDeallocate) {
    void* p = pool.allocate(32, 8);
    ASSERT_NE(p, nullptr);
    pool.deallocate(p, 32, 8);
    EXPECT_EQ(pool.active_nodes(), 0u);
    EXPECT_EQ(pool.node_count(), 1u);
}

TEST_F(PoolTest, ReusesBlockAfterFree) {
    void* p1 = pool.allocate(64, 8);
    pool.deallocate(p1, 64, 8);

    void* p2 = pool.allocate(32, 8);  
    EXPECT_EQ(p1, p2);
}

TEST_F(PoolTest, DoubleFreeIsCaught) {
    void* p = pool.allocate(16, 8);
    pool.deallocate(p, 16, 8);

    // второй раз — ошибка
    EXPECT_THROW({
        pool.deallocate(p, 16, 8);
    }, std::logic_error);
}

TEST_F(PoolTest, ForeignPtrDeallocThrows) {
    auto* upstream = std::pmr::new_delete_resource();
    void* foreign = upstream->allocate(8, 1);

    // пул не знает про этот указатель — кидаем
    EXPECT_THROW(pool.deallocate(foreign, 8, 1), std::logic_error);
    upstream->deallocate(foreign, 8, 1);
}

TEST_F(PoolTest, StdPmrListWorks) {
    std::pmr::list<int> lst(&pool);
    lst.push_back(10);
    lst.push_back(20);
    lst.push_back(30);

    EXPECT_EQ(lst.size(), 3u);
    EXPECT_GT(pool.active_nodes(), 0u);

    lst.pop_back();
    EXPECT_EQ(pool.active_nodes(), 2u);

    lst.clear();
    EXPECT_EQ(pool.active_nodes(), 0u);
}

TEST(PoolDtorTest, DestructorFreesAllUpstream) {
    size_t dealloc_count = 0;

    struct SpyResource : std::pmr::memory_resource {
        size_t& counter;
        explicit SpyResource(size_t& c) : counter(c) {}

        void* do_allocate(size_t bytes, size_t) override {
            return ::operator new(bytes);
        }
        void do_deallocate(void* p, size_t, size_t) override {
            ++counter;
            ::operator delete(p);
        }
        bool do_is_equal(const memory_resource&) const noexcept override {
            return true;
        }
    };

    SpyResource spy(dealloc_count);
    {
        ListMemoryResource local_pool(&spy);
        local_pool.allocate(100, 8);
        local_pool.allocate(200, 8);
    }

    EXPECT_EQ(dealloc_count, 2u);
}


TEST_F(PoolTest, MyListWorks) {
    MyList<int> lst(&pool);

    EXPECT_EQ(lst.size(), 0u);
    EXPECT_TRUE(lst.empty());
    EXPECT_EQ(pool.active_nodes(), 0u);
    EXPECT_EQ(pool.node_count(), 0u);

    lst.push_back(10);
    EXPECT_EQ(lst.size(), 1u);
    EXPECT_FALSE(lst.empty());
    EXPECT_EQ(pool.active_nodes(), 1u);

    lst.push_front(5);
    EXPECT_EQ(lst.size(), 2u);
    EXPECT_EQ(pool.active_nodes(), 2u);
     lst.push_back(20);
    EXPECT_EQ(lst.size(), 3u);
    EXPECT_EQ(pool.active_nodes(), 3u);

    std::vector<int> expected = {5, 10, 20};
    std::vector<int> actual;
    for (const auto& x : lst) {
        actual.push_back(x);
    }
    EXPECT_EQ(actual, expected);

    lst.pop_front();  
    EXPECT_EQ(lst.size(), 2u);
    EXPECT_EQ(pool.active_nodes(), 2u);

    lst.pop_back();   
    EXPECT_EQ(lst.size(), 1u);
    EXPECT_EQ(pool.active_nodes(), 1u);

    EXPECT_EQ(*lst.begin(), 10);
       lst.clear();
    EXPECT_EQ(lst.size(), 0u);
    EXPECT_TRUE(lst.empty());
    EXPECT_EQ(pool.active_nodes(), 0u);
    EXPECT_EQ(pool.node_count(), 3u);  
    EXPECT_EQ(pool.total_allocated_bytes(), 3 * sizeof(Node<int>));
}

TEST_F(PoolTest, SharedPoolBetweenStdAndMyList) {
    std::pmr::list<int> std_lst(&pool);
    MyList<int> my_lst(&pool);

    std_lst.push_back(100);
    my_lst.push_back(200);

    EXPECT_EQ(pool.active_nodes(), 2u);  

    auto it1 = std_lst.begin();
    auto it2 = my_lst.begin();

    EXPECT_EQ(*it1, 100);
    EXPECT_EQ(*it2, 200);

    std_lst.clear();
    my_lst.clear();

    EXPECT_EQ(pool.active_nodes(), 0u);
}