#ifndef SV_h
#define SV_h

#include "Particle.h"

class SV : public Particle {
public:
    SV();
    ~SV();

    inline void SetChi2(float chi2) { j_chi2 = chi2; }
    inline void SetDlen(float dlen) { j_dlen = dlen; }
    inline void SetDlenSig(float dlenSig) { j_dlenSig = dlenSig; }
    inline void SetDxy(float dxy) { j_dxy = dxy; }
    inline void SetDxySig(float dxySig) { j_dxySig = dxySig; }
    inline void SetNdof(float ndof) { j_ndof = ndof; }
    inline void SetNTracks(unsigned char ntracks) { j_ntracks = ntracks; }
    inline void SetPAngle(float pAngle) { j_pAngle = pAngle; }
    inline void SetPosition(float x, float y, float z) { j_x = x; j_y = y; j_z = z; }

    inline float Chi2() const { return j_chi2; }
    inline float Dlen() const { return j_dlen; }
    inline float DlenSig() const { return j_dlenSig; }
    inline float Dxy() const { return j_dxy; }
    inline float DxySig() const { return j_dxySig; }
    inline float Ndof() const { return j_ndof; }
    inline unsigned char NTracks() const { return j_ntracks; }
    inline float PAngle() const { return j_pAngle; }
    inline float X() const { return j_x; }
    inline float Y() const { return j_y; }
    inline float Z() const { return j_z; }

private:
    float j_chi2;
    float j_dlen;
    float j_dlenSig;
    float j_dxy;
    float j_dxySig;
    float j_ndof;
    unsigned char j_ntracks;
    float j_pAngle;
    float j_x;
    float j_y;
    float j_z;

    ClassDef(SV, 1)
};

#endif
