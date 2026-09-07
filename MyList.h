#ifndef MYLIST_H
#define MYLIST_H
#include <cassert>

class MyList {
public:
    int* data;
    int count;
    int capacity;

    MyList(int size = 60) : data(new int[size]), count(0), capacity(size) {}
    ~MyList() { delete[] data; }
    MyList(const MyList& other) : data(new int[other.capacity]), count(other.count), capacity(other.capacity) {
        for (int i = 0; i < count; ++i) data[i] = other.data[i];
    }
    MyList& operator=(const MyList& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = new int[capacity];
            for (int i = 0; i < count; ++i) data[i] = other.data[i];
        }
        return *this;
    }
    void push_back(int v) {
        if (count < capacity) data[count++] = v;
    }
    void erase(int v) {
        for (int i = 0; i < count; ++i) {
            if (data[i] == v) {
                for (int j = i; j < count - 1; ++j) data[j] = data[j + 1];
                --count;
                break;
            }
        }
    }
    bool replace(int oldVal, int newVal) {
        for (int i = 0; i < count; ++i) {
            if (data[i] == oldVal) {
                data[i] = newVal;
                return true;
            }
        }
        return false;
    }
    int find(int v) const {
        for (int i = 0; i < count; ++i) {
            if (data[i] == v) return i;
        }
        return -1;
    }
    void insert(int idx, int v) {
        assert(idx >= 0 && idx <= count && count < capacity);
        for (int j = count; j > idx; --j) data[j] = data[j - 1];
        data[idx] = v;
        ++count;
    }
    void erase_at(int idx) {
        assert(idx >= 0 && idx < count);
        for (int j = idx; j < count - 1; ++j) data[j] = data[j + 1];
        --count;
    }
    int* begin() { return data; }
    int* end() { return data + count; }
    int front() const { return count > 0 ? data[0] : -1; }
    int size() const {
        assert(this != nullptr);
        assert(count >= 0 && count <= capacity);
        return count;
    }
    int& operator[](int idx) {
        assert(idx >= 0 && idx < count);
        return data[idx];
    }
    const int& operator[](int idx) const {
        assert(idx >= 0 && idx < count);
        return data[idx];
    }
    bool operator==(const MyList& other) const {
        if (count != other.count) return false;
        for (int i = 0; i < count; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    bool operator!=(const MyList& other) const {
        return !(*this == other);
    }
};

#endif // MYLIST_H
