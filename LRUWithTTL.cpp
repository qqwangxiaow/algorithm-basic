#include <iostream>
#include <unordered_map>
#include <list>
#include <chrono>
#include <thread>

template<typename K, typename V>
class LRUWithTTL {
public:
    LRUWithTTL(size_t capacity) : _capacity(capacity) {
    }

    ~LRUWithTTL() = default;

    LRUWithTTL(const LRUWithTTL& other) = delete;
    LRUWithTTL& operator=(const LRUWithTTL& other) = delete;


    void put(const K& key, const V& value, std::chrono::milliseconds ttl) {
        if (_capacity == 0) {
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto f = _index.find(key);
        if (f != _index.end()) {
            f->second->value = value;
            f->second->expire_at = now + ttl;
            _data.splice(_data.begin(), _data, f->second);
            return;
        }

        while (_data.size() >= _capacity && !_data.empty() && isExpired(std::prev(_data.end()), now)) {
            eraseItem(std::prev(_data.end()));
        }

        if (_data.size() >= _capacity) {
            eraseItem(std::prev(_data.end()));
        }

        _data.emplace_front(Node{key, value, now + ttl});
        _index[key] = _data.begin();
    }

    bool get(const K& key, V* value) {
        if (value == nullptr) {
            return false;
        }

        auto f = _index.find(key);
        if (f == _index.end()) {
            return false;
        }

        auto now = std::chrono::steady_clock::now();
        if (isExpired(f->second, now)) {
            eraseItem(f->second);
            return false;
        }

        *value = f->second->value;
        _data.splice(_data.begin(), _data, f->second);
        return true;
    }

private:
    struct Node {
        K key;
        V value;
        std::chrono::steady_clock::time_point expire_at;
    };

    bool isExpired(typename std::list<Node>::iterator it, std::chrono::steady_clock::time_point now) {
        return it->expire_at <= now;
    }

    void eraseItem(typename std::list<Node>::iterator it) {
        _index.erase(it->key);
        _data.erase(it);
    }

    size_t _capacity;
    std::unordered_map<K, typename std::list<Node>::iterator> _index;
    std::list<Node> _data;
};

int main() {
    using namespace std::chrono_literals;

    LRUWithTTL<int, int> lru(3);
    lru.put(1, 1, 500ms);
    lru.put(2, 2, 100ms);
    lru.put(3, 3, 500ms);

    int temp = 0;

    std::this_thread::sleep_for(150ms);
    std::cout << lru.get(2, &temp) << std::endl;

    lru.put(4, 4, 500ms);
    std::cout << lru.get(1, &temp) << " " << temp << std::endl;
    std::cout << lru.get(4, &temp) << " " << temp << std::endl;
}
