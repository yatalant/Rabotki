#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <boost/thread.hpp>
#include <boost/ref.hpp>

void sort_part(std::vector<int>& data, size_t start, size_t end) {
    std::sort(data.begin() + start, data.begin() + end);
}

void parallel_sort(std::vector<int>& data, int num_threads) {
    if (num_threads <= 0) num_threads = 1;
    size_t part_size = data.size() / num_threads;
    std::vector<boost::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        size_t start = i * part_size;
        size_t end = (i == num_threads - 1) ? data.size() : start + part_size;
        threads.emplace_back(sort_part, boost::ref(data), start, end);
    }

    for (auto& t : threads) {
        t.join();
    }

    for (int i = 1; i < num_threads; ++i) {
        size_t start = 0;
        size_t mid = i * part_size;
        size_t end = (i + 1) * part_size;
        if (end > data.size()) end = data.size();
        std::inplace_merge(data.begin() + start, data.begin() + mid, data.begin() + end);
    }
}

int main() {
    const int size = 1000000;
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 999999);

    std::generate(data.begin(), data.end(), [&]() { return dist(gen); });
    
    for (int threads : {1, 2, 4, 8}) {
        std::vector<int> copy = data;

        auto start = std::chrono::high_resolution_clock::now();

        if (threads == 1) {
            std::sort(copy.begin(), copy.end());
        } else {
            parallel_sort(copy, threads);
        }

        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double>(end - start).count();

        bool sorted = std::is_sorted(copy.begin(), copy.end());
        std::cout << "Потоков: " << threads << ", Время: " << time
                  << " сек., Отсортирован: " << (sorted ? "да" : "нет") << std::endl;
    }

    return 0;
}

