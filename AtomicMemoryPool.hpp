#pragma once
#include "pch.h"

template<typename T>
class AtomicMemoryPool
{
private:
	struct Block
	{
		Block* next;
	};
	static constexpr size_t max_align = std::max(alignof(Block), alignof(T));
	using AlignedStorage = typename std::aligned_storage<sizeof(T), max_align>::type;
	static constexpr size_t block_size = sizeof(Block) + sizeof(AlignedStorage);
	static constexpr size_t aligned_block_size = (block_size + max_align - 1) & ~(max_align - 1);

	std::byte* memoryBlock = nullptr;;
	std::atomic<Block*> head = nullptr;;
	const size_t maxBlockCount;

	void initialize() noexcept 
	{
		memoryBlock = static_cast<std::byte* const>(_aligned_malloc(aligned_block_size * maxBlockCount, max_align));
		head.store(reinterpret_cast<Block* const>(memoryBlock));
		for (size_t i = 0; i < maxBlockCount - 1; ++i) 
		{
			std::byte* const block = memoryBlock + i * aligned_block_size;
			Block* const blockPtr = new (block) Block();
			blockPtr->next = reinterpret_cast<Block* const>(block + aligned_block_size);
		}
		std::byte* const lastBlock = memoryBlock + (maxBlockCount - 1) * aligned_block_size;
		Block* const lastBlockPtr = new (lastBlock) Block();
		lastBlockPtr->next = nullptr; 
	}

public:
	explicit AtomicMemoryPool(const size_t maxBlockCount_) noexcept
		: maxBlockCount{ maxBlockCount_ }
		, memoryBlock{nullptr}
		, head{ nullptr }
	{
		initialize();
	}

	~AtomicMemoryPool()noexcept
	{
		_aligned_free(memoryBlock);
	}

	void expand(const size_t newCount)noexcept
	{
		if (newCount <= maxBlockCount)
		{
			return;
		}
		maxBlockCount = newCount;
		initialize();
	}
	void* const allocate() noexcept
	{
		Block* old_head = head.load();
		while (old_head != nullptr && !head.compare_exchange_weak(old_head, old_head->next)) {

		}

		NAGOX_ASSERT(nullptr != old_head,"Cannot Allocate Memory");

		//return old_head ? reinterpret_cast<std::byte*>(old_head) + sizeof(Block) : nullptr;
		return reinterpret_cast<std::byte* const>(old_head) + sizeof(Block);
	}

	void deallocate(void* const block) noexcept
	{
		if (!block)
		{
			return;
		}
		Block* const blockPtr = reinterpret_cast<Block* const>(static_cast<std::byte* const>(block) - sizeof(Block));
		blockPtr->next = head.load();
		while (!head.compare_exchange_weak(blockPtr->next, blockPtr)) {

		}
	}
};