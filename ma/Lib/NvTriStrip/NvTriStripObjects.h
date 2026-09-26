
#ifndef NV_TRISTRIP_OBJECTS_H
#define NV_TRISTRIP_OBJECTS_H

#include <assert.h>
#include <windows.h>
#include <vector>
#include <list>
#include "VertexCache.h"

#define NV_MAX_FACEINFO		24288
#define NV_MAX_STRIPINFO	16384

/////////////////////////////////////////////////////////////////////////////////
//
// Types defined for stripification
//
/////////////////////////////////////////////////////////////////////////////////

struct MyVertex {
	float x, y, z;
	float nx, ny, nz;
};

typedef MyVertex MyVector;

struct MyFace {
	int v1, v2, v3;
	float nx, ny, nz;
};


class NvFaceInfo {
public:
	
	// vertex indices
	void Init( int v0, int v1, int v2, bool bDegenerate )
	{
		m_bDegenerate = bDegenerate;
		m_v0 = v0; m_v1 = v1; m_v2 = v2;
		m_stripId      = -1;
		m_testStripId  = -1;
		m_experimentId = -1;
	}
	
	// data members are left public
	bool  m_bDegenerate;
	int   m_v0, m_v1, m_v2;
	int   m_stripId;      // real strip Id
	int   m_testStripId;  // strip Id in an experiment
	int   m_experimentId; // in what experiment was it given an experiment Id?
};

// nice and dumb edge class that points knows its
// indices, the two faces, and the next edge using
// the lesser of the indices
class NvEdgeInfo {
public:
	
	// constructor puts 1 ref on us
	NvEdgeInfo (int v0, int v1){
		m_v0       = v0;
		m_v1       = v1;
		m_face0    = NULL;
		m_face1    = NULL;
		m_nextV0   = NULL;
		m_nextV1   = NULL;
		
		// we will appear in 2 lists.  this is a good
		// way to make sure we delete it the second time
		// we hit it in the edge infos
		m_refCount = 2; 
	}
	
	// ref and unref
	void Unref( void ) 
	{ 
		if (--m_refCount == 0) 
		{
			delete this; 
		}
	}
	
	// data members are left public
	UINT         m_refCount;
	NvFaceInfo  *m_face0, *m_face1;
	int          m_v0, m_v1;
	NvEdgeInfo  *m_nextV0, *m_nextV1;
};


// This class is a quick summary of parameters used
// to begin a triangle strip.  Some operations may
// want to create lists of such items, so they were
// pulled out into a class
class NvStripStartInfo {
public:
	NvStripStartInfo( void ){};
	NvStripStartInfo(NvFaceInfo *pFace, NvEdgeInfo *pEdge, bool toV1)
	{
		m_startFace    = pFace;
		m_startEdge    = pEdge;
		m_toV1         = toV1;
	}
	void Init(NvStripStartInfo &StripStart)
	{
		m_startFace    = StripStart.m_startFace;
		m_startEdge    = StripStart.m_startEdge;
		m_toV1         = StripStart.m_toV1;
	}
	NvFaceInfo    *m_startFace;
	NvEdgeInfo    *m_startEdge;
	bool           m_toV1;      
};


typedef std::vector<NvFaceInfo*>     NvFaceInfoVec;
typedef std::list  <NvFaceInfo*>     NvFaceInfoList;
typedef std::list  <NvFaceInfoVec*>  NvStripList;
typedef std::vector<NvEdgeInfo*>     NvEdgeInfoVec;

typedef std::vector<WORD> WordVec;
typedef std::vector<int> IntVec;
typedef std::vector<MyVertex> MyVertexVec;
typedef std::vector<MyFace> MyFaceVec;

template<class T> 
inline void SWAP(T& first, T& second) 
{
	T temp = first;
	first = second;
	second = temp;
}

// This is a summary of a strip that has been built
class NvStripInfo {
public:
	
