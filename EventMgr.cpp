#include "pch.h"
#include "EventMgr.h"
#include "CoRoutine.hpp"

EventMgr::EventMgr()
{
}

EventMgr::~EventMgr()
{
}

void EventMgr::Update()
{
	for (auto& eve : m_vecGameEvent)
	{
		std::visit(m_eveHandler, std::move(eve));
	}

	for (auto iter = m_listCoTask.begin(); iter != m_listCoTask.end();)
	{
		if (iter->done())
		{
			iter = m_listCoTask.erase(iter);
		}
		else
		{
			iter->resume();
			++iter;
		}
	}

	for (auto iter = m_mapCoTask.begin(); iter != m_mapCoTask.end();)
	{
		if (iter->second.done())
		{
			iter = m_mapCoTask.erase(iter);
		}
		else
		{
			iter->second.resume();
			++iter;
		}
	}

	for (const auto& eve : m_vecEvent)
	{
		eve();
	}

	m_vecEvent.clear();
	m_vecGameEvent.clear();
	m_vecDeadObj.clear();
}

void EventMgr::Init()
{
	m_vecEvent.reserve(100);
	m_vecGameEvent.reserve(100);
	m_mapCoTask.reserve(100);
}

void EventMgr::AddCoRoutine(string_view _strName, CoRoutine&& _coTask)
{
	m_mapCoTask.emplace(_strName, std::move(_coTask));
}

void EventMgr::AddCoRoutine(CoRoutine&& _coTask)
{
	m_listCoTask.emplace_back(std::move(_coTask));
}

void EventMgr::AddDeadObj(shared_ptr<GameObj>&& pDeadObj_)
{
	m_vecDeadObj.emplace_back(std::move(pDeadObj_));
}
