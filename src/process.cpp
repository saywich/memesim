#include "include/process.hpp"

namespace memesim {

Process::Process(Pid pid, Pager& pager)
    : pid_{pid}
    , virtual_space_{pager}
{}

Pid Process::PidValue() const {
    return pid_;
}

VPointer Process::Allocate(std::size_t size) {
    return virtual_space_.AllocatePages(size);
}

void Process::Deallocate(VPointer ptr, std::size_t size) {
    virtual_space_.DeallocatePages(ptr, size);
}

}
