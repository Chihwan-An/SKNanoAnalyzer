#include "MuMu.h"

ClassImp(MuMu)

MuMu::MuMu() : Particle() {
    j_charge = -999;
    j_l1Index = -999;
    j_l2Index = -999;
    j_pdgId = -999;
    j_pt = -999.0;
    j_eta = -999.0;
    j_phi = -999.0;     
    j_mass = -999.0;
    j_cos2D = -999.0;
    j_fit_cos2D = -999.0;
    j_fit_mass = -999.0;
    j_fit_massErr = -999.0;
    j_l_xy = -999.0;
    j_l_xy_unc = -999.0;
    j_svprob = -999.0;
    j_vtx_x = -999.0;
    j_vtx_y = -999.0;
    j_vtx_z = -999.0;
}

MuMu::~MuMu() {}

