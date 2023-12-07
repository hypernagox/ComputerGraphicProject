#pragma once
#include "pch.h"

template<typename T>
class AtomicMemoryPool
{
private:
    struct alignas(std::hardware_constructive_interference_size) Block
    {
        std::atomic<uint64_t> combined;
    };

    std::vector<std::byte> memoryBlock;
    std::atomic<uint64_t> head;
    const size_t blockSize;
    const size_t maxBlockCount;
    static constexpr uint16_t maxTagValue = (1 << 16) - 1;
private:
    const uint64_t packPointerAndTag(const Block* const ptr, const uint16_t tag) const noexcept {
        const uintptr_t ptrVal = reinterpret_cast<uintptr_t>(ptr);
        return (static_cast<const uint64_t>(ptrVal) & 0x0000FFFFFFFFFFFF) | (static_cast<const uint64_t>(tag) << 48);
    }

    Block* const unpackPointer(const uint64_t combined) const noexcept {
        return reinterpret_cast<Block* const>(combined & 0x0000FFFFFFFFFFFF);
    }

    const uint16_t unpackTag(const uint64_t combined) const noexcept {
        return static_cast<const uint16_t>(combined >> 48);
    }

    void initialize() noexcept
    {
        std::byte* const blockStart = memoryBlock.data();

        for (size_t i = 0; i < maxBlockCount - 1; ++i)
        {
            Block* const block = new (blockStart + i * blockSize) Block();
            const uint64_t nextCombined = packPointerAndTag(reinterpret_cast<Block*>(blockStart + (i + 1) * blockSize), 0);
            block->combined.store(nextCombined, std::memory_order_relaxed);
        }

        Block* const lastBlock = new (blockStart + (maxBlockCount - 1) * blockSize) Block();
        lastBlock->combined.store(packPointerAndTag(nullptr, 0), std::memory_order_relaxed);

        head.store(packPointerAndTag(reinterpret_cast<Block* const>(blockStart), 0), std::memory_order_relaxed);
    }
public:
    explicit AtomicMemoryPool(const size_t count)
        : blockSize{ sizeof(Block) + sizeof(T) }
        , maxBlockCount{ count }
    {
        const size_t totalSize = blockSize * maxBlockCount;
        std::byte* const rawMemory = static_cast<std::byte*>(operator new[](totalSize, std::align_val_t(std::hardware_constructive_interference_size)));
        memoryBlock.assign(rawMemory, rawMemory + totalSize);
        operator delete[](rawMemory, std::align_val_t(std::hardware_constructive_interference_size));
        initialize();
    }

    T* const allocate() noexcept
    {
        uint64_t oldCombined = head.load(std::memory_order_relaxed);
        uint64_t newCombined;
        Block* currentBlock;
        do {
            currentBlock = unpackPointer(oldCombined);
            if (!currentBlock)
            {
                NAGOX_ASSERT(false, "Out of Memory");
                return nullptr;
            }
            const uint16_t newTag = unpackTag(oldCombined) + 1;
            Block* const nextBlock = unpackPointer(currentBlock->combined.load(std::memory_order_relaxed));
            newCombined = packPointerAndTag(nextBlock, newTag);
        } while (!head.compare_exchange_weak(oldCombined, newCombined,
            std::memory_order_acquire,
            std::memory_order_relaxed));

        return reinterpret_cast<T* const>(currentBlock + 1);
    }

    void deallocate(void* const object) noexcept
    {
        if (!object)
        {
            return;
        }
        Block* const blockPtr = reinterpret_cast<Block* const>(object) - 1;
        uint64_t oldHead = head.load(std::memory_order_relaxed);
        uint64_t newCombined;
        do {
            const uint16_t newTag = unpackTag(oldHead) + 1;
            newCombined = packPointerAndTag(blockPtr, newTag);
            blockPtr->combined.store(oldHead, std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(oldHead, newCombined,
            std::memory_order_release,
            std::memory_order_relaxed));
    }

    void checkAndResetIfNeeded()noexcept
    {
        const uint16_t currentTag = unpackTag(head.load(std::memory_order_relaxed));
        if (currentTag >= maxTagValue)
        {
            initialize();
        }
    }

    const bool isNeedReset()const noexcept {
        return 59000 <= unpackTag(head.load(std::memory_order_relaxed));
    }
};