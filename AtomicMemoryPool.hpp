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
   const uint64_t packPointerAndTag(const Block* const ptr, const uint16_t tag) const noexcept{
        const uintptr_t ptrVal = reinterpret_cast<uintptr_t>(ptr);
        return (static_cast<const uint64_t>(ptrVal) & 0x0000FFFFFFFFFFFF) | (static_cast<const uint64_t>(tag) << 48);
    }

    Block* const unpackPointer(const uint64_t combined) const noexcept{
        return reinterpret_cast<Block* const>(combined & 0x0000FFFFFFFFFFFF);
    }

   const uint16_t unpackTag(const uint64_t combined) const noexcept{
        return static_cast<const uint16_t>(combined >> 48);
    }

   void initialize() noexcept 
   {
       std::byte* const blockStart = memoryBlock.data();

       for (size_t i = 0; i < maxBlockCount - 1; ++i) 
       {
           Block* const block = new (blockStart + i * blockSize) Block();
           const uint64_t nextCombined = packPointerAndTag(reinterpret_cast<Block*>(blockStart + (i + 1) * blockSize), 0);
           block->combined.store(nextCombined,std::memory_order_relaxed);
       }

       Block* const lastBlock = new (blockStart + (maxBlockCount - 1) * blockSize) Block();
       lastBlock->combined.store(packPointerAndTag(nullptr, 0),std::memory_order_relaxed);

       head.store(packPointerAndTag(reinterpret_cast<Block* const>(blockStart), 0),std::memory_order_relaxed);
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



//template<typename T>
//class AtomicMemoryPool
//{
//private:
//	struct Block
//	{
//		Block* next;
//	};
//	static constexpr size_t max_align = std::max(alignof(Block), alignof(T));
//	using AlignedStorage = typename std::aligned_storage<sizeof(T), max_align>::type;
//	static constexpr size_t block_size = sizeof(Block) + sizeof(AlignedStorage);
//	static constexpr size_t aligned_block_size = (block_size + max_align - 1) & ~(max_align - 1);
//
//	std::byte* memoryBlock = nullptr;;
//	std::atomic<Block*> head = nullptr;;
//	const size_t maxBlockCount;
//
//	void initialize() noexcept 
//	{
//		memoryBlock = static_cast<std::byte* const>(_aligned_malloc(aligned_block_size * maxBlockCount, std::hardware_destructive_interference_size));
//		head.store(reinterpret_cast<Block* const>(memoryBlock));
//		for (size_t i = 0; i < maxBlockCount - 1; ++i) 
//		{
//			std::byte* const block = memoryBlock + i * aligned_block_size;
//			Block* const blockPtr = new (block) Block();
//			blockPtr->next = reinterpret_cast<Block* const>(block + aligned_block_size);
//		}
//		std::byte* const lastBlock = memoryBlock + (maxBlockCount - 1) * aligned_block_size;
//		Block* const lastBlockPtr = new (lastBlock) Block();
//		lastBlockPtr->next = nullptr; 
//	}
//
//public:
//	explicit AtomicMemoryPool(const size_t maxBlockCount_) noexcept
//		: maxBlockCount{ maxBlockCount_ }
//		, memoryBlock{nullptr}
//		, head{ nullptr }
//	{
//		initialize();
//	}
//
//	~AtomicMemoryPool()noexcept
//	{
//		_aligned_free(memoryBlock);
//	}
//
//	void expand(const size_t newCount)noexcept
//	{
//		if (newCount <= maxBlockCount)
//		{
//			return;
//		}
//		maxBlockCount = newCount;
//		initialize();
//	}
//	void* const allocate() noexcept
//	{
//		Block* old_head = head.load(std::memory_order_relaxed);
//		while (old_head != nullptr && !head.compare_exchange_weak(old_head
//			, old_head->next
//			, std::memory_order_acquire
//			, std::memory_order_relaxed
//		))
//		{
//		}
//
//		NAGOX_ASSERT(nullptr != old_head,"Cannot Allocate Memory");
//
//		//return old_head ? reinterpret_cast<std::byte*>(old_head) + sizeof(Block) : nullptr;
//		return reinterpret_cast<std::byte* const>(old_head) + sizeof(Block);
//	}
//
//	void deallocate(void* const block) noexcept
//	{
//		if (!block)
//		{
//			return;
//		}
//		Block* const blockPtr = reinterpret_cast<Block* const>(static_cast<std::byte* const>(block) - sizeof(Block));
//		blockPtr->next = head.load(std::memory_order_relaxed);
//		while (!head.compare_exchange_weak(blockPtr->next
//			, blockPtr
//			, std::memory_order_release
//			, std::memory_order_relaxed
//		))
//		{
//		}
//	}
//};

