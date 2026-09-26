//////////////////////////////////////////////////////////////////////////////////////
// SplitScreen.h - Screen handler for split-screen multiplayer.
//
// Author: Dan Stanfill, Pinniped Software     
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
// 02/27/03 Stanfill    Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SplitScreen_H_
#define _SplitScreen_H_ 1

// The SplitType determines whether we split horizontally or vertically first
// as we add players.
enum SplitType_e {SPLIT_HORIZONTAL, SPLIT_VERTICAL};

extern void splitscreen_SetSplitType(SplitType_e eType);
extern BOOL splitscreen_SetupViewports( int nPlayer, int nTotalPlayers );
extern void splitscreen_DrawBorders( void );
extern f32  splitscreen_RemapFOV( s32 nPlayerIndex, f32 fFOV );
extern f32  splitscreen_UnmapFOV( s32 nPlayerIndex, f32 fFOV );

#endif // _SplitScreen_H_
