#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

using namespace std;

mutex mtx; 
condition_variable cv; 
atomic<int> completed_stages(0);

void assembly_stage() {
    unique_lock<mutex> lock(mtx);
    cout << "Этап 1: Сборка компонентов...\n";
    this_thread::sleep_for(chrono::seconds(1)); 
    completed_stages++;
    cout << "Этап 1 завершен\n";
    cv.notify_all(); 
}

void painting_stage() {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [] { return completed_stages >= 1; }); 
    cout << "Этап 2: Покраска...\n";
    this_thread::sleep_for(chrono::seconds(1));
    completed_stages++;
    cout << "Этап 2 завершен\n";
    cv.notify_all();
}

void packaging_stage() {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [] { return completed_stages >= 2; }); 
    cout << "Этап 3: Упаковка...\n";
    this_thread::sleep_for(chrono::seconds(1));
    completed_stages++;
    cout << "Этап 3 завершен\n";
}

int main() {
    thread t1(assembly_stage);
    thread t2(painting_stage);
    thread t3(packaging_stage);

    t1.join();
    t2.join();
    t3.join();

    cout << "Все этапы завершены\n";
    return 0;
}
