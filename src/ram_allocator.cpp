#include "include/ram_allocator.hpp"

#include <cstdint>
#include <new>
#include <sys/mman.h>

#include "include/meme.h"

namespace memesim {

RamAllocator::RamAllocator(std::size_t size)
    : size_{size}
    , frame_meta_{nullptr}
    , memory_{nullptr}
{
    memory_ = static_cast<RamFrame*>(ms_malloc(sizeof(RamFrame) * size_));
    frame_meta_ = static_cast<FrameMeta*>(ms_malloc(sizeof(FrameMeta) * size_));

    if (memory_ == nullptr || frame_meta_ == nullptr) {
        ms_free(frame_meta_);
        ms_free(memory_);

        throw std::bad_alloc{};
    }

    for (std::size_t i = 0; i < size_; i++) {
        new (&memory_[i]) RamFrame{};
        new (&frame_meta_[i]) FrameMeta{};
    }

    mlock(memory_, sizeof(RamFrame) * size_);
    mlock(frame_meta_, sizeof(FrameMeta) * size_);
}

RamAllocator::FramePtr RamAllocator::AllocateFrames(std::size_t count) {
    if (count == 0 || count > size_) {
        return RamAllocator::FramePtr{};
    }

    std::size_t sequence = 0;
    std::uintptr_t start = 0;

    for (std::size_t i = 0; i < size_; i++) {
        if (frame_meta_[i].allocated) {
            sequence = 0;
            continue;
        }

        if (sequence++ == 0) {
            start = i;
        }

        if (sequence == count) {
            for (std::uintptr_t frame = start; frame < start + count; frame++) {
                frame_meta_[frame].allocated = true;
            }

            return RamAllocator::FramePtr{start + 1, &memory_[start]};
        }
    }

    return RamAllocator::FramePtr{};
}

void RamAllocator::DeallocateFrame(RamAllocator::FramePtr ptr) {
    if (ptr == RamAllocator::FramePtr{} || memory_ == nullptr || frame_meta_ == nullptr) {
        return;
    }

    std::uintptr_t begin{reinterpret_cast<std::uintptr_t>(memory_)};
    std::uintptr_t target{ptr - 1};
    std::uintptr_t bytes{sizeof(RamFrame) * size_};

    if (target < begin || target >= begin + bytes) {
        return;
    }

    std::uintptr_t offset = target - begin;
    if (offset % sizeof(RamFrame) != 0) {
        return;
    }

    std::size_t frame{static_cast<std::size_t>(offset / sizeof(RamFrame))};
    frame_meta_[frame].allocated = false;
}

RamAllocator::~RamAllocator() {
    munlock(frame_meta_, sizeof(FrameMeta) * size_);
    munlock(memory_, sizeof(RamFrame) * size_);

    for (std::size_t i = 0; i < size_; i++) {
        frame_meta_[i].~FrameMeta();
        memory_[i].~RamFrame();
    }

    ms_free(frame_meta_);
    ms_free(memory_);
}

}
