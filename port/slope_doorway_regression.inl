// Opt-in diagnostic against the actual Mil City doorway and its collision mesh.
static void PortSlopeDoorwayRegression(CBot *actor, u32 now, int &step, u32 &stamp) {
    static int sample = 0;
    static u32 lastReport = 0;
    static BOOL contactReported=FALSE, descentChecked=FALSE;
    static const f32 starts[][3] = {
        {446,24,-817.3f}, {444,20,-817.3f}, {441,15,-817.3f},
        {446,24,-811.5f}, {446,24,-823}, {446,24,-647.4f}
    };
    if (!step) {
        for (CEntity *entity=CEntity::InWorldList_GetHead(); entity; entity=entity->InWorldList_GetNext())
            if (entity!=actor && (entity->TypeBits()&ENTITY_BIT_BOT)) entity->EnableAutoWork(FALSE);
        actor->SetInvincible(TRUE);
        step=1;
    }
    if (step==1) {
        CFMtx43A at=*actor->MtxToWorld();
        at.m_vPos.Set(starts[sample][0],starts[sample][1],starts[sample][2]);
        actor->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);
        CFVec3A stop=actor->m_Velocity_WS;stop.Negate();actor->ApplyVelocityImpulse_WS(stop);
        DEVPRINTF("SLOPE-TEST start sample=%d pos=(%.3f,%.3f,%.3f)\n",sample,at.m_vPos.x,at.m_vPos.y,at.m_vPos.z);
        stamp=now;lastReport=0;contactReported=FALSE;descentChecked=FALSE;step=2;return;
    }
    if (now-stamp>2600 && now-stamp<4200) {
        CFVec3A move(-15-actor->m_Velocity_WS.x,0,0);actor->ApplyVelocityImpulse_WS(move);
    }
    if (!descentChecked && now-stamp>2400) {
        descentChecked=TRUE;
        Check(actor->MtxToWorld()->m_vPos.y<5.0f,"native player descends past the doorway trap without escape force");
    }
    if (!lastReport || now-lastReport>450) {
        const CFVec3A &p=actor->MtxToWorld()->m_vPos;
        const CFVec3A &v=actor->m_Velocity_WS;
        DEVPRINTF("SLOPE-TEST sample=%d ms=%u pos=(%.4f,%.4f,%.4f) velocity=(%.4f,%.4f,%.4f) air=%d jump=%d coll=%x normal=(%.4f,%.4f,%.4f) best=%.4f\n",sample,now-stamp,p.x,p.y,p.z,v.x,v.y,v.z,actor->IsInAir(),actor->IsJumping(),actor->GetCollisionState(),actor->m_SurfaceUnitNorm_WS.x,actor->m_SurfaceUnitNorm_WS.y,actor->m_SurfaceUnitNorm_WS.z,actor->m_fBestCaseSurfaceUnitNormY_WS);
        lastReport=now;
    }
    if (!contactReported && now-stamp>1600) {
        contactReported=TRUE;
        CFSphere sphere;
        sphere.m_Pos=actor->MtxToWorld()->m_vPos.v3;
        sphere.m_Pos.y+=actor->ComputeCollSphereY()-.9f;
        sphere.m_fRadius=actor->m_pBotInfo_Gen->fCollSphere1Radius_MS;
        CFVec3A movement(-.2f,-.4f,0);
        CFCollData data;data.pMovement=&movement;
        data.nCollMask=FCOLL_MASK_COLLIDE_WITH_PLAYER;
        data.nFlags=FCOLL_DATA_IGNORE_BACKSIDE;
        data.nTrackerUserTypeBitsMask=~ENTITY_BIT_BOT;
        fcoll_Clear();fcoll_Check(&data,&sphere);
        for (u32 j=0;j<FColl_nImpactCount;++j) {
            const FCollImpact_t &hit=FColl_aImpactBuf[j];
            CEntity *entity=CGColl::ExtractEntity(&hit);
            DEVPRINTF("SLOPE-CONTACT sample=%d at=(%.3f,%.3f,%.3f) n=(%.5f,%.5f,%.5f) push=(%.5f,%.5f,%.5f) time=%.5f depth=%.5f entity=%s tag=%p tri=(%.3f,%.3f,%.3f)/(%.3f,%.3f,%.3f)/(%.3f,%.3f,%.3f)\n",sample,hit.ImpactPoint.x,hit.ImpactPoint.y,hit.ImpactPoint.z,hit.UnitFaceNormal.x,hit.UnitFaceNormal.y,hit.UnitFaceNormal.z,hit.PushUnitVec.x,hit.PushUnitVec.y,hit.PushUnitVec.z,hit.fUnitImpactTime,hit.fImpactDistInfo,entity&&entity->Name()?entity->Name():"terrain",hit.pTag,hit.aTriVtx[0].x,hit.aTriVtx[0].y,hit.aTriVtx[0].z,hit.aTriVtx[1].x,hit.aTriVtx[1].y,hit.aTriVtx[1].z,hit.aTriVtx[2].x,hit.aTriVtx[2].y,hit.aTriVtx[2].z);
        }
    }
    if (now-stamp>4600) {
        if (++sample<sizeof(starts)/sizeof(starts[0])) step=1;
        else {DEVPRINTF("SLOPE-TEST COMPLETE\n");step=99;}
    }
}
