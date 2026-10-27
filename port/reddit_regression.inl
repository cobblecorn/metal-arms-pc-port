// Opt-in gameplay regressions, included by coop_regression.cpp. Never run in normal play.
static void SystemsReportWork(CBot *player, u32 now, int &step, u32 &stamp) {
    static CDoorEntity *door=NULL;
    static BOOL locked=FALSE;
    static CFMeshAnimTC *light=NULL;
    static CCollectable *washers[16];
    static u32 count=0;
    static CFVec3A source;
    if(!step) {
        for(CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext()) {
            if(!(e->TypeBits()&ENTITY_BIT_DOOR)) continue;
            CFWorldMesh *mesh=((CDoorEntity*)e)->GetMeshInst();
            if(!mesh || !mesh->m_pMesh) continue;
            FMeshTexLayerHandle_t h=mesh->GetTexLayerHandle(20);
            if(h<0 || !mesh->GetTexLayerID(h)->pAnimTC) continue;
            door=(CDoorEntity*)e;light=mesh->GetTexLayerID(h)->pAnimTC;
            if(door->GetBehavior()!=CDoorEntity::DOOR_BEHAVIOR_OTHER) break;
        }
        Check(door!=NULL,"native door with real lock-indicator atlas exists");
        if(door) {
            locked=door->IsLocked();door->SetLockState(TRUE);
            Check(light->m_ScrollPos.x==0.0f && light->m_ScrollPos.y==0.0f,"native locked door selects red atlas half");
            door->SetLockState(FALSE);
            const f32 green=door->GetBehavior()==CDoorEntity::DOOR_BEHAVIOR_OTHER ? 0.0f : .5f;
            Check(light->m_ScrollPos.x==green && light->m_ScrollSTPerSec.x==0 && light->m_ScrollSTPerSec.y==0,"native unlocked door selects stable retail atlas half");
            DEVPRINTF("SYSTEMS-REPORT door=%s behavior=%d indicator=%.2f\n",door->Name(),door->GetBehavior(),green);
        }
        const CollectableType_e types[]={COLLECTABLE_MEGA_HEALTH,COLLECTABLE_MEGA_WASHER,COLLECTABLE_ARM_SERVO_L2};
        const f32 rgb[3][3]={{68.0f/255,138.0f/255,1},{1,194.0f/255,28.0f/255},{64.0f/255,64.0f/255,64.0f/255}};
        for(int n=0;n<3;++n) {
            CCollectable::NotifyCollectableUsedInWorld(types[n]);
            CCollectableType *type=CCollectable::GetCollectableTypeObjectFromType(types[n]);
            Check(type && (type->m_bSetup || type->Setup()),"native tinted pickup pool loads");
            if(!type || !type->m_bSetup) continue;
            CFWorldMesh *mesh=type->GetWorldMesh();
            Check(mesh!=NULL,"native pickup pool provides a real mesh");
            if(mesh) {
                CFColorRGBA *c=mesh->GetMeshTintAndAlpha();
                Check(FMATH_FABS(c->fRed-rgb[n][0])<.0001f && FMATH_FABS(c->fGreen-rgb[n][1])<.0001f && FMATH_FABS(c->fBlue-rgb[n][2])<.0001f,"native pickup instance retains retail color tint");
                type->ReturnWorldMesh(mesh);
            }
        }
        Check(CCollectable::GiveToPlayer(player,"Arm Servo L2",1.0f),"native arm-servo pickup placed at player");
        stamp=now;step=1;
    } else if(step==1 && now-stamp>2000) {
        Check(((CBotGlitch*)player)->GetServoLevel(CBotGlitch::SERVO_TYPE_ARMS)==1,"native arm-servo pickup applies tier two");
        CItemInst *servo=player->m_pInventory->IsItemInInventory("Arm Servo");
        Check(servo && servo->m_nClipAmmo==2,"native arm-servo pickup records its tier in saved inventory");
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WASHER);
        CCollectableType *type=CCollectable::GetCollectableTypeObjectFromType(COLLECTABLE_WASHER);
        Check(type && (type->m_bSetup || type->Setup()),"native dropped-washer resources load");
        source=player->MtxToWorld()->m_vPos;
        CGoodieBag bag;bag.AddGoodie(COLLECTABLE_WASHER,16);bag.ReleaseIntoWorld(player);
        for(CEntity *e=CEntity::InWorldList_GetHead();e && count<16;e=e->InWorldList_GetNext()) {
            if(!(e->TypeBits()&ENTITY_BIT_GOODIE)) continue;
            CCollectable *c=(CCollectable*)e;
            CFVec3A delta;delta.Sub(c->MtxToWorld()->m_vPos,source);
            if(c->GetCollectableType()->m_eType==COLLECTABLE_WASHER && delta.MagSq()<4) washers[count++]=c;
        }
        Check(count==16,"native goodie bag releases sixteen actual washers");
        for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
            CBot *bot=(CBot*)Player_aPlayer[n].m_pEntityOrig;
            CFMtx43A parked=*bot->MtxToWorld();parked.m_vPos=source;parked.m_vPos.z+=80+n*8;bot->Relocate_RotXlatFromUnitMtx_WS(&parked);bot->ZeroVelocity();
        }
        stamp=now;step=2;
    } else if(step==2 && now-stamp>4000) {
        if(door) {
            const f32 expected=door->GetBehavior()==CDoorEntity::DOOR_BEHAVIOR_OTHER ? 0.0f : .5f;
            Check(light->m_ScrollPos.x==expected && light->m_ScrollPos.y==0,"native lock indicator remains stable across animated frames");
            door->SetLockState(locked);
        }
        f32 furthest=0,spread=0;
        for(u32 n=0;n<count;++n) {
            CFVec3A delta;delta.Sub(washers[n]->MtxToWorld()->m_vPos,source);furthest=FMATH_MAX(furthest,delta.MagXZ());
            for(u32 j=0;j<n;++j){delta.Sub(washers[n]->MtxToWorld()->m_vPos,washers[j]->MtxToWorld()->m_vPos);spread=FMATH_MAX(spread,delta.MagXZ());}
        }
        DEVPRINTF("SYSTEMS-REPORT drop count=%u radius=%.2f spread=%.2f\n",count,furthest,spread);
        Check(furthest>3 && spread>6,"native washer drop has real ballistic separation");
        DEVPRINTF("SYSTEMS-REPORT COMPLETE\n");step=99;
    }
}

