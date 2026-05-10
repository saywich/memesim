#include "include/page_table.hpp"

#include <new>
#include <stdexcept>

#include "include/address.hpp"
#include "include/meme.h"

namespace memesim {

PageTable::PageTable()
    : head_{nullptr}
    , page_count_{0}
    , node_count_{0}
{}

PageTable::~PageTable() {
    PTNode* node = head_;

    while (node != nullptr) {
        PTNode* next = node->next;

        node->~PTNode();

        ms_free(node);

        node = next;
    }
}

PTEntry& PageTable::AllocatePage() {
    PTNode* tail_node = TailNode();

    if (tail_node == nullptr || tail_node->insert_idx == PT_ENTRY_SIZE) {
        PTNode* created = CreateNode();

        if (created == nullptr) {
            throw std::bad_alloc{};
        }

        if (tail_node == nullptr) {
            head_ = created;
        } else {
            tail_node->next = created;
        }

        tail_node = created;
    }

    std::uintptr_t virtual_address = vaddress::MakeAddress(
        node_count_ - 1,
        tail_node->insert_idx,
        0
    );
    PTEntry& entry = tail_node->entries[tail_node->insert_idx++];

    entry.resident_ = false;
    entry.clean_ = true;
    entry.refererence_bit_ = false;
    entry.virtual_address_ = virtual_address;
    entry.physical_address_ = RamAllocator::FramePtr{};

    ++page_count_;

    return entry;
}

std::size_t PageTable::EntryCount() const {
    return page_count_;
}

std::size_t PageTable::NodeCount() const {
    return node_count_;
}

PTEntry& PageTable::FindByAddress(std::uintptr_t address) {
    std::size_t node_idx = vaddress::PageTableNodeIndex(address);
    std::size_t page_idx = vaddress::PageIndex(address);

    std::size_t current_node_idx{0};
    PTNode* current_node{head_};

    while (current_node_idx < node_idx && current_node != nullptr) {
        current_node = current_node->next;
        ++current_node_idx;
    }

    if (current_node == nullptr || page_idx >= current_node->insert_idx) {
        throw std::out_of_range{"page table address is not mapped"};
    }

    PTEntry& entry = current_node->entries[page_idx];

    return entry;
}

PTEntry* PageTable::EntryAt(std::size_t index) {
    for (PTNode* node = head_; node != nullptr; node = node->next) {
        if (index < node->insert_idx) {
            return &node->entries[index];
        }

        index -= node->insert_idx;
    }

    return nullptr;
}

const PTEntry* PageTable::EntryAt(std::size_t index) const {
    for (const PTNode* node = head_; node != nullptr; node = node->next) {
        if (index < node->insert_idx) {
            return &node->entries[index];
        }

        index -= node->insert_idx;
    }

    return nullptr;
}

PTNode* PageTable::TailNode() {
    PTNode* node = head_;

    if (node == nullptr) {
        return nullptr;
    }

    while (node->next != nullptr) {
        node = node->next;
    }

    return node;
}

PTNode* PageTable::CreateNode() {
    void* storage = ms_malloc(sizeof(PTNode));

    if (storage == nullptr) {
        return nullptr;
    }

    PTNode* node = new (storage) PTNode{};
    node->insert_idx = 0;
    node->next = nullptr;

    ++node_count_;

    return node;
}

}
