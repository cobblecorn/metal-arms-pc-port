// AIGraphConvert.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "AIGraphConvert.h"
#include "fang.h"
#include "AI\AIGraph.h"
#include "AI\ftl.h"
#include  <tchar.h>


FCLASS_ALIGN_PREFIX class CGraphInstance
{
public:
	CGraphInstance(void) : m_bEndianFlip(FALSE) {};
	~CGraphInstance() {};
	CAIGraph m_Graph;
	BOOL m_bEndianFlip;

	FCLASS_STACKMEM_ALIGN(CGraphInstance);
} FCLASS_ALIGN_SUFFIX;


CNiList<CGraphInstance*> _InstanceList;
CGraphInstance *CNiIterator<CGraphInstance*>::s_ReturnError;

CGraphInstance *HandleToGraph(unsigned long uHandle)
{
	CNiIterator<CGraphInstance*> it = _InstanceList.Begin();
	while (it.IsValid())
	{
		CGraphInstance* pGraph = it.Get();

		if ((unsigned long) pGraph == uHandle)
		{
			return pGraph;
		}

		it.Next();
	}

	return NULL;
}

unsigned long aigraphconvert_fnAIGraphConvertAsciiToBinary(const char* pszFilePath)
{
	unsigned long uRval = 0;

	CGraphInstance *pGraph = fnew CGraphInstance();

	if( !pGraph || !pGraph->m_Graph.LoadFromAsciiFile( pszFilePath ) )
	{
		fdelete(pGraph); pGraph = NULL;
	}
	else
	{
		uRval = (unsigned long) pGraph;
		_InstanceList.PushHead(pGraph);
	}

	return uRval;
}



BOOL aigraphconvert_fnAIGraphEndianFlip(unsigned long uHandle)
{
	CGraphInstance *pGraph = HandleToGraph(uHandle);
	if (pGraph)
	{
		pGraph->m_bEndianFlip ^= 1;
	}

	return FALSE;
}


int aigraphconvert_fnAIGraphCalcConvertedFileSizeBytes(unsigned long uHandle)
{
	CGraphInstance *pGraph = HandleToGraph(uHandle);
	if (pGraph)
	{
		return pGraph->m_Graph.GetSizeOfBinaryData();
	}
	return 0;
}


BOOL aigraphconvert_fnAIGraphSaveToBuffer(unsigned long uHandle, const char* pszFilename, FILE* pFileStream /*NULL*/)
{
	CGraphInstance *pGraph = HandleToGraph(uHandle);
	if (pGraph)
	{

		BOOL bCloseFile = FALSE;
		if( !pFileStream ) {
			if( !pszFilename ) {
				// invalid filename
				return FALSE;
			}
			pFileStream = _tfopen( pszFilename, _T("wb") );
			if( !pFileStream ) {
				return FALSE;
			}
			bCloseFile = TRUE;
		}

		// write out our file
		pGraph->m_Graph.SaveToBinaryFile( pFileStream,  pGraph->m_bEndianFlip);
		// close our file
		if( bCloseFile ) {
			fclose( pFileStream );
		}

		return TRUE;
	}

	return FALSE;
}


void aigraphconvert_fvAIGraphFreeData(unsigned long uHandle)
{
	CGraphInstance *pGraph = HandleToGraph(uHandle);
	if (pGraph)
	{
		_InstanceList.Remove(pGraph);
		fdelete(pGraph); pGraph = NULL;
	}
}
