#ifndef MEMESIM_SWAPPER_HPP_
#define MEMESIM_SWAPPER_HPP_

#include <cstddef>
#include <cstdint>
#include <fstream>

#include "page_table.hpp"
#include "ram_allocator.hpp"

namespace memesim {

class Swapper {
public:
    Swapper(std::size_t swap_size, std::size_t ram_boundary, const char* swap_path); 

    inline std::uintptr_t NextSwapAddress();

    bool Swap(PTEntry& resident_entry, PTEntry& disk_entry);

    bool PageOut(PTEntry& resident_entry);

private:
    inline RamFrame* RamFrameAt(std::uintptr_t physical_address);

    std::uint64_t DiskOffset(std::uintptr_t physical_address) const;

    bool ReadDiskFrame(std::uintptr_t physical_address, RamFrame& frame);

    bool WriteDiskFrame(std::uintptr_t physical_address, const RamFrame& frame);

    std::size_t ram_boundary_;
    std::size_t swap_size_;
    std::size_t pages_swapped_;
    std::fstream swap_file_;
};

}

#endif // MEMESIM_SWAPPER_HPP_
