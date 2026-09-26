#ifndef _AIRULESGROUND_H_
#define _AIRULESGROUND_H_ 1


// RuleSets
BOOL InitRuleSet_0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_0(u32 uControl, void* pParam1, void* pParam2);			

BOOL InitRuleSet_1(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_1(u32 uControl, void* pParam1, void* pParam2);			

BOOL InitRuleSet_NPC(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_NPC(u32 uControl, void* pParam1, void* pParam2);			


BOOL InitRuleSet_Titan0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Titan0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Pred0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Pred0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Probe0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Probe0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Rat0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Rat0(u32 uControl, void* pParam1, void* pParam2);			

BOOL InitRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2);	

BOOL InitRuleSet_Zombie0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Zombie0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Corrosive0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Corrosive0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_ZombieBoss0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_ZombieBoss0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Jumper0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Jumper0(u32 uControl, void* pParam1, void* pParam2);		

BOOL InitRuleSet_Scientist0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_Scientist0(u32 uControl, void* pParam1, void* pParam2);			

//
// RuleSetStates
//
BOOL InRules_ARS_0_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Base(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_Hide(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Hide(u32 uControl, void* pParam1, void* pParam2);						

BOOL InRules_ARS_0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);			
BOOL DoRules_ARS_0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_0_Panic(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Panic(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2 );
BOOL DoRules_ARS_0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_Challenge(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Challenge(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_Search(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Search(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_SiteGunner(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_SiteGunner(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_Passenger(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_0_Passenger(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_0_Driver(u32 uControl, void* pParam1, void* pParam2);					
BOOL DoRules_ARS_0_Driver(u32 uControl, void* pParam1, void* pParam2);					

BOOL InRules_ARS_NPC_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_NPC_Base(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_NPC_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);			
BOOL DoRules_ARS_NPC_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_Titan0_Base(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Titan0_Base(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Titan0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		
BOOL DoRules_ARS_Titan0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		

BOOL InRules_ARS_Titan0_Search(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Titan0_Search(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Pred0_Base(u32 uControl, void* pParam1, void* pParam2);					
BOOL DoRules_ARS_Pred0_Base(u32 uControl, void* pParam1, void* pParam2);					

BOOL InRules_ARS_Pred0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		
BOOL DoRules_ARS_Pred0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		

BOOL InRules_ARS_Pred0_Search(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Pred0_Search(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2);				
BOOL OutRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Probe0_Base(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Probe0_Base(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Probe0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		
BOOL DoRules_ARS_Probe0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);		

BOOL InRules_ARS_Probe0_Search(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Probe0_Search(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_EliteGuard0_Base(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_EliteGuard0_Base(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_EliteGuard0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_EliteGuard0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_EliteGuard0_UseCover(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_EliteGuard0_UseCover(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_EliteGuard0_WatchDog(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_EliteGuard0_WatchDog(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRules_ARS_EliteGuard0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_EliteGuard0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_EliteGuard0_Search(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_EliteGuard0_Search(u32 uControl, void* pParam1, void* pParam2);			

BOOL InRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2);


BOOL InRules_ARS_Zombie0_Base(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Zombie0_Base(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Zombie0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2);	
BOOL DoRules_ARS_Zombie0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2);	

BOOL InRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2);	
BOOL DoRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2);	
BOOL OutRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2);	

BOOL InRules_ARS_Zombie0_Search(u32 uControl, void* pParam1, void* pParam2);				
BOOL DoRules_ARS_Zombie0_Search(u32 uControl, void* pParam1, void* pParam2);				

BOOL InRules_ARS_Corrosive0_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL OutRules_ARS_Corrosive0_Base(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Corrosive0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Corrosive0_Search(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_Search(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Corrosive0_AttackLocation(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_AttackLocation(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Corrosive0_PeerHere(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_PeerHere(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Corrosive0_GetEnemyOffMe(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_GetEnemyOffMe(u32 uControl, void* pParam1, void* pParam2);

BOOL InitRule_ZombieBoss_Search( u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Search( u32 uControl, void* pParam1, void* pParam2);
BOOL InitRule_ZombieBoss_Grab( u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Grab( u32 uControl, void* pParam1, void* pParam2);
BOOL InitRule_ZombieBoss_Smash(u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Smash(u32 uControl, void* pParam1, void* pParam2);
BOOL InitRule_ZombieBoss_Fish( u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Fish( u32 uControl, void* pParam1, void* pParam2);
BOOL InitRule_ZombieBoss_Lurch(  u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Lurch(  u32 uControl, void* pParam1, void* pParam2);
BOOL InitRule_ZombieBoss_Goto(  u32 uControl, void* pParam1, void* pParam2);
BOOL RuleWork_ZombieBoss_Goto(  u32 uControl, void* pParam1, void* pParam2);
BOOL InRules_ARS_Corrosive0_FingerFlick(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Corrosive0_FingerFlick(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Jumper0_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Jumper0_Base(u32 uControl, void* pParam1, void* pParam2);

BOOL InRules_ARS_Scientist0_Base(u32 uControl, void* pParam1, void* pParam2);
BOOL DoRules_ARS_Scientist0_Base(u32 uControl, void* pParam1, void* pParam2);

#endif //_AIRULESGROUND_H_