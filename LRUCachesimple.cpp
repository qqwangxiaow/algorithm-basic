#include <mutex>
#include <shared_mutex>
#include <list>
#include <unordered_map>
#include <iostream>

template<typename K, typename V>
class LRUCache {
public:
    LRUCache(size_t capacity) : _capacity(capacity){}
    
    ~LRUCache() = default;

    LRUCache(const LRUCache& other) = delete;
    LRUCache& operator=(const LRUCache& other) = delete;

    LRUCache(LRUCache&& other) = default;
    LRUCache& operator=(LRUCache&& other) = default;
    /*
    LRUCache(LRUCache&& other) noexcept : 
            _capacity(other.capacity) {
            _index(std::move(other._index));
            _lru(std::move(other._lru));
    }
    LRUCache& operator=(LRUCache&& other) noexcept {
        if (this != &other) {
            std::lock(_mutex, other._mutex);
            std::lock_guard<std::mutex> lhs_lock(_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> rhs_lock(other._mutex, std::adopt_lock);
            _capacity = other._capacity;
            _lru = std::move(other._lru);
            _index = std::move(other._index);
        }
    }
        */
    
    bool get(const K& key, V* value) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto iter = _index.find(key);
        if (iter == _index.end()) {
            return false;
        }
        *value = iter->second->second;
        _lru.splice(_lru.begin(), _lru, iter->second);
        //_index[key] = _lru.begin();
        return true;
    }

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto iter = _index.find(key);
        if (iter != _index.end()) {
            iter->second->second = value;
            _lru.splice(_lru.begin(), _lru, iter->second);
            //_index[key] = _lru.begin();
            return;
        }
        _lru.emplace_front(key, value);
        _index[key] = _lru.begin();

        if (_lru.size() > _capacity) {
            auto back = _lru.back();
            _index.erase(back.first);
            _lru.pop_back();
        }
        return;
    }

    
private:
    size_t _capacity;
    std::list<std::pair<K, V>> _lru;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> _index;
    std::mutex _mutex;
 };

 int main ()
 {
    LRUCache<int, int> cache(100);
    cache.put(1, 2);
    int value;
    std::cout << "value:" << cache.get(1, &value) << std::endl;
    return 0;
 } 