#include<iostream>
#include<cstddef>
#include<list>
#include<unordered_map>
#include<cassert>

template<typename K, typename V>
class LFUCache {
public:
    LFUCache(size_t capacity) : _capacity(capacity), _min_freq(1) {
        assert(_capacity > 0);
    }
    ~LFUCache() = default;
    LFUCache(const LFUCache& other) = delete;
    LFUCache& operator=(const LFUCache& other) = delete;

    bool get(const K& key, V* value) {
        if (!value) {
            return false;
        }
        auto it = _index.find(key);
        if (it != _index.end()) {
            *value = it->second->_value;
            move_item(it->second);
            return true;
        }
        return false;
    }

    void put(const K& key, const V& value) {
        if (_capacity == 0) {
            return;
        }
        auto it = _index.find(key);
        if (it != _index.end()) {
            it->second->_value = value;
            move_item(it->second);
            return;
        }
        if (_index.size() >= _capacity) {
            auto freq_it = _freq_to_list.find(_min_freq);
            if (freq_it != _freq_to_list.end() && !freq_it->second.empty()) {
                K key = freq_it->second.back()._key;
                _index.erase(key);
                freq_it->second.pop_back();
                if (freq_it->second.empty()) {
                    _freq_to_list.erase(freq_it);
                }
            }
        }
        _freq_to_list[1].emplace_front(Node(key, value, 1));
        _index[key] = _freq_to_list[1].begin();
        //must
        _min_freq = 1;
        return;
    }

private:
    struct Node {
        K _key;
        V _value;
        int _freq;
        Node(K key, V value, int freq) : _key(key), _value(value), _freq(freq) {}
    };
    void move_item(typename std::list<Node>::iterator it) {
        int old_freq = it->_freq;
        // 要引用  不能值拷贝
        auto& old_list = _freq_to_list[old_freq];
        auto& new_list = _freq_to_list[old_freq + 1];
        ++it->_freq;
        //优雅
        new_list.splice(new_list.begin(), old_list, it);
        if (old_list.empty()) {
            //erase
            _freq_to_list.erase(old_freq);
            if (_min_freq == old_freq) {
                ++_min_freq;
            }
        }
    }
    size_t _capacity;
    int _min_freq;
    std::unordered_map<K, typename std::list<Node>::iterator> _index;
    std::unordered_map<int, std::list<Node>> _freq_to_list;
};

int main() {
    LFUCache<int, int> cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(2, 2);
    cache.put(3, 3);
    int temp;
    if (cache.get(2, &temp)) {
        std::cout << "found :" << temp << std::endl;
    } else {
        std::cout << "not found" << std::endl;
    }

}