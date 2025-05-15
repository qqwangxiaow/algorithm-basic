#include <iostream>
#include <list>
#include <shared_mutex> // C++17
#include <unordered_map>

template <typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : _capacity(capacity){};
    ~LRUCache() = default;

    bool get(const Key& key, Value* value) {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache.find(key);
        if (it == _cache.end()) {
            return false;
        }
        lock.unlock();
        {
            std::unique_lock<std::shared_mutex> wlock(_mutex);
            auto it2 = _cache.find(key);
            if (it2 == _cache.end()) {
                return false;
            }
            _lru.splice(_lru.begin(), _lru, it2->second);
            *value = it2->second->second;
        }
        return true;
    }

    void put(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache.find(key);
        if (it != _cache.end()) {
            it->second->second = value;
            _lru.splice(_lru.begin(), _lru, it->second);
            return;
        }
        _lru.emplace_front(key, value);
        _cache[key] = _lru.begin();
        if (_cache.size() > _capacity) {
            auto last = _lru.end();
            --last;
            _cache.erase(last->first);
            _lru.pop_back();
        }
        return;
    }

    bool erase(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache.find(key);
        if (it == _cache.end()) {
            return false;
        }
        _lru.erase(it->second);
        _cache.erase(it);
        return true;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _cache.size();
    }

private:
    mutable std::shared_mutex _mutex;
    size_t _capacity;
    std::list<std::pair<Key, Value>> _lru;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> _cache;


};

int main() {
    LRUCache<int, int> cache(3);

    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    int a;
    cache.get(3, &a);
    std::cout << "a=" << a << std::endl;
}
