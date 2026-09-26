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

#include "MgcVector3.h"

const MgcVector3 MgcVector3::ZERO(0,0,0);
const MgcVector3 MgcVector3::UNIT_X(1,0,0);
const MgcVector3 MgcVector3::UNIT_Y(0,1,0);
const MgcVector3 MgcVector3::UNIT_Z(0,0,1);

//----------------------------------------------------------------------------
MgcVector3::MgcVector3 ()
{
    // For efficiency in construction of large arrays of vectors, the
    // default constructor does not initialize the vector.
}
//----------------------------------------------------------------------------
MgcVector3::MgcVector3 (MgcReal fX, MgcReal fY, MgcReal fZ)
{
    x = fX;
    y = fY;
    z = fZ;
}
//----------------------------------------------------------------------------
MgcVector3::MgcVector3 (MgcReal afCoordinate[3])
{
    x = afCoordinate[0];
    y = afCoordinate[1];
    z = afCoordinate[2];
}
//----------------------------------------------------------------------------
MgcVector3::MgcVector3 (const MgcVector3& rkVector)
{
    x = rkVector.x;
    y = rkVector.y;
    z = rkVector.z;
}
//----------------------------------------------------------------------------
MgcReal& MgcVector3::operator[] (int i) const
{
    // assert:  0 <= i < 2; x, y, and z are packed into 3*sizeof(MgcReal)
    //          bytes
    return (MgcReal&) *(&x + i);
}
//----------------------------------------------------------------------------
MgcVector3::operator MgcReal* ()
{
    return &x;
}
//----------------------------------------------------------------------------
MgcVector3& MgcVector3::operator= (const MgcVector3& rkVector)
{
    x = rkVector.x;
    y = rkVector.y;
    z = rkVector.z;
    return *this;
}
//----------------------------------------------------------------------------
bool MgcVector3::operator== (const MgcVector3& rkVector) const
{
    return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
}
//----------------------------------------------------------------------------
bool MgcVector3::operator!= (const MgcVector3& rkVector) const
{
    return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::operator+ (const MgcVector3& rkVector) const
{
    MgcVector3 kSum;
    kSum.x = x + rkVector.x;
    kSum.y = y + rkVector.y;
    kSum.z = z + rkVector.z;
    return kSum;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::operator- (const MgcVector3& rkVector) const
{
    MgcVector3 kDiff;
    kDiff.x = x - rkVector.x;
    kDiff.y = y - rkVector.y;
    kDiff.z = z - rkVector.z;
    return kDiff;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::operator* (MgcReal fScalar) const
{
    MgcVector3 kProd;
    kProd.x = fScalar*x;
    kProd.y = fScalar*y;
    kProd.z = fScalar*z;
    return kProd;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::operator- () const
{
    MgcVector3 kNeg;
    kNeg.x = -x;
    kNeg.y = -y;
    kNeg.z = -z;
    return kNeg;
}
//----------------------------------------------------------------------------
MgcVector3 operator* (MgcReal fScalar, const MgcVector3& rkVector)
{
    MgcVector3 kProd;
    kProd.x = fScalar*rkVector.x;
    kProd.y = fScalar*rkVector.y;
    kProd.z = fScalar*rkVector.z;
    return kProd;
}
//----------------------------------------------------------------------------
MgcVector3& MgcVector3::operator+= (const MgcVector3& rkVector)
{
    x += rkVector.x;
    y += rkVector.y;
    z += rkVector.z;
    return *this;
}
//----------------------------------------------------------------------------
MgcVector3& MgcVector3::operator-= (const MgcVector3& rkVector)
{
    x -= rkVector.x;
    y -= rkVector.y;
    z -= rkVector.z;
    return *this;
}
//----------------------------------------------------------------------------
MgcVector3& MgcVector3::operator*= (MgcReal fScalar)
{
    x *= fScalar;
    y *= fScalar;
    z *= fScalar;
    return *this;
}
//----------------------------------------------------------------------------
MgcReal MgcVector3::SquaredLength () const
{
    return x*x + y*y + z*z;
}
//----------------------------------------------------------------------------
MgcReal MgcVector3::Dot (const MgcVector3& rkVector) const
{
    return x*rkVector.x + y*rkVector.y + z*rkVector.z;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::operator/ (MgcReal fScalar) const
{
    MgcVector3 kQuot;

    if ( fScalar != 0.0 )
    {
        MgcReal fInvScalar = 1.0/fScalar;
        kQuot.x = fInvScalar*x;
        kQuot.y = fInvScalar*y;
        kQuot.z = fInvScalar*z;
        return kQuot;
    }
    else
    {
        return MgcVector3(MgcMath::INFINITY,MgcMath::INFINITY,
            MgcMath::INFINITY);
    }
}
//----------------------------------------------------------------------------
MgcVector3& MgcVector3::operator/= (MgcReal fScalar)
{
    if ( fScalar != 0.0 )
    {
        MgcReal fInvScalar = 1.0/fScalar;
        x *= fInvScalar;
        y *= fInvScalar;
        z *= fInvScalar;
    }
    else
    {
        x = MgcMath::INFINITY;
        y = MgcMath::INFINITY;
        z = MgcMath::INFINITY;
    }

    return *this;
}
//----------------------------------------------------------------------------
MgcReal MgcVector3::Length () const
{
    return MgcMath::Sqrt(x*x + y*y + z*z);
}
//----------------------------------------------------------------------------
MgcReal MgcVector3::Unitize (MgcReal fTolerance)
{
    MgcReal fLength = Length();

    if ( fLength > fTolerance )
    {
        MgcReal fInvLength = 1.0/fLength;
        x *= fInvLength;
        y *= fInvLength;
        z *= fInvLength;
    }
    else
    {
        fLength = 0.0;
    }

    return fLength;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::Cross (const MgcVector3& rkVector) const
{
    MgcVector3 kCross;

    kCross.x = y*rkVector.z - z*rkVector.y;
    kCross.y = z*rkVector.x - x*rkVector.z;
    kCross.z = x*rkVector.y - y*rkVector.x;

    return kCross;
}
//----------------------------------------------------------------------------
MgcVector3 MgcVector3::UnitCross (const MgcVector3& rkVector) const
{
    MgcVector3 kCross;

    kCross.x = y*rkVector.z - z*rkVector.y;
    kCross.y = z*rkVector.x - x*rkVector.z;
    kCross.z = x*rkVector.y - y*rkVector.x;
    kCross.Unitize();

    return kCross;
}
//----------------------------------------------------------------------------
void MgcVector3::Orthonormalize (MgcVector3 akVector[3])
{
    // If the input vectors are v0, v1, and v2, then the Gram-Schmidt
    // orthonormalization produces vectors u0, u1, and u2 as follows,
    //
    //   u0 = v0/|v0|
    //   u1 = (v1-(u0*v1)u0)/|v1-(u0*v1)u0|
    //   u2 = (v2-(u0*v2)u0-(u1*v2)u1)/|v2-(u0*v2)u0-(u1*v2)u1|
    //
    // where |A| indicates length of vector A and A*B indicates dot
    // product of vectors A and B.

    // compute u0
    akVector[0].Unitize();

    // compute u1
    MgcReal fDot0 = akVector[0].Dot(akVector[1]); 
    akVector[1] -= fDot0*akVector[0];
    akVector[1].Unitize();

    // compute u2
    MgcReal fDot1 = akVector[1].Dot(akVector[2]);
    fDot0 = akVector[0].Dot(akVector[2]);
    akVector[2] -= fDot0*akVector[0] + fDot1*akVector[1];
    akVector[2].Unitize();
}
//----------------------------------------------------------------------------
void MgcVector3::GenerateOrthonormalBasis (MgcVector3& rkU, MgcVector3& rkV,
    MgcVector3& rkW, bool bUnitLengthW)
{
    if ( !bUnitLengthW )
        rkW.Unitize();

    if ( MgcMath::Abs(rkW.x) >= MgcMath::Abs(rkW.y)
    &&   MgcMath::Abs(rkW.x) >= MgcMath::Abs(rkW.z) )
    {
        rkU.x = -rkW.y;
        rkU.y = +rkW.x;
        rkU.z = 0.0;
    }
    else
    {
        rkU.x = 0.0;
        rkU.y = +rkW.z;
        rkU.z = -rkW.y;
    }

    rkU.Unitize();
    rkV = rkW.Cross(rkU);
}
//----------------------------------------------------------------------------
