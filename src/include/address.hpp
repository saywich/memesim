#ifndef MEMESIM_ADDRESS_HPP_
#define MEMESIM_ADDRESS_HPP_

#include <cstddef>
#include <cstdint>

namespace memesim::vaddress {

constexpr std::uintptr_t kPageOffsetBits = 12;
constexpr std::uintptr_t kPageIndexBits = 10;
constexpr std::uintptr_t kPageTableNodeBits = 10;

constexpr std::uintptr_t kPageSize = std::uintptr_t{1} << kPageOffsetBits;
constexpr std::uintptr_t kPageOffsetMask = kPageSize - 1;
constexpr std::uintptr_t kPageIndexMask = (std::uintptr_t{1} << kPageIndexBits) - 1;

inline std::size_t PageTableNodeIndex(std::uintptr_t addr) {
    return static_cast<std::size_t>(
        (addr >> (kPageIndexBits + kPageOffsetBits)) & kPageIndexMask
    );
}

inline std::size_t PageIndex(std::uintptr_t addr) {
    return static_cast<std::size_t>((addr >> kPageOffsetBits) & kPageIndexMask);
}

inline std::size_t PageOffset(std::uintptr_t addr) {
    return static_cast<std::size_t>(addr & kPageOffsetMask);
}

inline std::uintptr_t PageBase(std::uintptr_t addr) {
    return addr & ~kPageOffsetMask;
}

inline bool IsPageAligned(std::uintptr_t addr) {
    return PageOffset(addr) == 0;
}

inline std::uintptr_t MakeAddress(
    std::size_t node_index,
    std::size_t page_index,
    std::size_t page_offset
) {
    return
        ((static_cast<std::uintptr_t>(node_index) & kPageIndexMask)
            << (kPageIndexBits + kPageOffsetBits)) |
        ((static_cast<std::uintptr_t>(page_index) & kPageIndexMask)
            << kPageOffsetBits) |
        (static_cast<std::uintptr_t>(page_offset) & kPageOffsetMask);
}

}

namespace memesim::paddress {

constexpr std::uintptr_t kNullptr = 0;

constexpr std::uintptr_t kPageOffsetBits = 12;
constexpr std::uintptr_t kPageIndexBits = 20;

constexpr std::uintptr_t kPageSize = std::uintptr_t{1} << kPageOffsetBits;
constexpr std::uintptr_t kPageOffsetMask = kPageSize - 1;
constexpr std::uintptr_t kPageIndexMask = (std::uintptr_t{1} << kPageIndexBits) - 1;

inline std::size_t PageIndex(std::uintptr_t addr) {
    return static_cast<std::size_t>((addr >> kPageOffsetBits) & kPageIndexMask);
}

inline std::size_t PageOffset(std::uintptr_t addr) {
    return static_cast<std::size_t>(addr & kPageOffsetMask);
}

inline std::uintptr_t PageBase(std::uintptr_t addr) {
    return addr & ~kPageOffsetMask;
}

inline bool IsPageAligned(std::uintptr_t addr) {
    return PageOffset(addr) == 0;
}

inline std::uintptr_t MakeAddress(
    std::size_t page_index,
    std::size_t page_offset
) {
    return
        ((static_cast<std::uintptr_t>(page_index) & kPageIndexMask)
            << kPageOffsetBits) |
        (static_cast<std::uintptr_t>(page_offset) & kPageOffsetMask);
}

}

#endif // MEMESIM_ADDRESS_HPP_
