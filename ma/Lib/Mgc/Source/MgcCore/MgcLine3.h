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

#ifndef MGCLINE3_H
#define MGCLINE3_H

#include "MgcVector3.h"


class MgcLine3
{
public:
    // Line is L(t) = b+t*m for any MgcReal-valued t
    // Ray has constraint t >= 0, b is the origin of the ray
    // Line segment has constraint 0 <= t <= 1, b and b+m are end points

    MgcLine3 ();

    MgcVector3& Origin ();
    const MgcVector3& Origin () const;

    MgcVector3& Direction ();
    const MgcVector3& Direction () const;

protected:
    MgcVector3 m_kOrigin;
    MgcVector3 m_kDirection;
};

#include "MgcLine3.inl"

#endif
