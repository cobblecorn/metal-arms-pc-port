#ifndef _APENEW_H_
#define _APENEW_H_ 1

#ifdef __cplusplus

/*
#include "fres.h"


typedef struct _arararffooo
{
} arararffooo;
void* operator new(unsigned int nSizeBytes, const arararffooo *bogusParam);
//void* operator new [] (unsigned int nSizeBytes, const arararffooo *bogusParam);
void operator delete( void* ptr, const arararffooo *bogusParam );

#ifndef DONT_USE_APE_NEW
#if !DONT_USE_APE_NEW


#define APE_DELETE(pO) (::operator delete((pO), (arararffooo *)0x2))
#define APE_NEW new((arararffooo*)0x002)

#endif
#else  //DONT_USE_APE_NEW
#define APE_DELETE(pO) (::operator delete((pO)))
#define APE_NEW new

#endif

*/
#define APE_NEW fnew
#define APE_DELETE(a) {if (a) {fdelete (a);}}
#define APE_ARRAYDELETE(b) { if (b) {fdelete_array(b);}}
#endif // __cplusplus
#endif	//_APENEW_H_
