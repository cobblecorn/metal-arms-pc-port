#ifndef MA_PORT_GCMESH_H
#define MA_PORT_GCMESH_H

#include "fang.h"

// Convert one serialized GameCube .ape mesh into the packed DirectX mesh
// representation consumed by fdx8load_Create. The returned data belongs to
// the current temporary fmem frame.
BOOL gcmesh_ConvertToDx( void *pGameCubeData, u32 nGameCubeBytes,
	void **ppDxData, u32 *pnDxBytes, cchar *pszResName );

// Asset tooling: decode a retail texture by name into MA_CHARACTER_EXPORT_DIR/textures.
BOOL gcmesh_ExportTextureByName( cchar *pszName );

#endif
