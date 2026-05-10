#ifndef MEMESIM_SIMULATOR_HPP_
#define MEMESIM_SIMULATOR_HPP_

#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "pager.hpp"
#include "process.hpp"

namespace memesim {

class Simulator {
public:
    Simulator(std::size_t ram_frame_count, std::size_t swap_size, const char* swap_path);

    ~Simulator();
    
    template<typename F>
    Process& SpawnProcess(F&& callable);

    void JoinAll();

private:
    std::mutex mutex_;
    Pager pager_;
    std::vector<std::unique_ptr<Process>> processes_;
    std::vector<std::thread> threads_;
    Pid next_pid_;
};

template<typename F>
Process& Simulator::SpawnProcess(F&& callable) {
    Process* process = nullptr;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto process_ptr = std::make_unique<Process>(next_pid_++, pager_);

        process = process_ptr.get();

        processes_.push_back(std::move(process_ptr));
    }

    using Callable = std::decay_t<F>;

    Callable task{std::forward<F>(callable)};

    std::thread thread{
        [process, task = std::move(task)]() mutable {
            task(*process);
        }
    };

    {
        std::lock_guard<std::mutex> lock(mutex_);
        threads_.push_back(std::move(thread));
    }

    return *process;
}

}

#endif // MEMESIM_SIMULATOR_HPP_
