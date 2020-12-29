
using namespace std;

template <class K>
class HashSetTM: public HashSetSequential<K> {
public:
    using HashSetSequential<K>::HashSetSequential;

    __attribute__((transaction_pure))
    bool add(K k) {
        bool result = false;
        
        double currfactor = (double) (this->n + 1) / (double) this->cap;
        if (currfactor > this->loadfactor) this->resize();

        __transaction_atomic {

            if (this->contains(k)) return result;

            auto curr_k = k;
            for (size_t i = 0; i < this->limit; ++i) {

                size_t h0 = this->hash0(curr_k, this->cap);
                size_t h1 = this->hash1(curr_k, this->cap);

                if (this->bucket0->try_set(h0, curr_k)) {
                    result = true;
                    break;
                }
                if (this->bucket1->try_set(h1, curr_k)) {
                    result = true;
                    break;
                }

                K replaced_k;
                if (i % 2 == 0) replaced_k = this->bucket0->replace(h0, curr_k);
                else replaced_k = this->bucket1->replace(h1, curr_k);
                curr_k = replaced_k;

                if (i + 1 == this->limit) {
                    this->resize();
                    i = -1;
                }
            }
        }
        this->n += 1;
        result = true;
        return result;
    }

    __attribute__((transaction_pure))
    bool contains(K k) {
        bool result = false;
        __transaction_atomic {
            size_t h0 = this->hash0(k, this->cap);
            size_t h1 = this->hash1(k, this->cap);
            if (this->bucket0->has(h0, k) || this->bucket1->has(h1, k)) result = true;
        }
        return result;
    }

    __attribute__((transaction_pure))
    bool remove(K k) {
        bool result = false;
        __transaction_atomic {
            if (!this->contains(k)) return result;

            size_t h0 = this->hash0(k, this->cap);
            size_t h1 = this->hash1(k, this->cap);

            if (this->bucket0->try_remove(h0, k)) result = true;
            else if (this->bucket1->try_remove(h1, k)) result = true;

        }
        if (result) this->n -= 1;
        return result;
    }

    __attribute__((transaction_pure))
    void resize() {
        __transaction_atomic {
            this->cap *= 2;
            this->limit = 5 * static_cast<int>(log2(this->cap));
            this->reset();
        }
    }

    __attribute__((transaction_pure))
    void reset() {
        __transaction_atomic {
            vector<K> keys = this->to_vector();
            delete this->bucket0;
            delete this->bucket1;
            this->bucket0 = new Bucket<K>(this->cap);
            this->bucket1 = new Bucket<K>(this->cap);

            this->n = 0;
            for (auto k : keys) {
                bool result = this->add(k);
            }
        }
    }
};
