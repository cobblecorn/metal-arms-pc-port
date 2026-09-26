// Sound.cpp: implementation of the CSound class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bottalkinator.h"
#include "Sound.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#include "ffile.h"

//=============================================================================================

BOOL CSound::LoadSound(FAudio_BankHandle_t hBank, CString strWAVFile)
{
	m_hSound = faudio_GetWaveHandle(hBank, strWAVFile);
	
	if (faudio_IsValidWaveHandle(m_hSound))
	{
		FAudio_WaveInfo_t oWaveInfo;
		faudio_GetWaveAttributes(m_hSound, &oWaveInfo);
		m_fLength = oWaveInfo.fLengthInSeconds;
	}
	else
	{
		// 
		DEVPRINTF("CSound::LoadSound() : Invalid wave name.\n");
		m_fLength = 1.0f;
	}
	
	return (TRUE);
}

//=============================================================================================

void CSound::Copy(CSound *pCopy)
{
	m_hSound = pCopy->m_hSound;
	m_fLength = pCopy->m_fLength;
}

//=============================================================================================

BOOL CSound::Play()
{
	return (CFAudioEmitter::Play2D(m_hSound) == FAUDIO_NO_ERROR);
}

//=============================================================================================

BOOL CSound::Play(f32 fDelTime)
{
	return (CFAudioEmitter::Play2D(m_hSound) == FAUDIO_NO_ERROR);
}

//=============================================================================================

void CSoundInst::Copy(const CSoundInst *pCopy)
{
	m_fStartTime = pCopy->m_fStartTime;
	m_pSound = pCopy->m_pSound;
}

//=============================================================================================

CSoundList::CSoundList()
{
	u32 uIdx;
	for (uIdx = 0; uIdx < SOUND_MAX_BANKS; ++uIdx)
	{
		m_aoSoundListBanks[uIdx] = NULL;
		m_uNumSounds[uIdx] = 0;
	}
	
	m_aoSoundListWords = NULL;
	m_astrWordList = NULL;
	
	m_uMaxSounds = 0;
	m_uNumBanks = 0;
	m_uNumWords = 0;
}

//=============================================================================================

CSoundList::~CSoundList()
{
	UnInit();
}

//=============================================================================================

BOOL CSoundList::Init(u32 uMaxSounds)
{
	u32 uIdx;
	for (uIdx = 0; uIdx < SOUND_MAX_BANKS; ++uIdx)
	{
		m_aoSoundListBanks[uIdx] = new CSound[uMaxSounds];
		if (m_aoSoundListBanks[uIdx] == NULL)
			return (FALSE);
	}
	
	m_aoSoundListWords = new CSound[uMaxSounds];
	if (m_aoSoundListWords == NULL)
		return (FALSE);
	
	m_astrWordList = new CString[uMaxSounds];
	if (m_astrWordList == NULL)
		return (FALSE);
	
	m_uMaxSounds = uMaxSounds;
	
	return (TRUE);
}

//=============================================================================================

BOOL CSoundList::InsertEntryBank(CSound *poNewSound, u32 uLower, u32 uUpper)
{
	u32 uBank;
	
	for (uBank = 0; uBank < m_uNumBanks; ++uBank)
	{
		if ((m_auBankUpper[uBank] == uUpper) && (m_auBankLower[uBank] == uLower))
			break;
	}
	
	if (uBank == SOUND_MAX_BANKS)
		return (FALSE);
	
	if (uBank == m_uNumBanks)
	{
		FASSERT(m_uNumSounds[uBank] == 0);
		m_auBankUpper[uBank] = uUpper;
		m_auBankLower[uBank] = uLower;
		++m_uNumBanks;
	}
	else if (m_uNumSounds[uBank] == m_uMaxSounds)
	{
		return (FALSE);
	}
	
	m_aoSoundListBanks[uBank][m_uNumSounds[uBank]].Copy(poNewSound);
	++m_uNumSounds[uBank];
	
	return (TRUE);
}

//=============================================================================================

BOOL CSoundList::InsertEntryWord(CSound *poNewSound, CString strKeyWord)
{
	strKeyWord.MakeLower();
	//// Check to make sure that the word hasn't already been loaded (that would be uncool).
	//
	u32 uWordNum;
	for (uWordNum = 0; uWordNum < m_uNumWords; ++uWordNum)
	{
		if (m_astrWordList[uWordNum] == strKeyWord)
			return (FALSE);
	}
	//
	////
	
	if (m_uNumWords == m_uMaxSounds)
		return (FALSE);
	
	m_astrWordList[m_uNumWords] = strKeyWord;
	m_aoSoundListWords[m_uNumWords].Copy(poNewSound);
	++m_uNumWords;
	
	return (TRUE);
}

//=============================================================================================

