// sas_user.h - stand-in for Swingin' Ape's per-developer config header.
//
// The original file selected which programmer's private debug code was compiled
// in (SAS_ACTIVE_USER == SAS_USER_xxx). It was not part of the source drop.
// For the port nobody is the "active user", so all of that code is compiled out.

#ifndef _SAS_USER_H_
#define _SAS_USER_H_ 1

#define SAS_USER_NONE		(-1)
#define SAS_USER_ALBERT		0
#define SAS_USER_CHRIS		1
#define SAS_USER_DAN		2
#define SAS_USER_ELLIOTT	3
#define SAS_USER_JEREMY		4
#define SAS_USER_JOHN		5
#define SAS_USER_JUSTIN		6
#define SAS_USER_MIKE		7
#define SAS_USER_NATHAN		8
#define SAS_USER_PAT		9
#define SAS_USER_RUSS		10
#define SAS_USER_SCHOLZ		11
#define SAS_USER_STEVE		12

#ifndef SAS_ACTIVE_USER
	#define SAS_ACTIVE_USER	-1
#endif

#endif
