#include <gtest/gtest.h>
#include "..\head\list.hpp" 
TEST(MyListBasicTest, CreateAndAddElements) {
    MyList<int> list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
    list.push_back(1);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1u);

    list.push_back(2);
    list.push_back(3);
    EXPECT_EQ(list.size(), 3u);
}

TEST(MyListBasicTest, PushFrontOrder) {
    MyList<int> list;
    list.push_front(3);
    list.push_front(2);
    list.push_front(1);

    auto it = list.begin();
    EXPECT_EQ(*it, 1); ++it;
    EXPECT_EQ(*it, 2); ++it;
    EXPECT_EQ(*it, 3); ++it;
    EXPECT_EQ(it, list.end());
}

TEST(MyListBasicTest, PopOperations) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_back();
    EXPECT_EQ(list.size(), 2u);

    list.pop_front();
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(*list.begin(), 2);

    list.pop_front();
    EXPECT_TRUE(list.empty());
}
TEST(MyListBasicTest, Clear) {
    MyList<std::string> list;
    list.push_back("hello");
    list.push_back("world");
    EXPECT_EQ(list.size(), 2u);

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0u);
}

TEST(MyListBasicTest, CopyConstructor) {
    MyList<int> original;
    original.push_back(10);
    original.push_back(20);

    MyList<int> copy = original;
 EXPECT_EQ(copy.size(), 2u);
    auto it = copy.begin();
    EXPECT_EQ(*it, 10); ++it;
    EXPECT_EQ(*it, 20); ++it;
    EXPECT_EQ(it, copy.end());
}
TEST(MyListBasicTest, CopyAssignment) {
    MyList<int> source;
    source.push_back(100);
    source.push_back(200);

    MyList<int> target;
    target.push_back(1); 
    target = source; 

    EXPECT_EQ(target.size(), 2u);
    auto it = target.begin();
    EXPECT_EQ(*it, 100); ++it;
    EXPECT_EQ(*it, 200); ++it;
    EXPECT_EQ(it, target.end());
}

TEST(MyListBasicTest, SelfAssignment) {
    MyList<int> list;
    list.push_back(42);

    list = list; 
    EXPECT_EQ(list.size(), 1u);
    EXPECT_EQ(*list.begin(), 42);
}

TEST(MyListBasicTest, WorksWithInt) {
    MyList<int> list;
    list.push_back(5);
    list.push_back(10);
    EXPECT_EQ(list.size(), 2u);
}

struct Person {
    std::string name;
    int age;

    Person(const std::string& n = "", int a = 0) : name(n), age(a) {}

    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};

TEST(MyListBasicTest, WorksWithComplexType) {
    MyList<Person> people;
    people.push_back(Person("Alice", 30));
    people.push_back(Person("Bob", 25));
   EXPECT_EQ(people.size(), 2u);    auto it = people.begin();
    EXPECT_EQ(it->name, "Alice");
    EXPECT_EQ(it->age, 30); ++it;

    EXPECT_EQ(it->name, "Bob");
    EXPECT_EQ(it->age, 25); ++it;
    EXPECT_EQ(it, people.end());
}

TEST(MyListBasicTest, IteratorsWork) {
    MyList<int> list;
    list.push_back(1);
    list.push_back(2);

    int sum = 0;
    for (const auto& x : list) {
        sum += x;
    }
    EXPECT_EQ(sum, 3);
}

TEST(MyListBasicTest, PopFromEmptyThrows) {
    MyList<int> list;
    EXPECT_THROW(list.pop_front(), std::out_of_range);
    EXPECT_THROW(list.pop_back(), std::out_of_range);
}