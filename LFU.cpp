// @lc code=start
#include <list>
#include <unordered_map>
#include <iostream>

template <typename K, typename V>
class LFUCache {
public:
    LFUCache(int capacity) : _capacity(capacity), _min_freq(0) {}

    LFUCache(const LFUCache& other) = delete;
    LFUCache& operator=(const LFUCache& other) = delete;

    ~LFUCache() = default;

    V get(const K& key) {
        auto it = _index.find(key);
        if (it == _index.end()) return V();
        V val = it->second->_value;
        move(it->second);
        return val;
    }

    void put(const K& key, const V& value) {
        if (_capacity <= 0) return;

        auto it = _index.find(key);
        if (it != _index.end()) {
            it->second->_value = value;
            move(it->second);
            return;
        }

        if (_index.size() == _capacity) {
            auto& minList = _freq_to_list[_min_freq];
            K delKey = minList.back()._key;
            minList.pop_back();
            _index.erase(delKey);
        }

        _freq_to_list[1].emplace_front(key, value, 1);
        _index[key] = _freq_to_list[1].begin();
        _min_freq = 1;
    }

private:
    struct Entry {
        K _key;
        V _value;
        int _freq;
        Entry(K k, V v, int f) : _key(std::move(k)), _value(std::move(v)), _freq(f) {}
    };

    void move(typename std::list<Entry>::iterator it) {
        Entry e = *it;
        int oldFreq = e._freq;

        _freq_to_list[oldFreq].erase(it);

        if (oldFreq == _min_freq && _freq_to_list[oldFreq].empty())
            _min_freq++;

        e._freq++;
        _freq_to_list[e._freq].emplace_front(std::move(e));
        _index[e._key] = _freq_to_list[e._freq].begin();
    }

    std::unordered_map<K, typename std::list<Entry>::iterator> _index;
    std::unordered_map<int, std::list<Entry>> _freq_to_list;
    int _capacity;
    int _min_freq;
};

int main() {
    LFUCache<int, int> cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    std::cout << "get(1): " << cache.get(1) << std::endl; // 1

    cache.put(3, 3); // 淘汰 key 2
    std::cout << "get(2): " << cache.get(2) << std::endl; // -1
    std::cout << "get(3): " << cache.get(3) << std::endl; // 3
}