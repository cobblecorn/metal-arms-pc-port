//////////////////////////////////////////////////////////////////////////////////////
// testbots.cpp - 
//
// Author: Mike Elliott     
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
// 12/11/02 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "user_elliott.h"
#include "user_mike.h"
#include "testbots.h"
#include "gamepad.h"

#if TESTBOTS_ENABLE

#include "bottitan.h"
#include "botgrunt.h"
#include "botpred.h"
#include "botminer.h"
#include "botprobe.h"
#include "botzom.h"
#include "botslosh.h"
#include "site_botweapon.h"
#include "ai/aiapi.h"
#include "ai/aibrain.h"
#include "sas_user.h"
#include "level.h"
#include "player.h"			
#include "vehicleloader.h"
#include "botkrunk.h"
#include "botEliteGuard.h"
#include "botjumper.h"
#include "botswarmer.h"
#include "botscout.h"
#include "botmortar.h"
#include "fclib.h"
#include "frenderer.h"
#include "botglitch.h"
#include "ai/aimover.h"
#include "botmozer.h"
#include "vehicleRat.h"
#include "vehicleSentinel.h"
#include "botcorrosive.h"
#include "botzombieboss.h"
#include "botswarmerboss.h"
#include "botsnarq.h"
#include "botscientist.h"


#define WANDER_AROUND TRUE
#define WAIT_AROUND FALSE

FCLASS_ALIGN_PREFIX class CTestBotIniData
{
public:
	CTestBotIniData(	const CFVec3A& Loc_WS,
						f32		fYaw,
						cchar*	pszLevelName,
						cchar*  pszName,
						u32		uNumClones,
						u32		uAIRace,
						u8		uUsesVehicleWhen,
						u8		uUsesVehicleTypes,
						BOOL	bWanderAtStart) : 
						m_Loc_WS(Loc_WS),
						m_fYaw(fYaw),
						m_pszLevelName(pszLevelName),
						m_pszName(pszName),
						m_uNumClones(uNumClones),
						m_uAIRace(uAIRace),
						m_uUsesVehicleWhen(uUsesVehicleWhen),
						m_uUsesVehicleTypes(uUsesVehicleTypes),
						m_bWanderAtStart(bWanderAtStart)
						{}



	CFVec3A m_Loc_WS;
	f32		m_fYaw;
	cchar*	m_pszLevelName;
	cchar*  m_pszName;
	u32		m_uNumClones;
	u32		m_uAIRace;
	u8		m_uUsesVehicleWhen;
	u8		m_uUsesVehicleTypes;
	BOOL	m_bWanderAtStart;
} FCLASS_ALIGN_SUFFIX;

//How many of each type of test bot do I want?

#define NUM_TEST_GRUNTS			0
#define NUM_TEST_PREDS			0
#define NUM_TEST_PROBES			0
#define NUM_TEST_SENTRYGUNS		0
#define NUM_TEST_PILLBOXES		0
#define NUM_TEST_GUARDS			0
#define NUM_TEST_TITANS			0
#define NUM_TEST_JUMPERS		0
#define NUM_TEST_SWARMERS		0
#define NUM_TEST_ZOMS			0
#define NUM_TEST_MINERS			0
#define NUM_TEST_SCOUTS			0
#define NUM_TEST_MORTAR			0
#define NUM_TEST_RATS			0
#define NUM_TEST_CORROSIVES		0
#define NUM_TEST_SLOSHES		0
#define NUM_TEST_MOZERS			0
#define NUM_TEST_SNARQS			0
#define NUM_TEST_SCIENTISTS		0
#define ONE_TEST_LOADER			0
#define ONE_TEST_SENTINEL		0

static CBotGrunt *		_apTestGrunt[1+NUM_TEST_GRUNTS];
static CTestBotIniData	_aTestGruntIniData[]  = 
{
//	CTestBotIniData(CFVec3A( -217.0f,4.0f,-17.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Steve",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_ALL,				WAIT_AROUND),
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Steve",	1,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_FLAG_RATGUN,		WAIT_AROUND),
	CTestBotIniData(CFVec3A(  -18.0f, 6.0f,-78.0f),		0.0f,						"wewtrace_01",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_ALL,				WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,134.0f),		0.0f,						"Jump_Test",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,144.0f),		0.0f,						"Jump_Test",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,154.0f),		0.0f,						"Jump_Test",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
//	CTestBotIniData(CFVec3A( -210.0f,20.0f,-10.0f),		0.0f,						"RampTest4",	"TB_Steve",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
};


static CBotTitan		*_pMETitan	= NULL;
static CBotPred			*_pMEPred	= NULL;
static CBotZom			*_pZom		= NULL;
static CVehicleLoader   *_pMELoader	= NULL;
static CVehicleSentinel *_pSentinel	= NULL;

