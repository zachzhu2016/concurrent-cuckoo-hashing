
using namespace std;

template <class K>
class HashSetSequential {
public: 

    Bucket<K, string> * bucket0;
    Bucket<K, string> * bucket1;
    
    int cap;
    int limit;
    double loadfactor;
    atomic<int> n_resize;

    HashSetSequential() {}

    ~HashSetSequential() {
        delete this->bucket0;
        delete this->bucket1;
    }

    HashSetSequential(int cap, int limit, double loadfactor) {
        this->bucket0 = new Bucket<K, string>(cap);
        this->bucket1 = new Bucket<K, string>(cap);

        this->loadfactor = loadfactor;
        this->limit = limit;
        this->cap = cap;
        this->n.store(0);
    }

    bool add(K k) {
        if (this->contains(k)) return false;

        auto curr_k = k;
        for (size_t i = 0; i < this->limit; ++i) {

            size_t h0 = this->hash0(curr_k, this->cap);
            size_t h1 = this->hash1(curr_k, this->cap);

            if (this->bucket0->try_set(h0, curr_k, "hello")) break;
            if (this->bucket1->try_set(h1, curr_k, "hello")) break;

            K replaced_k;
            if (i % 2 == 0) replaced_k = this->bucket0->replace(h0, curr_k, "hello");
            else replaced_k = this->bucket1->replace(h1, curr_k, "hello");
            curr_k = replaced_k;
            
            if (i + 1 == this->limit) {
                this->resize();
                i = 0;
            }
        }

        this->n++;
        return true;
    }

    bool contains(K k) {
        size_t h0 = this->hash0(k, this->cap);
        size_t h1 = this->hash1(k, this->cap);
        if (this->bucket0->has(h0, k) || this->bucket1->has(h1, k)) return true;
        return false;
    }

    bool remove(K k) {
        bool result = false;
        if (!this->contains(k)) return result;

        size_t h0 = this->hash0(k, this->cap);
        size_t h1 = this->hash1(k, this->cap);

        if (this->bucket0->try_remove(h0, k)) result = true;
        else if (this->bucket1->try_remove(h1, k)) result = true;

        if (result) this->n--;
        return result;
    }

    int populate(int n, int low, int high) {
        int populated = 0;
        while (populated < n) {
            auto k = (K) rand() % (high - low) + low;
            auto result = this->HashSetSequential::add(k);
            if (result) populated++;
        }
        return populated;
    }

    int size() {
        return this->to_vector().size(); 
    }

    int size_safe() {
        return this->n;
    }

    void resize() {
        this->cap *= 2;
        this->limit = 5 * static_cast<int>(log2(this->cap));
        this->n.store(0);
        // trials have proven that rehash is not necessary
        // this->rehash();
        this->reset();
        cout << this->n <<endl;
        this->n_resize++;
    }

    void rehash() {
        // is it an effective rehash?
        this->seed0++;
        this->seed1++;
    }

    void reset() {
        vector<K> keys = this->to_vector();

        delete this->bucket0;
        delete this->bucket1;
        this->bucket0 = new Bucket<K, string>(this->cap);
        this->bucket1 = new Bucket<K, string>(this->cap);

        for (auto k : keys) {
            bool result = this->HashSetSequential<K>::add(k);
            if (!result) {
                cout << "reset function failed" << endl;
                throw exception();
            }
        }
    }

    vector<K> to_vector() {
        vector<K> v;
        for (auto k : this->bucket0->get_all()) v.push_back(k);
        for (auto k : this->bucket1->get_all()) v.push_back(k);
        return v;
    }

    void print() {
        cout << endl;
        vector<K> v = this->to_vector();
        cout << "unordered keys: ";
        for (auto k : v) 
            cout << k << ", ";
        cout << endl;
        cout << "capacity: " << this->cap << endl;
        cout << "size: " << this->size() << endl;
        cout << "factor: " << 100 * (double) this->size() / (double) this->cap << "%" << endl;
        cout << "limit: " << this->limit << endl;
        cout << endl;
    }

protected:
    atomic<int> n;
    size_t seed0 = 1;
    size_t seed1 = 2;
    size_t hash(K k, int seed, int max) { return (std::hash<K>{}(k) + seed) % max; }
    size_t hash0(K k, int max) { return this->hash(k, this->seed0, max); }
    size_t hash1(K k, int max) { return this->hash(k, this->seed1, max); }
};