static void ArenaReportWork(CBot *player, int owner, u32 now, int &step, u32 &stamp) {
    static CBotTalkInst *talk=NULL;
    static BOOL sawSpeech=FALSE;
    static int command=0;
    static CWeapon *weapon=NULL;
    if(!step) {
        if(!CSpyVsSpy::PortTestStage(1)) return;
        for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
            Check(CHud2::GetHudForPlayer(n)->IsWSEnabled(),"native combat stage enables each player's weapon/reload input");
            Check(Player_aPlayer[n].HasEntityControl(),"native combat stage returns each player's character control");
        }
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RIVET_GUN_L1);
        Check(CCollectable::PostPlayerCreationSetup(),"native arena prepares second-weapon pickup resources");
        Check(player->m_pInventory->IsWeaponInInventory("Rivet Gun L1") || CCollectable::GiveWeaponToPlayer(player,"Rivet Gun L1",1),"native arena has a second weapon to select");
        CItemInst *item=player->m_pInventory->IsWeaponInInventory("Rivet Gun L1");
        if(!item || !item->m_pWeapon){Check(FALSE,"native arena second weapon exists");step=99;return;}
        weapon=item->m_pWeapon;weapon->SetClipAmmo(1,FALSE);weapon->SetReserveAmmo(50,TRUE);
        player->m_pInventory->SetCurWeapon(INV_INDEX_PRIMARY,item,FALSE,TRUE);
        stamp=now;step=1;
    } else if(step==1 && now-stamp>3000) {
        Check(player->GetPrimaryWeapon()==weapon,"native arena completes a real weapon switch");
        CHud2::GetHudForPlayer(owner)->PcWeaponSelectTap(0,player->m_pInventory,FALSE);
        stamp=now;step=2;
    } else if(step==2 && now-stamp>5000) {
        Check(weapon->GetClipAmmo()>1 && weapon->GetReserveAmmo()<50,"native arena tap reload transfers reserve into clip");
        CBot *instructor=(CBot*)CEntity::Find("win4");
        if(!instructor){Check(FALSE,"native instruction actor exists");step=99;return;}
        if(ai_IsTalking(instructor->AIBrain())) ai_TalkModeEnd(instructor->AIBrain());
        talk=CTalkSystem2::BTIPool_Get(command ? "brgt_stepl" : "brgt_cstepl");
        Check(talk!=NULL,"native real/decoy instruction data loads");
        if(!talk){step=99;return;}
        talk->SetHighPriority(TRUE);
        Check(CTalkSystem2::SubmitTalkRequest(talk,instructor),"native instructor command starts through talk system");
        Check((talk->GetFlags() & (BOTTALKINSTFLAG_FORCE_2D_AUDIO|BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING))==(BOTTALKINSTFLAG_FORCE_2D_AUDIO|BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING),"native true and decoy commands receive equal dialogue routing");
        sawSpeech=FALSE;stamp=now;step=3;
    } else if(step==3) {
        if(talk->m_pCurSound && talk->m_pCurSound->GetState()==FAUDIO_EMITTER_STATE_PLAYING) {
            if(!sawSpeech) Check(!(talk->GetFlags()&BOTTALKINSTFLAG_3DSOUND_PLAYING),"native command really uses 2D audio");
            sawSpeech=TRUE;
        }
        if(now-stamp>5000) {
            Check(sawSpeech,"native instructor speech reaches a playing emitter while muted");
            if(talk->IsWorking()) CTalkSystem2::TerminateActiveTalk(talk);
            if(command++==0){stamp=now-6000;step=2;return;}
            const char *names[]={"Pete","Jimmy","Joey","Mike","Fred","Tony"};
            for(int n=0;n<6;++n){CBot *bot=(CBot*)CEntity::Find(names[n]);Check(bot!=NULL,"native battle fighter exists");if(bot){bot->SetInvincible(FALSE);bot->Die(FALSE,FALSE);}}
            stamp=now;step=4;
        }
    } else if(step==4 && now-stamp>5000) {
        CDoorEntity *exit=(CDoorEntity*)CEntity::Find("battledoor");
        if(exit) DEVPRINTF("ARENA-REPORT exit behavior=%d state=%d pos=%.3f locked=%d\n",exit->GetBehavior(),exit->GetState(),exit->GetUnitPos(),exit->IsLocked());
        Check(exit && exit->GetUnitPos()>.9f,"native completed combat opens the actual exit door");
        for(int n=0;n<CPlayer::m_nPlayerCount;++n) Check(CHud2::GetHudForPlayer(n)->IsDrawEnabled(),"native victory guidance has a visible HUD for every player");
        DEVPRINTF("ARENA-REPORT COMPLETE\n");step=99;
    }
}

