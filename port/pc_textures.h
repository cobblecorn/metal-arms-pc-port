// Texture replacement: images in a folder are used in place of the game's textures of the same name.
//
// The folder is "textures" beside ma_port.exe (searched recursively), or the one given with
// -textures <dir> / MA_PORT_TEXTURES. A file named after a texture (case ignored, e.g.
// "tb_1conc01.png") replaces it. TGA is read directly; PNG, JPG and BMP go through Windows' image
// decoder. Where one name has several files, TGA is preferred (it keeps alpha), then PNG, BMP, JPG.
//
// A replacement keeps the original's aspect ratio (a different one is refused), is scaled to a power
// of two of at most 2048 and gets a full mip chain. An original with no transparency keeps none; an
// original with transparency lends its alpha to a replacement without any. Each use or refusal is
// logged ("PORT-TEX").
#pragma once

#include "fang.h"
#include "ftex.h"

// pTexInfo / *ppPixels / *pnBytes describe a decoded original (A8R8G8B8, all mips, allocated with
// fmem_Alloc). With a replacement they are changed to describe it (also fmem_Alloc'd) and TRUE is
// returned; otherwise they are left alone.
BOOL pctextures_Apply( FTexInfo_t *pTexInfo, void **ppPixels, u32 *pnBytes );
