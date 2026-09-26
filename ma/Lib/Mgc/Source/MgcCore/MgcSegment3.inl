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

//----------------------------------------------------------------------------
inline MgcSegment3::MgcSegment3 ()
{
    // no initialization for efficiency
}
//----------------------------------------------------------------------------
inline MgcVector3& MgcSegment3::Origin ()
{
    return m_kOrigin;
}
//----------------------------------------------------------------------------
inline const MgcVector3& MgcSegment3::Origin () const
{
    return m_kOrigin;
}
//----------------------------------------------------------------------------
inline MgcVector3& MgcSegment3::Direction ()
{
    return m_kDirection;
}
//----------------------------------------------------------------------------
inline const MgcVector3& MgcSegment3::Direction () const
{
    return m_kDirection;
}
//----------------------------------------------------------------------------