static void WasherReportWork(CBot *player, CBot *partner, u32 now, int &step, u32 &stamp) {
    static int funds=0;
    static CFVec3A start;
    if(!step) {
        CCollectableType *type=CCollectable::GetCollectableTypeObjectFromType(COLLECTABLE_WASHER);
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WASHER);
        Check(type && (type->m_bSetup || type->Setup()),"native washer resources available");
        if(!type){step=99;return;}
        player->SetInvincible(TRUE);start=player->MtxToWorld()->m_vPos;
        if(partner!=player){CFMtx43A parked=*partner->MtxToWorld();parked.m_vPos=start;parked.m_vPos.z+=60;partner->Relocate_RotXlatFromUnitMtx_WS(&parked);}
        funds=player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
        CFMtx43A at=*player->MtxToWorld();at.m_vPos.x+=12;at.m_vPos.y+=3;
        Check(CCollectable::PlaceIntoWorld(COLLECTABLE_WASHER,&at,NULL,1.0f,1),"native washer placed twelve units from stationary Glitch");
        DEVPRINTF("WASHER-REPORT waiting player=%d funds=%d\n",player->m_nPossessionPlayerIndex+1,funds);
        stamp=now;step=1;
    } else if(step==1) {
        const int current=player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
        if(current>funds) {
            CFVec3A delta;delta.Sub(player->MtxToWorld()->m_vPos,start);
            Check(delta.MagXZ()<1.0f,"native washer collected without walking into it");
            DEVPRINTF("WASHER-REPORT COMPLETE player=%d funds=%d\n",player->m_nPossessionPlayerIndex+1,current);step=99;
        } else if(now-stamp>12000){Check(FALSE,"native washer attracts and credits wallet");step=99;}
    }
}

