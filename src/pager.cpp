#include "include/pager.hpp"

#include <cstddef>
#include <mutex>
#include <stdexcept>

namespace memesim {

Pager::Pager(std::size_t ram_frame_count, std::size_t swap_size, const char* swap_path)
    : swapper_{swap_size, ram_frame_count * paddress::kPageSize, swap_path}
    , allocator_{ram_frame_count}
    , victim_cursor_{0}
{}

PTEntry& Pager::SelectVictim(PageTable& page_table) {
    std::lock_guard<std::mutex> lock(mutex_);

    return SelectVictimUnlocked(page_table);
}

PTEntry& Pager::SelectVictimUnlocked(PageTable& page_table) {
    std::size_t entry_count = page_table.EntryCount();

    if (entry_count == 0) {
        throw std::runtime_error{"empty page table has no eviction victim"};
    }

    victim_cursor_ %= entry_count;

    for (std::size_t victim_class = 0; victim_class < 4; victim_class++) {
        for (std::size_t scanned = 0; scanned < entry_count; scanned++) {
            std::size_t index = (victim_cursor_ + scanned) % entry_count;

            PTEntry* entry = page_table.EntryAt(index);
            if (entry == nullptr || !entry->resident_) {
                continue;
            }

            if (InVictimClass(*entry, victim_class)) {
                victim_cursor_ = (index + 1) % entry_count;
                return *entry;
            }
        }
    }

    throw std::runtime_error{"no resident page available for eviction"};
}

bool Pager::MapPageUnlocked(PTEntry& entry) {
    if (entry.resident_) {
        return true;
    }

    RamAllocator::FramePtr frame = allocator_.AllocateFrames(1);
    if (frame == RamAllocator::FramePtr{}) {
        return false;
    }

    entry.physical_address_ = frame;
    entry.resident_ = true;
    entry.clean_ = true;
    entry.refererence_bit_ = true;

    return true;
}

bool Pager::MapPage(PageTable& page_table, PTEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (MapPageUnlocked(entry)) {
        return true;
    }

    PTEntry& victim = SelectVictimUnlocked(page_table);
    return swapper_.Swap(victim, entry);
}

bool Pager::InVictimClass(const PTEntry& entry, std::size_t victim_class) {
    bool referenced = entry.refererence_bit_;
    bool dirty = !entry.clean_;

    switch (victim_class) {
        case 0:
            return !referenced && !dirty;
        case 1:
            return !referenced && dirty;
        case 2:
            return referenced && !dirty;
        case 3:
            return referenced && dirty;
        default:
            return false;
    }
}

}
