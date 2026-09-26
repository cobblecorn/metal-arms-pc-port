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

#include "MgcMath.h"
#include "MgcRTLib.h"

#if MGC_USE_DOUBLE
    const MgcReal MgcMath::INFINITY = DBL_MAX;
#else
    const MgcReal MgcMath::INFINITY = FLT_MAX;
#endif

const MgcReal MgcMath::PI = 4.0*atan(1.0);
const MgcReal MgcMath::TWO_PI = 2.0*PI;
const MgcReal MgcMath::HALF_PI = 0.5*PI;

//----------------------------------------------------------------------------
int MgcMath::IAbs (int iValue)
{
    return ( iValue >= 0 ? iValue : -iValue );
}
//----------------------------------------------------------------------------
int MgcMath::ICeil (float fValue)
{
    return int(ceil(fValue));
}
//----------------------------------------------------------------------------
int MgcMath::IFloor (float fValue)
{
    return int(floor(fValue));
}
//----------------------------------------------------------------------------
int MgcMath::ISign (int iValue)
{
    return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Abs (MgcReal fValue)
{
    return MgcReal(fabs(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::ACos (MgcReal fValue)
{
    if ( -1.0 < fValue )
    {
        if ( fValue < 1.0 )
            return MgcReal(acos(fValue));
        else
            return 0.0;
    }
    else
    {
        return PI;
    }
}
//----------------------------------------------------------------------------
MgcReal MgcMath::ASin (MgcReal fValue)
{
    if ( -1.0 < fValue )
    {
        if ( fValue < 1.0 )
            return MgcReal(asin(fValue));
        else
            return -HALF_PI;
    }
    else
    {
        return HALF_PI;
    }
}
//----------------------------------------------------------------------------
MgcReal MgcMath::ATan (MgcReal fValue)
{
    return MgcReal(atan(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::ATan2 (MgcReal fY, MgcReal fX)
{
    return MgcReal(atan2(fY,fX));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Ceil (MgcReal fValue)
{
    return MgcReal(ceil(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Cos (MgcReal fValue)
{
    return MgcReal(cos(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Exp (MgcReal fValue)
{
    return MgcReal(exp(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Floor (MgcReal fValue)
{
    return MgcReal(floor(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Log (MgcReal fValue)
{
    return MgcReal(log(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Pow (MgcReal fBase, MgcReal fExponent)
{
    return MgcReal(pow(fBase,fExponent));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Sign (MgcReal fValue)
{
    if ( fValue > 0.0 )
        return 1.0;

    if ( fValue < 0.0 )
        return -1.0;

    return 0.0;
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Sin (MgcReal fValue)
{
    return MgcReal(sin(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Sqr (MgcReal fValue)
{
    return fValue*fValue;
}
//----------------------------------------------------------------------------
MgcReal MgcMath::Sqrt (MgcReal fValue)
{
    return MgcReal(sqrt(fValue));
}
//----------------------------------------------------------------------------
MgcReal MgcMath::UnitRandom ()
{
    return MgcReal(rand())/MgcReal(RAND_MAX);
}
//----------------------------------------------------------------------------
MgcReal MgcMath::SymmetricRandom ()
{
    return 2.0*MgcReal(rand())/MgcReal(RAND_MAX) - 1.0;
}
//----------------------------------------------------------------------------