static void RebuildReportWork(CBot *player, u32 now, int &step, u32 &stamp) {
    static CWeapon *before=NULL,*after=NULL;
    static BOOL checked=FALSE;
    if(!step) {
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RIVET_GUN_L1);
        CCollectable::PostPlayerCreationSetup();
        if(!player->m_pInventory->IsWeaponInInventory("Rivet Gun L1")) CCollectable::GiveWeaponToPlayer(player,"Rivet Gun L1",20);
        before=player->GetPrimaryWeapon();
        CItemInst *next=NULL;
        CInventory *inv=player->m_pInventory;
        for(int n=0;n<inv->m_auNumWeapons[INV_INDEX_PRIMARY];++n) {
            CItemInst *item=&inv->m_aoWeapons[INV_INDEX_PRIMARY][n];
            if(item->m_pWeapon && item->m_pWeapon!=before && item->m_pItemData &&
               fclib_stricmp(item->m_pItemData->m_pszCodeName,"Empty Primary")){next=item;break;}
        }
        Check(before && next,"native rebuild has two actual weapons");
        if(!before || !next){step=99;return;}
        after=next->m_pWeapon;
        player->BreakIntoPieces(3.0f,TRUE);
        Check(player->m_pInventory->SetCurWeapon(INV_INDEX_PRIMARY,next,FALSE,TRUE),"native rebuild accepts a pending selection");
        stamp=now;step=1;
    } else if(step==1) {
        if(!checked && now-stamp>1000){
            Check(player->IsInPieces() && player->GetPrimaryWeapon()==before,"native disassembled body retains the equipped weapon");checked=TRUE;
        }
        if(player->IsInPieces() && player->GetPrimaryWeapon()!=before){Check(FALSE,"native rebuild never switches while in pieces");step=99;return;}
        if(now-stamp>12000){
            Check(!player->IsInPieces() && player->GetPrimaryWeapon()==after,"native body finishes rebuilding and then applies queued selection");
            DEVPRINTF("REBUILD-REPORT COMPLETE\n");step=99;
        }
    }
}

