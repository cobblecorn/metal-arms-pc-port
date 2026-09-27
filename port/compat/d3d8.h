// d3d8.h - Direct3D 8 compatibility layer on top of Direct3D 9.
//
// Modern Windows SDKs no longer ship the Direct3D 8 headers/libs, and the OS
// only exposes D3D9+. Fang's DX backend was written against D3D8, so this
// header presents the D3D8 interface the engine expects and forwards it to a
// D3D9 device. Only the parts of D3D8 that Fang actually uses are provided.
//
// Interfaces whose method signatures are identical in D3D8 and D3D9 (textures,
// surfaces, vertex/index buffers) are plain aliases. The two interfaces that
// changed materially, IDirect3D8 and IDirect3DDevice8, are thin wrapper classes
// implemented in d3d8_compat.cpp.

#ifndef _PORT_D3D8_H_
#define _PORT_D3D8_H_ 1

#include <windows.h>
#include <d3d9.h>

#define D3D_SDK_VERSION_D3D8	220

// ---------------------------------------------------------------------------
// Plain aliases
// ---------------------------------------------------------------------------
typedef IDirect3DSurface9			IDirect3DSurface8;
typedef IDirect3DTexture9			IDirect3DTexture8;
typedef IDirect3DCubeTexture9		IDirect3DCubeTexture8;
typedef IDirect3DVolumeTexture9		IDirect3DVolumeTexture8;
typedef IDirect3DBaseTexture9		IDirect3DBaseTexture8;
typedef IDirect3DVertexBuffer9		IDirect3DVertexBuffer8;
typedef IDirect3DIndexBuffer9		IDirect3DIndexBuffer8;
typedef IDirect3DResource9			IDirect3DResource8;

typedef IDirect3DSurface9			*LPDIRECT3DSURFACE8;
typedef IDirect3DTexture9			*LPDIRECT3DTEXTURE8;
typedef IDirect3DCubeTexture9		*LPDIRECT3DCUBETEXTURE8;
typedef IDirect3DVertexBuffer9		*LPDIRECT3DVERTEXBUFFER8;
typedef IDirect3DIndexBuffer9		*LPDIRECT3DINDEXBUFFER8;

typedef D3DCAPS9					D3DCAPS8;
typedef D3DLIGHT9					D3DLIGHT8;
typedef D3DMATERIAL9				D3DMATERIAL8;
typedef D3DVIEWPORT9				D3DVIEWPORT8;
typedef D3DADAPTER_IDENTIFIER9		D3DADAPTER_IDENTIFIER8;

// D3D9 renamed this presentation field.
#define FullScreen_PresentationInterval		PresentationInterval

// ---------------------------------------------------------------------------
// Xbox "linear" texture formats. On Windows every texture is already linear.
// ---------------------------------------------------------------------------
#define D3DFMT_LIN_A8R8G8B8		D3DFMT_A8R8G8B8
#define D3DFMT_LIN_X8R8G8B8		D3DFMT_X8R8G8B8
#define D3DFMT_LIN_R8G8B8		D3DFMT_R8G8B8
#define D3DFMT_LIN_R5G6B5		D3DFMT_R5G6B5
#define D3DFMT_LIN_A1R5G5B5		D3DFMT_A1R5G5B5
#define D3DFMT_LIN_X1R5G5B5		D3DFMT_X1R5G5B5
#define D3DFMT_LIN_A4R4G4B4		D3DFMT_A4R4G4B4
#define D3DFMT_LIN_D24S8		D3DFMT_D24S8
#define D3DFMT_LIN_D24X8		D3DFMT_D24X8
#define D3DFMT_LIN_D24X4S4		D3DFMT_D24X4S4
#define D3DFMT_LIN_D16			D3DFMT_D16
#define D3DFMT_LIN_D15S1		D3DFMT_D15S1
#define D3DFMT_LIN_D32			D3DFMT_D32

