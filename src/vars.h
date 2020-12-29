
using namespace std;

const int SLOT_SIZE = 8;

template <class K, class V> 
struct Entry {
    K key;
    V value;
};

template <class K, class V>
class Bucket {
public:
    int cap;
    atomic<int> version;
    vector<vector<Entry<K, V>>> slots;

    Bucket(int cap) {
        vector<vector<Entry<K, V>>> slots(cap, vector<Entry<K, V>>(SLOT_SIZE));
        this->slots = slots;
        this->version.store(0);
        this->cap = cap;
    }

    void upgrade() {
        this->version++;
    }

    bool try_set(size_t slot_i, K k, V v) { 
        for (size_t e = 0; e < SLOT_SIZE; ++e) {
            if (slots[slot_i][e].key == (K) NULL && k != (K) NULL) {
                slots[slot_i][e] = Entry<K, V>{k, "hello"}; 
                this->upgrade();
                return true;
            }
        }
        return false;
    }

    bool try_remove(size_t slot_i, K k) {
        for (size_t e = 0; e < SLOT_SIZE; ++e) {
            if (slots[slot_i][e].key == k) {
                slots[slot_i][e].key = (K) NULL;
                this->upgrade();
                return true;
            }
        }
        return false;
    }

    K replace(size_t slot_i, K k, V v) {
        size_t last = SLOT_SIZE - 1;
        K replaced_k = slots[slot_i][last].key;
        slots[slot_i][last] = Entry<K, V>{k, v};
        this->upgrade();
        return replaced_k;
    }

    bool has(size_t slot_i, K k) {
        for (size_t e = 0; e < SLOT_SIZE; ++e) {
            auto entry = slots[slot_i][e];
            if (entry.key == k && entry.key != (K) NULL) return true;
        }
        return false;
    }
    
    vector<K> get_all() {
        vector<K> v;
        for (size_t s = 0; s < this->cap; ++s) {
            for (size_t e = 0; e < SLOT_SIZE; ++e) {
                auto entry = slots[s][e];
                if (entry.key != (K) NULL) v.push_back(entry.key); 
            }
        }
        return v;
    }
};

