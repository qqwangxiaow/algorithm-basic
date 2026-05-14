#include <cassert>
#include <chrono>
#include <iostream>
#include <cstddef>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <list>
template<typename K, typename V>
class LRUCache {
public:
    LRUCache(size_t capacity) : _capacity(capacity){
        assert(capacity > 0);
    }
    ~LRUCache() = default;
    LRUCache(const LRUCache& other) = delete;
    LRUCache& operator=(const LRUCache& other) = delete;


    void put(const K& key, const V& value, std::chrono::milliseconds ttl) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _index.find(key);
        auto now = std::chrono::steady_clock::now();
        if (it != _index.end()) {
            it->second->_value = value;
            it->second->_expire_at = ttl + now;
            _data.splice(_data.begin(), _data, it->second);
            return;
        }
        while (!_data.empty() && expire(std::prev(_data.end()), now)) {
            erase_item(std::prev(_data.end()));
        }
        while (_data.size() >= _capacity) {
            erase_item(std::prev(_data.end()));
        }
        _data.emplace_front(Node(key, value, now + ttl));
        _index[key] = _data.begin();
        return;

    }

    bool get(const K& key, V* value) {
        if (!value) {
            return false;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _index.find(key);
        if (it == _index.end()) {
            return false;
        }
        auto now = std::chrono::steady_clock::now();
        if (expire(it->second, now)) {
            //删掉
            erase_item(it->second);
            return false;
        }
        *value = it->second->_value;
        _data.splice(_data.begin(), _data, it->second);
        return true;
    }

private:
    struct Node {
        K _key;
        V _value;
        std::chrono::steady_clock::time_point _expire_at;
        Node(K key, V value, std::chrono::steady_clock::time_point ttl) : _key(key), _value(value), _expire_at(ttl) {}
    };
    // <= 
    bool expire(typename std::list<Node>::iterator it, std::chrono::steady_clock::time_point now) {
        return it->_expire_at <= now;
    }
    void erase_item(typename std::list<Node>::iterator it) {
        _index.erase(it->_key);
        _data.erase(it);
        return;
    }

    std::mutex _mutex;
    size_t _capacity;
    std::list<Node> _data;
    std::unordered_map<K, typename std::list<Node>::iterator> _index;
    
};
int main() {
    LRUCache<int, int> cache(2);
    auto a = std::chrono::milliseconds(10000);
    cache.put(1, 1, a);
    cache.put(2, 2, a);
    cache.put(3, 3, a);
    int temp;
    if (cache.get(3, &temp)) {
        std::cout << "result is :" << temp << std::endl;
    } else {
        std::cout << "not found" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    if (cache.get(3, &temp)) {
        std::cout << "result is :" << temp << std::endl;
    } else {
        std::cout << "not found" << std::endl;
    }



}