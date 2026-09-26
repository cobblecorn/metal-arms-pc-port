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

#ifndef MGCVECTOR3_H
#define MGCVECTOR3_H

#include "MgcMath.h"


class MgcVector3 
{
public:
    // construction
    MgcVector3 ();
    MgcVector3 (MgcReal fX, MgcReal fY, MgcReal fZ);
    MgcVector3 (MgcReal afCoordinate[3]);
    MgcVector3 (const MgcVector3& rkVector);

    // member access (allows V.x or V[0], V.y or V[1], V.z or V[2])
    MgcReal x, y, z;
    MgcReal& operator[] (int i) const;
    operator MgcReal* ();

    // assignment and comparison
    MgcVector3& operator= (const MgcVector3& rkVector);
    bool operator== (const MgcVector3& rkVector) const;
    bool operator!= (const MgcVector3& rkVector) const;

    // arithmetic operations
    MgcVector3 operator+ (const MgcVector3& rkVector) const;
    MgcVector3 operator- (const MgcVector3& rkVector) const;
    MgcVector3 operator* (MgcReal fScalar) const;
    MgcVector3 operator/ (MgcReal fScalar) const;
    MgcVector3 operator- () const;
    friend MgcVector3 operator* (MgcReal fScalar, const MgcVector3& rkVector);

    // arithmetic updates
    MgcVector3& operator+= (const MgcVector3& rkVector);
    MgcVector3& operator-= (const MgcVector3& rkVector);
    MgcVector3& operator*= (MgcReal fScalar);
    MgcVector3& operator/= (MgcReal fScalar);

    // vector operations
    MgcReal Length () const;
    MgcReal SquaredLength () const;
    MgcReal Dot (const MgcVector3& rkVector) const;
    MgcReal Unitize (MgcReal fTolerance = 1e-06);
    MgcVector3 Cross (const MgcVector3& rkVector) const;
    MgcVector3 UnitCross (const MgcVector3& rkVector) const;

    // Gram-Schmidt orthonormalization.
    static void Orthonormalize (MgcVector3 akVector[3]);

    // Input W must be initialize to a nonzero vector, output is {U,V,W}
    // an orthonormal basis.  A hint is provided about whether or not W
    // is already unit length.
    static void GenerateOrthonormalBasis (MgcVector3& rkU, MgcVector3& rkV,
        MgcVector3& rkW, bool bUnitLengthW = true);

    // special points
    static const MgcVector3 ZERO;
    static const MgcVector3 UNIT_X;
    static const MgcVector3 UNIT_Y;
    static const MgcVector3 UNIT_Z;
};

#endif
