#ifndef MEMESIM_PROCESS_HPP_
#define MEMESIM_PROCESS_HPP_

#include <cstddef>
#include <unistd.h>

#include "virtual_space.hpp"

namespace memesim {

using Pid = pid_t;

class Process {
public:
    Process(Pid pid, Pager& pager);

    Process(const Process&) = delete;

    Process& operator=(const Process&) = delete;

    Pid PidValue() const;

    VPointer Allocate(std::size_t size);

    void Deallocate(VPointer ptr, std::size_t size);

private:
    Pid pid_;
    VirtualSpace virtual_space_;
};

}

#endif // MEMESIM_PROCESS_HPP_
