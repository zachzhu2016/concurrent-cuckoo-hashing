
using namespace std;

struct ReportParams {
    int trials;
    int cap;
    double loadfactor;
    int n_threads;
    int n_operations;
    int limit;
    int n_populate;
    int low;
    int high;

    void print() {
        cout << "Number of Trials:      " << trials << endl;
        cout << "Initial Capacity:      " << cap << endl;
        cout << "Load Factor:           " << loadfactor << endl;
        cout << "Number of Threads:     " << n_threads << endl;
        cout << "Number of Operations:  " << n_operations << endl;
        cout << "Limit (Max Iteration): " << limit << endl;
        cout << "Set Associativity:     " << SLOT_SIZE << endl;
        cout << "Random Range Low:      " << low << endl;
        cout << "Random Range High:     " << high << endl;
    }
};

struct BenchmarkResult {
    int n_resize;
    double runtime;
    double add_success_rate;
    double remove_success_rate;
    double contains_success_rate;
};

template <class K, class HashSetClass>
BenchmarkResult benchmark(string title, HashSetClass * set, int n_operations, int n_threads, int n_populate, int low, int high) {

    set->populate(n_populate, low, high);
    int start_size = set->size();

    vector<K> rand_keys(n_operations);
    for (auto& k : rand_keys) {
        k = (K) (rand() % (high - low) + low);
    }

    vector<int> rand_ops(n_operations);
    for (auto& op : rand_ops) {
        op = rand() % 4 + 1;
    }

    int l = n_operations / n_threads;
    vector<int> successes(n_threads * 3);
    vector<long> elapsed_vec(n_threads);
    vector<thread> threads(n_threads);
    for (size_t t = 0; t < n_threads; ++t) {
        threads[t] = thread([&, t]{ 
                         chrono::steady_clock::time_point time1 = chrono::steady_clock::now();

                         bool result;
                         size_t add_i = 0, remove_i = 0, contains_i = 0;
                         int added = 0, removed = 0, contained = 0;
                         size_t begin = t * l;
                         size_t end = t + 1 == n_threads ? n_operations : (t + 1) * l;

                         for (size_t i = begin; i < end; ++i) {
                             if (rand_ops[i] == 1) {
                                 result = set->add(rand_keys[add_i]);
                                 add_i++;
                                 if (result) added++;
                             }
                             else if (rand_ops[i] == 2) {
                                 result = set->remove(rand_keys[remove_i]);
                                 remove_i++;
                                 if (result) removed++;
                             } 
                             else {
                                 result = set->contains(rand_keys[contains_i]);
                                 contains_i++;
                                 if (result) contained++;
                             }
                         }

                         chrono::steady_clock::time_point time2 = chrono::steady_clock::now();
                         double elapsed = chrono::duration_cast<chrono::microseconds>(time2 - time1).count();
                         successes[t * 3] = added;
                         successes[t * 3 + 1] = removed;
                         successes[t * 3 + 2] = contained;
                         elapsed_vec[t] = elapsed; 
                     });
    };

    for(auto& thread: threads)
        thread.join();

    long max_elapsed = 0;
    for(auto& elapsed : elapsed_vec) 
        max_elapsed = elapsed > max_elapsed ? elapsed : max_elapsed;

    int added = 0, removed = 0, contained = 0;
    for (size_t i = 0; i < n_threads * 3; i += 3) {
        added += successes[i];
        removed += successes[i + 1];
        contained += successes[i + 2];
    }

    int actual_size = set->size();
    int expected_size = start_size + added - removed;
    if (actual_size != expected_size) {
        cout << "=============================================" << endl;
        set->print();
        cout << title << " failed" << endl;
        cout << "n: " << set->size_safe() << endl;
        cout << "actual size: " << actual_size << endl;
        cout << "expected size: " << expected_size << endl;
        exit(0);
    }

    BenchmarkResult result;
    result.n_resize = set->n_resize.load();
    result.runtime = max_elapsed;
    result.add_success_rate = 100 * (double) added / (double) n_operations;
    result.remove_success_rate = 100 * (double) removed / (double) n_operations;
    result.contains_success_rate = 100 * (double) contained / (double) n_operations;
    return result;
}

int median(vector<double> &v)
{
    size_t n = v.size() / 2;
    nth_element(v.begin(), v.begin() + n, v.end());
    return v[n];
}