// D3D8 raster/adapter caps that were renamed or dropped in D3D9.
#define D3DPRASTERCAPS_ZBIAS			D3DPRASTERCAPS_DEPTHBIAS
#define D3DCAPS2_CANRENDERWINDOWED		0x00080000L		// D3D9 has no such cap: every adapter can render windowed

// D3D9 removed D3DSURFACE_DESC::Size. Bytes used by one mip level of the described surface.
UINT D3D8Compat_SurfaceSize( const D3DSURFACE_DESC *pDesc );

// D3D8-only usage/lock flags that have no D3D9 equivalent.
#define D3DUSAGE_PERSISTENTDIFFUSE		0
#define D3DUSAGE_PERSISTENTSPECULAR		0

// ---------------------------------------------------------------------------
// D3D8 texture-stage states that D3D9 moved to sampler states. The values are
// the original D3D8 numbers (unused by D3D9's own D3DTSS_ enum); the device
// wrapper's SetTextureStageState() reroutes them to SetSamplerState().
// ---------------------------------------------------------------------------
#define D3DTSS_ADDRESSU			13
#define D3DTSS_ADDRESSV			14
#define D3DTSS_BORDERCOLOR		15
#define D3DTSS_MAGFILTER		16
#define D3DTSS_MINFILTER		17
#define D3DTSS_MIPFILTER		18
#define D3DTSS_MIPMAPLODBIAS	19
#define D3DTSS_MAXMIPLEVEL		20
#define D3DTSS_MAXANISOTROPY	21
#define D3DTSS_ADDRESSW			25

// D3D8-only render states. The wrapper ignores them (or maps them).
#define D3DRS_ZBIAS						((D3DRENDERSTATETYPE)47)		// wrapper -> D3DRS_DEPTHBIAS
#define D3DRS_PATCHSEGMENTS				((D3DRENDERSTATETYPE)164)
#define D3DRS_LINEPATTERN				((D3DRENDERSTATETYPE)10)
#define D3DRS_ZVISIBLE					((D3DRENDERSTATETYPE)30)
#define D3DRS_EDGEANTIALIAS				((D3DRENDERSTATETYPE)40)
#define D3DRS_SOFTWAREVERTEXPROCESSING	((D3DRENDERSTATETYPE)153)	// wrapper -> SetSoftwareVertexProcessing()

// ---------------------------------------------------------------------------
// D3D8 vertex shader declaration tokens (removed in D3D9). The wrapper's
// CreateVertexShader() translates the token stream to a D3D9 vertex declaration.
// ---------------------------------------------------------------------------
#define D3DVSD_TOKENTYPESHIFT		29
#define D3DVSD_TOKENTYPEMASK		(7 << D3DVSD_TOKENTYPESHIFT)
#define D3DVSD_STREAMNUMBERSHIFT	0
#define D3DVSD_STREAMNUMBERMASK		(0xF << D3DVSD_STREAMNUMBERSHIFT)
#define D3DVSD_DATALOADTYPESHIFT	28
#define D3DVSD_DATALOADTYPEMASK		(0x1 << D3DVSD_DATALOADTYPESHIFT)
#define D3DVSD_DATATYPESHIFT		16
#define D3DVSD_DATATYPEMASK			(0xF << D3DVSD_DATATYPESHIFT)
#define D3DVSD_SKIPCOUNTSHIFT		16
#define D3DVSD_SKIPCOUNTMASK		(0xF << D3DVSD_SKIPCOUNTSHIFT)
#define D3DVSD_VERTEXREGSHIFT		0
#define D3DVSD_VERTEXREGMASK		(0x1F << D3DVSD_VERTEXREGSHIFT)

#define D3DVSD_TOKEN_NOP			0
#define D3DVSD_TOKEN_STREAM			1
#define D3DVSD_TOKEN_STREAMDATA		2
#define D3DVSD_TOKEN_TESSELLATOR	3
#define D3DVSD_TOKEN_CONSTMEM		4
#define D3DVSD_TOKEN_EXT			5
#define D3DVSD_TOKEN_END			7