static void RecruitReportWork(CBot *player, u32 now, int &step, u32 &stamp) {
    static CVehicle *vehicle=NULL;
    static CBot *pilot=NULL;
    static u8 oldRace=0;
    if(!step) {
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RECRUITER);
        Check(CCollectable::PostPlayerCreationSetup(),"native recruitment effect and icon resources load");
        for(CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext()) {
            if(e->TypeBits()&ENTITY_BIT_VEHICLE) {
                CVehicle *v=(CVehicle*)e;
                if(v->m_nPossessionPlayerIndex<0 && v->AIBrain()) {
                    CBot *driver=v->GetDriverBot();
                    if(driver && !driver->IsPlayerBot()) {
                        driver->DataPort_Open(TRUE);
                        if(driver->Recruit_CanBeRecruited()){vehicle=v;pilot=driver;break;}
                    } else if(!driver) vehicle=v;
                }
            }
        }
        Check(vehicle!=NULL,"native recruit fixture has an authored vehicle with a firing brain");
        if(!vehicle){step=99;return;}
        if(!pilot) {
            for(CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext()) {
                if(!(e->TypeBits()&ENTITY_BIT_BOTGRUNT)) continue;
                CBot *bot=(CBot*)e;
                if(!bot->IsDeadOrDying() && !bot->IsPlayerBot() && !bot->m_pDrivingVehicle && bot->AIBrain()) {
                    bot->DataPort_Open(TRUE);
                    if(bot->Recruit_CanBeRecruited()){pilot=bot;break;}
                }
            }
            Check(pilot && vehicle->EnterStation(pilot,CVehicle::STATION_DRIVER,FALSE,TRUE)==CVehicle::STATION_STATUS_ENTERING,"native NPC enters the real vehicle driver station");
        }
        if(!pilot){step=99;return;}
        pilot->SetInvincible(TRUE);vehicle->SetInvincible(TRUE);
        oldRace=ai_GetRace(vehicle->AIBrain());
        pilot->DataPort_Open(TRUE);
        Check(pilot->Recruit_CanBeRecruited(),"native recruit fixture opens the driver's required data port");
        DEVPRINTF("RECRUIT-REPORT vehicle=%s pilot=%s oldRace=%d\n",vehicle->Name(),pilot->Name(),oldRace);
        Check(pilot->Recruit(player),"native recruiter effect accepts the NPC driver");
        stamp=now;step=1;
    } else if(step==1 && now-stamp>3000) {
        Check(pilot->Recruit_IsRecruited(),"native driver completes recruitment");
        Check(vehicle->GetDriverBot()==pilot && pilot->m_pDrivingVehicle==vehicle,"native recruited driver remains aboard the vehicle");
        Check(vehicle->GetTeam()==pilot->GetTeam() && ai_GetRace(vehicle->AIBrain())==ai_GetRace(pilot->AIBrain()),"native vehicle firing brain inherits recruited pilot allegiance");
        Check(aiutils_IsFriendly(vehicle,player),"native recruited vehicle recognizes Glitch as friendly");
        pilot->Unrecruit();stamp=now;step=2;
    } else if(step==2 && now-stamp>2000) {
        Check(!pilot->Recruit_IsRecruited(),"native driver unrecruits normally");
        Check(ai_GetRace(vehicle->AIBrain())==oldRace,"native vehicle firing brain restores original allegiance on unrecruit");
        DEVPRINTF("RECRUIT-REPORT COMPLETE\n");step=99;
    }
}

static void HealthReportWork(CBot *player, int owner, u32 now, int &step, u32 &stamp) {
    static CBot *controlled=NULL;
    if(!step) {
        controlled=(CBot*)CEntity::Find("xa_autobot_grunt27");
        Check(controlled && (controlled->TypeBits()&ENTITY_BIT_BOTGRUNT),"native possession fixture has a real Mil");
        if(!controlled){step=99;return;}
        player->m_pInventory->m_uNumBatteries=3;
        controlled->AddToWorld();controlled->SetMaxHealth();controlled->SetInvincible(TRUE);
        controlled->Possess(owner,1);controlled->DataPort_SetPossessionDist(10000);
        stamp=now;step=1;
    } else if(step==1) {
        if(Player_aPlayer[owner].m_pEntityCurrent==controlled && Player_aPlayer[owner].HasEntityControl()) {
            Check(controlled->HealthContainerCount()==1,"native possessed Mil has one health container");
            Check(player->m_pInventory->m_uNumBatteries==3,"native possession preserves Glitch's three-battery inventory");
            DEVPRINTF("HEALTH-REPORT controlled Mil P%d; rendering real HUD before completion\n",owner+1);
            stamp=now;step=2;
        } else if(now-stamp>20000){Check(FALSE,"native Mil possession completes");step=99;}
    } else if(step==2 && now-stamp>6000) {
        Check(Player_aPlayer[owner].m_pEntityCurrent==controlled,"native possessed Mil remains under player control while HUD renders");
        DEVPRINTF("HEALTH-REPORT COMPLETE\n");step=99;
    }
}

