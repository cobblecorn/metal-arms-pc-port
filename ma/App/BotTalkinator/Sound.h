// Sound.h: interface for the CSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUND_H__FB9FBC76_21D9_4E48_A8B5_FC9914454E64__INCLUDED_)
#define AFX_SOUND_H__FB9FBC76_21D9_4E48_A8B5_FC9914454E64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"
#include "faudio.h"

class CSound  
{
public:
	BOOL LoadSound(FAudio_BankHandle_t hBank, CString strWAVFile);
	void Copy(CSound *pCopy);
	BOOL Play();
	BOOL Play(f32 fCurTime);

	FAudio_WaveHandle_t m_hSound;
	f32 m_fLength;
};

class CSoundInst
{
public:
	void Copy(const CSoundInst *pCopy);
	CSound *m_pSound;
	f32 m_fStartTime;
};

#define SOUND_MAX_BANKS		16

class CSoundList
{
public:
	CSoundList();
	~CSoundList();
	BOOL Init(u32 uMaxSounds);
	BOOL InsertEntryBank(CSound *poNewSound, u32 uUpper, u32 uLower);
	BOOL InsertEntryWord(CSound *poNewSound, CString strKeyWord);
	BOOL FindEntry(CString strText, CSound **ppoRetSound);
	BOOL LoadFromFile(const CString strFileName, const FAudio_BankHandle_t hBank, u32 uVoice);
	void UnInit();

	CSound *m_aoSoundListBanks[SOUND_MAX_BANKS];
	u32 m_auBankUpper[SOUND_MAX_BANKS], m_auBankLower[SOUND_MAX_BANKS];
	CString *m_astrWordList;
	CSound *m_aoSoundListWords;

	u32 m_uMaxSounds, m_uNumSounds[SOUND_MAX_BANKS], m_uNumBanks, m_uNumWords;
};

#endif // !defined(AFX_SOUND_H__FB9FBC76_21D9_4E48_A8B5_FC9914454E64__INCLUDED_)
