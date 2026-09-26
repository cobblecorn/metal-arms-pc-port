#ifndef MA_PORT_GCDATA_H
#define MA_PORT_GCDATA_H

#include "fang.h"
#include "ftex.h"

// Convert a GameCube asset blob to the byte order and runtime representation
// expected by the Windows engine. Unhandled resource types are left untouched.
BOOL gcdata_Convert( cchar *pszExtension, cchar *pszResName, void *pData, u32 nBytes );
BOOL gcdata_DecodeTga( const void *pFileData, u32 nFileBytes, FTexInfo_t *pTexInfo, void **ppImageData, u32 *pnImageBytes );

#endif
