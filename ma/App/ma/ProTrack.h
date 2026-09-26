//////////////////////////////////////////////////////////////////////////////////////
// ProTrack.h - 
//
// Author: Pat MacKellar 
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
// 05/02/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////



//////////
//
//   What is ProTrack?
//
//		- Protrack is a system to help profile many sections of a project.
//		- Enable it by #defining PROTRACK_ENABLE = 1 in your build. you can do this in protrack.h if you want.  
//		  Following BuildDefaults (DEBUG, RELEASE, TEST: PROTRACK_ENABLE =1         PRODUCTION: PROTRACK_ENABLE=0 )
//		- Use it by placing code to be profiled within PROTRACK_BEGINBLOCK(name) PROTRACK_ENDBLOCK segments.
//		  These can be hierarchical.  Make sure you always match up PROTRACK_ENDBLOCK with PROTRACK_BEGINBLOCK.
//		- Then, use the Secret key combination to enable the protrack profiling menu!
//
//		   !!!!Note.  On Xbox, use controller Number 2!!!!!!!
//
//			   Secret Key Combinations to enable Protrack!
//                 Hold Down FIRE_PRIMARY and FIRE_SECONDARY, then press GAMEPAD_MAIN_JUMP
//
//
//	 How to use the ProTrack Menu?
//
//			!!!!Note.  Don't forget to use controller Number 2 on Xbox, !!!!!!!!!!
//
//			 - LookUp/LookDown stick to scroll through the Menu
//			 - Press GAMEPAD_MAIN_SELECT_PRIMARY(B button) to expand a menu item (those with the little '>' character)
//			 - Press GAMEPAD_MAIN_SELECT_SECONDARY(X button) to change the sampling mode (SINGLE_FRAME, 5_FRAME_AVG, 30_FRAME_AVG, RUNNING_AVERAGE)
//			 - Press GAMEPAD_MAIN_ACTION(Y button) to switch from Hierarchical Tree Mode to Global Sum Mode
//
//////////

#ifndef _PROTRACK_H_
#define _PROTRACK_H_ 1


#if defined(__cplusplus)
extern "C"
{
#endif


#include "sas_user.h"

#ifndef PROTRACK_ENABLE		//including this file will never change the PROTRACK_ENABLE setting if it is already defined

#ifndef FANG_PRODUCTION_BUILD
#error				   //this  better bet defined (either 0, or 1)
#endif

#if FANG_PRODUCTION_BUILD		  //production build gets no protrack by default
#define PROTRACK_ENABLE 0
#else
//ARG - >>>>>
#if	FANG_PLATFORM_PS2
#define PROTRACK_ENABLE 0	//ARG - no PROTRACK on PS2
#else
//ARG - <<<<<
#define PROTRACK_ENABLE 1		  //all other builds (debug, release, test) get no protrack by default
#endif	//ARG
#endif

#endif



#if PROTRACK_ENABLE == 1

extern BOOL PROTRACK_INITSYSTEM(void);
extern void PROTRACK_UNINITSYSTEM(void);
extern void PROTRACK_BEGINBLOCK(cchar *pszName);
extern void PROTRACK_ENDBLOCK(void);
extern void PROTRACK_WORK(void);
extern void PROTRACK_DRAW(void);
extern BOOL PROTRACK_ISMENUON(void);

#else

#define PROTRACK_BEGINBLOCK(pszName)
#define PROTRACK_ENDBLOCK()
#define PROTRACK_WORK()
#define PROTRACK_DRAW()
#define PROTRACK_INITSYSTEM()
#define PROTRACK_UNINITSYSTEM()
#define PROTRACK_ISMENUON() 0

#endif



#if defined(__cplusplus)
}
#endif



#endif //_PROTRACK_H_

