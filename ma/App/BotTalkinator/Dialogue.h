#ifndef _DIALOGUE_H
#define _DIALOGUE_H 1

#include "Gesture.h"
#include "Sound.h"
#include "CharacterInfo.h"

#define _DIALOGUEINST_MAXGESTURES 32
#define _DIALOGUEINST_MAXSOUNDS 256

static const u32 Dialogue_nNumSoundLists = 3;

class CDialogueEntryN;


// This is a class corresponding to a single instance of a dialogue.
// It is the data contained in this class that is loaded by the game.
class CDialogueInstN
{
public:
	CDialogueInstN();
	~CDialogueInstN();

	void Clear();
	void Copy(const CDialogueInstN *pCopy);
	BOOL IsDifferent(const CDialogueInstN *pOtherDI);
	void CalculateTime();
	BOOL FindGestureAtTime(f32 fTime, cchar *pGestName, CGestureInst **ppoGestureInst);
	BOOL FindSoundAtTime(f32 fTime, CSoundInst **ppoSoundInst);
	BOOL AddGesture(CGesture *pNewGesture, f32 fStartTime);
	BOOL RemoveGesture(CGestureInst *pGoneGesture);
	void RecalcUserAnimSlots();
	BOOL ExportToFile(CString strFileName);

	void SetCurTimePos(f32 fCurTimePos);
	void Draw();

	BOOL m_bUseTalkLoop;

	CString m_strName;									// the robot name
	CString m_strTitle;									// the title of this dialogue instance
	CString m_strRace;									// the robot's race
	CString m_strDialogue;								// what the robot is going to say	

	CString m_strExportFile;							// The CSV filename
	
	BOOL m_bUseWAV;										// If TRUE, play a streamed wav or SFX from a bank, FALSE use Milspeak
	BOOL m_bIs2d;										// If TRUE, ...
	CString m_strWAVFile;								// The SFX or WAV filename
	u32 m_uMilVoice;									// Only valid if m_bUseWAV is FALSE.
	f32 m_fTotalTime;									// Only valid if m_bUseWAV is TRUE (will be the max of m_fWavSecs and m_fMinTotalTime)
	f32 m_fWavSecs;										// Only valid if m_bUseWAV is TRUE, this is the WAV or SFX play time
	f32 m_fMinTotalTime;								// Only used if m_bUseWAV is TRUE and if greater than m_fWavSecs
	f32 m_fWavStartOffset;								// Only used if m_bUseWAV is TRUE, this is the number of secs from the beginning before playing the wave

	CGestureInst m_aoGestureInst[_DIALOGUEINST_MAXGESTURES];	// Array of gesture instances.
	u8 m_uNumGestures;											// Number of useful entries in m_aoGestureInst[].

	CSoundInst m_aoSoundInst[_DIALOGUEINST_MAXSOUNDS];			// Array of sound instances.
	u8 m_uNumSounds;											// Number of useful entries in m_aoSoundInst[].

	static CSoundList *m_apoSoundList[Dialogue_nNumSoundLists];	// Pointer to the master sound lists (we need to make queries
																//   to it when we calculate the time for this dialogue.
	CDialogueEntryN *m_pDE;

private:
	CRITICAL_SECTION m_CriticalSection;
};


// This class corresponding to a set of dialogues associated with a given character (mesh, really).
// It is a logical grouping of instances.
class CDialogueEntryN
{
public:
	CDialogueEntryN();
	~CDialogueEntryN();

	BOOL AddEntry(CDialogueInstN *pNewDI);
	BOOL DeleteEntry(CDialogueInstN *pDelDI);

	CPtrList m_lpDialogues;			// The list of all dialogues that pertain to this character.

	CCharacterInfo *m_poCharInfo;
};


static const u32 CDialogueListN_uMaxCharacters = 40;

// This class houses all of the dialogue entries, along with everything else to which they might be associated.
class CDialogueListN
{
public:
	CDialogueListN();
	~CDialogueListN();

	BOOL Init(u32 uMaxGestures, u32 uMaxSounds);
	void UnInit();
	BOOL LoadCharInfoFromFile(const char *pszFileName);
	BOOL LoadSoundsFromFile(const char *pszFileName);

	// Gestures and sounds must be loaded before you load the dialogues.
	BOOL LoadDialoguesFromFile(const char *pszFileName);

	BOOL SaveToFile(const CString strOutputFileName);

	BOOL AddDialogueInst(CDialogueInstN *pNewDI);
	CDialogueEntryN *AddDialogueEntry(CString strName);
	CDialogueEntryN *FindDialogueEntry(CString strName);

	BOOL m_bIsInitialized;
	CCharacterInfoList m_oCharInfoList;
	CSoundList m_aoSoundList[Dialogue_nNumSoundLists];
	CPtrList m_lpDialogueEntries;	// List of dialogue entries (each entry corresponds to one name).

	// JUSTIN: Figure out a way to have this be non-static.
	static CDialogueEntryN *s_pCurDE;

	FAudio_BankHandle_t m_hBank;
private:

};

#endif