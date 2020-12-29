#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <cmath> 
#include <random>
#include <vector>
#include <exception>
#include <functional>
#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <stdlib.h>

#include "vars.h"
#include "sequential.h"
#include "concurrent.h"
#include "tm.h"
#include "benchmark.h"

using namespace std;

int main(int argc, char* argv[]) {

    srand(time(0));

    ReportParams params;

    params.trials = (int) strtol(argv[1], NULL, 10);
    params.cap = (int) strtol(argv[2], NULL, 10);
    params.loadfactor = (double) strtol(argv[3], NULL, 10) / 100.0; 
    params.n_threads = (int) strtol(argv[4], NULL, 10);
    params.n_operations = (int) strtol(argv[5], NULL, 10);
    params.limit = 5 * static_cast<int>(log2(params.cap));
    params.n_populate = (int) params.cap * 0.4;
    params.low = 1;
    params.high = 1000;
    
    get_report(params);
}

