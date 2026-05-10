#ifndef MEMESIM_PAGE_TABLE_HPP_
#define MEMESIM_PAGE_TABLE_HPP_

#include <cstddef>
#include <cstdint>

#include "ram_allocator.hpp"

#ifndef PT_ENTRY_SIZE
#define PT_ENTRY_SIZE 1024
#endif // PT_ENTRY_SIZE

namespace memesim {

class Swapper;
class Pager;

struct PTEntry {
    bool resident_;
    bool clean_;
    bool refererence_bit_;

    std::uintptr_t virtual_address_;
    RamAllocator::FramePtr physical_address_;
};

struct PTNode {
    PTEntry entries[PT_ENTRY_SIZE];
    std::size_t insert_idx;
    PTNode* next;
};

class PageTable {
public:
    PageTable();

    PageTable(const PageTable&) = delete;

    PageTable& operator=(const PageTable&) = delete;

    ~PageTable();

    PTEntry& AllocatePage();

    PTEntry& FindByAddress(std::uintptr_t address);

    PTEntry* EntryAt(std::size_t index);

    const PTEntry* EntryAt(std::size_t index) const;

    std::size_t EntryCount() const;

    std::size_t NodeCount() const;

private:
    PTNode* TailNode();

    PTNode* CreateNode();

    PTNode* head_;
    std::size_t page_count_;
    std::size_t node_count_;
};

}

#endif // MEMESIM_PAGE_TABLE_HPP_
