#include "include/swapper.hpp"
#include "include/address.hpp"

#include <algorithm>

namespace memesim {

Swapper::Swapper(std::size_t swap_size, std::size_t ram_boundary, const char* swap_path)
    : ram_boundary_{ram_boundary}
    , swap_size_{swap_size}
    , pages_swapped_{0}
    , swap_file_{}
{
    if (!paddress::IsPageAligned(ram_boundary)) {
        throw std::runtime_error{"ram_boundary must be page-aligned addres of first page in swap space"};
    }

    swap_file_.open(swap_path, std::ios::in | std::ios::out | std::ios::binary);
}

inline std::uintptr_t Swapper::NextSwapAddress() {
    return ram_boundary_ + pages_swapped_ * paddress::kPageSize;
}

bool Swapper::Swap(PTEntry& resident_entry, PTEntry& disk_entry) {
    if (!resident_entry.resident_) {
        return false;
    }

    RamAllocator::FramePtr resident_frame{resident_entry.physical_address_};    

    if (!PageOut(resident_entry)) {
        return false;
    }

    RamFrame temp;
    bool disk_obtained = ReadDiskFrame(disk_entry.physical_address_, temp);

    if (disk_obtained) {
        std::copy_n(temp.data, paddress::kPageSize, (*resident_frame).data);
    } else {
        std::fill_n((*resident_frame).data, paddress::kPageSize, 0);
    }

    disk_entry.physical_address_ = resident_frame;

    resident_entry.resident_ = false;
    resident_entry.clean_ = true;
    resident_entry.refererence_bit_ = false;

    disk_entry.resident_ = true;
    disk_entry.clean_ = true;
    disk_entry.refererence_bit_ = true;

    return true;
}

bool Swapper::PageOut(PTEntry& resident_entry) {
    if (!resident_entry.resident_ || resident_entry.physical_address_ >= ram_boundary_) {
        return false;
    }

    RamFrame* resident_frame{resident_entry.physical_address_};
    if (resident_frame == nullptr) {
        return false;
    }

    std::uintptr_t swap_address = NextSwapAddress();

    if (!WriteDiskFrame(swap_address, *resident_frame)) {
        return false;
    }
    
    resident_entry.physical_address_ = RamAllocator::FramePtr{swap_address, nullptr};

    ++pages_swapped_;

    resident_entry.resident_ = false;
    resident_entry.clean_ = true;
    resident_entry.refererence_bit_ = false;

    return true;
}

std::uint64_t Swapper::DiskOffset(std::uintptr_t physical_address) const {
    return static_cast<std::uint64_t>(paddress::PageBase(physical_address) - ram_boundary_);
}

bool Swapper::ReadDiskFrame(std::uintptr_t physical_address, RamFrame& frame) {
    if (!swap_file_.is_open()) {
        return false;
    }

    std::uint64_t offset = DiskOffset(physical_address);

    swap_file_.clear();
    swap_file_.seekg(0, std::ios::end);

    std::streamoff end = swap_file_.tellg();
    if (end < 0 || static_cast<std::uint64_t>(end) < offset + paddress::kPageSize) {
        return false;
    }

    swap_file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    swap_file_.read(frame.data, static_cast<std::streamsize>(paddress::kPageSize));

    return static_cast<std::uint64_t>(swap_file_.gcount()) == paddress::kPageSize;
}

bool Swapper::WriteDiskFrame(std::uintptr_t physical_address, const RamFrame& frame) {
    if (!swap_file_.is_open()) {
        return false;
    }

    std::uint64_t offset = DiskOffset(physical_address);

    swap_file_.clear();
    swap_file_.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    swap_file_.write(frame.data, static_cast<std::streamsize>(paddress::kPageSize));
    swap_file_.flush();

    return static_cast<bool>(swap_file_);
}

}
