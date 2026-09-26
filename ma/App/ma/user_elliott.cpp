



//////////////////////////////////
/////////////////////////////////
////DEBUG STUFF
//
//
//void CBotTitan::DebugDraw( CEntity *pEntity ) {
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//
//	CBotTitan *pThis = (CBotTitan*)pEntity;
//
//
//#if 0
//    CFVec3A vStartPos, vEndPos;
//
//	vStartPos	= pThis->m_MtxToWorld.m_vPos;
//	vEndPos		= pThis->m_MtxToWorld.m_vFront;
//	vEndPos.y	= 0.0f;
//	vEndPos.Unitize();
//	vEndPos.Mul( 20.0f );
//	vEndPos.Add( vStartPos );
//	vStartPos.y += 1.0f;
//	vEndPos.y += 1.0f;
//
//	fdraw_SolidLine( &vStartPos.v3, &vEndPos.v3, &FColor_MotifGreen );
//
//
//	vStartPos	= pThis->m_MtxToWorld.m_vPos;
//	vEndPos		= pThis->m_MtxToWorld.m_vFront;
//	vEndPos.y	= 0.0f;
//	vEndPos.Unitize();
//	vEndPos.RotateY( pThis->m_fLegsYaw_MS );
//	vEndPos.Mul( 20.0f );
//	vEndPos.Add( vStartPos );
//	vStartPos.y += 1.0f;
//	vEndPos.y += 1.0f;
//
//	fdraw_SolidLine( &vStartPos.v3, &vEndPos.v3, &FColor_MotifRed );
//#endif
//
///*
//	CFMtx43A *pmtxHead, *pmtxLight;
//	CFVec3A   vTmpHead, vTmpLight;
//
//	pmtxHead	= pThis->m_pWorldMesh->GetBoneMtxPalette()[pThis->m_pWorldMesh->FindBone("Head")];
//	pmtxLight	= pThis->m_pWorldMesh->GetBoneMtxPalette()[pThis->m_pWorldMesh->FindBone("Spotlight")];
//
//	
//	fdraw_SolidLine( &pmtxHead->m_vPos.v3, &pThis->m_vHeadLookPoint_WS.v3, &FColor_MotifGreen );
//	fdraw_SolidLine( &pmtxLight->m_vPos.v3, &pThis->m_vHeadLookPoint_WS.v3, &FColor_MotifGreen );
//
//	vTmpLight.Mul( pmtxLight->m_vFront, 1000.0f );
//	vTmpLight.Add( pmtxLight->m_vPos );
//
//	vTmpHead.Mul( pmtxHead->m_vFront, 1000.0f );
//	vTmpHead.Add( pmtxHead->m_vPos );
//
//	fdraw_SolidLine( &pmtxHead->m_vPos.v3, &vTmpHead.v3, &FColor_MotifRed );
//	fdraw_SolidLine( &pmtxLight->m_vPos.v3, &vTmpLight.v3, &FColor_MotifRed );
//*/
//
//	if( pThis->m_nBotFlags & BOTFLAG_TARGET_LOCKED ) {
//		fdraw_FacetedWireSphere( &pThis->m_TargetedPoint_WS.v3, 1, 2, 2, &FColor_MotifGreen );
//	} else {
//		fdraw_FacetedWireSphere( &pThis->m_TargetedPoint_WS.v3, 1, 2, 2, &FColor_MotifRed );
//	}
//
//
//	//u32 uTmpBone = pThis->m_pWorldMesh->FindBone( "R_Arm_Lower" );
//	//CFMtx43A *pmtxArm = pThis->m_pWorldMesh->GetBoneMtxPalette()[uTmpBone];
//	//CFVec3A vTmp;
//	//CFVec3A vTmp2;
//	//CFVec3A vTmp3;
//	//CFVec3A vTmp4;
//
//	//vTmp2 = pmtxArm->m_vFront;
//	//vTmp2.Mul( 1000.0f );
//	//vTmp = pmtxArm->m_vP;
//	//vTmp.Add( vTmp2 );
//
//	//fdraw_FacetedWireSphere( &pThis->m_TargetedPoint_WS.v3, 10, 2, 2, &FColor_MotifGreen);
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &pThis->m_TargetedPoint_WS.v3, &FColor_MotifBlue );
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &vTmp.v3, &FColor_MotifRed );
//
//	//uTmpBone = pThis->m_pWorldMesh->FindBone( "L_Arm_Lower" );
//	//pmtxArm = pThis->m_pWorldMesh->GetBoneMtxPalette()[uTmpBone];
//
//	//vTmp2 = pmtxArm->m_vFront;
//	//vTmp2.Mul( 1000.0f );
//	//vTmp = pmtxArm->m_vP;
//	//vTmp.Add( vTmp2 );
//
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &pThis->m_TargetedPoint_WS.v3, &FColor_MotifBlue );
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &vTmp.v3, &FColor_MotifRed );
//
//
//	////rocket launcher
//	//pmtxArm = pThis->m_pWorldMesh->GetBoneMtxPalette()[pThis->m_nBoneIndexRocketFire];
//
//	//vTmp2 = pmtxArm->m_vFront;
//	//vTmp2.Mul( 1000.0f );
//	//vTmp = pmtxArm->m_vP;
//	//vTmp.Add( vTmp2 );
//
//	//
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &pThis->m_TargetedPoint_WS.v3, &FColor_MotifBlue );
//	//fdraw_SolidLine( &pmtxArm->m_vP.v3, &vTmp.v3, &FColor_MotifRed );
//
//	//ftext_DebugPrintf( 0.75f, 0.6f, "~w1RAT:  %0.2f", pThis->m_fRocketAimTimer );
//#endif
//}
//
//
////WILL BE INCORPORATED AS DEBUG STUFF INTO CBOT
//#if 0
//void CBotTitan::_ttSetVariables( void ) {
//	return;
//	if( !m_bControls_Human ) {
//		return;
//	}
//
//	// Human controls...
//	CHumanControl *pHumanControl = (CHumanControl *)Controls();
//	BOOL bUpdate = FALSE;
//
//	if(Gamepad_aapSample[0][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK) {
////	if( pHumanControl->m_nPadFlagsSelect1 & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
//		m_nTempIdx += 1;
//		bUpdate = TRUE;
//	}
//
//	if(Gamepad_aapSample[0][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK) {
////	if( pHumanControl->m_nPadFlagsSelect2 & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
//		m_nTempIdx -= 1;
//		bUpdate = TRUE;
//	}
//
//	if(Gamepad_aapSample[0][GAMEPAD_MAIN_FIRE_PRIMARY]->uLatches ) {
//		*m_pfTempVar += 2.0f * m_fTempScale * FLoop_fPreviousLoopSecs;
//		bUpdate = TRUE;
//	}
//
//	if(Gamepad_aapSample[0][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches /* & GAMEPAD_BUTTON_1ST_PRESS_MASK */) {
//		*m_pfTempVar -= 2.0f * m_fTempScale * FLoop_fPreviousLoopSecs;
//		bUpdate = TRUE;
//	}
//
//	if( bUpdate || m_nTempIdx > 31 ) {
//		switch( m_nTempIdx ) {
//			case 0:
//				m_pfTempVar = &m_pBotInfo_Walk->fSneakVelocity;
//				fclib_strcpy( m_szTempDesc, "fSneakVelocity" );
//				m_fTempScale = .10f;
//				break;
//			case 1:
//				m_pfTempVar = &m_pBotInfo_Walk->fMinWalkVelocity;
//				fclib_strcpy( m_szTempDesc, "fMinWalkVelocity" );
//				m_fTempScale = .10f;
//				break;
//			case 2:
//				m_pfTempVar = &m_pBotInfo_Walk->fMaxRunBlendVelocity;
//				fclib_strcpy( m_szTempDesc, "fMaxRunBlendVelocity" );
//				m_fTempScale = 1.0f;
//				break;
//			case 3:
//				m_pfTempVar = &m_pBotInfo_Walk->fMaxXlatVelocity;
//				fclib_strcpy( m_szTempDesc, "fMaxXlatVelocity" );
//				m_fTempScale = 1.0f;
//				break;
//			case 4:
//				m_pfTempVar = &m_pBotInfo_Walk->fNormVelocityStepSize;
//				fclib_strcpy( m_szTempDesc, "fNormVelocityStepSize" );
//				m_fTempScale = 0.1f;
//				break;
//			case 5:
//				m_pfTempVar = &m_pBotInfo_Walk->fStepSizeSlopeFactor;
//				fclib_strcpy( m_szTempDesc, "fStepSizeSlopeFactor" );
//				m_fTempScale = 0.1f;
//				break;
//			case 6:
//				m_pfTempVar = &m_pBotInfo_Walk->fMaxSneakStickMag;
//				fclib_strcpy( m_szTempDesc, "fMaxSneakStickMag" );
//				m_fTempScale = 0.01f;
//				break;
//			case 7:
//				m_pfTempVar = &m_pBotInfo_Walk->fTopSpeedMultiplierForSlope;
//				fclib_strcpy( m_szTempDesc, "fTopSpeedMultiplierForSlope" );
//				m_fTempScale = 0.01f;
//				break;
//			case 8:
//				m_pfTempVar = &m_pBotInfo_Walk->fFasterThanUsualNormSpeed;
//				fclib_strcpy( m_szTempDesc, "fFasterThanUsualNormSpeed" );
//				m_fTempScale = 0.01f;
//				break;
//			case 9:
//				m_pfTempVar = &m_pBotInfo_Walk->fWithinUsualNormSpeed;
//				fclib_strcpy( m_szTempDesc, "fWithinUsualNormSpeed" );
//				m_fTempScale = 0.01f;
//				break;
//			case 10:
//				m_pfTempVar = &m_pBotInfo_Walk->fSteepSlopeUnitNormY;
//				fclib_strcpy( m_szTempDesc, "fSteepSlopeUnitNormY" );
//				m_fTempScale = 0.01f;
//				break;
//			case 11:
//				m_pfTempVar = &m_pBotInfo_Walk->fSlipStickBiasSpeed;
//				fclib_strcpy( m_szTempDesc, "fSlipStickBiasSpeed" );
//				m_fTempScale = 0.01f;
//				break;
//			case 12:
//				m_pfTempVar = &m_pBotInfo_Walk->fMaxSlipStickBias;
//				fclib_strcpy( m_szTempDesc, "fMaxSlipStickBias" );
//				m_fTempScale = 0.1f;
//				break;
//			case 13:
//				m_pfTempVar = &m_pBotInfo_Walk->fSlideGravityMult;
//				fclib_strcpy( m_szTempDesc, "fSlideGravityMult" );
//				m_fTempScale = 0.1f;
//				break;
//			case 14:
//				m_pfTempVar = &m_pBotInfo_Walk->fSlideStrafeSpeed;
//				fclib_strcpy( m_szTempDesc, "fSlideStrafeSpeed" );
//				m_fTempScale = 0.1f;
//				break;
//			case 15:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerFootSneak;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerFootSneak" );
//				m_fTempScale = 0.01f;
//				break;
//			case 16:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerFootWalk" );
//				m_fTempScale = 0.01f;
//				break;
//			case 17:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerFootRun" );
//				m_fTempScale = 0.001f;
//				break;
//			case 18:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerRadian;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerRadian" );
//				m_fTempScale = 0.1f;
//				break;
//			case 19:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimSlideWalkMult;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimSlideWalkMult" );
//				m_fTempScale = 0.01f;
//				break;
//			case 20:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerFootAlertWalk;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerFootAlertWalk" );
//				m_fTempScale = 0.01f;
//				break;
//			case 21:
//				m_pfTempVar = &m_pBotInfo_Walk->fDeltaFeetAnimPerFootPanicRun;
//				fclib_strcpy( m_szTempDesc, "fDeltaFeetAnimPerFootPanicRun" );
//				m_fTempScale = 0.01f;
//				break;
//			case 22:
//				m_pfTempVar = &m_pBotInfo_Walk->fAnimAtRestWalkRunUnitTime;
//				fclib_strcpy( m_szTempDesc, "fAnimAtRestWalkRunUnitTime" );
//				m_fTempScale = 0.01f;
//				break;
//			case 23:
//				m_pfTempVar = &m_pBotInfo_Walk->fIdleToWalkDeltaSpeed;
//				fclib_strcpy( m_szTempDesc, "fIdleToWalkDeltaSpeed" );
//				m_fTempScale = 0.01f;
//				break;
//			case 24:
//				m_pfTempVar = &m_pBotInfo_Walk->fIdleToSneakDeltaSpeed;
//				fclib_strcpy( m_szTempDesc, "fIdleToSneakDeltaSpeed" );
//				m_fTempScale = 0.1f;
//				break;
//			case 25:
//				m_pfTempVar = &m_pBotInfo_Walk->fAnimLeftFootDownUnitTime;
//				fclib_strcpy( m_szTempDesc, "fAnimLeftFootDownUnitTime" );
//				m_fTempScale = 0.01f;
//				break;
//			case 26:
//				m_pfTempVar = &m_pBotInfo_Walk->fAnimRightFootDownUnitTime;
//				fclib_strcpy( m_szTempDesc, "fAnimRightFootDownUnitTime" );
//				m_fTempScale = 0.01f;
//				break;
//			case 27:
//				m_pfTempVar = &m_pBotInfo_Walk->fSwivelHipsReturnSpeedThreshold;
//				fclib_strcpy( m_szTempDesc, "fSwivelHipsReturnSpeedThreshold" );
//				m_fTempScale = 1.0f;
//				break;
//			case 28:
//				m_pfTempVar = &m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec;
//				fclib_strcpy( m_szTempDesc, "fSwivelHipsDeltaRadiansPerSec" );
//				m_fTempScale = 1.0f;
//				break;
//			case 29:
//				m_pfTempVar = &m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold;
//				fclib_strcpy( m_szTempDesc, "fHipYawSlackWhileStoppedThreshold" );
//				m_fTempScale = 0.01f;
//				break;
//			case 30:
//				m_pfTempVar = &m_pBotInfo_Walk->fHipFlipHysteresisRads;
//				fclib_strcpy( m_szTempDesc, "fHipFlipHysteresisRads" );
//				m_fTempScale = 0.01f;
//				break;
//			case 31:
//				m_pfTempVar = &m_pBotInfo_Walk->fAirControlNudgeSpeed;
//				fclib_strcpy( m_szTempDesc, "fAirControlNudgeSpeed" );
//				m_fTempScale = 0.1f;
//				break;
//			default:
//				m_pfTempVar = &m_pBotInfo_Walk->fSneakVelocity;
//				fclib_strcpy( m_szTempDesc, "fVerticalVelocityJump1" );
//				m_fTempScale = 1.0f;
//				m_nTempIdx = 0;
//		}
//		
//			m_pBotInfo_Walk->fSneakNormVelocity			= m_pBotInfo_Walk->fSneakVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
//			m_pBotInfo_Walk->fMinWalkNormVelocity		= m_pBotInfo_Walk->fMinWalkVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
//			m_pBotInfo_Walk->fMaxRunBlendNormVelocity	= m_pBotInfo_Walk->fMaxRunBlendVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
//			m_pBotInfo_Walk->fMaxXlatVelocity			= m_pBotInfo_Walk->fMaxXlatVelocity;;
//			m_pBotInfo_Walk->fInvMaxXlatVelocity		= 1.0f / m_pBotInfo_Walk->fMaxXlatVelocity;
//	}
//
//#endif
//#if 0	
//	if( bUpdate || m_nTempIdx > 8 ) {
//		switch( m_nTempIdx ) {
//			case 0:
//				m_pfTempVar = &m_pBotInfo_Jump->fVerticalVelocityJump1;
//				fclib_strcpy( m_szTempDesc, "fVerticalVelocityJump1" );
//				break;
//			case 1:
//				m_pfTempVar = &m_pBotInfo_Jump->fVerticalVelocityJump2;
//				fclib_strcpy( m_szTempDesc, "fVerticalVelocityJump2" );
//				break;
//			case 2:
//				m_pfTempVar = &m_pBotInfo_Jump->fFlipOriginX;
//				fclib_strcpy( m_szTempDesc, "fFlipOriginX" );
//				break;
//			case 3:
//				m_pfTempVar = &m_pBotInfo_Jump->fFlipOriginY;
//				fclib_strcpy( m_szTempDesc, "fFlipOriginY" );
//				break;
//			case 4:
//				m_pfTempVar = &m_pBotInfo_Jump->fFlipOriginZ;
//				fclib_strcpy( m_szTempDesc, "fFlipOriginZ" );
//				break;
//			case 5:
//				m_pfTempVar = &m_pBotInfo_Jump->fTuckUntuckAnimSpeedMult;
//				fclib_strcpy( m_szTempDesc, "fTuckUntuckAnimSpeedMult" );
//				break;
//			case 6:
//				m_pfTempVar = &m_pBotInfo_Jump->fUntuckToFlyBlendSpeed;
//				fclib_strcpy( m_szTempDesc, "fUntuckToFlyBlendSpeed" );
//				break;
//			case 7:
//				m_pfTempVar = &m_pBotInfo_Jump->fGroundToAirBlendSpeed;
//				fclib_strcpy( m_szTempDesc, "fGroundToAirBlendSpeed" );
//				break;
//			case 8:
//				m_pfTempVar = &m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
//				fclib_strcpy( m_szTempDesc, "fMovingLandAnimSpeedMult" );
//				break;
//			default:
//				m_pfTempVar = &m_pBotInfo_Jump->fVerticalVelocityJump1;
//				fclib_strcpy( m_szTempDesc, "fVerticalVelocityJump1" );
//				m_nTempIdx = 0;
//		}											
//	}
//
//	ftext_DebugPrintf( 0.2f, 0.7f, "~w1%s = %0.4f", m_szTempDesc, *m_pfTempVar );
//}
//#endif
//
//
//#if 0	//to be incorporated into cbot
//	void _ttSetVariables( void );
//	f32	 *m_pfTempVar;
//	u32	 m_nTempIdx;
//	char m_szTempDesc[256];
//	f32	 m_fTempScale;
//#endif

