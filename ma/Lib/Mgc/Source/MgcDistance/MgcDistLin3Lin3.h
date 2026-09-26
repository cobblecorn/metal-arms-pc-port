// Magic Software, Inc.
// http://www.magic-software.com
// Copyright (c) 2000, All Rights Reserved
//
// Source code from Magic Software is supplied under the terms of a license
// agreement and may not be copied or disclosed except in accordance with the
// terms of that agreement.  The various license agreements may be found at
// the Magic Software web site.  This file is subject to the license
//
// FREE SOURCE CODE
// http://www.magic-software.com/License/free.pdf

#ifndef MGCDISTLIN3LIN3_H
#define MGCDISTLIN3LIN3_H

#include "MgcLine3.h"
#include "MgcRay3.h"
#include "MgcSegment3.h"

// squared distance measurements

MgcReal MgcSqrDistance (const MgcLine3& rkLine0, const MgcLine3& rkLine1,
    MgcReal* pfLinP0 = 0, MgcReal* pfLinP1 = 0);

MgcReal MgcSqrDistance (const MgcLine3& rkLine, const MgcRay3& rkRay,
    MgcReal* pfLinP = 0, MgcReal* pfRayP = 0);

MgcReal MgcSqrDistance (const MgcLine3& rkLine, const MgcSegment3& rkSeg,
    MgcReal* pfLinP = 0, MgcReal* pfSegP = 0);

MgcReal MgcSqrDistance (const MgcRay3& rkRay0, const MgcRay3& rkRay1,
    MgcReal* pfRayP0 = 0, MgcReal* pfRayP1 = 0);

MgcReal MgcSqrDistance (const MgcRay3& rkRay, const MgcSegment3& rkSeg,
    MgcReal* pfRayP = 0, MgcReal* pfSegP = 0);

MgcReal MgcSqrDistance (const MgcSegment3& rkSeg0, const MgcSegment3& rkSeg1,
    MgcReal* pfSegP0 = 0, MgcReal* pfSegP1 = 0);


// distance measurements

MgcReal MgcDistance (const MgcLine3& rkLine0, const MgcLine3& rkLine1,
    MgcReal* pfLinP0 = 0, MgcReal* pfLinP1 = 0);

MgcReal MgcDistance (const MgcLine3& rkLine, const MgcRay3& rkRay,
    MgcReal* pfLinP = 0, MgcReal* pfRayP = 0);

MgcReal MgcDistance (const MgcLine3& rkLine, const MgcSegment3& rkSeg,
    MgcReal* pfLinP = 0, MgcReal* pfSegP = 0);

MgcReal MgcDistance (const MgcRay3& rkRay0, const MgcRay3& rkRay1,
    MgcReal* pfRayP0 = 0, MgcReal* pfRayP1 = 0);

MgcReal MgcDistance (const MgcRay3& rkRay, const MgcSegment3& rkSeg,
    MgcReal* pfRayP = 0, MgcReal* pfSegP = 0);

MgcReal MgcDistance (const MgcSegment3& rkSeg0, const MgcSegment3& rkSeg1,
    MgcReal* pfSegP0 = 0, MgcReal* pfSegP1 = 0);

#endif
