#include <vector>
#include <cstddef>
#include <iostream>

template<typename K, typename V>
class HashTable {
public:
    explicit HashTable(int capacity) : _capacity(capacity) {
        _hash_table.resize(_capacity, nullptr);
    }
    ~HashTable() {
        for (auto node : _hash_table) {
            while (node) {
                Node* tmp = node;
                node = node->_next;
                delete tmp;
            }
        }
    }

    HashTable(const HashTable& other) = delete;
    HashTable& operator=(const HashTable& other) = delete;
    HashTable(HashTable&& other) noexcept = default;
    HashTable& operator=(HashTable&& other) noexcept = default;

    bool get(const K& key, V* value) {
        size_t index = hash(key);
        Node* node = _hash_table[index];
        while(node) {
            if (node->_key == key) {
                *value = node->_value;
                return true;
            }
            node = node->_next;
        }
        return false;
        
    }

    void insert(const K& key, const V& value) {
        int index = hash(key);
        if (_hash_table[index] == nullptr) {
            _hash_table[index] = new Node(key, value, nullptr);
        } else {
            Node* node = _hash_table[index];
            while (node) {
                if (node->_key == key) {
                    node->_value = value;
                    return;
                }
                node = node->_next;
            }
            _hash_table[index] = new Node(key, value, _hash_table[index]);
        }
    }

    size_t hash(const K& key) const  {
        return std::hash<K>{}(key) % _capacity;
    }


private:
    struct Node {
        K _key;
        V _value;
        Node* _next = nullptr;
        Node(K key, V value, Node* next) : _key(key), _value(value), _next(next){}    
    };
    std::vector<Node*> _hash_table;
    size_t _capacity = 0;
};

int main() {
	std::cout << "hello world" << std::endl;
    HashTable<int, int> a(3);
    a.insert(1, 1);
    a.insert(2, 2);
    a.insert(3, 3);
    int b;
    a.get(3, &b);
    std::cout << "b: " << b << std::endl;
}
	