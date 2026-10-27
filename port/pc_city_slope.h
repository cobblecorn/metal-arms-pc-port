#ifndef MA_PC_CITY_SLOPE_H
#define MA_PC_CITY_SLOPE_H

// Two buried terrain panels oppose the visible sloped walls in Mil City Hub.
// Keep their interior blocking face; ignore it only outside the actual roof
// plane (within contact tolerance). The caller gates the world and player.
static bool PortMilCityBuriedDoorwayWall(const FCollImpact_t &hit, const CFSphere &sphere) {
    if (hit.pTag || hit.UnitFaceNormal.x < .999f ||
        hit.UnitFaceNormal.y < -.01f || hit.UnitFaceNormal.y > .01f ||
        hit.UnitFaceNormal.z < -.01f || hit.UnitFaceNormal.z > .01f) return false;
    const float shift = hit.aTriVtx[0].z < -750.0f ? 0.0f : 169.20f;
    bool low=false, high=false;
    for (int i=0;i<3;++i) {
        const float x=hit.aTriVtx[i].x, y=hit.aTriVtx[i].y, z=hit.aTriVtx[i].z-shift;
        if (x<439.0f || x>439.25f ||
            !((y>4.70f && y<4.86f) || (y>24.20f && y<24.36f)) ||
            !((z>-846.55f && z<-846.30f) || (z>-793.40f && z<-793.10f))) return false;
        low=low || y<10.0f; high=high || y>20.0f;
    }
    if (!low || !high) return false;
    const float outside = -.8233f*(sphere.m_Pos.x-440.303f) +
                           .5671f*(sphere.m_Pos.y-10.637f);
    return outside >= sphere.m_fRadius-.12f;
}

#endif
