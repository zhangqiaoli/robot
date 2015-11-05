#include "stdafx.h"
#include "SessionManager.h"


SessionManager::SessionManager(void)
{
}


SessionManager::~SessionManager(void)
{
	Free();
}

bool SessionManager::Init()
{
	return true;
}

void SessionManager::Free()
{
	m_mapSession.RemoveAll();
}

WebSocketSessionPtr SessionManager::CreateSession(SAWebSocketPtr spWebSocket)
{
	return WebSocketSessionPtr(new WebSocketSession(spWebSocket));
}

void SessionManager::AddSession(WebSocketSessionPtr spSession)
{
	if (spSession)
	{
		m_mapSession.SetAt(spSession->GetSessionID(), spSession);
	}
}

WebSocketSessionPtr SessionManager::FindSession(const string& sSessionID)
{
	WebSocketSessionPtr spSession;
	m_mapSession.Lookup(sSessionID, spSession);
	return spSession;
}

void SessionManager::RemoveSession(WebSocketSessionPtr spSession)
{
	if (spSession)
	{
		spSession->Close();
		m_mapSession.RemoveKey(spSession->GetSessionID());
		SALOG.debug(format("SessionManager::RemoveSession remove session %s", spSession->GetSessionName()));
	}
}

void SessionManager::CheckSession()
{
	SALOG.trace(format("start check session,current session count=%z", m_mapSession.GetCount()));
	SAArray<WebSocketSessionPtr, SAContainerFast> arrRemove;
	for (SAMap<string, WebSocketSessionPtr>::Iterator it = m_mapSession.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		WebSocketSessionPtr spClient = it.GetValue();
		if (!spClient->IsValid())
		{
			//SALOG.debug(format("remove session %s",spClient->GetSessionName()));
			arrRemove.Add(spClient);
		}
	}
	for (SAArray<WebSocketSessionPtr, SAContainerFast>::Iterator it = arrRemove.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		WebSocketSessionPtr spClient = it.Get();
		RemoveSession(spClient);
	}
}

void SessionManager::GetAllSession(vector<WebSocketSessionPtr>& arrSession)
{
	for (SAMap<string, WebSocketSessionPtr>::Iterator it = m_mapSession.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		WebSocketSessionPtr spClient = it.GetValue();
		if (spClient->IsValid())
		{
			arrSession.push_back(spClient);
		}
	}
}
