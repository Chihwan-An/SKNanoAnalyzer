#ifndef MuMu_h
#define MuMu_h

#include "Particle.h"
#include "MuMuTaggingParameter.h"
#include "MuMuConstituent.h"

class MuMu : public Particle
{
public:
  MuMu();
  ~MuMu();


private:
  // ───── Basic MuMu Properties ─────────────
  int j_charge, j_l1Index, j_l2Index, j_pdgId;
  float j_pt, j_eta, j_phi, j_mass, j_cos2D;

  // ───── Fitted MuMu Properties ─────────────
  float j_fit_cos2D, j_fit_mass, j_fit_massErr;

  // ───── Vertex and Lifetime Info ─────────────
  float j_l_xy, j_l_xy_unc, j_svprob;
  float j_vtx_x, j_vtx_y, j_vtx_z;
  ClassDef(MuMu, 1)
};

#endif
