// Device ownership for local join screens. Connections are previews until a player joins.
#pragma once
struct PcSessionPadRouting {
    enum { PORTS=4 };
    int owned[PORTS], routed[PORTS];
    unsigned joined;
    bool joining;
    PcSessionPadRouting() { Reset(false,-1); }
    void Reset(bool join, int ownerPad) {
        joining=join;joined=join?0:1;
        for(unsigned p=0;p<PORTS;++p) owned[p]=routed[p]=-1;
        owned[0]=ownerPad;
    }
    void Update(unsigned connected) {
        // P1 may replace a disconnected controller. Other players retain their
        // device identity across disconnects and must never inherit a spare pad.
        if(!joining && owned[0]>=0 && !(connected&(1u<<owned[0]))) owned[0]=-1;
        unsigned used=0;
        for(unsigned p=0;p<PORTS;++p) {
            routed[p]=-1;
            if(owned[p]>=0) {
                used|=1u<<owned[p];
                if(connected&(1u<<owned[p])) routed[p]=owned[p];
            }
        }
        for(unsigned pad=0;pad<PORTS;++pad) {
            if(!(connected&(1u<<pad)) || (used&(1u<<pad))) continue;
            int port=-1;
            if(joining) {
                for(unsigned p=1;p<PORTS && port<0;++p)
                    if(owned[p]<0 && routed[p]<0) port=(int)p;
                if(port<0 && owned[0]<0 && routed[0]<0) port=0;
            } else if((joined&1) && owned[0]<0) {
                port=0;owned[0]=(int)pad;
            }
            if(port<0) continue;
            routed[port]=(int)pad;used|=1u<<pad;
        }
    }
    void Claim(unsigned port) {
        if(!joining || port>=PORTS) return;
        owned[port]=routed[port];joined|=1u<<port;
    }
    void KeepJoined(unsigned mask) {
        if(!joining) return;
        for(unsigned p=0;p<PORTS;++p)
            if((joined&(1u<<p)) && !(mask&(1u<<p))) owned[p]=-1;
        joined=mask&15;
    }
    void Finish(unsigned mask) {
        joining=false;joined=mask&15;
        for(unsigned p=0;p<PORTS;++p) if(!(joined&(1u<<p))) owned[p]=-1;
    }
};