BOOL CSoundList::LoadFromFile(const CString strFileName, const FAudio_BankHandle_t hBank, u32 uVoice)
{
	enum SoundMode_e
	{
		SOUNDMODE_NONE,
		SOUNDMODE_RANGE,
		SOUNDMODE_WORD
	};
	
	FILE *fInFile;
	char sLineBuffer[1024*SOUND_MAX_BANKS];
	char *sTemp;
	CString strKeyWord;
	u32 uLower, uUpper = 0, uCurVoice;
	BOOL bVoiceIsValid = FALSE;
	SoundMode_e eCurSoundMode = SOUNDMODE_NONE;
	
	fInFile = fopen(strFileName, "r");
	if (fInFile == NULL)
		return (FALSE);
	
	char sSigLine[1024];
	fscanf(fInFile, "%[^\n]\n", sSigLine);
	if (strcmp(sSigLine, "### BOTTALKINATOR VOCABULARY FILE ###") != 0)
	{
		return (FALSE);
	}
	
	while (!feof(fInFile))
	{
		fscanf(fInFile, "%[^\n]\n", sLineBuffer);
		switch (sLineBuffer[0])
		{
			case '<':
				{
					sTemp = strtok(& (sLineBuffer[1]), ">");
					uCurVoice = atoi(sTemp);
					bVoiceIsValid = TRUE;
					eCurSoundMode = SOUNDMODE_NONE;
					uUpper = 0;
					break;
				}
			case '[':
				{
					if (!bVoiceIsValid)
					{
						AfxMessageBox("Error during sounds file input: Range header found before voice header.");
						break;
					}
					uLower = uUpper + 1;
					sTemp = strtok(& (sLineBuffer[1]), "]");
					uUpper = atoi(sTemp);
					eCurSoundMode = SOUNDMODE_RANGE;
					break;
				}
			case '\"':
				{
					if (!bVoiceIsValid)
					{
						AfxMessageBox("Error during sounds file input: Word header found before voice header.");
						break;
					}
					eCurSoundMode = SOUNDMODE_WORD;
					sTemp = strtok(& (sLineBuffer[1]), "\"");
					strKeyWord = sTemp;
					break;
				}
			default:		// this means it is a filename.
				{
					switch (eCurSoundMode)
					{
					case SOUNDMODE_NONE:
						{
							AfxMessageBox("Error during sounds file input: Sound file found without any label.");
							break;
						}
					case SOUNDMODE_RANGE:
						{
							FASSERT(bVoiceIsValid);
							if (uCurVoice == uVoice)
							{
								CSound oTempSound;
								if (oTempSound.LoadSound(hBank, sLineBuffer))
								{
									InsertEntryBank(&oTempSound, uLower, uUpper);
								}
								else
								{
									CString strError;
									strError.Format("Error while loading file [%s].\nPlease verify that this file exists and is in the master file.", sLineBuffer);
									AfxMessageBox(strError);
								}
							}
							break;
						}
					case SOUNDMODE_WORD:
						{
							FASSERT(bVoiceIsValid);
							if (uCurVoice == uVoice)
							{
								CSound oTempSound;
								if (oTempSound.LoadSound(hBank, sLineBuffer))
								{
									InsertEntryWord(&oTempSound, strKeyWord);
								}
							}
							break;
						}
					}
					break;
				}
		}
	}
	
	return (TRUE);
}

//=============================================================================================

BOOL CSoundList::FindEntry(CString strText, CSound **ppoRetSound)
{
	s32 nHash = 0, nRange;
	u32 uLength, uBank, uIdx;
	
	// Hash the word to an integer.
	for (uIdx = 0; uIdx < (u32)(strText.GetLength()); ++uIdx)
		nHash += strText.GetAt((s32)(uIdx));
	
	for (uIdx = 0; uIdx < m_uNumWords; ++uIdx)
	{
		if (m_astrWordList[uIdx] == strText)
			break;
	}
	
	if (uIdx == m_uNumWords)
	{
		// Word was not found in the word list.
		uLength = strText.GetLength();
		for (uBank = 0; uBank < m_uNumBanks; ++uBank)
		{
			if ((m_auBankUpper[uBank] >= uLength) && (m_auBankLower[uBank] <= uLength))
				break;
		}
		FASSERT(uBank != m_uNumBanks);
		nRange = m_uNumSounds[uBank];
		
		nHash %= nRange;
		
		*ppoRetSound = & (m_aoSoundListBanks[uBank][nHash]);
	}
	else
	{
		*ppoRetSound = & (m_aoSoundListWords[uIdx]);
	}
	
	return (TRUE);
}

//=============================================================================================

void CSoundList::UnInit()
{
	delete[] m_astrWordList;
	m_astrWordList = NULL;
	
	delete[] m_aoSoundListWords;
	m_aoSoundListWords = NULL;
	
	u32 uBank;
	for (uBank = 0; uBank < SOUND_MAX_BANKS; ++uBank)
	{
		if (m_aoSoundListBanks[uBank] != NULL)
		{
			delete[] m_aoSoundListBanks[uBank];
			m_aoSoundListBanks[uBank] = NULL;
		}
	}
}

//=============================================================================================