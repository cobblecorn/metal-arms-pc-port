// d3d8_compat.cpp - Direct3D 8 -> Direct3D 9 forwarding layer. See d3d8.h.

#include "d3d8.h"
#include "d3dx8.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static void _CompatLog( const char *pszFormat, ... )
{
	char szBuf[512];
	va_list Args;
	va_start( Args, pszFormat );
	_vsnprintf( szBuf, sizeof(szBuf) - 1, pszFormat, Args );
	va_end( Args );
	szBuf[sizeof(szBuf) - 1] = 0;
	OutputDebugStringA( szBuf );
	fputs( szBuf, stdout );
	fflush( stdout );
}

// ===========================================================================
// Vertex shader declaration translation
// ===========================================================================
//
// D3D8 described vertex layouts with a token stream and bound each vertex
// component to a numbered input register. D3D9 vs_1_x shaders bind by semantic
// through a fixed register table, so the register number picks the semantic:
//
//   v0 POSITION   v1 BLENDWEIGHT  v2 BLENDINDICES  v3 NORMAL  v4 PSIZE
//   v5 COLOR0     v6 COLOR1       v7..v14 TEXCOORD0..7          v15 POSITION1
//
// This keeps every shader's "vN" references valid without touching the shaders.

static BOOL _RegisterToUsage( DWORD nReg, BYTE *pnUsage, BYTE *pnUsageIndex )
{
	static const struct { BYTE nUsage, nIndex; } aMap[16] = {
		{ D3DDECLUSAGE_POSITION,	0 },	// v0
		{ D3DDECLUSAGE_BLENDWEIGHT,	0 },	// v1
		{ D3DDECLUSAGE_BLENDINDICES,0 },	// v2
		{ D3DDECLUSAGE_NORMAL,		0 },	// v3
		{ D3DDECLUSAGE_PSIZE,		0 },	// v4
		{ D3DDECLUSAGE_COLOR,		0 },	// v5
		{ D3DDECLUSAGE_COLOR,		1 },	// v6
		{ D3DDECLUSAGE_TEXCOORD,	0 },	// v7
		{ D3DDECLUSAGE_TEXCOORD,	1 },
		{ D3DDECLUSAGE_TEXCOORD,	2 },
		{ D3DDECLUSAGE_TEXCOORD,	3 },
		{ D3DDECLUSAGE_TEXCOORD,	4 },
		{ D3DDECLUSAGE_TEXCOORD,	5 },
		{ D3DDECLUSAGE_TEXCOORD,	6 },
		{ D3DDECLUSAGE_TEXCOORD,	7 },	// v14
		{ D3DDECLUSAGE_POSITION,	1 },	// v15
	};
	if( nReg >= 16 ) return FALSE;
	*pnUsage = aMap[nReg].nUsage;
	*pnUsageIndex = aMap[nReg].nIndex;
	return TRUE;
}

static BOOL _VsdTypeInfo( DWORD nVsdType, BYTE *pnDeclType, UINT *pnBytes )
{
	switch( nVsdType )
	{
	case D3DVSDT_FLOAT1:	*pnDeclType = D3DDECLTYPE_FLOAT1;	*pnBytes = 4;  return TRUE;
	case D3DVSDT_FLOAT2:	*pnDeclType = D3DDECLTYPE_FLOAT2;	*pnBytes = 8;  return TRUE;
	case D3DVSDT_FLOAT3:	*pnDeclType = D3DDECLTYPE_FLOAT3;	*pnBytes = 12; return TRUE;
	case D3DVSDT_FLOAT4:	*pnDeclType = D3DDECLTYPE_FLOAT4;	*pnBytes = 16; return TRUE;
	case D3DVSDT_D3DCOLOR:	*pnDeclType = D3DDECLTYPE_D3DCOLOR;	*pnBytes = 4;  return TRUE;
	case D3DVSDT_UBYTE4:	*pnDeclType = D3DDECLTYPE_UBYTE4;	*pnBytes = 4;  return TRUE;
	case D3DVSDT_SHORT2:	*pnDeclType = D3DDECLTYPE_SHORT2;	*pnBytes = 4;  return TRUE;
	case D3DVSDT_SHORT4:	*pnDeclType = D3DDECLTYPE_SHORT4;	*pnBytes = 8;  return TRUE;
	}
	return FALSE;
}

