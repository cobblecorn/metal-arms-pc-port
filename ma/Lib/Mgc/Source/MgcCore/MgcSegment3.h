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

#ifndef MGCSEGMENT3_H
#define MGCSEGMENT3_H

#include "MgcVector3.h"


class MgcSegment3
{
public:
    // segment has endpoints P and P+D
    MgcSegment3 ();

    MgcVector3& Origin ();
    const MgcVector3& Origin () const;

    MgcVector3& Direction ();
    const MgcVector3& Direction () const;

protected:
    MgcVector3 m_kOrigin;  // P
    MgcVector3 m_kDirection;  // D
};

#include "MgcSegment3.inl"

#endif
