//////////////////////////////////////////////////////////////////////////////////////
// cam_file_def.h - 
//
// Author: Russell Foushee   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/22/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAM_FILE_DEF_H_
#define _CAM_FILE_DEF_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fversion.h"

//////////////////////////////////////////////////////////////////////////////////////
// NOTE: WHEN ADDING TO A STRUCT TAKE ELEMENTS FROM nUnused[] THIS WAY PASM WILL STILL
// BE ABLE TO READ THE OLD CAM FILE.
// IF YOU NEED TO ADD NEW STRUCTURES OR DELETE FIELDS, THEN YOU WILL NEED TO CHANGE
// THE VERSION NUMBER INSIDE FVERSION.H.
// BE WARNED:
// IF YOU CHANGE THE APE VERSION NUMBER, YOU WILL NEED TO RE-EXPORT ALL MAX/MAYA FILES
// AS THEY WILL NO LONGER BE ABLE TO BE COMPILED BY PASM.
//////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// FILE FORMAT:
// CamInfo_t -- has CamAnim_t's defined
// CamFrame_t[] -- array of animation frames defined by CamInfo_t::nFrames
// szUserPropStringData -- Defined in CamInfo_t::nOffsetToString
/////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_NAME_LEN			16


typedef struct {
	FVersionHeader_t Header;	// has a signature and version info
	u32 nBytesInFile;			// what is the total file size of the entire .cam file

	char szCameraName[CAMERA_NAME_LEN];
	u32 nFrames;				// CamFrame_t structs follow this structure
	u32 nBytesOfUserData;		// how many bytes of user data follow this struct
	u32 nOffsetToString;		// offset into the user data buffer to the string data -- after the array of CamFrame_t's

	u8 nUnused[64];				// USE THESE BYTES TO ADD NEW FIELDS
} CamInfo_t;


typedef struct {
	f32 fSecsFromStart;
	f32 fFOV;		// in radians, the full angle
	CFMtx43 Orientation;	// a left handed orientation, Rot & Pos
} CamFrame_t;

#endif