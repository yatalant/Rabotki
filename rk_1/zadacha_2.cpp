#include <iostream>
#include <thread>
#include <semaphore.h>
#include <vector>
#include <chrono>

using namespace std;

sem_t order_processing_sem; 
sem_t order_delivery_sem;   

void process_order(int order_id) {
    cout << "Заказ #" << order_id << ": Начата обработка...\n";
    this_thread::sleep_for(chrono::seconds(1)); 
    cout << "Заказ #" << order_id << ": Обработка завершена\n";
    sem_post(&order_processing_sem); 
}

void deliver_order(int order_id) {
    sem_wait(&order_processing_sem); 
    cout << "Заказ #" << order_id << ": Начата доставка...\n";
    this_thread::sleep_for(chrono::seconds(1));
    cout << "Заказ #" << order_id << ": Доставка завершена\n";
    sem_post(&order_delivery_sem); 
}

int main() {
    const int num_orders = 5;
    sem_init(&order_processing_sem, 0, 0); 
    sem_init(&order_delivery_sem, 0, 0);

    vector<thread> processing_threads;
    vector<thread> delivery_threads;

    
    for (int i = 1; i <= num_orders; ++i) {
        processing_threads.emplace_back(process_order, i);
        delivery_threads.emplace_back(deliver_order, i);
    }

    for (auto& t : processing_threads) t.join();
    for (auto& t : delivery_threads) t.join();

    sem_destroy(&order_processing_sem);
    sem_destroy(&order_delivery_sem);

    cout << "Все заказы обработаны и доставлены\n";
    return 0;
}
