#include "SV.h"

ClassImp(SV)

SV::SV() : Particle() {
    j_chi2 = -999.f;
    j_dlen = -999.f;
    j_dlenSig = -999.f;
    j_dxy = -999.f;
    j_dxySig = -999.f;
    j_ndof = -999.f;
    j_ntracks = 0;
    j_pAngle = -999.f;
    j_x = -999.f;
    j_y = -999.f;
    j_z = -999.f;
}

SV::~SV() {}
