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

#ifndef MGCMATH_H
#define MGCMATH_H

// disable: "conversion from 'double' to 'float', possible loss of data
#pragma warning (disable : 4244)

// disable: "truncation from 'double' to 'float'
#pragma warning (disable : 4305)


#ifdef MGC_USE_DOUBLE
    typedef double MgcReal;
#else
    typedef float MgcReal;
#endif


class MgcMath
{
public:
    static int IAbs (int iValue);
    static int ICeil (float fValue);
    static int IFloor (float fValue);
    static int ISign (int iValue);

    static MgcReal Abs (MgcReal fValue);
    static MgcReal ACos (MgcReal fValue);
    static MgcReal ASin (MgcReal fValue);
    static MgcReal ATan (MgcReal fValue);
    static MgcReal ATan2 (MgcReal fY, MgcReal fX);
    static MgcReal Ceil (MgcReal fValue);
    static MgcReal Cos (MgcReal fValue);
    static MgcReal Exp (MgcReal fValue);
    static MgcReal Floor (MgcReal fValue);
    static MgcReal Log (MgcReal fValue);
    static MgcReal Pow (MgcReal kBase, MgcReal kExponent);
    static MgcReal Sign (MgcReal fValue);
    static MgcReal Sin (MgcReal fValue);
    static MgcReal Sqr (MgcReal fValue);
    static MgcReal Sqrt (MgcReal fValue);
    static MgcReal UnitRandom ();  // in [0,1]
    static MgcReal SymmetricRandom ();  // in [-1,1]

    static const MgcReal INFINITY;
    static const MgcReal PI;
    static const MgcReal TWO_PI;
    static const MgcReal HALF_PI;
};

#endif
