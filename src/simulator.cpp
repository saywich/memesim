#include "include/simulator.hpp"

#include <thread>
#include <vector>

namespace memesim {

Simulator::Simulator(std::size_t ram_frame_count, std::size_t swap_size, const char* swap_path)
    : mutex_{}
    , pager_{ram_frame_count, swap_size, swap_path}
    , processes_{}
    , threads_{}
    , next_pid_{1}
{}

Simulator::~Simulator() {
    JoinAll();
}

void Simulator::JoinAll() {
    std::vector<std::thread> threads;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        threads.swap(threads_);
    }

    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

}
