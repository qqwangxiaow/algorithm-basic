#include <vector>
#include <list>
#include <utility>
#include <iostream>

template<typename K, typename V>
class HashTable {
public:
    // 构造：默认桶大小
    explicit HashTable(size_t bucket = 97) : _buckets(bucket) {}
    ~HashTable() = default;
    HashTable(const HashTable& other) = delete;
    HashTable& operator=(const HashTable& other) = delete;

    // 增 / 改
    void put(const K& key, const V& val) {
        size_t idx = hash(key) % _buckets.size();
        auto& list = _buckets[idx];
        
        // 查找是否已存在
        for (auto& p : list) {
            if (p.first == key) {
                p.second = val;
                return;
            }
        }
        // 不存在则插入
        list.emplace_back(key, val);
    }

    bool get(const K& key, V* out_val) {
        if (!out_val) return false;
        size_t idx = hash(key) % _buckets.size();
        auto& list = _buckets[idx];
        
        for (auto& p : list) {
            if (p.first == key) {
                *out_val = p.second;
                return true;
            }
        }
        return false;

    }

    bool erase(const K& key) {
        size_t idx = hash(key) % _buckets.size();
        auto& list = _buckets[idx];
        
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->first == key) {
                list.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    size_t hash(const K& key) const {
        return std::hash<K>{}(key);
    }
    std::vector<std::list<std::pair<K, V>>> _buckets;
};

int main() {
    HashTable<int, int> table(11);
    table.put(1, 1);
    int temp;
    if (table.get(1, &temp)) {
        std::cout << "found value: " << temp;
    } else {
        std::cout << "not found";
    }
}