#ifndef MA_PORT_DOLPHIN_GX_ENUM_H
#define MA_PORT_DOLPHIN_GX_ENUM_H

// The retail mesh structures use these Dolphin GX enum values. The Windows
// port needs the values for decoding serialized GameCube vertex descriptors;
// it does not call the Dolphin SDK.
typedef enum GXCompType {
	GX_U8 = 0,
	GX_S8 = 1,
	GX_U16 = 2,
	GX_S16 = 3,
	GX_F32 = 4
} GXCompType;

enum {
	GX_NONE = 0,
	GX_DIRECT = 1,
	GX_INDEX8 = 2,
	GX_INDEX16 = 3
};

#endif