static void PickupReportWork(CBot *player, const char *mode, u32 now, int &step, u32 &stamp) {
    const BOOL scope=!strcmp(mode,"pickup-scope");
    const u32 expected=!strcmp(mode,"pickup-battery3")?3:2;
    if(!step) {
        CFVec3A found;
        const CollectableType_e type=scope ? CCollectable::GetCollectableTypeFromString("EUK Scope") : COLLECTABLE_BATTERY;
        Check(CCollectable::PortFindNearest(type,player->MtxToWorld()->m_vPos,&found),"native authored pickup exists with the correct type");
        if(scope) Check(!player->m_pInventory->IsWeaponInInventory("Scope L1"),"native first scope starts unowned");
        else Check(player->m_pInventory->m_uNumBatteries==expected-1,"native battery starts below its numbered upgrade");
        CFMtx43A at=*player->MtxToWorld();at.m_vPos=found;at.m_vPos.y+=.1f;
        player->SetInvincible(TRUE);player->Relocate_RotXlatFromUnitMtx_WS(&at);player->ZeroVelocity();
        DEVPRINTF("PICKUP-REPORT approach %s at %.2f,%.2f,%.2f\n",scope?"EUK Scope":"Battery",found.x,found.y,found.z);
        stamp=now;step=1;
    } else if(step==1) {
        CItemInst *item=scope ? player->m_pInventory->IsWeaponInInventory("Scope L1") : NULL;
        const BOOL success=scope ? (item && item->m_pWeapon && item->m_pWeapon->IsCreated() && item->m_nUpgradeLevel==1) :
            (player->m_pInventory->m_uNumBatteries==expected && player->HealthContainerCount()==expected);
        if(success) {
            Check(TRUE,scope?"native world EUK Scope grants the first usable attachment":"native numbered battery increases inventory and actor capacity");
            DEVPRINTF("PICKUP-REPORT COMPLETE %s batteries=%u\n",scope?"Scope L1":"Battery",player->m_pInventory->m_uNumBatteries);step=99;
        } else if(now-stamp>8000){Check(FALSE,"native authored pickup applies on collision");step=99;}
    }
}

static void SwarmerReportWork(CBot *player, u32 now, int &step, u32 &stamp) {
    static CSpawnNet *net=NULL;
    static BOOL sawLiving=FALSE;
    static BOOL sawMoving=FALSE, sawAttached=FALSE;
    static CFVec3A positions[256];
    static BOOL tracked[256]={FALSE};
    if(!step) {
        for(CSpawnNet *p=(CSpawnNet*)flinklist_GetHead(&CSpawnSys::m_SpawnNetList);p;p=(CSpawnNet*)flinklist_GetNext(&CSpawnSys::m_SpawnNetList,p))
            if((p->m_uSpawnNetFlags & CSpawnNet::SPAWNNET_NO_AUTO_DEFCON) && p->m_pVisibleSpawnPts && p->m_pVisibleSpawnPts->PointCount()){net=p;break;}
        Check(net!=NULL,"native retail scripted swarmer net exists");
        if(!net){step=99;return;}
        player->SetInvincible(TRUE);
        CFMtx43A at=*player->MtxToWorld();at.m_vPos=net->m_pVisibleSpawnPts->PointArray()[0];at.m_vPos.x+=20;at.m_vPos.y+=2;
        player->Relocate_RotXlatFromUnitMtx_WS(&at);player->ZeroVelocity();
        net->SetDefconLevel(3);stamp=now;step=1;
        DEVPRINTF("SWARMER-REPORT net=%s pool=%u points=%u target=%u\n",net->m_pszSpawnNetName,CSpawnSys::m_auAutoBotPoolSize[0],net->m_pVisibleSpawnPts->PointCount(),net->m_anNumLivingLimit[3]);
    } else if(step==1) {
        for(u32 n=0;n<CSpawnSys::m_auAutoBotPoolSize[0];++n) {
            CBot *bot=CSpawnSys::m_papAutoBotPool[0][n];
            if(bot && bot->IsInWorld() && !bot->IsDeadOrDying()) {
                sawLiving=TRUE;
                sawAttached |= bot->GetParent()==player;
                if(n<256) {
                    CFVec3A delta;delta.Sub(bot->MtxToWorld()->m_vPos,positions[n]);
                    if(tracked[n] && delta.MagSq()>0.01f) sawMoving=TRUE;
                    positions[n]=bot->MtxToWorld()->m_vPos;tracked[n]=TRUE;
                }
            }
        }
        if(now-stamp>17000) {
            DEVPRINTF("SWARMER-REPORT state defcon=%u spawned=%u living=%d moving=%d attached=%d\n",net->m_uDefConLevel,net->m_uNumSpawnedSoFarLifetime,sawLiving,sawMoving,sawAttached);
            Check(sawLiving && net->m_uNumSpawnedSoFarLifetime>0,"native retail swarmer net actually spawns living enemies");
            Check(sawMoving,"native retail swarmers move after spawning");
            Check(net->m_uDefConLevel==3,"native scripted swarmer activation survives automatic timeout");
            DEVPRINTF("SWARMER-REPORT COMPLETE\n");step=99;
        }
    }
}