static CBotPred  *_apTestPred[1+NUM_TEST_PREDS];
static CTestBotIniData	_aTestPredIniData[]  = 
{
CTestBotIniData(CFVec3A( -221.0f, 37.0f,340.0f),		0.0f,						"Jump_Test",	"TB_Pred",	  0,	AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WANDER_AROUND),
	CTestBotIniData(CFVec3A( -218.0f,45.0f,-35.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Pred",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WANDER_AROUND),
	CTestBotIniData(CFVec3A( -200.0f,35.0f,-35.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Pred",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WANDER_AROUND),
	CTestBotIniData(CFVec3A( -154.0f,40.0f,327.0f),		FMATH_PI,					"Jump_Test",	"TB_Pred",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
};

static CBotZom  *_apTestZom[1+NUM_TEST_ZOMS];
static CTestBotIniData	_aTestZomIniData[]  = 
{
	CTestBotIniData(CFVec3A( -358.0f,0.5f,124.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,134.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,144.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,154.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,164.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,174.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	    WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,184.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,194.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	    WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,204.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	    WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,0.5f,214.0f),		0.0f,						"Jump_Test",	"TB_Zom",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
};

static CBotProbe  *_apTestProbe[1+NUM_TEST_PROBES];
static CTestBotIniData	_aTestProbeIniData[]  = 
{
	CTestBotIniData(CFVec3A( -218.0f,45.0f,-35.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Nate",		0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -200.0f,35.0f,-35.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Nate2",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
};

static CBotSiteWeapon *	_apTestSentryGun[1+NUM_TEST_SENTRYGUNS];
static CTestBotIniData _aTestSentryGunIniData[]  = 
{
	CTestBotIniData(CFVec3A( -20.0f, 0.0f, -10.0f),		1.0f*FMATH_QUARTER_PI,		"RampTest4",	 "TB_Sentry",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
	CTestBotIniData(CFVec3A( 21.0f, 44.8f,  31.675f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4", "TB_Sentry",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f, 0.0f, 124.0f),		-3.0f*FMATH_QUARTER_PI,		"jump_test", "TB_Sentry",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
};

static CBotSiteWeapon *	_apTestPillBox[1+NUM_TEST_PILLBOXES];
static CTestBotIniData _aTestPillBoxIniData[] = 
{
	CTestBotIniData(CFVec3A( -177.0f, 0.0f,33.0f),		1.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Pill",	0,			AIRACE_AMBIENT, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
	CTestBotIniData(CFVec3A( -217.0f, 0.0f,-17.0f),		1.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Pill",	0,			AIRACE_AMBIENT, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
	CTestBotIniData(CFVec3A( -21.7f, 33.9f, 69.7f),	-3.0f*FMATH_QUARTER_PI,			"RampTest4",	"TB_Pill",	0,			AIRACE_AMBIENT, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
};


static CBotEliteGuard *	_apTestGuard[1+NUM_TEST_GUARDS];
static CTestBotIniData	_aTestGuardIniData[] = 
{
	CTestBotIniData(CFVec3A( -350.0f,0.5f,111.0f),		0.0f,						"Jump_Test",	"TB_Dude",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -217.0f, 0.0f,-17.0f),		1.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Dude",		0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
	CTestBotIniData(CFVec3A( 27.0f, 40.0f,71.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Dude1",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
};

static CBotMortar*		_apTestMortar[1+NUM_TEST_MORTAR];
static CTestBotIniData		_aTestMortarIniData[]  = 
{
	CTestBotIniData(CFVec3A( -350.0f,0.5f,111.0f),		0.0f,						"Jump_Test",	"TB_Mortie",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -352.0f,0.5f,112.0f),		0.0f,						"Jump_Test",	"TB_Mortie",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -219.0f, 0.0f,-19.0f),		1.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Mortie",		0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
	CTestBotIniData(CFVec3A( 25.0f, 40.0f,73.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_Mortie",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
};

static CBotTitan*	_apTestTitan[1+NUM_TEST_TITANS];
static CTestBotIniData _aTestTitanIniData[] = {
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Steve",    1,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_ALL,			  WAIT_AROUND),
	CTestBotIniData(CFVec3A(  -18.0f, 6.0f,-78.0f),		0.0f,						"wewtrace_01",	"TB_Steve",		2,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_SENTINEL,  WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Steve",		0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
};

static CVehicleRat*	_apTestRat[1+NUM_TEST_RATS];
static CTestBotIniData _aTestRatIniData[] = {
	CTestBotIniData(CFVec3A( -96.0f, 1.0f,-13.0f),		2.0f*FMATH_QUARTER_PI,						"RampTest4",	"TB_Rat",    1,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_ALL,			  WAIT_AROUND),
	CTestBotIniData(CFVec3A( -30.0f, 1.0f,-13.0f),		0.0f,						"RampTest4",	"TB_Rat",    1,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_ALL,			  WAIT_AROUND),
	CTestBotIniData(CFVec3A(  -18.0f, 6.0f,-78.0f),		0.0f,						"wewtrace_01",	"TB_Rat",	2,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_SENTINEL,  WAIT_AROUND),
	CTestBotIniData(CFVec3A( -250.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Rat",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
};



static CBotJumper*	_apTestJumper[1+NUM_TEST_JUMPERS];
static CTestBotIniData _aTestJumperIniData[] = {
//	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Jump",    5,		AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Jump",    1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),//WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Jump",	  5,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -27.0f, 10.0f,-436.0f),	0.0f,						"WESSstatn02",	"TB_Jump",	  1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WANDER_AROUND),
};


static CBotCorrosive*	_apTestCorrosive[1+NUM_TEST_CORROSIVES];
static CTestBotIniData _aTestCorrosiveIniData[] = {
	CTestBotIniData(CFVec3A( 1733.0f,0.0f,365.0f),		0.0f,						"WESRrepair1",	"TB_Russ",		1,		 AIRACE_MIL,   CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_ALL,			   WAIT_AROUND),
};


static CBotSwarmer*	_apTestSwarmer[1+NUM_TEST_SWARMERS];
static CTestBotIniData	_aTestSwarmerIniData[]  = 
{
	//ceiling																								 //clones
	CTestBotIniData(CFVec3A( -150.8f, 113.0f, 233.5f),	3.0f*FMATH_QUARTER_PI,		"Jump_Test",	"TB_Nate",	   5,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
	//wall
	CTestBotIniData(CFVec3A( -250.8f, 50.0f, 96.5f),	3.0f*FMATH_QUARTER_PI,		"Jump_Test",	"TB_Nate",	   5,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
	//floor
	CTestBotIniData(CFVec3A( -438.0f,10.0f,252.0f),		3.0f*FMATH_QUARTER_PI,		"Jump_Test",	"TB_Nate",	   5,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WAIT_AROUND),
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Nate",    1,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
	CTestBotIniData(CFVec3A( -39.0f, 22.0f,-105.0f),	3.0f*FMATH_QUARTER_PI,		"WERRreactr1",	"TB_Nate",	   50,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	   WAIT_AROUND),
};

static CBotMiner*		_apTestMiner[1+NUM_TEST_MINERS];
static CTestBotIniData	_aTestMinerIniData[]  = 
{
	CTestBotIniData(CFVec3A( -217.0f,4.0f,-17.0f),		-3.0f*FMATH_QUARTER_PI,		"RampTest4",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_ALL,			  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_John",    1,	 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_ALL,			  WANDER_AROUND),
	CTestBotIniData(CFVec3A(  -18.0f, 6.0f,-78.0f),		0.0f,						"wewtrace_01",	"TB_John",	2,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_INIT,		CAIBrain::MECH_USEAGE_TYPE_FLAG_RATGUN,		  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,134.0f),		0.0f,						"Jump_Test",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,144.0f),		0.0f,						"Jump_Test",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,154.0f),		0.0f,						"Jump_Test",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
//	CTestBotIniData(CFVec3A( -210.0f,20.0f,-10.0f),		0.0f,						"RampTest4",	"TB_John",	0,		 AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
};


static CBotScout*		_apTestScout[1+NUM_TEST_SCOUTS];
static CTestBotIniData	_aTestScoutIniData[]  = 
{
	CTestBotIniData(CFVec3A( -358.0f, 10.0f,  140.0f),		0.0f,						"Jump_Test",	"TB_Chris1",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
	CTestBotIniData(CFVec3A(  -67.0f,  2.0f, -135.0f),		0.0f,						"RampTest4",	"TB_Chris1",	0,		 AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	  WANDER_AROUND),
};

static CBotSlosh *_apTestSloshes[1+NUM_TEST_SLOSHES];
static CTestBotIniData	_aTestSloshIniData[]  = 
{
	CTestBotIniData(CFVec3A( -358.0f,0.5f,124.0f),		0.0f,						"Jump_Test",	"TB_Slosh",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
};

static CBotMozer *_apTestMozers[1+NUM_TEST_MOZERS];
static CTestBotIniData	_aTestMozerIniData[]  = 
{
	CTestBotIniData(CFVec3A( -358.0f,0.5f,124.0f),		0.0f,						"Jump_Test",	"TB_Moze",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
	CTestBotIniData(CFVec3A( -275.0f,-90.0f,-684.0f),		0.0f,						"testlevel01",	"TB_Moze",	0,		 AIRACE_ZOMBIE, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,		WAIT_AROUND),
};

static CBotSnarq*	_apTestSnarqs[1+NUM_TEST_SNARQS];
static CTestBotIniData _aTestSnarqIniData[] = {
//	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Snarq",    5,		AIRACE_DROID, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Snarq",    1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),//WANDER_AROUND),
	CTestBotIniData(CFVec3A(  -18.0f, 6.0f,-78.0f),		0.0f,						"wewtrace_01",	"TB_Snarq",	  2,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,   WAIT_AROUND),
	CTestBotIniData(CFVec3A( -221.0f, 37.0f,340.0f),		0.0f,						"Jump_Test",	"TB_Snarq",	  0,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -358.0f, 30.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Snarq",	  5,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -27.0f, 10.0f,-436.0f),	0.0f,						"WESSstatn02",	"TB_Snarq",	  1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WANDER_AROUND),
};


static CBotScientist*	_apTestScientists[1+NUM_TEST_SCIENTISTS];
static CTestBotIniData _aTestScientistIniData[] = {
	CTestBotIniData(CFVec3A( -150.0f,20.0f,-17.0f),		0.0f,						"RampTest4",	"TB_Jump",    1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),//WANDER_AROUND),
	CTestBotIniData(CFVec3A( -358.0f,10.0f,124.0f),		0.0f,						"Jump_Test",	"TB_Jump",	  5,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WAIT_AROUND),
	CTestBotIniData(CFVec3A( -27.0f, 10.0f,-436.0f),	0.0f,						"WESSstatn02",	"TB_Jump",	  1,		AIRACE_MIL, CAIBrain::MECH_USEAGE_CTRL_NEVER,		CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,	WANDER_AROUND),
};


BOOL _FindLevelNameInArrayIniData(cchar* pszTestString, CTestBotIniData* paIniData, u32 uArrayLen, u32* pResult)
{
	for (u32 i= 0; i < uArrayLen; i++)
	{
		if (!fclib_stricmp(pszTestString, paIniData[i].m_pszLevelName))
		{
			if (pResult)
			{
				*pResult = i;
			}
			return TRUE;
		}
	}
	return FALSE;
}


void SetupTestBotAI(CBot* pBot, CTestBotIniData* pIniData)
{
	if (pBot && pBot->AIBrain()) {
		pBot->AIBrain()->SetRace(pIniData->m_uAIRace);
//		if( pBot->AIBrain()->GetAIMover()->GetWeaponCtrlPtr(0) ) {
//			pBot->AIBrain()->GetAIMover()->GetWeaponCtrlPtr(0)->m_fAccuracy = 1.0f;
//		}

		pBot->AIBrain()->SetMechUseage( pIniData->m_uUsesVehicleWhen,  pIniData->m_uUsesVehicleTypes, 200);
		if( !(pBot->AIBrain()->GetMechUseageCtrl() & CAIBrain::MECH_USEAGE_CTRL_INIT))
		{
			if (pIniData->m_bWanderAtStart)
			{

				ai_AssignJob_Wander(pBot->AIBrain(),
								0.0f,		  //radius to stay within
								TRUE,		  //stay in room
								70,			  //min speed
								80,			  //max speed
								100,		  //unused
								20,			  //length of breaks (+- 5 secs)
								40);		  //take a break chance (evaluated every 20 or so secs)
			}
		}
	}
}







///////////////////////////////////////////////////////////////////////////////////
//INIT



void testbots_Init( void ) {

	if (!(Level_nLoadedIndex > -1 &&
		Level_nLoadedIndex < Level_nCount &&
		Level_aInfo[Level_nLoadedIndex].pszWorldResName))	{
		return;
	}


	#if 0
		if (Level_nLoadedIndex > -1 &&
			Level_nLoadedIndex < Level_nCount &&
			Level_aInfo[Level_nLoadedIndex].pszWorldResName &&
			(!fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName)))
		{

			// Get a frame...
			FResFrame_t ResFrame = fres_GetFrame();

			FASSERT(_pZom == NULL);

			CFMtx43A InitMat;
			InitMat.Identity();
			InitMat.m_vPos.x = -358.0f;
			InitMat.m_vPos.y = 1.0f;
			InitMat.m_vPos.z = 104.0f;

			_pZom = fnew CBotZom();
			if( _pZom == NULL ) {
				return;
			}

			if( !_pZom->Create(-1, TRUE, "ZombieBot", &InitMat, "Default" ) ) {
				fdelete( _pZom );
				_pZom = NULL;
				fres_ReleaseFrame( ResFrame );
			} else {
				_pZom->DataPort_Open( TRUE );
				_pZom->DataPort_SetPossessionDist( 150.0f );
//				_pZom->AIBrain()->SetRace( AIRACE_MIL );
				_pZom->AIBrain()->SetRace( AIRACE_DROID );
				if( _pZom->AIBrain() ) {
#if 1
					ai_AssignJob_Wander(_pZom->AIBrain(),
										200.0f,		  //radius to stay within
										TRUE,		  //stay in room
										70,			  //min speed
										80,			  //max speed
										100,		  //unused
										20,			  //length of breaks (+- 5 secs)
										40);		  //take a break chance (evaluated every 20 or so secs)
#else
					ai_TurnOffPerceptor(_pZom->AIBrain(), AI_PERCEPTOR_EYES );
					ai_TurnOffPerceptor(_pZom->AIBrain(), AI_PERCEPTOR_EARS );
					ai_TurnOffPerceptor(_pZom->AIBrain(), AI_PERCEPTOR_TOUCH );
					ai_TurnOffPerceptor(_pZom->AIBrain(), AI_PERCEPTOR_RADIO );
#endif
				}
			}
		}
	#endif

//	#if SAS_ACTIVE_USER==SAS_USER_ELLIOTT
	#if 0
		if (Level_nLoadedIndex > -1 &&
			Level_nLoadedIndex < Level_nCount &&
			Level_aInfo[Level_nLoadedIndex].pszWorldResName &&
			((!fclib_stricmp("ramptest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName) ||
			(!fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName)) )))
		{

			// Get a frame...
			FResFrame_t ResFrame = fres_GetFrame();

			FASSERT(_pMEPred == NULL);

			CFMtx43A InitMat;
			InitMat.Identity();
			InitMat.m_vPos.x = -330.0f;
			InitMat.m_vPos.y = 30.0f;
			InitMat.m_vPos.z = 200.0f;

			_pMEPred = fnew CBotPred();
			if( _pMEPred == NULL ) {
				return;
			}

			if( !_pMEPred->Create(-1, TRUE, "MePred", &InitMat, "Default" ) ) {
				fdelete( _pMEPred );
				_pMEPred = NULL;
				fres_ReleaseFrame( ResFrame );
			} else {
				_pMEPred->DataPort_Open( TRUE );
				_pMEPred->DataPort_SetPossessionDist( 550.0f );
				_pMEPred->AIBrain()->SetRace( AIRACE_MIL );
				if( _pMEPred->AIBrain() ) {
					ai_AssignJob_Wander(_pMEPred->AIBrain(),
										200.0f,		  //radius to stay within
										TRUE,		  //stay in room
										70,			  //min speed
										80,			  //max speed
										100,		  //unused
										20,			  //length of breaks (+- 5 secs)
										40);		  //take a break chance (evaluated every 20 or so secs)
				}
			}
		}
	#endif


#if ONE_TEST_LOADER
		if (Level_nLoadedIndex > -1 &&
			Level_nLoadedIndex < Level_nCount &&
			Level_aInfo[Level_nLoadedIndex].pszWorldResName &&
			(!fclib_stricmp("ramptest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName) ||
			(!fclib_stricmp("temp", Level_aInfo[Level_nLoadedIndex].pszWorldResName)) ) )
		{

			// Get a frame...
			FResFrame_t ResFrame = fres_GetFrame();

			FASSERT(_pMELoader == NULL);

			CFMtx43A InitMat;
			InitMat.Identity();

			if (!fclib_stricmp("ramptest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName))
			{

				InitMat.m_vPos.x = -106.0f;
				InitMat.m_vPos.y = 5.0f;
				InitMat.m_vPos.z = 12.0f;
			}
			else
			{
				InitMat.m_vPos.x = -122.0f;
				InitMat.m_vPos.y = 5.0f;
				InitMat.m_vPos.z = 50.0f;
			}

			_pMELoader = fnew CVehicleLoader();
			if( _pMELoader == NULL ) {
				return;
			}

			if( !_pMELoader->Create(-1, TRUE, "TB_MeLoader", &InitMat, "Default" ) ) {
				fdelete( _pMELoader );
				_pMELoader = NULL;
				fres_ReleaseFrame( ResFrame );
			}
		}
#endif

#if ONE_TEST_SENTINEL
		if (Level_nLoadedIndex > -1 &&
			Level_nLoadedIndex < Level_nCount &&
			Level_aInfo[Level_nLoadedIndex].pszWorldResName &&
			!fclib_stricmp( "jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName ) ||
			!fclib_stricmp( "ramptest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName ) ||
			!fclib_stricmp( "temp", Level_aInfo[Level_nLoadedIndex].pszWorldResName ) )
		{

			// Get a frame...
			FResFrame_t ResFrame = fres_GetFrame();

			FASSERT(_pSentinel == NULL);

			CFMtx43A InitMat;
			InitMat.Identity();

			if (!fclib_stricmp("ramptest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName))
			{

				InitMat.m_vPos.x = -106.0f;
				InitMat.m_vPos.y = 5.0f;
				InitMat.m_vPos.z = 12.0f;
			}
			else if (!fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName))
			{

				InitMat.m_vPos.x = -200.0f;
				InitMat.m_vPos.y = 10.0f;
				InitMat.m_vPos.z = 124.0f;
			}
			else if (!fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName))
			{

				InitMat.m_vPos.x = -200.0f;
				InitMat.m_vPos.y = 10.0f;
				InitMat.m_vPos.z = 124.0f;
			}
			else
			{
				InitMat.m_vPos.x = -122.0f;
				InitMat.m_vPos.y = 5.0f;
				InitMat.m_vPos.z = 50.0f;
			}

			_pSentinel = fnew CVehicleSentinel();
			if( _pSentinel == NULL ) {
				return;
			}

			if( !_pSentinel->Create(-1, FALSE, "Test_Sentinel", &InitMat, "Default" ) ) {
				fdelete( _pSentinel );
				_pSentinel = NULL;
				fres_ReleaseFrame( ResFrame );
			}
		}
#endif


		//Grunts
		u32 uNumInitData = sizeof(_aTestGruntIniData)/sizeof(CTestBotIniData);
		u32 uGruntCount;
		u32 uIni = 0;
		for (uGruntCount = 0; uGruntCount < NUM_TEST_GRUNTS && uIni < uNumInitData; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestGruntIniData+uIni, uNumInitData-uIni, &uRelIni))
			{
				uIni+=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestGruntIniData[uIni].m_uNumClones && uGruntCount < NUM_TEST_GRUNTS; uCloneCount++)
				{
					FASSERT(_apTestGrunt[uGruntCount] == NULL);
					_apTestGrunt[uGruntCount] = fnew CBotGrunt();
					if( _apTestGrunt[uGruntCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestGruntIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestGruntIniData[uIni].m_Loc_WS;
					InitMat.m_vPos.x+=(uCloneCount%10)*10.0f;
					InitMat.m_vPos.z+=(uCloneCount/10)*10.0f;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestGruntIniData[uIni].m_pszName, uGruntCount);

					if( !_apTestGrunt[uGruntCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default", 0, CBotBuilder::_NPC_WEAPON_ROCKET) )
					{
						fdelete( _apTestGrunt[uGruntCount] );
						_apTestGrunt[uGruntCount] = NULL;
					} 
					else
					{
						_apTestGrunt[uGruntCount]->AIBrain()->SetRace( _aTestGruntIniData[uIni].m_uAIRace );
						_apTestGrunt[uGruntCount]->DataPort_Open( TRUE );
						_apTestGrunt[uGruntCount]->DataPort_SetPossessionDist( 150.0f );
						SetupTestBotAI(_apTestGrunt[uGruntCount], _aTestGruntIniData+uIni);

//						_apTestGrunt[uGruntCount]->SetArmorModifier( 0.5f );
					}
					uGruntCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestSentryGunIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uSentryCount;
		for (uSentryCount = 0; uIni < uNumInitData && uSentryCount < NUM_TEST_SENTRYGUNS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestSentryGunIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestSentryGunIniData[uIni].m_uNumClones && uSentryCount < NUM_TEST_SENTRYGUNS; uCloneCount++)
				{
					FASSERT(_apTestSentryGun[uSentryCount] == NULL);
					_apTestSentryGun[uSentryCount] = fnew CBotSiteWeapon();
					if( _apTestSentryGun[uSentryCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestSentryGunIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestSentryGunIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestSentryGunIniData[uIni].m_pszName, uSentryCount);

					if( !_apTestSentryGun[uSentryCount]->Create(BOTSUBCLASS_SITEWEAPON_FLOORSENTRY, -1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestSentryGun[uSentryCount] );
						_apTestSentryGun[uSentryCount] = NULL;
					} 
					uSentryCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestPillBoxIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uPillCount;
		for (uPillCount = 0; uIni < uNumInitData && uPillCount < NUM_TEST_PILLBOXES; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestPillBoxIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestPillBoxIniData[uIni].m_uNumClones && uPillCount < NUM_TEST_PILLBOXES; uCloneCount++)
				{
					FASSERT(_apTestPillBox[uPillCount] == NULL);
					_apTestPillBox[uPillCount] = fnew CBotSiteWeapon();
					if( _apTestPillBox[uPillCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestPillBoxIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestPillBoxIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestPillBoxIniData[uIni].m_pszName, uPillCount);

					if( !_apTestPillBox[uPillCount]->Create(BOTSUBCLASS_SITEWEAPON_PILLBOX, -1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestPillBox[uPillCount] );
						_apTestPillBox[uPillCount] = NULL;
					} 
					uPillCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


		uNumInitData = sizeof(_aTestMortarIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uMortarCount;
		for (uMortarCount = 0; uIni < uNumInitData && uMortarCount < NUM_TEST_MORTAR; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestMortarIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestMortarIniData[uIni].m_uNumClones && uMortarCount < NUM_TEST_MORTAR; uCloneCount++)
				{
					FASSERT(_apTestMortar[uMortarCount] == NULL);
					_apTestMortar[uMortarCount] = fnew CBotMortar();
					if( _apTestMortar[uMortarCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestMortarIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestMortarIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestMortarIniData[uIni].m_pszName, uMortarCount);
					if( !_apTestMortar[uMortarCount]->Create(-1, szNameTmp, &InitMat) )
					{
						fdelete( _apTestMortar[uMortarCount] );
						_apTestMortar[uMortarCount] = NULL;
					} 
					uMortarCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestSloshIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uSloshCount;
		for (uSloshCount = 0; uIni < uNumInitData && uSloshCount < NUM_TEST_SLOSHES; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestSloshIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestSloshIniData[uIni].m_uNumClones && uSloshCount < NUM_TEST_SLOSHES; uCloneCount++)
				{
					FASSERT(_apTestSloshes[uSloshCount] == NULL);
					_apTestSloshes[uSloshCount] = fnew CBotSlosh();
					if( _apTestSloshes[uSloshCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestSloshIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestSloshIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestSloshIniData[uIni].m_pszName, uSloshCount);
					if( !_apTestSloshes[uSloshCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestSloshes[uSloshCount] );
						_apTestSloshes[uSloshCount] = NULL;
					} 
					uSloshCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


		uNumInitData = sizeof(_aTestMozerIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uMozerCount;
		for (uMozerCount = 0; uIni < uNumInitData && uMozerCount < NUM_TEST_MOZERS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestMozerIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestMozerIniData[uIni].m_uNumClones && uMozerCount < NUM_TEST_MOZERS; uCloneCount++)
				{
					FASSERT(_apTestMozers[uMozerCount] == NULL);
					_apTestMozers[uMozerCount] = fnew CBotMozer();
					if( _apTestMozers[uMozerCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestMozerIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestMozerIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestMozerIniData[uIni].m_pszName, uMozerCount);
					if( !_apTestMozers[uMozerCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestMozers[uMozerCount] );
						_apTestMozers[uMozerCount] = NULL;
					} 
					uMozerCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestPredIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uPredCount;
		for (uPredCount = 0; uIni < uNumInitData && uPredCount < NUM_TEST_PREDS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestPredIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestPredIniData[uIni].m_uNumClones && uPredCount < NUM_TEST_PREDS; uCloneCount++)
				{
					FASSERT(_apTestPred[uPredCount] == NULL);
					_apTestPred[uPredCount] = fnew CBotPred();
					if( _apTestPred[uPredCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestPredIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestPredIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestPredIniData[uIni].m_pszName, uPredCount);
					if( !_apTestPred[uPredCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestPred[uPredCount] );
						_apTestPred[uPredCount] = NULL;
					} 
					else
					{
						_apTestPred[uPredCount]->DataPort_Open( TRUE );
						_apTestPred[uPredCount]->DataPort_SetPossessionDist( 10050.0f );
						SetupTestBotAI(_apTestPred[uPredCount], _aTestPredIniData+uIni);
					}
					uPredCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


		uNumInitData = sizeof(_aTestZomIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uZomCount;
		for (uZomCount = 0; uIni < uNumInitData && uZomCount < NUM_TEST_ZOMS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestZomIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestZomIniData[uIni].m_uNumClones && uZomCount < NUM_TEST_ZOMS; uCloneCount++)
				{
					FASSERT(_apTestZom[uZomCount] == NULL);
					_apTestZom[uZomCount] = fnew CBotZom();
					if( _apTestZom[uZomCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestZomIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestZomIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestZomIniData[uIni].m_pszName, uZomCount);
					if( !_apTestZom[uZomCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default", TRUE) )
					{
						fdelete( _apTestZom[uZomCount] );
						_apTestZom[uZomCount] = NULL;
					} 
					else
					{
//						_apTestZom[uZomCount]->DataPort_Open( TRUE );
//						_apTestZom[uZomCount]->DataPort_SetPossessionDist( 10050.0f );
//						SetupTestBotAI(_apTestZom[uZomCount], _aTestZomIniData+uIni);
					}
					uZomCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


		uNumInitData = sizeof(_aTestProbeIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uProbeCount;
		for (uProbeCount = 0; uIni < uNumInitData && uProbeCount < NUM_TEST_PROBES; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestProbeIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestProbeIniData[uIni].m_uNumClones && uProbeCount < NUM_TEST_PROBES; uCloneCount++)
				{
					FASSERT(_apTestProbe[uProbeCount] == NULL);
					_apTestProbe[uProbeCount] = fnew CBotProbe();
					if( _apTestProbe[uProbeCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestProbeIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestProbeIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestProbeIniData[uIni].m_pszName, uProbeCount);
					if( !_apTestProbe[uProbeCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestProbe[uProbeCount] );
						_apTestProbe[uProbeCount] = NULL;
					} 
					else
					{
						_apTestProbe[uProbeCount]->DataPort_Open( TRUE );
						_apTestProbe[uProbeCount]->DataPort_SetPossessionDist( 10050.0f );
						SetupTestBotAI(_apTestProbe[uProbeCount], _aTestProbeIniData+uIni);
					}
					uProbeCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestGuardIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uGuardCount;
		for (uGuardCount = 0; uIni < uNumInitData && uGuardCount < NUM_TEST_GUARDS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestGuardIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestGuardIniData[uIni].m_uNumClones && uGuardCount < NUM_TEST_GUARDS; uCloneCount++)
				{
					FASSERT(_apTestGuard[uGuardCount] == NULL);
					_apTestGuard[uGuardCount] = fnew CBotEliteGuard();
					if( _apTestGuard[uGuardCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestGuardIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestGuardIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestGuardIniData[uIni].m_pszName, uGuardCount);
					if( !_apTestGuard[uGuardCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestGuard[uGuardCount] );
						_apTestGuard[uGuardCount] = NULL;
					} 
					else
					{
						_apTestGuard[uGuardCount]->DataPort_Open( TRUE );
						_apTestGuard[uGuardCount]->DataPort_SetPossessionDist( 1000.0f );
						SetupTestBotAI(_apTestGuard[uGuardCount], _aTestGuardIniData+uIni);
					}
					uGuardCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestScientistIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uScientistCount;
		for (uScientistCount = 0; uIni < uNumInitData && uScientistCount < NUM_TEST_SCIENTISTS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestScientistIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestScientistIniData[uIni].m_uNumClones && uScientistCount < NUM_TEST_SCIENTISTS; uCloneCount++)
				{
					FASSERT(_apTestScientists[uScientistCount] == NULL);
					_apTestScientists[uScientistCount] = fnew CBotScientist();
					if( _apTestScientists[uScientistCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestScientistIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestScientistIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestScientistIniData[uIni].m_pszName, uScientistCount);
					if( !_apTestScientists[uScientistCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestScientists[uScientistCount] );
						_apTestScientists[uScientistCount] = NULL;
					} 
					else
					{
						_apTestScientists[uScientistCount]->DataPort_Open( TRUE );
						_apTestScientists[uScientistCount]->DataPort_SetPossessionDist( 1000.0f );
						SetupTestBotAI(_apTestScientists[uScientistCount], _aTestScientistIniData+uIni);
					}
					uScientistCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestTitanIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uTitanCount;
		for (uTitanCount = 0; uIni < uNumInitData && uTitanCount < NUM_TEST_TITANS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestTitanIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestTitanIniData[uIni].m_uNumClones && uTitanCount < NUM_TEST_TITANS; uCloneCount++)
				{
					FASSERT(_apTestTitan[uTitanCount] == NULL);
					_apTestTitan[uTitanCount] = fnew CBotTitan();
					if( _apTestTitan[uTitanCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestTitanIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestTitanIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestTitanIniData[uIni].m_pszName, uTitanCount);
					if( !_apTestTitan[uTitanCount]->Create(-1,
															TRUE,
															szNameTmp,
															&InitMat,
															"Default",
															FALSE) )	//TRUE/FALSE Shield/nosheild
					{
						fdelete( _apTestTitan[uTitanCount] );
						_apTestTitan[uTitanCount] = NULL;
					} 
					else
					{
						_apTestTitan[uTitanCount]->DataPort_Open( TRUE );
						_apTestTitan[uTitanCount]->DataPort_SetPossessionDist( 10050.0f );
						SetupTestBotAI(_apTestTitan[uTitanCount], _aTestTitanIniData+uIni);
					}
					uTitanCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		//// swarmers
		uNumInitData = sizeof(_aTestSwarmerIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uSwarmerCount;
		for (uSwarmerCount = 0; uIni < uNumInitData && uSwarmerCount < NUM_TEST_SWARMERS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestSwarmerIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestSwarmerIniData[uIni].m_uNumClones && uSwarmerCount < NUM_TEST_SWARMERS; uCloneCount++)
				{
					FASSERT(_apTestSwarmer[uSwarmerCount] == NULL);
					_apTestSwarmer[uSwarmerCount] = fnew CBotSwarmer();
					if( _apTestSwarmer[uSwarmerCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.m_vPos = _aTestSwarmerIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestSwarmerIniData[uIni].m_pszName, uSwarmerCount);
					if( !_apTestSwarmer[uSwarmerCount]->Create(-1, FALSE, szNameTmp, &InitMat, "Default"))// CBotSwarmer::BotTypeVermin) )
					{
						fdelete( _apTestSwarmer[uSwarmerCount] );
						_apTestSwarmer[uSwarmerCount] = NULL;
					} 
					else
					{
							_apTestSwarmer[uSwarmerCount]->PositionForSpawn(_aTestSwarmerIniData[uIni].m_Loc_WS);
					}
					uSwarmerCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


		// Jump Troopers
		uNumInitData = sizeof(_aTestJumperIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uJumperCount;
		for (uJumperCount = 0; uIni < uNumInitData && uJumperCount < NUM_TEST_JUMPERS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestJumperIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestJumperIniData[uIni].m_uNumClones && uJumperCount < NUM_TEST_JUMPERS; uCloneCount++)
				{
					FASSERT(_apTestJumper[uJumperCount] == NULL);
					_apTestJumper[uJumperCount] = fnew CBotJumper();
					if( _apTestJumper[uJumperCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestJumperIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestJumperIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestJumperIniData[uIni].m_pszName, uJumperCount);
					if( !_apTestJumper[uJumperCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestJumper[uJumperCount] );
						_apTestJumper[uJumperCount] = NULL;
					} 
					else
					{
						_apTestJumper[uJumperCount]->AIBrain()->SetRace( _aTestJumperIniData[uIni].m_uAIRace );
						_apTestJumper[uJumperCount]->DataPort_Open( TRUE );
						_apTestJumper[uJumperCount]->DataPort_SetPossessionDist( 10050.0f );
						SetupTestBotAI(_apTestJumper[uJumperCount], _aTestJumperIniData+uIni);
					}
					uJumperCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

//Rats
		uNumInitData = sizeof(_aTestRatIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uRatCount;
		for (uRatCount = 0; uIni < uNumInitData && uRatCount < NUM_TEST_RATS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestRatIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestRatIniData[uIni].m_uNumClones && uRatCount < NUM_TEST_RATS; uCloneCount++)
				{
					FASSERT(_apTestRat[uRatCount] == NULL);
					_apTestRat[uRatCount] = fnew CVehicleRat();
					if( _apTestRat[uRatCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestRatIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestRatIniData[uIni].m_Loc_WS;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestRatIniData[uIni].m_pszName, uRatCount);
					if( !_apTestRat[uRatCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestRat[uRatCount] );
						_apTestRat[uRatCount] = NULL;
					} 
					else
					{
//						_apTestRat[uRatCount]->DataPort_Open( TRUE );
//						_apTestRat[uRatCount]->DataPort_SetPossessionDist( 10050.0f );
//						SetupTestBotAI(_apTestRat[uRatCount], _aTestRatIniData+uIni);
					}
					uRatCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

   //Miners
	{
		u32 uNumInitData = sizeof(_aTestMinerIniData)/sizeof(CTestBotIniData);
		u32 uMinerCount;
		u32 uIni = 0;
		for (uMinerCount = 0; uMinerCount < NUM_TEST_MINERS && uIni < uNumInitData; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestMinerIniData+uIni, uNumInitData-uIni, &uRelIni))
			{
				uIni+=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestMinerIniData[uIni].m_uNumClones && uMinerCount < NUM_TEST_MINERS; uCloneCount++)
				{
					FASSERT(_apTestMiner[uMinerCount] == NULL);
					_apTestMiner[uMinerCount] = fnew CBotMiner();
					if( _apTestMiner[uMinerCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestMinerIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestMinerIniData[uIni].m_Loc_WS;
					InitMat.m_vPos.x+=(uCloneCount%10)*10.0f;
					InitMat.m_vPos.z+=(uCloneCount/10)*10.0f;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestMinerIniData[uIni].m_pszName, uMinerCount);

					if( !_apTestMiner[uMinerCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestMiner[uMinerCount] );
						_apTestMiner[uMinerCount] = NULL;
					} 
					else
					{
						_apTestMiner[uMinerCount]->DataPort_Open( TRUE );
						_apTestMiner[uMinerCount]->DataPort_SetPossessionDist( 150.0f );
						SetupTestBotAI(_apTestMiner[uMinerCount], _aTestMinerIniData+uIni);
					}
					uMinerCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}
	}


	//General Corrosives
	uNumInitData = sizeof(_aTestCorrosiveIniData)/sizeof(CTestBotIniData);
	uIni = 0;
	u32 uCorrosiveCount;
	for (uCorrosiveCount = 0; uIni < uNumInitData && uCorrosiveCount < NUM_TEST_CORROSIVES; uIni++)
	{
		u32 uRelIni = 0;
		if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestCorrosiveIniData+uIni, uNumInitData-uIni, &uRelIni) &&
			Level_aInfo[Level_nLoadedIndex].pszWorldResName)
		{
			uIni +=uRelIni;
			for (u32 uCloneCount = 0; uCloneCount < 1+_aTestCorrosiveIniData[uIni].m_uNumClones && uCorrosiveCount < NUM_TEST_CORROSIVES; uCloneCount++)
			{
				FASSERT(_apTestCorrosive[uCorrosiveCount] == NULL);
				_apTestCorrosive[uCorrosiveCount] = fnew CBotCorrosive();
				if( _apTestCorrosive[uCorrosiveCount] == NULL )
				{
					return;
				}

				CFMtx43A InitMat;
				InitMat.Identity();
				InitMat.SetRotationY(_aTestCorrosiveIniData[uIni].m_fYaw);
				InitMat.m_vPos = _aTestCorrosiveIniData[uIni].m_Loc_WS;
				char szNameTmp[32];
				szNameTmp[0] = 0;
				sprintf(szNameTmp, "%s_%d", _aTestCorrosiveIniData[uIni].m_pszName, uCorrosiveCount);
				if( !_apTestCorrosive[uCorrosiveCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
				{
					fdelete( _apTestCorrosive[uCorrosiveCount] );
					_apTestCorrosive[uCorrosiveCount] = NULL;
				} 
				else
				{
//						_apTestCorrosive[uCorrosiveCount]->DataPort_Open( TRUE );
//						_apTestCorrosive[uCorrosiveCount]->DataPort_SetPossessionDist( 10050.0f );
						SetupTestBotAI(_apTestCorrosive[uCorrosiveCount], _aTestCorrosiveIniData+uIni);
				}
				uCorrosiveCount++;
			}
		}
		else
		{
			uIni = uNumInitData;
		}
	}

   //Scouts
	{
		u32 uNumInitData = sizeof(_aTestScoutIniData)/sizeof(CTestBotIniData);
		u32 uScoutCount;
		u32 uIni = 0;
		for (uScoutCount = 0; uScoutCount < NUM_TEST_SCOUTS && uIni < uNumInitData; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestScoutIniData+uIni, uNumInitData-uIni, &uRelIni))
			{
				uIni+=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestScoutIniData[uIni].m_uNumClones && uScoutCount < NUM_TEST_SCOUTS; uCloneCount++)
				{
					FASSERT(_apTestScout[uScoutCount] == NULL);
					_apTestScout[uScoutCount] = fnew CBotScout();
					if( _apTestScout[uScoutCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestScoutIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestScoutIniData[uIni].m_Loc_WS;
					InitMat.m_vPos.x+=(uCloneCount%10)*10.0f;
					InitMat.m_vPos.z+=(uCloneCount/10)*10.0f;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestScoutIniData[uIni].m_pszName, uGruntCount);
					if( !_apTestScout[uScoutCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestScout[uScoutCount] );
						_apTestScout[uScoutCount] = NULL;
					} 
					else
					{
						SetupTestBotAI(_apTestScout[uScoutCount], _aTestScoutIniData+uIni);
					}
					uScoutCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}

		uNumInitData = sizeof(_aTestSnarqIniData)/sizeof(CTestBotIniData);
		uIni = 0;
		u32 uSnarqCount;
		for (uSnarqCount = 0; uIni < uNumInitData && uSnarqCount < NUM_TEST_SNARQS; uIni++)
		{
			u32 uRelIni = 0;
			if (_FindLevelNameInArrayIniData(Level_aInfo[Level_nLoadedIndex].pszWorldResName, _aTestSnarqIniData+uIni, uNumInitData-uIni, &uRelIni) &&
				Level_aInfo[Level_nLoadedIndex].pszWorldResName)
			{
				uIni +=uRelIni;
				for (u32 uCloneCount = 0; uCloneCount < 1+_aTestSnarqIniData[uIni].m_uNumClones && uSnarqCount < NUM_TEST_SNARQS; uCloneCount++)
				{
					FASSERT(_apTestSnarqs[uSnarqCount] == NULL);
					_apTestSnarqs[uSnarqCount] = fnew CBotSnarq();
					if( _apTestSnarqs[uSnarqCount] == NULL )
					{
						return;
					}

					CFMtx43A InitMat;
					InitMat.Identity();
					InitMat.SetRotationY(_aTestSnarqIniData[uIni].m_fYaw);
					InitMat.m_vPos = _aTestSnarqIniData[uIni].m_Loc_WS;
					InitMat.m_vPos.x+=(uCloneCount%10)*10.0f;
					InitMat.m_vPos.z+=(uCloneCount/10)*10.0f;
					char szNameTmp[32];
					szNameTmp[0] = 0;
					sprintf(szNameTmp, "%s_%d", _aTestSnarqIniData[uIni].m_pszName, uSnarqCount);
					if( !_apTestSnarqs[uSnarqCount]->Create(-1, TRUE, szNameTmp, &InitMat, "Default") )
					{
						fdelete( _apTestSnarqs[uSnarqCount] );
						_apTestSnarqs[uSnarqCount] = NULL;
					} else {
						SetupTestBotAI(_apTestSnarqs[uSnarqCount], _aTestSnarqIniData+uIni);
					}

					uSnarqCount++;
				}
			}
			else
			{
				uIni = uNumInitData;
			}
		}


	}
}





///////////////////////////////////////////////////////////////////////////////////
//WORK

void testbots_Work( void ) {
	if (0)//_apTestGrunt[0] && _apTestGrunt[0]->IsDead())
	{
		_apTestGrunt[0]->UnDie();
		_apTestGrunt[0]->SetNormHealth(1.0f);
		SetupTestBotAI(_apTestGrunt[0], _aTestGruntIniData+0);
	}


	#if SAS_ACTIVE_USER == SAS_USER_STEVE
		CBotZom *pBotZom = (CBotZom *)Player_aPlayer[0].m_pEntityOrig;

		if( pBotZom->TypeBits() & ENTITY_BIT_BOTZOM ) {
			//xxxxxxxxxxxxxxxxxx
			if( pBotZom->m_nPossessionPlayerIndex >= 0 ) {
				if( (Gamepad_aapSample[0][GAMEPAD_MAIN_MELEE2]->uLatches & (FPAD_LATCH_CHANGED | FPAD_LATCH_ON)) == (FPAD_LATCH_CHANGED | FPAD_LATCH_ON) ) {

					pBotZom->Hop( FALSE );
#if 0
					if( pBotZom->GetZomMode() == CBotZom::ZOMMODE_ALIVE ) {
						pBotZom->CrumbleIntoDebris();
					} else {
						pBotZom->StartForming();
					}
#endif
				}

#if 0
				if( (Gamepad_aapSample[0][GAMEPAD_MAIN_UP_EUK]->uLatches & (FPAD_LATCH_CHANGED | FPAD_LATCH_ON)) == (FPAD_LATCH_CHANGED | FPAD_LATCH_ON) ) {
					// Blow into debris...

//					pBotZom->CrumbleIntoDebris();
					pBotZom->CollapseIntoDebris();
				}
#endif
			}
		}
	#endif
}


///////////////////////////////////////////////////////////////////////////////////
//DRAW

// called from:  _DrawDebugStuff_Persp()
void testbots_Draw( s32 nPlayer  ) {
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	frenderer_Push( FRENDERER_DRAW );
//	CBotPred::DebugDraw( (CBotPred*)Player_aPlayer[0].m_pEntityCurrent );
	
	CVehicleLoader::DebugDraw( (CVehicleLoader*)_pMELoader );
	if( nPlayer != 0 ) {
		return;
	}

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	//CBotGlitch::DebugDraw( (CBotGlitch*)Player_aPlayer[0].m_pEntityOrig );
	CBotMiner::DebugDraw( (CBotMiner*)Player_aPlayer[0].m_pEntityOrig );
#endif

	frenderer_Pop();
#endif
}



///////////////////////////////////////////////////////////////////////////////////
//CLEANUP

void testbots_Cleanup( void ) {

	u32 i;
	for (i = 0; i < NUM_TEST_GRUNTS; i++)
	{
		if( _apTestGrunt[i] ) {
			fdelete( _apTestGrunt[i] );
			_apTestGrunt[i] = NULL;
		}
	}

	if( _pMETitan ) {
		fdelete( _pMETitan );
		_pMETitan = NULL;
	}

	if( _pMEPred ) {
		fdelete( _pMEPred );
		_pMEPred = NULL;
	}

	if( _pMELoader ) {
		fdelete( _pMELoader );
		_pMELoader = NULL;
	}

	if( _pSentinel ) {
		fdelete( _pSentinel );
		_pSentinel = NULL;
	}

	if( _pZom ) {
		fdelete( _pZom );
		_pZom = NULL;
	}

	
	for (i = 0; i < NUM_TEST_SENTRYGUNS; i++)	{
		fdelete (_apTestSentryGun[i]);
		_apTestSentryGun[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_PILLBOXES; i++)	{
		fdelete (_apTestPillBox[i]);
		_apTestPillBox[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_PREDS; i++)	{
		fdelete (_apTestPred[i]);
		_apTestPred[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_ZOMS; i++)	{
		fdelete (_apTestZom[i]);
		_apTestZom[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_PROBES; i++)	{
		fdelete (_apTestProbe[i]);
		_apTestProbe[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_GUARDS; i++)	{
		fdelete (_apTestGuard[i]);
		_apTestGuard[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_TITANS; i++)	{
		fdelete (_apTestTitan[i]);
		_apTestTitan[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_JUMPERS; i++)	{
		fdelete (_apTestJumper[i]);
		_apTestJumper[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_SWARMERS; i++)	{
		fdelete (_apTestSwarmer[i]);
		_apTestSwarmer[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_MINERS; i++)	{
		fdelete (_apTestMiner[i]);
		_apTestMiner[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_SCOUTS; i++)	{
		fdelete (_apTestScout[i]);
		_apTestScout[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_MORTAR; i++)	{
		fdelete (_apTestMortar[i]);
		_apTestMortar[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_RATS; i++)	{
		fdelete (_apTestRat[i]);
		_apTestRat[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_CORROSIVES; i++)	{
		fdelete (_apTestCorrosive[i]);
		_apTestCorrosive[i] = NULL;
	}

	for (i = 0; i < NUM_TEST_SLOSHES; i++)	{
		fdelete (_apTestSloshes[i]);
		_apTestSloshes[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_MOZERS; i++)	{
		fdelete (_apTestMozers[i]);
		_apTestMozers[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_SNARQS; i++)	{
		fdelete (_apTestSnarqs[i]);
		_apTestSnarqs[i] = NULL;
	}
	for (i = 0; i < NUM_TEST_SCIENTISTS; i++)	{
		fdelete (_apTestScientists[i]);
		_apTestScientists[i] = NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////////
//OVERRIDEPLAYERBOT


static BOOL _Steve_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Mike_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Pat_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _John_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Nathan_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Chris_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Elliott_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Jeremy_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Scholz_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Russ_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );
static BOOL _Dan_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix );

// return FALSE if the player bot has not been created, TRUE if it has
BOOL testbots_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
//	frenderer_DrawBones_Enable( TRUE );
//	return FALSE;

#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
	return _Steve_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_MIKE )
	return _Mike_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_PAT )
	return _Pat_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_JOHN )
	return _John_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_NATHAN )
	return _Nathan_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_CHRIS )
	return _Chris_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_ELLIOTT )
	return _Elliott_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_JEREMY )
	return _Jeremy_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_SCHOLZ )
	return _Scholz_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_RUSS )
	return _Russ_OverridePlayerBot( pPlayer, pStartMatrix );

#elif( SAS_ACTIVE_USER == SAS_USER_DAN )
	return _Dan_OverridePlayerBot( pPlayer, pStartMatrix );

#endif

	return FALSE;
}

static BOOL _Steve_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
#if 0
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotZom;
	if( pPlayer->m_pEntityOrig == NULL ) {
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

	CFMtx43A InitMat;
	InitMat.Identity();
	InitMat.m_vPos.x = -366.7f;
	InitMat.m_vPos.y = 0.855f;
	InitMat.m_vPos.z = 146.6f;

	if( !((CBotZom*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) {
//	if( !((CBotTitan*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", &InitMat ) ) {
		FASSERT_NOW;
	} else {
		return TRUE;
	}

#endif
	return FALSE;
}

static BOOL _Mike_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
#if 0
	if( fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName) &&
		fclib_stricmp("RampTest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName) ) {
		 return FALSE;
	}

	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotKrunk;
	if( pPlayer->m_pEntityOrig == NULL ) {
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
	if( !((CBotKrunk *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) {
		FASSERT_NOW;
		return FALSE;
	}

	return TRUE;
#endif
	return FALSE;
}

static BOOL _Pat_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	//one rainbow roll please.
	return FALSE;
}

static BOOL _John_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	return FALSE;
}

static BOOL _Nathan_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	return FALSE;
}

static BOOL _Chris_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	return FALSE;
}

static BOOL _Elliott_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	if( fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName) &&
		fclib_stricmp("RampTest4", Level_aInfo[Level_nLoadedIndex].pszWorldResName) &&
		fclib_stricmp("WE_E3_03", Level_aInfo[Level_nLoadedIndex].pszWorldResName) &&
//		fclib_stricmp("WERRswarmr1", Level_aInfo[Level_nLoadedIndex].pszWorldResName) &&
		fclib_stricmp("WESSstatn03", Level_aInfo[Level_nLoadedIndex].pszWorldResName) ) {
		 return FALSE;
	}

	//return FALSE;

	// Allocate a new player bot...
	//pPlayer->m_pEntityOrig = fnew CBotPred;
	//pPlayer->m_pEntityOrig = fnew CBotTitan;
	//	Player_aPlayer[0].m_pEntityOrig = fnew CBotGrunt;
	//pPlayer->m_pEntityOrig = fnew CBotMiner;
	//pPlayer->m_pEntityOrig = fnew CBotJumper;
	//pPlayer->m_pEntityOrig = fnew CBotSwarmerBoss;
	pPlayer->m_pEntityOrig = fnew CBotSnarq;

	if( pPlayer->m_pEntityOrig == NULL ) {
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
	{
		CFMtx43A InitMat;
		InitMat.Identity();
		//InitMat.m_vPos.x = -358.0f;
		//InitMat.m_vPos.y = 30.0f;
		//InitMat.m_vPos.z = 124.0f;

		InitMat.m_vPos.x = -110.0f;
		InitMat.m_vPos.y = 55.0f;
		InitMat.m_vPos.z = -170.0f;

//		if( !((CBotMiner*)pPlayer->m_pEntityOrig)->Create(  0, FALSE, "Player0", &InitMat/*pStartEntity->MtxToWorld()*/ ) ) {
		//if( !((CBotJumper*)pPlayer->m_pEntityOrig)->Create(  0, FALSE, "Player0", &InitMat/*pStartEntity->MtxToWorld()*/ ) ) {
//		if( !((CBotPred *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartEntity->MtxToWorld() ) ) {
//		if( !((CBotTitan *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", &InitMat ) ) { //pStartEntity->MtxToWorld() ) ) {
//		if( !((CBotJumper *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) {

//		if( !((CBotGrunt *)Player_aPlayer[0].m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) { //pStartEntity->MtxToWorld() ) ) {
//		if( !((CBotSwarmerBoss *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) {
		if( !((CBotSnarq*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) {
			FASSERT_NOW;
		}
		

	}
	return TRUE;
}

static BOOL _Jeremy_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	return FALSE;
}

static BOOL _Scholz_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) { 
#define BE_GLITCH	
#define BE_BOSS
#define BE_MOZER
#define BE_MORTAR
#define BE_GUARD
#define BE_SLOSH
#define BE_TITAN

#ifdef BE_GLITCH
//	if( fclib_stricmp("jump_test", Level_aInfo[Level_nLoadedIndex].pszWorldResName) != 0)
	{
		return FALSE;
	}
#endif 

#ifdef BE_BOSS
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotZombieBoss;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	if( !((CBotZombieBoss *)pPlayer->m_pEntityOrig)->Create( 0, "Boss", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
		return TRUE;
	}
#endif 


#ifdef BE_MOZER
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotMozer;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	if( !((CBotMozer *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Mozer", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
		return TRUE;
	}
#endif 

#ifdef BE_MORTAR
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotMortar;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	if( !((CBotMortar *)pPlayer->m_pEntityOrig)->Create(0,"Michael", pStartMatrix ))
	{
		FASSERT_NOW;
	}
	else
	{
		pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
		return TRUE;
	}
#endif 

#ifdef BE_GUARD
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotEliteGuard;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	if( !((CBotEliteGuard *)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "GUARD", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
		return TRUE;
	}
#endif 

#ifdef BE_SLOSH
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotSlosh;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

	if( !((CBotSlosh*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
		return TRUE;
	}
#endif

#ifdef BE_TITAN
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotTitan;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

	if( !((CBotTitan*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		return TRUE;
	}
#endif
	return FALSE;
}

static BOOL _Russ_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {

#define RUSS_BE_GLITCH	
//#define RUSS_BE_TITAN
#define RUSS_BE_CORROSIVE

#ifdef RUSS_BE_GLITCH
	return FALSE;
#endif

#ifdef RUSS_BE_TITAN
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotTitan;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

	if( !((CBotTitan*)pPlayer->m_pEntityOrig)->Create( 0, FALSE, "Player0", pStartMatrix ) ) 
	{
		FASSERT_NOW;
	}
	else
	{
		return TRUE;
	}
#endif

#ifdef RUSS_BE_CORROSIVE
	// Allocate a new player bot...
	pPlayer->m_pEntityOrig = fnew CBotCorrosive;
	if( pPlayer->m_pEntityOrig == NULL ) 
	{
		DEVPRINTF( "game_StartGame(): Not enough memory to create Player 0's bot.\n" );
		FASSERT_NOW;
	}

	// Remember player entity pointers...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;
	{

		if( !((CBotCorrosive*)pPlayer->m_pEntityOrig)->Create( 0, TRUE, "Player0", pStartMatrix ) ) 
		{
			FASSERT_NOW;
		}
		else
		{
			return TRUE;
		}
	}
#endif

	return FALSE;
}

static BOOL _Dan_OverridePlayerBot( CPlayer *pPlayer, const CFMtx43A *pStartMatrix ) {
	return FALSE;
}

#endif //TESTBOTS_ENABLE
