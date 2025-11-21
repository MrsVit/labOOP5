#include <gtest/gtest.h>
#include "..\head\list.hpp"

TEST(MyListBasicTest, CreateEmptyList) {
    MyList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListBasicTest, PushBackWorks) {
    MyList<int> list;
    list.push_back(1);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1u);

    list.push_back(2);
    list.push_back(3);
    EXPECT_EQ(list.size(), 3u);
    EXPECT_FALSE(list.empty());
}

TEST(MyListBasicTest, PushFrontWorks) {
    MyList<int> list;
    list.push_front(3);
    list.push_front(2);
    list.push_front(1);

    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(MyListBasicTest, PopBackReducesSize) {
    MyList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    list.pop_back();
    EXPECT_EQ(list.size(), 2u);
    EXPECT_EQ(*list.begin(), 10);
}

TEST(MyListBasicTest, PopFrontReducesSizeAndShifts) {
    MyList<int> list;
    list.push_back(100);
    list.push_back(200);
    list.push_back(300);

    list.pop_front();
    EXPECT_EQ(list.size(), 2u);
    EXPECT_EQ(*list.begin(), 200);

    list.pop_front();
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(*list.begin(), 300);

    list.pop_front();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListBasicTest, ClearWorks) {
    MyList<std::string> list;
    list.push_back("one");
    list.push_back("two");

    EXPECT_EQ(list.size(), 2u);
    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListBasicTest, CopyCtorCopiesData) {
    MyList<int> orig;
    orig.push_back(5);
    orig.push_back(6);

    MyList<int> copy(orig);
    EXPECT_EQ(copy.size(), 2u);
    auto it = copy.begin();
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 6);
    ++it;
    EXPECT_EQ(it, copy.end());
}

TEST(MyListBasicTest, AssignmentOperatorWorks) {
    MyList<int> src;
    src.push_back(77);
    src.push_back(88);

    MyList<int> dst;
    dst.push_back(1);
    dst.push_back(2);
    dst.push_back(3);

    dst = src;
    EXPECT_EQ(dst.size(), 2u);
    auto it = dst.begin();
    EXPECT_EQ(*it, 77);
    ++it;
    EXPECT_EQ(*it, 88);
    ++it;
    EXPECT_EQ(it, dst.end());
}

TEST(MyListBasicTest, SelfAssignDoesNotBreak) {
    MyList<int> list;
    list.push_back(999);
    list = list;
    EXPECT_EQ(list.size(), 1u);
}

TEST(MyListBasicTest, WorksOnInt) {
    MyList<int> xs;
    xs.push_back(100);
    xs.push_back(200);
    EXPECT_EQ(xs.size(), 2u);
    EXPECT_FALSE(xs.empty());
}

struct Person {
    std::string name;
    int age;
    Person(const std::string& n = "", int a = 0) : name(n), age(a) {}
    bool operator==(const Person& o) const { return name == o.name && age == o.age; }
};

TEST(MyListBasicTest, WorksOnCustomType) {
    MyList<Person> ppl;
    ppl.push_back(Person("Ivan", 25));
    ppl.push_back(Person("Anna", 30));

    EXPECT_EQ(ppl.size(), 2u);

    auto it = ppl.begin();
    EXPECT_EQ(it->name, "Ivan");
    EXPECT_EQ(it->age, 25);
    ++it;
    EXPECT_EQ(it->name, "Anna");
    EXPECT_EQ(it->age, 30);
    ++it;
    EXPECT_EQ(it, ppl.end());
}

TEST(MyListBasicTest, RangeForWorks) {
    MyList<int> list;
    list.push_back(3);
    list.push_back(7);

    int total = 0;
    for (auto x : list) {
        total += x;
    }
    EXPECT_EQ(total, 10);
}

TEST(MyListBasicTest, IterManualLoopWorks) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);

    int i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (i == 0) EXPECT_EQ(*it, 1);
        if (i == 1) EXPECT_EQ(*it, 2);
        ++i;
    }
}

TEST(MyListBasicTest, PopEmptyThrows) {
    MyList<int> list;
    EXPECT_THROW(list.pop_front(), std::out_of_range);
}

TEST(MyListBasicTest, PopBackOnEmptyAlsoThrows) {
    MyList<int> list;
    EXPECT_THROW(list.pop_back(), std::out_of_range);
}