	// A little information about the creation of the triangle strips
	NvStripInfo(){};
	void Init( NvFaceInfo *pFace, NvEdgeInfo *pEdge, bool toV1, int stripId, int experimentId = -1 )
	{
		m_startInfo.Init( NvStripStartInfo( pFace, pEdge, toV1 ) );
		m_stripId      = stripId;
		m_experimentId = experimentId;
		visited = false;
		m_numDegenerates = 0;
		m_faces.clear();
	}

	// This is an experiment if the experiment id is >= 0
	inline bool IsExperiment () const { return m_experimentId >= 0; }
	  
	inline bool IsInStrip (const NvFaceInfo *faceInfo) const 
	{
		if(faceInfo == NULL)
			return false;
		  
		return (m_experimentId >= 0 ? faceInfo->m_testStripId == m_stripId : faceInfo->m_stripId == m_stripId);
	}
	  
	bool SharesEdge(const NvFaceInfo* faceInfo, NvEdgeInfoVec &edgeInfos);
	  
	// take the given forward and backward strips and combine them together
	void Combine(const NvFaceInfoVec &forward, const NvFaceInfoVec &backward);
	  
	//returns true if the face is "unique", i.e. has a vertex which doesn't exist in the faceVec
	bool Unique(NvFaceInfoVec& faceVec, NvFaceInfo* face);
	  
	// mark the triangle as taken by this strip
	bool IsMarked    (NvFaceInfo *faceInfo);
	void MarkTriangle(NvFaceInfo *faceInfo);
	  
	// build the strip
	void Build(NvEdgeInfoVec &edgeInfos, NvFaceInfoVec &faceInfos);
	  
	// public data members
	NvStripStartInfo m_startInfo;
	NvFaceInfoVec    m_faces;
	int              m_stripId;
	int              m_experimentId;
	  
	bool visited;

	int m_numDegenerates;

	NvStripInfo *m_pNext;
};

typedef std::vector<NvStripInfo*>    NvStripInfoVec;


//The actual stripifier
class NvStripifier {
public:
	
	// Constructor
	NvStripifier();
	~NvStripifier();
	
	//the target vertex cache size, the structure to place the strips in, and the input indices
	void Stripify(const WordVec &in_indices, const int in_cacheSize, const int in_minStripLength, 
				  const unsigned short maxIndex, NvStripInfoVec &allStrips, NvFaceInfoVec &allFaces);
	void CreateStrips(const NvStripInfoVec& allStrips, IntVec& stripIndices, const bool bStitchStrips, unsigned int& numSeparateStrips);
	
	static int GetUniqueVertexInB(NvFaceInfo *faceA, NvFaceInfo *faceB);
	//static int GetSharedVertex(NvFaceInfo *faceA, NvFaceInfo *faceB);
	static void GetSharedVertices(NvFaceInfo *faceA, NvFaceInfo *faceB, int* vertex0, int* vertex1);

	static bool IsDegenerate(const NvFaceInfo* face);
	
protected:
	
	WordVec indices;
	int cacheSize;
	int minStripLength;
	float meshJump;
	bool bFirstTimeResetPoint;
	
	/////////////////////////////////////////////////////////////////////////////////
	//
	// Big mess of functions called during stripification
	//
	/////////////////////////////////////////////////////////////////////////////////

	//********************
	bool IsMoneyFace(const NvFaceInfo& face);
	bool FaceContainsIndex(const NvFaceInfo& face, const unsigned int index);

	bool IsCW(NvFaceInfo *faceInfo, int v0, int v1);
	bool NextIsCW(const int numIndices);
	
	bool IsDegenerate(const unsigned short v0, const unsigned short v1, const unsigned short v2);
	
