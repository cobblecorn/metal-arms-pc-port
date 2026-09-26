#include <stdafx.h>
#include "fang.h"
#include "fres.h"
#include "maimain.h"
#include "maieditgraph.h"
#include "maitoolbar.h"

#define NUM_UNDO_BUFFERS 5
const int kINIT_BUFFERSIZE= 4096;

void *_apBuffer[NUM_UNDO_BUFFERS];
s32 _anBufferSizes[NUM_UNDO_BUFFERS];
s32 _anBufferFull[NUM_UNDO_BUFFERS];
s32 _nCurBuffer = 0;
BOOL _bJustRestored = 0;

void maiundo_Init(void)
{
	_nCurBuffer  = 0;
	for (int i = 0; i < NUM_UNDO_BUFFERS;i++)
	{
		_anBufferFull[i] = 0;
		_anBufferSizes[i] = kINIT_BUFFERSIZE;
		_apBuffer[i] = fang_MallocAndZero(_anBufferSizes[i], 16);
	}
}


void maiundo_Uninit(void)
{
	for (int i = 0; i < NUM_UNDO_BUFFERS;i++)
	{
		_anBufferFull[i] = 0;
		_anBufferSizes[i] = 0;
		if (_apBuffer[i])
		{
			fang_Free(_apBuffer[i]);
			_apBuffer[i] = NULL;
		}
	}
}

void maiundo_Store(void)
{
	s32 nSizeNeeded = maimain_pEditGraph->GetSizeOfBinaryData();

	if (_bJustRestored)
	{
		_anBufferFull[0] = 0;
		_anBufferFull[1] = 0;
		_anBufferFull[2] = 0;
		_anBufferFull[3] = 0;
		_anBufferFull[4] = 0;

		_bJustRestored = 0;
	}
	
	s32 i = _nCurBuffer;
	if (nSizeNeeded >= _anBufferSizes[i])
	{
		FASSERT(_CrtCheckMemory());
		//grow this buffer
		fang_Free(_apBuffer[i]);
		_anBufferSizes[i] = nSizeNeeded+512;
		_apBuffer[i] = fang_MallocAndZero(_anBufferSizes[i], 16);
		FASSERT(_CrtCheckMemory());
		FASSERT(_apBuffer[i]);
	}

	
	maimain_pEditGraph->SaveToMemory(_apBuffer[i]);
	FASSERT(_CrtCheckMemory());
	_anBufferFull[i] = 1;
	_nCurBuffer = (_nCurBuffer +1) % NUM_UNDO_BUFFERS;
}


void maiundo_Restore(void)
{
	//current tool loses focus.
	_bJustRestored = 1;

	//switch the graph....back to an old copy
	maimain_pEditGraph->ClearSelected();
	
	if (maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() && maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_pCurTool)
	{
		 maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_pCurTool->OnFocus();
	}
	else if (maimain_pMainToolBarWindow->Get3DGraphEditToolWnd() && maimain_pMainToolBarWindow->Get3DGraphEditToolWnd()->m_pCurTool)
	{
		 maimain_pMainToolBarWindow->Get3DGraphEditToolWnd()->m_pCurTool->OnFocus();
	}

	_nCurBuffer = (_nCurBuffer-1+NUM_UNDO_BUFFERS)%NUM_UNDO_BUFFERS;
	if (_anBufferFull[_nCurBuffer])
	{
		maimain_pEditGraph->ResetFromMemory(_apBuffer[_nCurBuffer]);
		_anBufferFull[_nCurBuffer] = 0;
	}
	else
	{ //don't go back then
		_nCurBuffer = (_nCurBuffer +1) % NUM_UNDO_BUFFERS;
	}

}
