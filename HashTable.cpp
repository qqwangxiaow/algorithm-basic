#include <iostream>
#include <vector>
#include <optional>
#include <string>
#include <shared_mutex>
#include <type_traits>
#include <mutex>

template<typename K, typename V, size_t bucket_num = 16>
class HashTable {
private:
    struct Node {
        K key;
        V value;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    std::vector<Node*> _buckets;
    size_t _memory_used;
    size_t _memory_limit;
    mutable std::shared_mutex _mtx;

    size_t hash(const K& key) const {
        return std::hash<K>{}(key) % bucket_num;
    }

    size_t key_mem_usage(const K& key) const {
        if constexpr (std::is_same<K, std::string>::value) {
            return key.capacity();
        } else {
            return sizeof(K);
        }
    }
    size_t value_mem_usage(const V& value) const {
        if constexpr (std::is_same<V, std::string>::value) {
            return value.capacity();
        } else {
            return sizeof(V);
        }
    }
    size_t node_mem_usage(const K& key, const V& value) const {
        return sizeof(Node) + key_mem_usage(key) + value_mem_usage(value);
    }
    size_t node_mem_usage(const Node* node) const {
        return node_mem_usage(node->key, node->value);
    }

    // 释放所有节点
    void clear() {
        for (size_t i = 0; i < _buckets.size(); ++i) {
            Node* curr = _buckets[i];
            while (curr) {
                Node* next = curr->next;
                delete curr;
                curr = next;
            }
            _buckets[i] = nullptr;
        }
        _memory_used = 0;
    }

public:
    HashTable(size_t mem_limit)
        : _buckets(bucket_num, nullptr), _memory_used(0), _memory_limit(mem_limit) {}

    // 禁用拷贝
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // 移动构造
    HashTable(HashTable&& other) noexcept
        : _buckets(std::move(other._buckets)),
          _memory_used(other._memory_used),
          _memory_limit(other._memory_limit) {
        // 不能拷贝锁，直接初始化新锁
        other._memory_used = 0;
        other._memory_limit = 0;
        // buckets被move后，other的桶指针已为空
    }

    // 移动赋值
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            std::unique_lock lock1(_mtx, std::defer_lock);
            std::unique_lock lock2(other._mtx, std::defer_lock);
            std::lock(lock1, lock2);

            clear();
            _buckets = std::move(other._buckets);
            _memory_used = other._memory_used;
            _memory_limit = other._memory_limit;

            other._memory_used = 0;
            other._memory_limit = 0;
        }
        return *this;
    }

    // 析构函数
    ~HashTable() {
        clear();
    }

    bool insert(const K& key, const V& value) {
        std::unique_lock lock(_mtx);
        size_t idx = hash(key);
        Node* curr = _buckets[idx];
        while (curr) {
            if (curr->key == key) {
                _memory_used -= value_mem_usage(curr->value);
                _memory_used += value_mem_usage(value);
                curr->value = value;
                return true;
            }
            curr = curr->next;
        }
        size_t need_mem = node_mem_usage(key, value);
        if (_memory_used + need_mem > _memory_limit) {
            return false;
        }
        Node* new_node = new Node(key, value);
        new_node->next = _buckets[idx];
        _buckets[idx] = new_node;
        _memory_used += need_mem;
        return true;
    }

    std::optional<V> find(const K& key) const {
        std::shared_lock lock(_mtx);
        size_t idx = hash(key);
        Node* curr = _buckets[idx];
        while (curr) {
            if (curr->key == key) {
                return curr->value;
            }
            curr = curr->next;
        }
        return std::nullopt;
    }

    bool erase(const K& key) {
        std::unique_lock lock(_mtx);
        size_t idx = hash(key);
        Node* curr = _buckets[idx];
        Node* prev = nullptr;
        while (curr) {
            if (curr->key == key) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    _buckets[idx] = curr->next;
                }
                _memory_used -= node_mem_usage(curr);
                delete curr;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    size_t get_memory_used() const {
        std::shared_lock lock(_mtx);
        return _memory_used;
    }
    size_t get_memory_limit() const { return _memory_limit; }
};

int main() {
    HashTable<std::string, std::string, 8> ht(256);

    int i = 0;
    while (ht.insert("key" + std::to_string(i), "val" + std::to_string(i))) {
        std::cout << "Inserted: key" << i << ", memory used: " << ht.get_memory_used() << " bytes\n";
        ++i;
    }
    std::cout << "Insert failed for key" << i << ", memory limit reached!\n";

    ht.erase("key0");
    std::cout << "After erase, memory used: " << ht.get_memory_used() << " bytes\n";
    if (ht.insert("key100", "val100")) {
        std::cout << "Inserted key100 after freeing space!\n";
    }
    return 0;
}