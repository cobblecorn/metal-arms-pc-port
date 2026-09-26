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

#ifndef MGCRAY3_H
#define MGCRAY3_H

#include "MgcVector3.h"


class MgcRay3
{
public:
    // ray starts at origin P and has direction D
    MgcRay3 ();

    MgcVector3& Origin ();
    const MgcVector3& Origin () const;

    MgcVector3& Direction ();
    const MgcVector3& Direction () const;

protected:
    MgcVector3 m_kOrigin;  // P
    MgcVector3 m_kDirection;  // D
};

#include "MgcRay3.inl"

#endif
