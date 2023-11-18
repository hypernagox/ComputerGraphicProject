#pragma once
#include "pch.h"

template <typename Key, typename Value>
class ConcurrentList
{
public:
	struct Node
	{
		std::pair<Key, Value> data;
		Node* next = nullptr;
	};
private:
	Node head;
	mutable std::shared_mutex m_sharedMutex;
	const HANDLE m_handle = GetProcessHeap();
public:
	ConcurrentList() noexcept = default;
	~ConcurrentList() noexcept
	{
		clear();
	}
	ConcurrentList(ConcurrentList&& other) noexcept
		: head(std::move(other.head))
		, m_handle{other.m_handle}
	{
	}
	template<typename ...Args>
	std::pair<Key, Value>* emplace_front(const Key& key_, Args&&... args)noexcept
	{
		Node* const newNode = static_cast<Node* const>(HeapAlloc(m_handle, NULL, sizeof(Node)));
		std::construct_at(&newNode->data, key_, Value{ std::forward<Args>(args)... });
		newNode->next = nullptr;
		std::unique_lock<std::shared_mutex> s_lock{ m_sharedMutex };
		newNode->next = head.next;
		head.next = newNode;
		return &newNode->data;
	}
	std::pair<Key, Value>* find(const Key& key_) noexcept
	{
		Node* curNode = &head;
		std::pair<Key, Value>* targetData = nullptr;
		{
			std::shared_lock<std::shared_mutex> s_lock{ m_sharedMutex };
			curNode = head.next;
			while (curNode)
			{
				if (key_ == curNode->data.first)
				{
					targetData = &curNode->data;
					break;
				}
				curNode = curNode->next;
			}
		}
		return targetData;
	}
	const std::pair<Key, Value>* find(const Key& key_)const noexcept
	{
		const Node* curNode = &head;
		const std::pair<Key, Value>* targetData = nullptr;
		{
			std::shared_lock<std::shared_mutex> s_lock{ m_sharedMutex };
			curNode = head.next;
			while (curNode)
			{
				if (key_ == curNode->data.first)
				{
					targetData = &curNode->data;
					break;
				}
				curNode = curNode->next;
			}
		}
		return targetData;
	}
	void erase(const Key& key_)noexcept
	{
		Node* prevNode = &head;
		std::unique_lock<std::shared_mutex> s_lock{ m_sharedMutex };
		Node* curNode = prevNode->next;
		while (curNode)
		{
			if (key_ == curNode->data.first)
			{
				prevNode->next = curNode->next;
				std::destroy_at(curNode);
				HeapFree(m_handle, NULL, curNode);
				return;
			}
			prevNode = curNode;
			curNode = curNode->next;
		}
	}
	void clear()noexcept
	{
		Node* current = head.next;
		while (current)
		{
			Node* const temp = current;
			current = current->next;
			std::destroy_at(temp);
			HeapFree(m_handle, NULL, temp);
		}
	}
};

template <typename Key, typename Value>
class ConcurrentHashMap
{
private:
	std::vector<ConcurrentList<const Key, Value>> buckets;
	std::hash<Key> hasher;
public:
	ConcurrentHashMap() noexcept : buckets(1024) { }

	template <typename ...Args>
	std::pair<const Key, Value>* emplace(const Key& key, Args&&... args) noexcept
	{
		const size_t index = hasher(key) % buckets.size();
		const auto iter = buckets[index].find(key);
		return iter ? iter : buckets[index].emplace_front(key, std::forward<Args>(args)...);
	}

	template <typename ...Args>
	std::pair<std::pair<const Key, Value>*,bool> try_emplace(const Key& key, Args&&... args) noexcept
	{
		const size_t index = hasher(key) % buckets.size();
		const auto iter = buckets[index].find(key);
		return iter ? std::make_pair(iter,false) : std::make_pair(buckets[index].emplace_front(key, std::forward<Args>(args)...),true);
	}

	const std::pair<const Key, Value>* find(const Key& key)const noexcept
	{
		const size_t index = hasher(key) % buckets.size();
		return buckets[index].find(key);
	}

	std::pair<const Key, Value>* find(const Key& key) noexcept
	{
		const size_t index = hasher(key) % buckets.size();
		return buckets[index].find(key);
	}


	void erase(const Key& key) noexcept
	{
		const size_t index = hasher(key) % buckets.size();
		buckets[index].erase(key);
	}

	void reserve(const size_t newCapacity) noexcept
	{
		buckets.resize(newCapacity);
	}

	void clear() noexcept
	{
		for (auto& bucket : buckets)
		{
			bucket.clear();
		}
	}
};