void get_report(ReportParams params) {

    vector<double> sequential_rts(params.trials);
    vector<double> concurrent_rts(params.trials);
    vector<double> tm_rts(params.trials);
    double sequential_total_rt = 0, concurrent_total_rt = 0, tm_total_rt = 0;
    double seq_total_add_success_rate = 0, seq_total_remove_success_rate = 0, seq_total_contains_success_rate = 0;
    double con_total_add_success_rate = 0, con_total_remove_success_rate = 0, con_total_contains_success_rate = 0;
    double tm_total_add_success_rate = 0, tm_total_remove_success_rate = 0, tm_total_contains_success_rate = 0;
    int seq_total_resize = 0, con_total_resize = 0;

    for (size_t i = 0; i < params.trials; ++i) {
        BenchmarkResult result; 
        double sequential_rt = 0, concurrent_rt = 0, tm_rt = 0;
        
        HashSetSequential<int> sequential_set(params.cap, params.limit, params.loadfactor);
        result = benchmark<int, HashSetSequential<int>>("HashSetSequential", &sequential_set, params.n_operations, 1, params.n_populate, params.low, params.high);
        sequential_rt = result.runtime;
        sequential_rts[i] = sequential_rt;
        sequential_total_rt += sequential_rt;
        seq_total_add_success_rate += result.add_success_rate;
        seq_total_remove_success_rate += result.remove_success_rate;
        seq_total_contains_success_rate += result.contains_success_rate;
        seq_total_resize += result.n_resize;
        
        HashSetConcurrent<int> concurrent_set(params.cap, params.limit, params.loadfactor);
        result = benchmark<int, HashSetConcurrent<int>>("HashSetConcurrent", &concurrent_set, params.n_operations, params.n_threads, params.n_populate, params.low, params.high);
        concurrent_rt = result.runtime;
        concurrent_rts[i] = concurrent_rt;
        concurrent_total_rt += concurrent_rt;
        con_total_add_success_rate += result.add_success_rate;
        con_total_remove_success_rate += result.remove_success_rate;
        con_total_contains_success_rate += result.contains_success_rate;
        con_total_resize += result.n_resize;

        // HashSetTM<int> tm_set(params.cap, params.limit, params.loadfactor);
        // result = benchmark<int, HashSetTM<int>>("HashSetTM", &tm_set, params.n_operations, params.n_threads, params.n_populate, params.low, params.high);
        // tm_rt = result.runtime;
        // tm_rts[i] = tm_rt;
        // tm_total_rt += tm_rt;
        // tm_total_add_success_rate += result.add_success_rate;
        // tm_total_remove_success_rate += result.remove_success_rate;
        // tm_total_contains_success_rate += result.contains_success_rate;
    }

    cout << endl;
    cout << "=============================================" << endl;
    params.print();
    
    cout << endl << "> Sequential Hash Set: " << endl;
    cout << "Average  Runtime:      " << sequential_total_rt / (double) params.trials              << endl;
    cout << "Median   Runtime:      " << median(sequential_rts)                                    << endl;
    cout << "Add      Success Rate: " << seq_total_add_success_rate / (double) params.trials       << endl;
    cout << "Remove   Success Rate: " << seq_total_remove_success_rate / (double) params.trials    << endl;
    cout << "Contains Success Rate: " << seq_total_contains_success_rate / (double) params.trials  << endl;
    cout << "Average  Resizes:      " << seq_total_resize / (double) params.trials                 << endl;
    
    cout << endl << "> Concurrent Hash Set: " << endl;
    cout << "Average Runtime:       " << concurrent_total_rt / (double) params.trials             << endl;
    cout << "Median  Runtime:       " << median(concurrent_rts)                                   << endl;
    cout << "Add      Success Rate: " << con_total_add_success_rate / (double) params.trials      << endl;
    cout << "Remove   Success Rate: " << con_total_remove_success_rate / (double) params.trials   << endl;
    cout << "Contains Success Rate: " << con_total_contains_success_rate / (double) params.trials << endl;
    cout << "Average  Resizes:      " << con_total_resize / (double) params.trials                << endl;

    //cout << endl << "> TM Hash Set: " << endl;
    //cout << "Average Runtime:       " << tm_total_rt / (double) params.trials                     << endl;
    //cout << "Median  Runtime:       " << median(tm_rts)                                           << endl;
    //cout << "Add      Success Rate: " << tm_total_add_success_rate / (double) params.trials       << endl;
    //cout << "Remove   Success Rate: " << tm_total_remove_success_rate / (double) params.trials    << endl;
    //cout << "Contains Success Rate: " << tm_total_contains_success_rate / (double) params.trials  << endl;
    cout << endl;

}
