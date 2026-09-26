//////////////////////////////////////////////////////////////////////////////////////
// FEventListener.h - Event Listener Class for Mettle Arms.
//
// Author: Justin Link
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/14/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FEVENTLISTENER_H_
#define _FEVENTLISTENER_H_ 1

#include "fang.h"

#define	FEVENT_LISTENER_MAX_STATE_SAVES	( 2 )

typedef void EventListenerCallback_t(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);

FCLASS_ALIGN_PREFIX class CFEventListener
{
public:
	FINLINE CFEventListener() { m_pfcnCallback = NULL; m_uUserData = 0; m_uEventFlags = 0; m_bDisabled = FALSE; }

	BOOL FINLINE Init(EventListenerCallback_t *pfcnCallback, u32 uUserData, u64 *puEventFlags)
	{
		FASSERT(pfcnCallback != NULL);

		m_pfcnCallback = pfcnCallback;
		m_uUserData = uUserData;
		if(puEventFlags == NULL)
		{
			m_uEventFlags = 0;
		}
		else
		{
			m_uEventFlags = *puEventFlags;
		}

		return(TRUE);
	}

	void FINLINE SetNotify(s32 nWhichEvent)
	{
		m_uEventFlags |= (1 << nWhichEvent);
	}

	void FINLINE StopNotify(s32 nWhichEvent)
	{
		m_uEventFlags &= ~(1 << nWhichEvent);
	}

	BOOL FINLINE OnEvent(s32 nWhichEvent, u32 uEventData1, u32 uEventData2, u32 uEventData3)
	{
		FASSERT(m_pfcnCallback != NULL);
		m_pfcnCallback(nWhichEvent, m_uUserData, uEventData1, uEventData2, uEventData3);

		return(TRUE);
	}

	u64 m_uEventFlags;
	BOOL m_bDisabled;
	u32 m_uUserData;
	s32 m_hStateSave[FEVENT_LISTENER_MAX_STATE_SAVES];	// allows clients to associate a state save data handle with event listeners

private:
	EventListenerCallback_t *m_pfcnCallback;

} FCLASS_ALIGN_SUFFIX;

#endif
