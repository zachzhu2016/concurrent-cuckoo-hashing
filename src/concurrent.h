
using namespace std;

template <class K>
class HashSetConcurrent: public HashSetSequential<K> {
public:
    using HashSetSequential<K>::HashSetSequential;

    vector<mutex> * mutex_vec0;
    vector<mutex> * mutex_vec1;

    HashSetConcurrent(int cap, int limit, double loadfactor) {
        this->bucket0 = new Bucket<K, string>(cap);
        this->bucket1 = new Bucket<K, string>(cap);

        this->loadfactor = loadfactor;
        this->limit = limit;
        this->cap = cap;
        this->n.store(0);

        int msize = 100;
        this->mutex_vec0 = new vector<mutex>(msize);
        this->mutex_vec1 = new vector<mutex>(msize);
    }

    ~HashSetConcurrent() {
        delete this->mutex_vec0;
        delete this->mutex_vec1;
    }

    void acquire_all() {
        for (auto& m : *(this->mutex_vec0)) m.lock();
    }

    void release_all() {
        for (auto& m : *(this->mutex_vec0)) m.unlock();
    }

    void acquire(K k) {
        size_t h0 = this->lock_hash0(k, this->mutex_vec0->size());
        size_t h1 = this->lock_hash1(k, this->mutex_vec1->size());
        this->mutex_vec0->at(h0).lock();
        this->mutex_vec1->at(h1).lock();
    }
    
    void release(K k) {
        size_t h0 = this->lock_hash0(k, this->mutex_vec0->size());
        size_t h1 = this->lock_hash1(k, this->mutex_vec1->size());
        this->mutex_vec0->at(h0).unlock();
        this->mutex_vec1->at(h1).unlock();
    }

    bool add(K k) {

        this->acquire(k);
        if (this->HashSetSequential<K>::contains(k)) {
            this->release(k);
            return false;
        }

        auto curr_k = k;

        vector<K> acquired;
        acquired.push_back(curr_k);

        for (size_t i = 0; i < this->limit; ++i) {
            if (k != curr_k) {
                this->acquire(curr_k);
                acquired.push_back(curr_k);
                if (this->HashSetSequential<K>::contains(curr_k)) {
                    for (auto k : acquired) this->release(k);
                    return false;
                }
            }

            size_t h0 = this->hash0(curr_k, this->cap);
            size_t h1 = this->hash1(curr_k, this->cap);

            if (this->bucket0->try_set(h0, curr_k, "hello")) break;
            if (this->bucket1->try_set(h1, curr_k, "hello")) break;

            K replaced_k;
            if (i % 2 == 0) replaced_k = this->bucket0->replace(h0, curr_k, "hello");
            else replaced_k = this->bucket1->replace(h1, curr_k, "hello");

            if (curr_k == replaced_k) {
                for (auto k : acquired) this->release(k);
                return false;
            }
            curr_k = replaced_k;
            int oldcap = this->cap;
            if (i + 1 == this->limit) {
                this->acquire_all();
                int currcap = this->cap;
                if (oldcap == currcap) {
                    this->HashSetSequential<K>::resize();
                }
                this->release_all();
            }
        }

        this->n++;
        for (auto k : acquired) this->release(k);
        
        return true;
    };

    bool contains(K k) {
        while (true) {
            int b0_version = this->bucket0->version.load();
            int b1_version = this->bucket1->version.load();

            auto oldcap = this->cap;
            size_t h0 = this->hash0(k, oldcap);
            size_t h1 = this->hash1(k, oldcap);

            if (this->bucket0->has(h0, k)) {
                if (b0_version == this->bucket0->version.load() && this->cap == oldcap) return true;
            }
            else if (this->bucket1->has(h1, k)) {
                if (b1_version == this->bucket1->version.load() && this->cap == oldcap) return true;
            }
            else {
                return false;
            }
        }
    }

    bool remove(K k) {
        this->acquire(k);
        bool result = this->HashSetSequential<K>::remove(k);
        this->release(k);
        return result;
    }

protected:
    size_t lock_hash0(K k, int max) { return this->hash(k, 1, max); }
    size_t lock_hash1(K k, int max) { return this->hash(k, 2, max); }
};

