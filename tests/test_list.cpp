#include <gtest/gtest.h>
#include "..\head\list.hpp"

template<typename T>
bool operator==(const MyList<T>& lhs, const MyList<T>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();
    while (it1 != lhs.end() && it2 != rhs.end()) {
        if (*it1 != *it2) return false;
        ++it1;
        ++it2;
    }
    return true;
}

// Тесты
TEST(MyListTest, DefaultConstructor) {
    MyList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListTest, PushBack) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 3u);
    int expected[] = {1, 2, 3};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, expected[i++]);
    }
}

TEST(MyListTest, PushFront) {
    MyList<int> list;
    list.push_front(3);
    list.push_front(2);
    list.push_front(1);

    EXPECT_EQ(list.size(), 3u);
    int expected[] = {1, 2, 3};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, expected[i++]);
    }
}

TEST(MyListTest, PushFrontAndBackMixed) {
    MyList<int> list;
    list.push_back(2);
    list.push_front(1);
    list.push_back(3);

    int expected[] = {1, 2, 3};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, expected[i++]);
    }
}

TEST(MyListTest, PopBack) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_back();
    EXPECT_EQ(list.size(), 2u);
    int expected[] = {1, 2};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, expected[i++]);
    }

    list.pop_back();
    list.pop_back();
    EXPECT_TRUE(list.empty());
}

TEST(MyListTest, PopFront) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_front();
    EXPECT_EQ(list.size(), 2u);
    int expected[] = {2, 3};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, expected[i++]);
    }

    list.pop_front();
    list.pop_front();
    EXPECT_TRUE(list.empty());
}

TEST(MyListTest, Clear) {
    MyList<std::string> list;
    list.push_back("a");
    list.push_back("b");
    list.push_back("c");

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListTest, CopyConstructor) {
    MyList<int> list1;
    list1.push_back(10);
    list1.push_back(20);
    list1.push_back(30);

    MyList<int> list2(list1);
    EXPECT_EQ(list1, list2);
    EXPECT_EQ(list2.size(), 3u);
}

TEST(MyListTest, CopyAssignment) {
    MyList<int> list1;
    list1.push_back(100);
    list1.push_back(200);

    MyList<int> list2;
    list2.push_back(1);
    list2 = list1;

    EXPECT_EQ(list1, list2);
    EXPECT_EQ(list2.size(), 2u);
}

TEST(MyListTest, SelfAssignment) {
    MyList<int> list;
    list.push_back(42);
    list = list;  // should not crash or corrupt
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(*list.begin(), 42);
}

TEST(MyListTest, EmptyListOperationsThrow) {
    MyList<int> list;

    EXPECT_THROW(list.pop_front(), std::out_of_range);
    EXPECT_THROW(list.pop_back(), std::out_of_range);
}

TEST(MyListTest, IteratorBeginEndEmpty) {
    MyList<int> list;
    EXPECT_EQ(list.begin(), list.end());
}

TEST(MyListTest, IteratorTraversal) {
    MyList<int> list;
    list.push_back(5);
    list.push_back(10);
    list.push_back(15);

    auto it = list.begin();
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(*it, 15);
    ++it;
    EXPECT_EQ(it, list.end());
}

// Дополнительно — проверка с custom memory_resource (опционально)
TEST(MyListTest, CustomAllocator) {
    std::pmr::monotonic_buffer_resource mbr{1024};
    MyList<int> list(&mbr);
    list.push_back(1);
    list.push_back(2);
    EXPECT_EQ(list.size(), 2u);
    // Проверим, что использует mbr (косвенно — не упало и работает)
    int arr[] = {1, 2};
    int i = 0;
    for (const auto& x : list) {
        EXPECT_EQ(x, arr[i++]);
    }
}