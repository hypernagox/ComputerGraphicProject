#pragma once
#include "pch.h"

template <typename T>
class DoubleLockQueue
{
private:
	struct Node {
		T data;
		Node* next;
	};
	Node* head;
	Node* tail;
	const HANDLE heapHandle = ::GetProcessHeap();
	SpinLock headLock, tailLock;
public:
	DoubleLockQueue()noexcept {
		head = tail = static_cast<Node* const>(::HeapAlloc(heapHandle,NULL,sizeof(Node)));
		head->next = tail->next = nullptr;
	}
	~DoubleLockQueue()noexcept {
		while (head != nullptr) {
			Node* const tmp = head;
			head = head->next;
			::HeapFree(heapHandle, NULL, tmp);
		}
	}
	template <typename... Args>
	void emplace(Args&&... args) noexcept {
		Node* const value = static_cast<Node* const>(::HeapAlloc(heapHandle, NULL, sizeof(Node)));
		std::construct_at(&value->data, std::forward<Args>(args)...);
		value->next = nullptr;
		std::lock_guard<SpinLock> lock{ tailLock };
		tail->next = value;
		tail = value;
	}
	const bool try_pop(T& _target)noexcept {
		//std::lock_guard<SpinLock> lock{ headLock };
		const Node* const oldHead = head;
		Node* const newHead = head->next;
		if (newHead)
		{
			_target = std::move(newHead->data);
			head = newHead;
			::HeapFree(heapHandle, NULL, oldHead);
			return true;
		}
		else
		{
			return false;
		}
	}
	void pop(T& _target, std::unique_lock<SpinLock>& _lock)noexcept {
		Node* const oldHead = head;
		Node* const newHead = head->next;
		_target = std::move(newHead->data);
		head = newHead;
		_lock.unlock();
		::HeapFree(heapHandle, NULL, oldHead);
	}
	const bool empty()const noexcept {
		return !head->next;
	}
};