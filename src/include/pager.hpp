#ifndef MEMESIM_PAGER_HPP_
#define MEMESIM_PAGER_HPP_

#include <mutex>

#include "page_table.hpp"
#include "ram_allocator.hpp"
#include "swapper.hpp"

namespace memesim {

class Pager {
public:
    Pager(std::size_t ram_frame_count, std::size_t swap_size, const char* swap_path);

    PTEntry& SelectVictim(PageTable& page_table);

    
    bool MapPage(PageTable& page_table, PTEntry& entry);
    
private:
    static bool InVictimClass(const PTEntry& entry, std::size_t victim_class);
    
    PTEntry& SelectVictimUnlocked(PageTable& page_table);

    bool MapPageUnlocked(PTEntry& entry);

    std::mutex mutex_;
    Swapper swapper_;
    RamAllocator allocator_;
    std::size_t victim_cursor_;
};

}

#endif // MEMESIM_PAGER_HPP_
