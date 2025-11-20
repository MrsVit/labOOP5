#include <gtest/gtest.h>
#include "..\head\Memorylist.hpp"  // ← ваш заголовок

// Вспомогательная функция для создания/очистки блоков
struct Allocation {
    void* ptr;
    size_t bytes;
    size_t alignment;

    Allocation(void* p, size_t b, size_t a) : ptr(p), bytes(b), alignment(a) {}
};

class ListMemoryResourceTest : public ::testing::Test {
protected:
    // Для чистоты — отдельный upstream ресурс (можно и nullptr)
    std::pmr::synchronized_pool_resource upstream_pool;
    ListMemoryResource resource{&upstream_pool};

    // Утилита: выделить и сразу записать уникальный ID (для проверки перезаписи)
    Allocation allocate_with_id(size_t bytes, size_t alignment, int id) {
        void* p = resource.allocate(bytes, alignment);
        if (bytes >= sizeof(int)) {
            *static_cast<int*>(p) = id;
        }
        return Allocation{p, bytes, alignment};
    }

    void deallocate(Allocation& a) {
        resource.deallocate(a.ptr, a.bytes, a.alignment);
        a.ptr = nullptr;
    }
};

// 1. Базовое выделение и освобождение
TEST_F(ListMemoryResourceTest, AllocateDeallocateBasic) {
    void* p1 = resource.allocate(32, alignof(int));
    void* p2 = resource.allocate(64, alignof(double));

    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
    EXPECT_NE(p1, p2);

    resource.deallocate(p1, 32, alignof(int));
    resource.deallocate(p2, 64, alignof(double));

    EXPECT_EQ(resource.active_nodes(), 0u);
    EXPECT_EQ(resource.node_count(), 2u);
}

// 2. Повторное использование освобождённого блока
TEST_F(ListMemoryResourceTest, ReuseFreedBlock) {
    auto a1 = allocate_with_id(32, alignof(int), 100);
    deallocate(a1);  // освобождаем

    // Следующее выделение такого же или меньшего размера должно использовать тот же блок
    void* p2 = resource.allocate(32, alignof(int));
    EXPECT_EQ(p2, a1.ptr);  // ← тот же адрес

    // Проверим, что данные внутри "перезаписываемы"
    *static_cast<int*>(p2) = 200;
    EXPECT_EQ(*static_cast<int*>(p2), 200);

    resource.deallocate(p2, 32, alignof(int));
}

// 3. Блок меньшего размера может использовать освобождённый большой
TEST_F(ListMemoryResourceTest, ReuseLargerBlockForSmallerRequest) {
    auto large = allocate_with_id(128, 16, 500);
    deallocate(large);  // освобождён блок 128 байт

    // Запросим 64 байта — должно взяться из освобождённого
    void* p = resource.allocate(64, 8);
    EXPECT_EQ(p, large.ptr);  // тот же адрес
    EXPECT_EQ(resource.active_nodes(), 1u);
    EXPECT_EQ(resource.node_count(), 1u);

    resource.deallocate(p, 64, 8);
}

// 4. Блок большего размера НЕ может использовать меньший освобождённый
TEST_F(ListMemoryResourceTest, CannotReuseTooSmallBlock) {
    auto small = allocate_with_id(32, 8, 1);
    deallocate(small);

    // Запросим 64 байта — должно выделиться НОВОЕ место
    void* p = resource.allocate(64, 8);
    EXPECT_NE(p, small.ptr);  // другой адрес

    EXPECT_EQ(resource.active_nodes(), 1u);
    EXPECT_EQ(resource.node_count(), 2u);

    resource.deallocate(p, 64, 8);
}

// 5. Двойное освобождение → исключение
TEST_F(ListMemoryResourceTest, DoubleDeallocateThrows) {
    auto a = allocate_with_id(32, 8, 99);
    resource.deallocate(a.ptr, a.bytes, a.alignment);

    // Повторное освобождение того же указателя — ошибка
    EXPECT_THROW({
        resource.deallocate(a.ptr, a.bytes, a.alignment);
    }, std::logic_error);
}

// 6. Освобождение неизвестного указателя → исключение
TEST_F(ListMemoryResourceTest, DeallocateUnknownPointerThrows) {
    // Получим указатель из upstream напрямую (обойдя наш ресурс)
    void* foreign = upstream_pool.allocate(32, 8);

    // Попытка освободить его через наш ресурс — должна упасть
    EXPECT_THROW({
        resource.deallocate(foreign, 32, 8);
    }, std::logic_error);

    upstream_pool.deallocate(foreign, 32, 8);  // cleanup
}

// 7. Проверка is_equal
TEST_F(ListMemoryResourceTest, IsEqual) {
    ListMemoryResource other_resource{&upstream_pool};

    EXPECT_TRUE(resource.is_equal(resource));
    EXPECT_FALSE(resource.is_equal(other_resource));
    EXPECT_FALSE(resource.is_equal(*std::pmr::new_delete_resource()));
}

// 8. Статистика
TEST_F(ListMemoryResourceTest, StatsConsistency) {
    auto a1 = allocate_with_id(16, 8, 1);
    auto a2 = allocate_with_id(32, 8, 2);

    EXPECT_EQ(resource.active_nodes(), 2u);
    EXPECT_EQ(resource.node_count(), 2u);
    EXPECT_GE(resource.total_allocated_bytes(), 48u);  // ≥ 16+32

    deallocate(a1);

    EXPECT_EQ(resource.active_nodes(), 1u);
    EXPECT_EQ(resource.node_count(), 2u);
    EXPECT_GE(resource.total_allocated_bytes(), 48u);

    resource.stats();  // просто проверим, что не падает
}

// 9. Выравнивание: запрос с большим выравниванием не должен использовать блок с меньшим
TEST_F(ListMemoryResourceTest, AlignmentIsRespected) {
    // Выделим блок с выравниванием 8
    auto align8 = allocate_with_id(32, 8, 10);
    deallocate(align8);

    // Теперь запросим выравнивание 16 → должен создаться НОВЫЙ блок
    void* p = resource.allocate(32, 16);
    EXPECT_NE(p, align8.ptr);  // другой адрес — т.к. 8 < 16

    resource.deallocate(p, 32, 16);
    EXPECT_EQ(resource.active_nodes(), 0u);
    EXPECT_EQ(resource.node_count(), 2u);
}

// 10. Деструктор корректно освобождает upstream
TEST(ListMemoryResourceTest, DestructorCallsUpstreamDeallocate) {
    // Используем счётчик вызовов через custom upstream
    struct CountingResource : std::pmr::memory_resource {
        size_t alloc_count = 0;
        size_t dealloc_count = 0;

        void* do_allocate(size_t bytes, size_t) override {
            ++alloc_count;
            return ::operator new(bytes);
        }

        void do_deallocate(void* p, size_t, size_t) override {
            ++dealloc_count;
            ::operator delete(p);
        }

        bool do_is_equal(const memory_resource&) const noexcept override {
            return true;
        }
    };

    CountingResource counter;
    {
        ListMemoryResource res{&counter};
        res.allocate(16, 8);
        res.allocate(32, 8);
        // res уходит из scope → деструктор вызовет deallocate 2 раза
    }

    EXPECT_EQ(counter.alloc_count, 2u);
    EXPECT_EQ(counter.dealloc_count, 2u);
}