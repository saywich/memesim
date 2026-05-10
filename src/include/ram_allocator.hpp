#ifndef MEMESIM_RAM_ALLOCATOR_HPP_
#define MEMESIM_RAM_ALLOCATOR_HPP_

#include <cstddef>

#include "address.hpp"
#include "pointer.hpp"

namespace memesim {

struct RamFrame {
    char data[paddress::kPageSize];
};

struct FrameMeta {
    FrameMeta() : allocated{false} {}

    bool allocated;
};

class RamAllocator {
public:
    using FramePtr = Pointer<std::uintptr_t, RamFrame*>;

    explicit RamAllocator(std::size_t size);

    FramePtr AllocateFrames(std::size_t count);

    void DeallocateFrame(FramePtr ptr);

    ~RamAllocator();

private:
    std::size_t size_;
    FrameMeta* frame_meta_;
    RamFrame* memory_;
};

}

#endif // MEMESIM_RAM_ALLOCATOR_HPP_