static void VendorAmmoWork(CBot *buyer, CBot *partner, int owner, u32 now, int &step, u32 &stamp) {
    static int purchase=0, funds=0, partnerFunds=0;
    static CWeapon *weapon=NULL;
    static BOOL pressed=FALSE;
    const char *names[]={"Rivet Gun L1","Spew L1","Blaster L1","RLauncher L1"};
    const int count=sizeof(names)/sizeof(names[0]);
    if(!step) {
        for(int n=0;n<count;n++) {
            CItemInst *item=buyer->m_pInventory->IsWeaponInInventory(names[n]);
            if(!item) {CCollectable::GiveWeaponToPlayer(buyer,names[n],1);item=buyer->m_pInventory->IsWeaponInInventory(names[n]);}
            Check(item && item->m_pWeapon,"native ammo buyer owns weapon");
            if(!item || !item->m_pWeapon){step=99;return;}
            item->m_pWeapon->SetUpgradeLevel(2);
            item->m_pWeapon->SetClipAmmo(1,FALSE);
            item->m_pWeapon->SetReserveAmmo(0,TRUE);
        }
        CEntity *point=CEntity::Find("barter1");
        Check(point && bartersystem_PlaceInWorldAndStartAttracting("barter1"),"native ammo vendor placed");
        if(!point){step=99;return;}
        CFMtx43A at=*point->MtxToWorld();CFVec3A offset(at.m_vFront);offset.Mul(9);at.m_vPos.Add(offset);
        at.m_vFront.Negate();at.m_vRight.Negate();buyer->Relocate_RotXlatFromUnitMtx_WS(&at);
        if(CPlayer::m_nPlayerCount>1){at.m_vPos.x+=30;partner->Relocate_RotXlatFromUnitMtx_WS(&at);}
        buyer->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo=1000;step=1;stamp=now;
    } else if(step==1) {
        if(!bartersystem_IsActive() && now-stamp>1000)
            for(CEntity *p=CEntity::InWorldList_GetHead();p;p=p->InWorldList_GetNext())
                if(bartersystem_IsBarterBot(p)){p->ActionNearby(buyer);break;}
        if(bartersystem_PortReadyForInput()){step=2;stamp=now;}
        else if(now-stamp>25000){Check(FALSE,"native ammo vendor opens");step=99;}
    } else if(step==2 && bartersystem_PortReadyForInput()) {
        if(purchase==count){Gamepad_aapSample[Player_aPlayer[owner].m_nControllerIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches|=GAMEPAD_BUTTON_1ST_PRESS_MASK;step=4;stamp=now;return;}
        CBarterItem *catalog=CBarterItem::FindBarterItem(1,names[purchase]);
        CBarterItemInst *offer=CBarterLevel::GetCurrentItemInst();
        Check(catalog && offer,"native ammo catalog/offer exists");
        if(!catalog || !offer){step=99;return;}
        CCollectableType *type=CCollectable::GetCollectableTypeObjectFromType(catalog->m_eCollectableType);
        if(!type->m_bSetup){CCollectable::NotifyCollectableUsedInWorld(type->m_eType);if(!type->Setup()){Check(FALSE,"native ammo resources load");step=99;return;}}
        offer->RemoveFromWorld();if(!offer->Init(catalog,TRUE,1,37)){Check(FALSE,"native ammo offer initializes");step=99;return;}
        offer->SetAmmo(TRUE);offer->SetNeeded(TRUE);offer->SetSelected(TRUE);offer->AddToWorld();
        weapon=buyer->m_pInventory->IsWeaponInInventory(names[purchase])->m_pWeapon;
        Check(CCollectable::PlayerNeeds((CBotGlitch*)buyer,type->m_eType,TRUE),"native upgraded weapon needs offered ammo");
        funds=buyer->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
        partnerFunds=partner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
        pressed=FALSE;step=3;stamp=now;
    } else if(step==3) {
        if(!pressed && now-stamp>600 && bartersystem_PortReadyForInput()) {
            Gamepad_aapSample[Player_aPlayer[owner].m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches|=GAMEPAD_BUTTON_1ST_PRESS_MASK;pressed=TRUE;
        }
        if(pressed && buyer->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo==funds-37 && now-stamp>1800) {
            const s32 expected=CCollectable::GetCollectableTypeObjectFromString(names[purchase])->m_nAmmoCount;
            DEVPRINTF("VENDOR-AMMO bought shopper=P%d item='%s' reserve=%d expected=%d savedReserve=%d washers=%d\n",owner+1,names[purchase],weapon->GetReserveAmmo(),expected,weapon->GetItemInst()->m_nReserveAmmo,funds-37);
            Check(weapon->GetReserveAmmo()==expected && weapon->GetItemInst()->m_nReserveAmmo==expected,"native ammo purchase fills live and saved reserve");
            Check(weapon->GetUpgradeLevel()==2,"native ammo purchase preserves weapon upgrade");
            if(CPlayer::m_nPlayerCount>1) Check(partner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo==partnerFunds,"native ammo purchase preserves partner wallet");
            purchase++;step=2;stamp=now;
        } else if(now-stamp>15000){Check(FALSE,"native ammo purchase resolves");step=99;}
    } else if(step==4 && !bartersystem_IsActive()) {
        Check(buyer->IsDrawEnabled() && Player_aPlayer[owner].HasEntityControl(),"native ammo shop exit restores player");
        step=5;purchase=0;stamp=now;
    } else if(step==4 && now-stamp>15000){Check(FALSE,"native ammo shop closes");step=99;}
    else if(step==5 && now-stamp>700) {
        weapon=buyer->m_pInventory->IsWeaponInInventory(names[purchase])->m_pWeapon;
        weapon->SetReserveAmmo(0,TRUE);
        const CollectableType_e type=CCollectable::GetCollectableTypeFromString(names[purchase]);
        Check(CCollectable::GiveToPlayer(buyer,type,1.0f),"native physical ammo pickup spawned for buyer");
        step=6;stamp=now;
    } else if(step==6 && now-stamp>1700) {
        const s32 expected=CCollectable::GetCollectableTypeObjectFromString(names[purchase])->m_nAmmoCount;
        DEVPRINTF("VENDOR-AMMO pickup shopper=P%d item='%s' reserve=%d expected=%d savedReserve=%d\n",owner+1,names[purchase],weapon->GetReserveAmmo(),expected,weapon->GetItemInst()->m_nReserveAmmo);
        Check(weapon->GetReserveAmmo()==expected && weapon->GetItemInst()->m_nReserveAmmo==expected,"native physical ammo pickup fills live and saved reserve");
        Check(weapon->GetUpgradeLevel()==2,"native physical ammo pickup preserves upgrade");
        purchase++;stamp=now;
        if(purchase<count){step=5;return;}
        DEVPRINTF("VENDOR-AMMO COMPLETE shopper=P%d purchased=%d physical=%d\n",owner+1,purchase,purchase);step=99;
    }
}