	static int  GetNextIndex(const WordVec &indices, NvFaceInfo *face);
	static NvEdgeInfo *FindEdgeInfo(NvEdgeInfoVec &edgeInfos, int v0, int v1);
	static BOOL FacesHaveDifferentVerts( NvFaceInfo *faceInfo1, NvFaceInfo *faceInfo2 );
	static NvEdgeInfo *FindEdgeInfoWithoutBackface( NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo );
	static NvFaceInfo *FindOtherFace(NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo);
	NvFaceInfo *FindGoodResetPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);
	
	void FindAllStrips(NvStripInfoVec &allStrips, NvFaceInfoVec &allFaceInfos, NvEdgeInfoVec &allEdgeInfos, int numSamples);
	void SplitUpStripsAndOptimize(NvStripInfoVec &allStrips, NvStripInfoVec &outStrips, NvEdgeInfoVec& edgeInfos, NvFaceInfoVec& outFaceList);
	void RemoveSmallStrips(NvStripInfoVec& allStrips, NvStripInfoVec& allBigStrips, NvFaceInfoVec& faceList);
	
	bool FindTraversal(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, NvStripInfo *strip, NvStripStartInfo &startInfo);
	int  CountRemainingTris(std::list<NvStripInfo*>::iterator iter, std::list<NvStripInfo*>::iterator  end);
	
	void CommitStrips(NvStripInfoVec &allStrips, const NvStripInfoVec &strips);
	
	float AvgStripSize(const NvStripInfoVec &strips);
	int FindStartPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);
	
	void UpdateCacheStrip(VertexCache* vcache, NvStripInfo* strip);
	void UpdateCacheFace(VertexCache* vcache, NvFaceInfo* face);
	float CalcNumHitsStrip(VertexCache* vcache, NvStripInfo* strip);
	int CalcNumHitsFace(VertexCache* vcache, NvFaceInfo* face);
	int NumNeighbors(NvFaceInfo* face, NvEdgeInfoVec& edgeInfoVec);
	
	void BuildStripifyInfo(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, const unsigned short maxIndex);
	bool AlreadyExists(NvFaceInfo* faceInfo, NvFaceInfoVec& faceInfos);
	
	// let our strip info classes and the other classes get
	// to these protected stripificaton methods if they want
	friend NvStripInfo;
};


extern NvFaceInfo NV_FaceInfo[NV_MAX_FACEINFO];
extern int NV_nUsedFaceCount;
__inline NvFaceInfo* AllocateFaceInfo( int v0, int v1, int v2, bool bDegenerate )
{
	if ( NV_nUsedFaceCount == NV_MAX_FACEINFO )
	{
		OutputDebugString( "Exceeded NvFaceInfo buffer size.\n" );
		return NULL;
	}

	NvFaceInfo *pReturn = &NV_FaceInfo[NV_nUsedFaceCount];
	NV_nUsedFaceCount++;

	pReturn->Init( v0, v1, v2, bDegenerate );

	return pReturn;
}

extern NvStripInfo NV_StripInfo[NV_MAX_STRIPINFO];
extern NvStripInfo *NV_pFreeStripInfoList;
extern int NV_nUsedStripCount;
extern int NV_nHighUsedStripCount;
__inline NvStripInfo* AllocateStripInfo( NvFaceInfo *pFace, NvEdgeInfo *pEdge, bool toV1, int nStripID, int nExperimentID )
{
	if ( NV_pFreeStripInfoList == NULL )
	{
		OutputDebugString( "AllocateStripInfo() - Exceeded NvStripInfo buffer size!\n" );
		return NULL;
	}

	NvStripInfo *pReturn = NV_pFreeStripInfoList;
	NV_pFreeStripInfoList = pReturn->m_pNext;
	NV_nUsedStripCount++;

	pReturn->Init( pFace, pEdge, toV1, nStripID, nExperimentID );

	return pReturn;
}

__inline void FreeStripInfo( NvStripInfo *pStripInfo )
{
	if ( !pStripInfo )
	{
		return;
	}

	pStripInfo->m_pNext = NV_pFreeStripInfoList;
	NV_pFreeStripInfoList = pStripInfo;
	NV_nUsedStripCount--;
}


#endif