#define D3DVSD_MAKETOKENTYPE(t)		(((DWORD)(t) << D3DVSD_TOKENTYPESHIFT) & D3DVSD_TOKENTYPEMASK)
#define D3DVSD_STREAM(_n)			(D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (_n))
#define D3DVSD_REG(_reg, _type)		(D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | ((DWORD)(_type) << D3DVSD_DATATYPESHIFT) | (_reg))
#define D3DVSD_SKIP(_dwords)		(D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | 0x10000000 | ((DWORD)(_dwords) << D3DVSD_SKIPCOUNTSHIFT))
#define D3DVSD_END()				0xFFFFFFFF

#define D3DVSDT_FLOAT1			0
#define D3DVSDT_FLOAT2			1
#define D3DVSDT_FLOAT3			2
#define D3DVSDT_FLOAT4			3
#define D3DVSDT_D3DCOLOR		4
#define D3DVSDT_UBYTE4			5
#define D3DVSDT_SHORT2			6
#define D3DVSDT_SHORT4			7

#define D3DVSDE_POSITION		0
#define D3DVSDE_BLENDWEIGHT		1
#define D3DVSDE_BLENDINDICES	2
#define D3DVSDE_NORMAL			3
#define D3DVSDE_PSIZE			4
#define D3DVSDE_DIFFUSE			5
#define D3DVSDE_SPECULAR		6
#define D3DVSDE_TEXCOORD0		7
#define D3DVSDE_TEXCOORD1		8
#define D3DVSDE_TEXCOORD2		9
#define D3DVSDE_TEXCOORD3		10
#define D3DVSDE_TEXCOORD4		11
#define D3DVSDE_TEXCOORD5		12
#define D3DVSDE_TEXCOORD6		13
#define D3DVSDE_TEXCOORD7		14
#define D3DVSDE_POSITION2		15
#define D3DVSDE_NORMAL2			16

// ---------------------------------------------------------------------------
// IDirect3DDevice8 / IDirect3D8 wrappers
// ---------------------------------------------------------------------------
class IDirect3D8;

class IDirect3DDevice8
{
public:
	// Construct around an existing D3D9 device (takes ownership of one reference).
	explicit IDirect3DDevice8( IDirect3DDevice9 *pDev9 );
	~IDirect3DDevice8();

	// Not COM: Release() deletes the wrapper when the count hits zero.
	ULONG AddRef();
	ULONG Release();

	IDirect3DDevice9 *GetD3D9Device() const { return m_pDev; }

	// --- device / caps / present
	HRESULT TestCooperativeLevel();
	HRESULT GetDeviceCaps( D3DCAPS8 *pCaps );
	HRESULT Reset( D3DPRESENT_PARAMETERS *pPresentationParameters );
	HRESULT Present( CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion );
	HRESULT GetBackBuffer( UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer );
	HRESULT GetFrontBuffer( IDirect3DSurface8 *pDestSurface );
	void	SetGammaRamp( DWORD Flags, CONST D3DGAMMARAMP *pRamp );
	void	GetGammaRamp( D3DGAMMARAMP *pRamp );
	BOOL	ShowCursor( BOOL bShow );
	HRESULT SetCursorProperties( UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8 *pCursorBitmap );
	void	SetCursorPosition( int X, int Y, DWORD Flags );

	// --- resource creation
	HRESULT CreateTexture( UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 **ppTexture );
	HRESULT CreateCubeTexture( UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture );
	HRESULT CreateVertexBuffer( UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8 **ppVertexBuffer );
	HRESULT CreateIndexBuffer( UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8 **ppIndexBuffer );
	HRESULT CreateDepthStencilSurface( UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8 **ppSurface );
	HRESULT CreateImageSurface( UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8 **ppSurface );
	HRESULT CopyRects( IDirect3DSurface8 *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface8 *pDestinationSurface, CONST POINT *pDestPointsArray );

