#include "include/virtual_space.hpp"

#include <mutex>
#include <stdexcept>

#include "include/address.hpp"

namespace memesim {

VirtualSpace::VirtualSpace(Pager& pager)
    : start_{0}
    , size_{UINTPTR_MAX - 1}
    , page_table_{}
    , pager_{pager}
{}

VPointer VirtualSpace::AllocatePages(std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (size == 0) {
        return VPointer{};
    }

    std::size_t page_count = size / vaddress::kPageSize;
    if (size % vaddress::kPageSize != 0) {
        page_count++;
    }

    PTEntry* first_entry = nullptr;

    for (std::size_t i = 0; i < page_count; i++) {
        PTEntry& entry = page_table_.AllocatePage();

        if (first_entry == nullptr) {
            first_entry = &entry;
        }
    }

    if (first_entry == nullptr) {
        return VPointer{};
    }

    return VPointer{first_entry->virtual_address_, this};
}

void VirtualSpace::DeallocatePages(VPointer pointer, std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (size == 0 || pointer.vs_ != this) {
        return;
    }

    std::size_t page_count = size / vaddress::kPageSize;
    if (size % vaddress::kPageSize != 0) {
        page_count++;
    }

    std::uintptr_t address = vaddress::PageBase(pointer.address_);

    for (std::size_t i = 0; i < page_count; i++) {
        PTEntry& entry = page_table_.FindByAddress(address + i * vaddress::kPageSize);

        entry.resident_ = false;
        entry.clean_ = true;
        entry.refererence_bit_ = false;
        entry.physical_address_ = RamAllocator::FramePtr{};
    }
}

VPointer::VPointer()
    : address_{0}
    , vs_{nullptr}
{}

VPointer::VPointer(std::uintptr_t address, VirtualSpace* vs)
    : address_{address}
    , vs_{vs}
{}

VPointer::operator char*() {
    if (vs_ == nullptr) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(vs_->mutex_);

    PTEntry& entry = vs_->page_table_.FindByAddress(address_); 

    if (!vs_->pager_.MapPage(vs_->page_table_, entry)) {
        throw std::runtime_error{"failed to map virtual page"};
    }

    entry.refererence_bit_ = true;

    return (*entry.physical_address_).data + vaddress::PageOffset(address_);
}

char& VPointer::operator*() {
    char* ptr = static_cast<char*>(*this);

    if (ptr == nullptr) {
        throw std::runtime_error{"dereferenced null virtual pointer"};
    }

    return *ptr;
}

std::uintptr_t VPointer::Address() const {
    return address_;
}

}
