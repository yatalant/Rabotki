#include <iostream>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>

std::queue<int> rawTaskList;

void taskGenerator(int workerId, int jobCount) {
    for (int i = 0; i < jobCount; ++i) {
        rawTaskList.push(workerId * 1000 + i);
    }
}

void taskProcessor(int workerId) {
    while (!rawTaskList.empty()) {
        int currentJob = rawTaskList.front();
        rawTaskList.pop();
    }
}

std::queue<int> protectedTaskList;
std::atomic<bool> processingActive(false);

void safeGenerator(int workerId, int jobCount) {
    for (int i = 0; i < jobCount; ++i) {
        protectedTaskList.push(workerId * 1000 + i);
    }
}

void safeProcessor(int workerId) {
    while (!processingActive || !protectedTaskList.empty()) {
        if (!protectedTaskList.empty()) {
            int currentJob = protectedTaskList.front();
            protectedTaskList.pop();
        }
    }
}

std::queue<int> lockedTaskList;
std::mutex queueGuard;

void guardedGenerator(int workerId, int jobCount) {
    for (int i = 0; i < jobCount; ++i) {
        std::lock_guard<std::mutex> accessLock(queueGuard);
        lockedTaskList.push(workerId * 1000 + i);
    }
}

void guardedProcessor(int workerId) {
    while (true) {
        std::lock_guard<std::mutex> accessLock(queueGuard);
        if (lockedTaskList.empty()) break;
        int currentJob = lockedTaskList.front();
        lockedTaskList.pop();
    }
}

void runPerformanceTest(int genThreads, int procThreads, int jobsPerGen) {
    // Тест первого подхода
    {
        std::vector<std::thread> generators, processors;
        auto timerStart = std::chrono::steady_clock::now();

        for (int i = 0; i < genThreads; ++i) {
            generators.emplace_back(taskGenerator, i, jobsPerGen);
        }

        for (int i = 0; i < procThreads; ++i) {
            processors.emplace_back(taskProcessor, i);
        }

        for (auto& t : generators) t.join();
        for (auto& t : processors) t.join();

        auto timerEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
        std::cout << "Незащищенная обработка: " << elapsed.count() << " мс" << std::endl;
    }

    {
        std::vector<std::thread> generators, processors;
        auto timerStart = std::chrono::steady_clock::now();

        for (int i = 0; i < genThreads; ++i) {
            generators.emplace_back(safeGenerator, i, jobsPerGen);
        }

        for (int i = 0; i < procThreads; ++i) {
            processors.emplace_back(safeProcessor, i);
        }

        for (auto& t : generators) t.join();
        processingActive = true;
        for (auto& t : processors) t.join();
        processingActive = false;

        auto timerEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
        std::cout << "Атомарная защита: " << elapsed.count() << " мс" << std::endl;
    }

    {
        std::vector<std::thread> generators, processors;
        auto timerStart = std::chrono::steady_clock::now();

        for (int i = 0; i < genThreads; ++i) {
            generators.emplace_back(guardedGenerator, i, jobsPerGen);
        }

        for (int i = 0; i < procThreads; ++i) {
            processors.emplace_back(guardedProcessor, i);
        }

        for (auto& t : generators) t.join();
        for (auto& t : processors) t.join();

        auto timerEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
        std::cout << "Блокировка мьютексом: " << elapsed.count() << " мс" << std::endl;
    }
}

int main() {
    const int generatorCount = 2;
    const int processorCount = 2;
    const int tasksPerGenerator = 1000;

    runPerformanceTest(generatorCount, processorCount, tasksPerGenerator);

    return 0;
}