static HRESULT _TranslateD3D8Decl( const DWORD *pTokens, D3DVERTEXELEMENT9 *aElem, BYTE *aInputReg, UINT *pnElem )
{
	UINT nElem = 0;
	WORD nStream = 0;
	UINT nOffset = 0;

	for( const DWORD *pTok = pTokens; *pTok != D3DVSD_END(); ++pTok )
	{
		const DWORD nTok = *pTok;
		const DWORD nType = (nTok & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;

		switch( nType )
		{
		case D3DVSD_TOKEN_NOP:
			break;

		case D3DVSD_TOKEN_STREAM:
			nStream = (WORD)(nTok & D3DVSD_STREAMNUMBERMASK);
			nOffset = 0;
			break;

		case D3DVSD_TOKEN_STREAMDATA:
			if( nTok & D3DVSD_DATALOADTYPEMASK )
			{
				// D3DVSD_SKIP: leave a gap of N dwords
				nOffset += ((nTok & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * 4;
			}
			else
			{
				BYTE nDeclType, nUsage, nUsageIndex;
				UINT nBytes;
				const DWORD nReg = nTok & D3DVSD_VERTEXREGMASK;

				if( !_VsdTypeInfo( (nTok & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT, &nDeclType, &nBytes ) ||
					!_RegisterToUsage( nReg, &nUsage, &nUsageIndex ) ||
					nElem >= MAXD3DDECLLENGTH )
				{
					return D3DERR_INVALIDCALL;
				}

				D3DVERTEXELEMENT9 &e = aElem[nElem];
				e.Stream = nStream;
				e.Offset = (WORD)nOffset;
				e.Type = nDeclType;
				e.Method = D3DDECLMETHOD_DEFAULT;
				e.Usage = nUsage;
				e.UsageIndex = nUsageIndex;
				aInputReg[nElem] = (BYTE)nReg;
				nElem++;
				nOffset += nBytes;
			}
			break;

		case D3DVSD_TOKEN_CONSTMEM:
			// Constant data embedded in the declaration: skip its payload (4 dwords each).
			pTok += ((nTok >> 25) & 0xF) * 4;
			break;

		default:
			return D3DERR_INVALIDCALL;
		}
	}

	*pnElem = nElem;
	return D3D_OK;
}

// ===========================================================================
// IDirect3DDevice8
// ===========================================================================

struct IDirect3DDevice8::_VShader
{
	IDirect3DVertexShader9		*pShader;
	IDirect3DVertexDeclaration9	*pDecl;
};

struct IDirect3DDevice8::_PShader
{
	IDirect3DPixelShader9		*pShader;
};

#define _VSHANDLE_FLAG		0x80000000u

IDirect3DDevice8::IDirect3DDevice8( IDirect3DDevice9 *pDev9 ) :
	m_pDev( pDev9 ), m_nRefs( 1 ), m_nBaseVertexIndex( 0 ),
	m_paVShaders( NULL ), m_nVShaderCount( 0 ), m_nVShaderCap( 0 ),
	m_paPShaders( NULL ), m_nPShaderCount( 0 ), m_nPShaderCap( 0 )
{
	memset( &m_LastPP, 0, sizeof(m_LastPP) );
}

IDirect3DDevice8::~IDirect3DDevice8()
{
	for( UINT i=0; i<m_nVShaderCount; i++ )
	{
		if( m_paVShaders[i].pShader ) m_paVShaders[i].pShader->Release();
		if( m_paVShaders[i].pDecl ) m_paVShaders[i].pDecl->Release();
	}
	for( UINT i=0; i<m_nPShaderCount; i++ )
	{
		if( m_paPShaders[i].pShader ) m_paPShaders[i].pShader->Release();
	}
	free( m_paVShaders );
	free( m_paPShaders );
	if( m_pDev ) m_pDev->Release();
}

ULONG IDirect3DDevice8::AddRef()
{
	return ++m_nRefs;
}

ULONG IDirect3DDevice8::Release()
{
	ULONG n = --m_nRefs;
	if( n == 0 ) delete this;
	return n;
}

HRESULT IDirect3DDevice8::TestCooperativeLevel()								{ return m_pDev->TestCooperativeLevel(); }
HRESULT IDirect3DDevice8::GetDeviceCaps( D3DCAPS8 *pCaps )						{ return m_pDev->GetDeviceCaps( pCaps ); }
HRESULT IDirect3DDevice8::BeginScene()											{ return m_pDev->BeginScene(); }
HRESULT IDirect3DDevice8::EndScene()											{ return m_pDev->EndScene(); }
HRESULT IDirect3DDevice8::SetViewport( CONST D3DVIEWPORT8 *pViewport )			{ return m_pDev->SetViewport( pViewport ); }
HRESULT IDirect3DDevice8::SetTransform( D3DTRANSFORMSTATETYPE s, CONST D3DMATRIX *m ) { return m_pDev->SetTransform( s, m ); }
HRESULT IDirect3DDevice8::GetTransform( D3DTRANSFORMSTATETYPE s, D3DMATRIX *m )	{ return m_pDev->GetTransform( s, m ); }
HRESULT IDirect3DDevice8::SetMaterial( CONST D3DMATERIAL8 *pMaterial )			{ return m_pDev->SetMaterial( pMaterial ); }
HRESULT IDirect3DDevice8::SetLight( DWORD i, CONST D3DLIGHT8 *pLight )			{ return m_pDev->SetLight( i, pLight ); }
HRESULT IDirect3DDevice8::LightEnable( DWORD i, BOOL b )						{ return m_pDev->LightEnable( i, b ); }
HRESULT IDirect3DDevice8::SetTexture( DWORD s, IDirect3DBaseTexture8 *pTex )	{ return m_pDev->SetTexture( s, pTex ); }

HRESULT IDirect3DDevice8::Reset( D3DPRESENT_PARAMETERS *pPP )
{
	m_LastPP = *pPP;
	return m_pDev->Reset( pPP );
}

HRESULT IDirect3DDevice8::Present( CONST RECT *pSrc, CONST RECT *pDst, HWND hWnd, CONST RGNDATA *pDirty )
{
	return m_pDev->Present( pSrc, pDst, hWnd, pDirty );
}

HRESULT IDirect3DDevice8::GetBackBuffer( UINT n, D3DBACKBUFFER_TYPE t, IDirect3DSurface8 **pp )
{
	return m_pDev->GetBackBuffer( 0, n, t, pp );
}

HRESULT IDirect3DDevice8::GetFrontBuffer( IDirect3DSurface8 *pDest )
{
	return m_pDev->GetFrontBufferData( 0, pDest );
}

void IDirect3DDevice8::SetGammaRamp( DWORD nFlags, CONST D3DGAMMARAMP *pRamp )	{ m_pDev->SetGammaRamp( 0, nFlags, pRamp ); }
void IDirect3DDevice8::GetGammaRamp( D3DGAMMARAMP *pRamp )						{ m_pDev->GetGammaRamp( 0, pRamp ); }
BOOL IDirect3DDevice8::ShowCursor( BOOL bShow )									{ return m_pDev->ShowCursor( bShow ); }
HRESULT IDirect3DDevice8::SetCursorProperties( UINT x, UINT y, IDirect3DSurface8 *pBmp )	{ return m_pDev->SetCursorProperties( x, y, pBmp ); }
void IDirect3DDevice8::SetCursorPosition( int x, int y, DWORD nFlags )			{ m_pDev->SetCursorPosition( x, y, nFlags ); }

// --- resource creation ------------------------------------------------------

HRESULT IDirect3DDevice8::CreateTexture( UINT w, UINT h, UINT nLevels, DWORD nUsage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DTexture8 **pp )
{
	return m_pDev->CreateTexture( w, h, nLevels, nUsage, fmt, pool, pp, NULL );
}

HRESULT IDirect3DDevice8::CreateCubeTexture( UINT nEdge, UINT nLevels, DWORD nUsage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DCubeTexture8 **pp )
{
	return m_pDev->CreateCubeTexture( nEdge, nLevels, nUsage, fmt, pool, pp, NULL );
}

HRESULT IDirect3DDevice8::CreateVertexBuffer( UINT nLen, DWORD nUsage, DWORD nFVF, D3DPOOL pool, IDirect3DVertexBuffer8 **pp )
{
	return m_pDev->CreateVertexBuffer( nLen, nUsage, nFVF, pool, pp, NULL );
}

HRESULT IDirect3DDevice8::CreateIndexBuffer( UINT nLen, DWORD nUsage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBuffer8 **pp )
{
	return m_pDev->CreateIndexBuffer( nLen, nUsage, fmt, pool, pp, NULL );
}

HRESULT IDirect3DDevice8::CreateDepthStencilSurface( UINT w, UINT h, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE ms, IDirect3DSurface8 **pp )
{
	return m_pDev->CreateDepthStencilSurface( w, h, fmt, ms, 0, TRUE, pp, NULL );
}

HRESULT IDirect3DDevice8::CreateImageSurface( UINT w, UINT h, D3DFORMAT fmt, IDirect3DSurface8 **pp )
{
	// D3D8 "image surfaces" are system-memory surfaces.
	return m_pDev->CreateOffscreenPlainSurface( w, h, fmt, D3DPOOL_SYSTEMMEM, pp, NULL );
}

HRESULT IDirect3DDevice8::CopyRects( IDirect3DSurface8 *pSrc, CONST RECT *pSrcRects, UINT nRects, IDirect3DSurface8 *pDst, CONST POINT *pDstPts )
{
	D3DSURFACE_DESC DstDesc;
	pDst->GetDesc( &DstDesc );

	if( DstDesc.Pool == D3DPOOL_SYSTEMMEM )
	{
		// Read back a render target (screenshots, readbacks).
		return m_pDev->GetRenderTargetData( pSrc, pDst );
	}

	// Copy between video-memory surfaces. Only the whole-surface / rect-list forms Fang uses.
	if( nRects == 0 || pSrcRects == NULL )
	{
		return m_pDev->StretchRect( pSrc, NULL, pDst, NULL, D3DTEXF_NONE );
	}

	HRESULT hr = D3D_OK;
	for( UINT i=0; i<nRects && SUCCEEDED( hr ); i++ )
	{
		RECT DstRect = pSrcRects[i];
		if( pDstPts )
		{
			LONG w = DstRect.right - DstRect.left, h = DstRect.bottom - DstRect.top;
			DstRect.left = pDstPts[i].x;  DstRect.top = pDstPts[i].y;
			DstRect.right = DstRect.left + w;  DstRect.bottom = DstRect.top + h;
		}
		hr = m_pDev->StretchRect( pSrc, &pSrcRects[i], pDst, &DstRect, D3DTEXF_NONE );
	}
	return hr;
}

// --- render targets ---------------------------------------------------------

HRESULT IDirect3DDevice8::SetRenderTarget( IDirect3DSurface8 *pRT, IDirect3DSurface8 *pZ )
{
	HRESULT hr = D3D_OK;
	if( pRT ) hr = m_pDev->SetRenderTarget( 0, pRT );
	if( SUCCEEDED( hr ) && pZ ) hr = m_pDev->SetDepthStencilSurface( pZ );
	return hr;
}

HRESULT IDirect3DDevice8::GetDepthStencilSurface( IDirect3DSurface8 **pp )
{
	return m_pDev->GetDepthStencilSurface( pp );
}

HRESULT IDirect3DDevice8::Clear( DWORD nCount, CONST D3DRECT *pRects, DWORD nFlags, D3DCOLOR nColor, float fZ, DWORD nStencil )
{
	return m_pDev->Clear( nCount, pRects, nFlags, nColor, fZ, nStencil );
}

// --- render / texture-stage state -------------------------------------------

HRESULT IDirect3DDevice8::SetRenderState( D3DRENDERSTATETYPE nState, DWORD nValue )
{
	switch( (DWORD)nState )
	{
	case D3DRS_SOFTWAREVERTEXPROCESSING:
		return m_pDev->SetSoftwareVertexProcessing( nValue );

	case D3DRS_ZBIAS:
		{
			// D3D8: integer 0..16, larger = closer. D3D9: float depth offset, negative = closer.
			float fBias = -(float)nValue * 1.5e-5f;
			return m_pDev->SetRenderState( D3DRS_DEPTHBIAS, *(DWORD *)&fBias );
		}

	// D3D8-only states with no D3D9 equivalent and no visible effect here.
	case D3DRS_PATCHSEGMENTS:
	case D3DRS_LINEPATTERN:
	case D3DRS_ZVISIBLE:
	case D3DRS_EDGEANTIALIAS:
		return D3D_OK;
	}
	return m_pDev->SetRenderState( nState, nValue );
}

HRESULT IDirect3DDevice8::SetTextureStageState( DWORD nStage, DWORD nType, DWORD nValue )
{
	switch( nType )
	{
	case D3DTSS_ADDRESSU:		return m_pDev->SetSamplerState( nStage, D3DSAMP_ADDRESSU, nValue );
	case D3DTSS_ADDRESSV:		return m_pDev->SetSamplerState( nStage, D3DSAMP_ADDRESSV, nValue );
	case D3DTSS_ADDRESSW:		return m_pDev->SetSamplerState( nStage, D3DSAMP_ADDRESSW, nValue );
	case D3DTSS_BORDERCOLOR:	return m_pDev->SetSamplerState( nStage, D3DSAMP_BORDERCOLOR, nValue );
	case D3DTSS_MAGFILTER:		return m_pDev->SetSamplerState( nStage, D3DSAMP_MAGFILTER, nValue );
	case D3DTSS_MINFILTER:		return m_pDev->SetSamplerState( nStage, D3DSAMP_MINFILTER, nValue );
	case D3DTSS_MIPFILTER:		return m_pDev->SetSamplerState( nStage, D3DSAMP_MIPFILTER, nValue );
	case D3DTSS_MIPMAPLODBIAS:	return m_pDev->SetSamplerState( nStage, D3DSAMP_MIPMAPLODBIAS, nValue );
	case D3DTSS_MAXMIPLEVEL:	return m_pDev->SetSamplerState( nStage, D3DSAMP_MAXMIPLEVEL, nValue );
	case D3DTSS_MAXANISOTROPY:	return m_pDev->SetSamplerState( nStage, D3DSAMP_MAXANISOTROPY, nValue );
	}
	return m_pDev->SetTextureStageState( nStage, (D3DTEXTURESTAGESTATETYPE)nType, nValue );
}

// --- shaders ----------------------------------------------------------------

HRESULT IDirect3DDevice8::CreateVertexShader( CONST DWORD *pDecl, CONST DWORD *pFunc, DWORD *pHandle, DWORD /*nUsage*/ )
{
	D3DVERTEXELEMENT9 aElem[MAXD3DDECLLENGTH + 1];
	BYTE aInputReg[MAXD3DDECLLENGTH];
	UINT nElem = 0;
	IDirect3DVertexDeclaration9 *pVDecl = NULL;
	IDirect3DVertexShader9 *pVS = NULL;

	HRESULT hr = _TranslateD3D8Decl( pDecl, aElem, aInputReg, &nElem );
	if( FAILED( hr ) )
	{
		_CompatLog( "d3d8compat: vertex declaration translation/creation failed (hr=0x%08x)\n", (unsigned)hr );
		return hr;
	}
	D3DVERTEXELEMENT9 End = D3DDECL_END();
	aElem[nElem] = End;
	hr = m_pDev->CreateVertexDeclaration( aElem, &pVDecl );
	if( FAILED( hr ) )
	{
		_CompatLog( "d3d8compat: vertex declaration creation failed (hr=0x%08x)\n", (unsigned)hr );
		return hr;
	}

	if( pFunc )
	{
		// D3D8 attached input semantics to the shader at CreateVertexShader time.
		// D3D9 needs equivalent dcl_usage tokens inside the vs_1_x function stream.
		const DWORD *pEnd = pFunc + 1;
		UINT nCodeWords = 1;
		while( nCodeWords < 65536 && *pEnd != 0x0000ffffu )
		{
			pEnd++;
			nCodeWords++;
		}
		if( nCodeWords >= 65536 )
		{
			pVDecl->Release();
			return D3DERR_INVALIDCALL;
		}
		++nCodeWords; // include D3DSIO_END

		DWORD *pD3D9Func = (DWORD *)malloc( (nCodeWords + nElem * 3) * sizeof(DWORD) );
		if( !pD3D9Func )
		{
			pVDecl->Release();
			return E_OUTOFMEMORY;
		}
		UINT nOut = 0;
		pD3D9Func[nOut++] = pFunc[0];
		for( UINT i=0; i<nElem; i++ )
		{
			pD3D9Func[nOut++] = 31u; // D3DSIO_DCL
			pD3D9Func[nOut++] = 0x80000000u | ((DWORD)aElem[i].Usage << 16) | (DWORD)aElem[i].UsageIndex;
			pD3D9Func[nOut++] = 0x900f0000u | (DWORD)aInputReg[i]; // D3DSPR_INPUT, xyzw
		}
		memcpy( pD3D9Func + nOut, pFunc + 1, (nCodeWords - 1) * sizeof(DWORD) );
		hr = m_pDev->CreateVertexShader( pD3D9Func, &pVS );
		if( FAILED( hr ) )
		{
			_CompatLog( "d3d8compat: CreateVertexShader failed (hr=0x%08x, version token 0x%08x)\n", (unsigned)hr, (unsigned)pFunc[0] );
			free( pD3D9Func );
			pVDecl->Release();
			return hr;
		}
		free( pD3D9Func );
	}

	if( m_nVShaderCount == m_nVShaderCap )
	{
		UINT nNewCap = m_nVShaderCap ? m_nVShaderCap * 2 : 64;
		_VShader *pNew = (_VShader *)realloc( m_paVShaders, nNewCap * sizeof(_VShader) );
		if( !pNew )
		{
			if( pVS ) pVS->Release();
			pVDecl->Release();
			return E_OUTOFMEMORY;
		}
		m_paVShaders = pNew;
		m_nVShaderCap = nNewCap;
	}

	m_paVShaders[m_nVShaderCount].pShader = pVS;
	m_paVShaders[m_nVShaderCount].pDecl = pVDecl;
	*pHandle = _VSHANDLE_FLAG | m_nVShaderCount;
	m_nVShaderCount++;
	return D3D_OK;
}

HRESULT IDirect3DDevice8::SetVertexShader( DWORD nHandle )
{
	if( nHandle & _VSHANDLE_FLAG )
	{
		UINT nIdx = nHandle & ~_VSHANDLE_FLAG;
		if( nIdx >= m_nVShaderCount || m_paVShaders[nIdx].pDecl == NULL ) return D3DERR_INVALIDCALL;

		HRESULT hr = m_pDev->SetVertexDeclaration( m_paVShaders[nIdx].pDecl );
		if( FAILED( hr ) ) return hr;
		return m_pDev->SetVertexShader( m_paVShaders[nIdx].pShader );
	}

	// Not a shader handle: it's an FVF code (fixed-function pipeline).
	m_pDev->SetVertexShader( NULL );
	if( nHandle == 0 ) return D3D_OK;
	return m_pDev->SetFVF( nHandle );
}

HRESULT IDirect3DDevice8::DeleteVertexShader( DWORD nHandle )
{
	if( !(nHandle & _VSHANDLE_FLAG) ) return D3DERR_INVALIDCALL;
	UINT nIdx = nHandle & ~_VSHANDLE_FLAG;
	if( nIdx >= m_nVShaderCount ) return D3DERR_INVALIDCALL;

	if( m_paVShaders[nIdx].pShader ) { m_paVShaders[nIdx].pShader->Release(); m_paVShaders[nIdx].pShader = NULL; }
	if( m_paVShaders[nIdx].pDecl )   { m_paVShaders[nIdx].pDecl->Release();   m_paVShaders[nIdx].pDecl = NULL; }
	return D3D_OK;
}

HRESULT IDirect3DDevice8::SetVertexShaderConstant( DWORD nReg, CONST void *pData, DWORD nCount )
{
	return m_pDev->SetVertexShaderConstantF( nReg, (const float *)pData, nCount );
}

HRESULT IDirect3DDevice8::CreatePixelShader( CONST DWORD *pFunc, DWORD *pHandle )
{
	if( !pFunc || !pHandle ) return D3DERR_INVALIDCALL;
	IDirect3DPixelShader9 *pPS = NULL;
	HRESULT hr = m_pDev->CreatePixelShader( pFunc, &pPS );
	if( FAILED( hr ) )
	{
		_CompatLog( "d3d8compat: CreatePixelShader failed (hr=0x%08x, version token 0x%08x)\n", (unsigned)hr, pFunc ? (unsigned)pFunc[0] : 0u );
		return hr;
	}

	if( m_nPShaderCount == m_nPShaderCap )
	{
		UINT nNewCap = m_nPShaderCap ? m_nPShaderCap * 2 : 64;
		_PShader *pNew = (_PShader *)realloc( m_paPShaders, nNewCap * sizeof(_PShader) );
		if( !pNew )
		{
			pPS->Release();
			return E_OUTOFMEMORY;
		}
		m_paPShaders = pNew;
		m_nPShaderCap = nNewCap;
	}

	m_paPShaders[m_nPShaderCount].pShader = pPS;
	*pHandle = ++m_nPShaderCount;		// handle 0 means "no pixel shader"
	return D3D_OK;
}

HRESULT IDirect3DDevice8::SetPixelShader( DWORD nHandle )
{
	if( nHandle == 0 ) return m_pDev->SetPixelShader( NULL );
	if( nHandle > m_nPShaderCount ) return D3DERR_INVALIDCALL;
	return m_pDev->SetPixelShader( m_paPShaders[nHandle - 1].pShader );
}

HRESULT IDirect3DDevice8::DeletePixelShader( DWORD nHandle )
{
	if( nHandle == 0 || nHandle > m_nPShaderCount ) return D3DERR_INVALIDCALL;
	_PShader &ps = m_paPShaders[nHandle - 1];
	if( ps.pShader ) { ps.pShader->Release(); ps.pShader = NULL; }
	return D3D_OK;
}

HRESULT IDirect3DDevice8::SetPixelShaderConstant( DWORD nReg, CONST void *pData, DWORD nCount )
{
	return m_pDev->SetPixelShaderConstantF( nReg, (const float *)pData, nCount );
}

// --- geometry ---------------------------------------------------------------

HRESULT IDirect3DDevice8::SetStreamSource( UINT nStream, IDirect3DVertexBuffer8 *pVB, UINT nStride )
{
	return m_pDev->SetStreamSource( nStream, pVB, 0, nStride );
}

HRESULT IDirect3DDevice8::SetIndices( IDirect3DIndexBuffer8 *pIB, UINT nBaseVertexIndex )
{
	m_nBaseVertexIndex = nBaseVertexIndex;
	return m_pDev->SetIndices( pIB );
}

HRESULT IDirect3DDevice8::DrawPrimitive( D3DPRIMITIVETYPE t, UINT nStart, UINT nPrims )
{
	return m_pDev->DrawPrimitive( t, nStart, nPrims );
}

HRESULT IDirect3DDevice8::DrawIndexedPrimitive( D3DPRIMITIVETYPE t, UINT nMinIndex, UINT nNumVerts, UINT nStartIndex, UINT nPrims )
{
	return m_pDev->DrawIndexedPrimitive( t, (INT)m_nBaseVertexIndex, nMinIndex, nNumVerts, nStartIndex, nPrims );
}

HRESULT IDirect3DDevice8::DrawPrimitiveUP( D3DPRIMITIVETYPE t, UINT nPrims, CONST void *pVerts, UINT nStride )
{
	return m_pDev->DrawPrimitiveUP( t, nPrims, pVerts, nStride );
}

HRESULT IDirect3DDevice8::DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE t, UINT nMinVtx, UINT nNumVtx, UINT nPrims, CONST void *pIdx, D3DFORMAT idxFmt, CONST void *pVerts, UINT nStride )
{
	return m_pDev->DrawIndexedPrimitiveUP( t, nMinVtx, nNumVtx, nPrims, pIdx, idxFmt, pVerts, nStride );
}

// ===========================================================================
// IDirect3D8
// ===========================================================================

IDirect3D8::IDirect3D8( IDirect3D9 *pD3D9 ) : m_pD3D( pD3D9 ), m_nRefs( 1 )
{
}

IDirect3D8::~IDirect3D8()
{
	if( m_pD3D ) m_pD3D->Release();
}

ULONG IDirect3D8::AddRef()
{
	return ++m_nRefs;
}

ULONG IDirect3D8::Release()
{
	ULONG n = --m_nRefs;
	if( n == 0 ) delete this;
	return n;
}

UINT IDirect3D8::GetAdapterCount()															{ return m_pD3D->GetAdapterCount(); }
HRESULT IDirect3D8::GetAdapterIdentifier( UINT a, DWORD f, D3DADAPTER_IDENTIFIER8 *p )		{ return m_pD3D->GetAdapterIdentifier( a, f, p ); }
HRESULT IDirect3D8::GetAdapterDisplayMode( UINT a, D3DDISPLAYMODE *p )						{ return m_pD3D->GetAdapterDisplayMode( a, p ); }
HRESULT IDirect3D8::CheckDeviceType( UINT a, D3DDEVTYPE t, D3DFORMAT d, D3DFORMAT b, BOOL w ) { return m_pD3D->CheckDeviceType( a, t, d, b, w ); }
HRESULT IDirect3D8::CheckDeviceFormat( UINT a, D3DDEVTYPE t, D3DFORMAT af, DWORD u, D3DRESOURCETYPE r, D3DFORMAT cf ) { return m_pD3D->CheckDeviceFormat( a, t, af, u, r, cf ); }
HRESULT IDirect3D8::CheckDepthStencilMatch( UINT a, D3DDEVTYPE t, D3DFORMAT af, D3DFORMAT rt, D3DFORMAT ds ) { return m_pD3D->CheckDepthStencilMatch( a, t, af, rt, ds ); }
HRESULT IDirect3D8::GetDeviceCaps( UINT a, D3DDEVTYPE t, D3DCAPS8 *p )						{ return m_pD3D->GetDeviceCaps( a, t, p ); }
HMONITOR IDirect3D8::GetAdapterMonitor( UINT a )											{ return m_pD3D->GetAdapterMonitor( a ); }

HRESULT IDirect3D8::CheckDeviceMultiSampleType( UINT a, D3DDEVTYPE t, D3DFORMAT f, BOOL w, D3DMULTISAMPLE_TYPE ms )
{
	return m_pD3D->CheckDeviceMultiSampleType( a, t, f, w, ms, NULL );
}

// D3D8 enumerated every display mode regardless of format. D3D9 needs a format,
// so walk the formats it supports for the adapter and concatenate the lists.
static const D3DFORMAT _aDisplayFormats[] = { D3DFMT_X8R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A2R10G10B10 };
#define _DISPLAY_FORMAT_COUNT	(sizeof(_aDisplayFormats) / sizeof(_aDisplayFormats[0]))

UINT IDirect3D8::GetAdapterModeCount( UINT nAdapter )
{
	UINT nTotal = 0;
	for( UINT i=0; i<_DISPLAY_FORMAT_COUNT; i++ )
	{
		nTotal += m_pD3D->GetAdapterModeCount( nAdapter, _aDisplayFormats[i] );
	}
	return nTotal;
}

HRESULT IDirect3D8::EnumAdapterModes( UINT nAdapter, UINT nMode, D3DDISPLAYMODE *pMode )
{
	for( UINT i=0; i<_DISPLAY_FORMAT_COUNT; i++ )
	{
		UINT nCount = m_pD3D->GetAdapterModeCount( nAdapter, _aDisplayFormats[i] );
		if( nMode < nCount )
		{
			return m_pD3D->EnumAdapterModes( nAdapter, _aDisplayFormats[i], nMode, pMode );
		}
		nMode -= nCount;
	}
	return D3DERR_INVALIDCALL;
}

HRESULT IDirect3D8::CreateDevice( UINT nAdapter, D3DDEVTYPE nType, HWND hFocus, DWORD nBehavior, D3DPRESENT_PARAMETERS *pPP, IDirect3DDevice8 **ppDev )
{
	IDirect3DDevice9 *pDev9 = NULL;
	HRESULT hr = m_pD3D->CreateDevice( nAdapter, nType, hFocus, nBehavior, pPP, &pDev9 );
	if( FAILED( hr ) )
	{
		*ppDev = NULL;
		return hr;
	}
	*ppDev = new IDirect3DDevice8( pDev9 );
	return D3D_OK;
}

IDirect3D8 *Direct3DCreate8( UINT )
{
	IDirect3D9 *pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
	if( !pD3D9 ) return NULL;
	return new IDirect3D8( pD3D9 );
}

// ===========================================================================
// D3DX texture helpers
// ===========================================================================

HRESULT D3DXCreateCubeTexture( IDirect3DDevice8 *pDevice, UINT nSize, UINT nLevels, DWORD nUsage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DCubeTexture8 **ppCube )
{
	return pDevice->CreateCubeTexture( nSize, nLevels, nUsage, fmt, pool, ppCube );
}

// Bytes per pixel, or bytes per 4x4 block for DXT formats (bBlock set).
static UINT _FormatBytes( D3DFORMAT fmt, BOOL *pbBlock )
{
	*pbBlock = FALSE;
	switch( fmt )
	{
	case D3DFMT_DXT1:											*pbBlock = TRUE; return 8;
	case D3DFMT_DXT2: case D3DFMT_DXT3:
	case D3DFMT_DXT4: case D3DFMT_DXT5:							*pbBlock = TRUE; return 16;
	case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8:					return 4;
	case D3DFMT_R8G8B8:											return 3;
	case D3DFMT_R5G6B5: case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5: case D3DFMT_A4R4G4B4:
	case D3DFMT_V8U8: case D3DFMT_L16: case D3DFMT_A8L8:
	case D3DFMT_A8R3G3B2:										return 2;
	case D3DFMT_L8: case D3DFMT_A8: case D3DFMT_P8:
	case D3DFMT_R3G3B2: case D3DFMT_A4L4:						return 1;
	}
	return 0;
}

UINT D3D8Compat_SurfaceSize( const D3DSURFACE_DESC *pDesc )
{
	BOOL bBlock;
	UINT nBytes = _FormatBytes( pDesc->Format, &bBlock );
	if( bBlock )
	{
		return ((pDesc->Width + 3) / 4) * ((pDesc->Height + 3) / 4) * nBytes;
	}
	return pDesc->Width * pDesc->Height * nBytes;
}

// Copies the source image into the surface. Only same-format loads are supported
// (plus 32-bit A8R8G8B8/X8R8G8B8 interchange); that covers how Fang uses D3DX.
HRESULT D3DXLoadSurfaceFromMemory( IDirect3DSurface8 *pDest, CONST PALETTEENTRY *, CONST RECT *pDestRect,
								   LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, CONST PALETTEENTRY *,
								   CONST RECT *pSrcRect, DWORD, D3DCOLOR )
{
	if( !pDest || !pSrcMemory || !pSrcRect ) return D3DERR_INVALIDCALL;

	D3DSURFACE_DESC Desc;
	pDest->GetDesc( &Desc );

	BOOL bSrcBlock, bDstBlock;
	UINT nSrcBytes = _FormatBytes( SrcFormat, &bSrcBlock );
	UINT nDstBytes = _FormatBytes( Desc.Format, &bDstBlock );
	if( !nSrcBytes || !nDstBytes || bSrcBlock != bDstBlock ) return D3DERR_NOTAVAILABLE;

	const BOOL bSameFormat = (SrcFormat == Desc.Format) || (nSrcBytes == 4 && nDstBytes == 4);
	if( !bSameFormat ) return D3DERR_NOTAVAILABLE;

	UINT nWidth  = pSrcRect->right - pSrcRect->left;
	UINT nHeight = pSrcRect->bottom - pSrcRect->top;
	UINT nDstW = pDestRect ? (pDestRect->right - pDestRect->left) : Desc.Width;
	UINT nDstH = pDestRect ? (pDestRect->bottom - pDestRect->top) : Desc.Height;
	if( nWidth > nDstW ) nWidth = nDstW;
	if( nHeight > nDstH ) nHeight = nDstH;

	UINT nRows, nRowBytes, nSrcColBytes;
	if( bSrcBlock )
	{
		nRows = (nHeight + 3) / 4;
		nRowBytes = ((nWidth + 3) / 4) * nSrcBytes;
		nSrcColBytes = 0;
	}
	else
	{
		nRows = nHeight;
		nRowBytes = nWidth * nSrcBytes;
		nSrcColBytes = nSrcBytes;
	}

	D3DLOCKED_RECT Locked;
	if( FAILED( pDest->LockRect( &Locked, pDestRect, 0 ) ) ) return D3DERR_INVALIDCALL;

	const BYTE *pSrc = (const BYTE *)pSrcMemory;
	if( bSrcBlock )
	{
		pSrc += (pSrcRect->top / 4) * SrcPitch + (pSrcRect->left / 4) * nSrcBytes;
	}
	else
	{
		pSrc += pSrcRect->top * SrcPitch + pSrcRect->left * nSrcColBytes;
	}

	BYTE *pDst = (BYTE *)Locked.pBits;
	for( UINT y=0; y<nRows; y++ )
	{
		memcpy( pDst, pSrc, nRowBytes );
		pSrc += SrcPitch;
		pDst += Locked.Pitch;
	}

	pDest->UnlockRect();
	return D3D_OK;
}
