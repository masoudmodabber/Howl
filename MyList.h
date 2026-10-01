#ifndef MYLIST_H
#define MYLIST_H
#include <cassert>

class MyList {
public:
    static constexpr int Capacity = 16;
    int data[Capacity];
    int count;

    MyList(int = 60) noexcept : count(0) {}
    ~MyList() = default;
    MyList(const MyList& other) noexcept : count(other.count) {
        for (int i = 0; i < count; ++i) data[i] = other.data[i];
    }
    MyList& operator=(const MyList& other) noexcept {
        if (this != &other) {
            count = other.count;
            for (int i = 0; i < count; ++i) data[i] = other.data[i];
        }
        return *this;
    }
    void push_back(int v) noexcept {
        assert(count < Capacity);
        data[count++] = v;
    }
    void erase(int v) noexcept {
        for (int i = 0; i < count; ++i) {
            if (data[i] == v) {
                for (int j = i; j < count - 1; ++j) data[j] = data[j + 1];
                --count;
                break;
            }
        }
    }
    bool replace(int oldVal, int newVal) noexcept {
        for (int i = 0; i < count; ++i) {
            if (data[i] == oldVal) {
                data[i] = newVal;
                return true;
            }
        }
        return false;
    }
    int find(int v) const noexcept {
        for (int i = 0; i < count; ++i) {
            if (data[i] == v) return i;
        }
        return -1;
    }
    void insert(int idx, int v) noexcept {
        assert(idx >= 0 && idx <= count && count < Capacity);
        for (int j = count; j > idx; --j) data[j] = data[j - 1];
        data[idx] = v;
        ++count;
    }
    void erase_at(int idx) noexcept {
        assert(idx >= 0 && idx < count);
        for (int j = idx; j < count - 1; ++j) data[j] = data[j + 1];
        --count;
    }
    int* begin() noexcept { return data; }
    int* end() noexcept { return data + count; }
    const int* begin() const noexcept { return data; }
    const int* end() const noexcept { return data + count; }
    bool empty() const noexcept { return count == 0; }
    int front() const noexcept { return count > 0 ? data[0] : -1; }
    int size() const noexcept {
        assert(this != nullptr);
        assert(count >= 0 && count <= Capacity);
        return count;
    }
    int& operator[](int idx) noexcept {
        assert(idx >= 0 && idx < count);
        return data[idx];
    }
    const int& operator[](int idx) const noexcept {
        assert(idx >= 0 && idx < count);
        return data[idx];
    }
    bool operator==(const MyList& other) const noexcept {
        if (count != other.count) return false;
        for (int i = 0; i < count; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    bool operator!=(const MyList& other) const noexcept {
        return !(*this == other);
    }
};

#endif // MYLIST_H
