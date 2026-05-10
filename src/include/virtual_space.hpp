#ifndef MEMESIM_VIRTUAL_SPACE_HPP_
#define MEMESIM_VIRTUAL_SPACE_HPP_

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "page_table.hpp"
#include "pager.hpp"

namespace memesim {

class VPointer;

class VirtualSpace {
public:
    explicit VirtualSpace(Pager& pager);

    VPointer AllocatePages(std::size_t size);

    void DeallocatePages(VPointer pointer, std::size_t size);

private:
    friend class VPointer;

    std::uintptr_t start_;
    std::uintptr_t size_;
    PageTable page_table_;
    Pager& pager_;
    std::mutex mutex_;
};

class VPointer {
public:
    VPointer();

    VPointer(std::uintptr_t address, VirtualSpace* vs);

    operator char*();

    char& operator*();

    std::uintptr_t Address() const;

private:
    friend class VirtualSpace;

    std::uintptr_t address_;
    VirtualSpace* vs_;
};

}

#endif // MEMESIM_VIRTUAL_SPACE_HPP_