	// --- render targets
	HRESULT SetRenderTarget( IDirect3DSurface8 *pRenderTarget, IDirect3DSurface8 *pNewZStencil );
	HRESULT GetDepthStencilSurface( IDirect3DSurface8 **ppZStencilSurface );
	HRESULT Clear( DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil );
	HRESULT BeginScene();
	HRESULT EndScene();
	HRESULT SetViewport( CONST D3DVIEWPORT8 *pViewport );

	// --- state
	HRESULT SetTransform( D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix );
	HRESULT GetTransform( D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix );
	HRESULT SetRenderState( D3DRENDERSTATETYPE State, DWORD Value );
	HRESULT SetTextureStageState( DWORD Stage, DWORD Type, DWORD Value );
	HRESULT SetTexture( DWORD Stage, IDirect3DBaseTexture8 *pTexture );
	HRESULT SetMaterial( CONST D3DMATERIAL8 *pMaterial );
	HRESULT SetLight( DWORD Index, CONST D3DLIGHT8 *pLight );
	HRESULT LightEnable( DWORD Index, BOOL Enable );

	// --- shaders (D3D8 handle model; see d3d8_compat.cpp)
	HRESULT CreateVertexShader( CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage );
	HRESULT SetVertexShader( DWORD Handle );
	HRESULT DeleteVertexShader( DWORD Handle );
	HRESULT SetVertexShaderConstant( DWORD Register, CONST void *pConstantData, DWORD ConstantCount );
	HRESULT CreatePixelShader( CONST DWORD *pFunction, DWORD *pHandle );
	HRESULT SetPixelShader( DWORD Handle );
	HRESULT DeletePixelShader( DWORD Handle );
	HRESULT SetPixelShaderConstant( DWORD Register, CONST void *pConstantData, DWORD ConstantCount );

	// --- geometry
	HRESULT SetStreamSource( UINT StreamNumber, IDirect3DVertexBuffer8 *pStreamData, UINT Stride );
	HRESULT SetIndices( IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex );
	HRESULT DrawPrimitive( D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount );
	HRESULT DrawIndexedPrimitive( D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount );
	HRESULT DrawPrimitiveUP( D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );
	HRESULT DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );

private:
	struct _VShader;
	struct _PShader;

	IDirect3DDevice9		*m_pDev;
	bool					m_bIsEx;
	ULONG					m_nRefs;
	UINT					m_nBaseVertexIndex;		// D3D8 kept this in SetIndices(); D3D9 wants it per draw call
	D3DPRESENT_PARAMETERS	m_LastPP;

	_VShader				*m_paVShaders;			// D3D8 vertex shader "handles" index into these
	UINT					m_nVShaderCount, m_nVShaderCap;
	_PShader				*m_paPShaders;
	UINT					m_nPShaderCount, m_nPShaderCap;
};

class IDirect3D8
{
public:
	explicit IDirect3D8( IDirect3D9 *pD3D9 );
	~IDirect3D8();

	ULONG AddRef();
	ULONG Release();

	UINT	GetAdapterCount();
	HRESULT GetAdapterIdentifier( UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier );
	UINT	GetAdapterModeCount( UINT Adapter );
	HRESULT EnumAdapterModes( UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode );
	HRESULT GetAdapterDisplayMode( UINT Adapter, D3DDISPLAYMODE *pMode );
	HRESULT CheckDeviceType( UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed );
	HRESULT CheckDeviceFormat( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat );
	HRESULT CheckDeviceMultiSampleType( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType );
	HRESULT CheckDepthStencilMatch( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat );
	HRESULT GetDeviceCaps( UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps );
	HMONITOR GetAdapterMonitor( UINT Adapter );
	HRESULT CreateDevice( UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface );

private:
	IDirect3D9	*m_pD3D;
	ULONG		m_nRefs;
};

typedef IDirect3D8			*LPDIRECT3D8;
typedef IDirect3DDevice8	*LPDIRECT3DDEVICE8;

// Returns NULL on failure. The argument is ignored (D3D8 SDK version).
IDirect3D8 *Direct3DCreate8( UINT SDKVersion );

#endif	// _PORT_D3D8_H_
