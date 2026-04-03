#include <cstddef>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

template<typename K, typename V>
class LinearProbingHashTable {
public:
    explicit LinearProbingHashTable(size_t capacity)
        : _capacity(nextPrime(capacity < 4 ? 4 : capacity)),
          _table(_capacity),
          _size(0) {
    }

    bool get(const K& key, V* value) const {
        if (value == nullptr) {
            return false;
        }

        size_t index = findIndex(key);
        if (index == npos) {
            return false;
        }

        *value = _table[index].value.second;
        return true;
    }

    bool insert(const K& key, const V& value) {
        return insertInternal(key, value);
    }

    bool erase(const K& key) {
        size_t index = findIndex(key);
        if (index == npos) {
            return false;
        }

        _table[index].state = State::Deleted;
        --_size;
        return true;
    }

    size_t size() const {
        return _size;
    }

private:
    enum class State {
        Empty,
        Occupied,
        Deleted,
    };

    struct Bucket {
        std::pair<K, V> value;
        State state = State::Empty;
    };

    static constexpr size_t npos = static_cast<size_t>(-1);

    size_t hash(const K& key) const {
        return std::hash<K>{}(key) % _capacity;
    }

    size_t probe(size_t base, size_t step) const {
        return (base + step) % _capacity;
    }

    size_t findIndex(const K& key) const {
        size_t base = hash(key);
        for (size_t step = 0; step < _capacity; ++step) {
            size_t index = probe(base, step);
            const Bucket& bucket = _table[index];

            if (bucket.state == State::Empty) {
                return npos;
            }

            if (bucket.state == State::Occupied && bucket.value.first == key) {
                return index;
            }
        }

        return npos;
    }

    bool insertInternal(const K& key, const V& value) {
        size_t base = hash(key);
        size_t first_deleted = npos;

        for (size_t step = 0; step < _capacity; ++step) {
            size_t index = probe(base, step);
            Bucket& bucket = _table[index];

            if (bucket.state == State::Occupied && bucket.value.first == key) {
                bucket.value.second = value;
                return true;
            }

            if (bucket.state == State::Deleted && first_deleted == npos) {
                first_deleted = index;
                continue;
            }

            if (bucket.state == State::Empty) {
                size_t target = first_deleted == npos ? index : first_deleted;
                _table[target].value = std::make_pair(key, value);
                _table[target].state = State::Occupied;
                ++_size;
                return true;
            }
        }

        if (first_deleted != npos) {
            _table[first_deleted].value = std::make_pair(key, value);
            _table[first_deleted].state = State::Occupied;
            ++_size;
            return true;
        }

        return false;
    }

    static bool isPrime(size_t value) {
        if (value < 2) {
            return false;
        }

        for (size_t factor = 2; factor * factor <= value; ++factor) {
            if (value % factor == 0) {
                return false;
            }
        }

        return true;
    }

    static size_t nextPrime(size_t value) {
        while (!isPrime(value)) {
            ++value;
        }

        return value;
    }

    size_t _capacity;
    std::vector<Bucket> _table;
    size_t _size;
};

int main() {
    LinearProbingHashTable<int, int> table(5);
    table.insert(1, 10);
    table.insert(6, 60);
    table.insert(11, 110);

    int value = 0;
    std::cout << table.get(6, &value) << " " << value << std::endl;

    table.erase(6);
    std::cout << table.get(6, &value) << std::endl;

    std::cout << table.insert(16, 160) << std::endl;
    std::cout << table.get(16, &value) << " " << value << std::endl;
}