#!/usr/bin/env python3
"""Refactor repeated histogram-filling blocks into lambda+loop pattern."""

FILE = "/data6/Users/achihwan/SKNanoAnalyzer-v13/Analyzers/src/Reproduce20_002_copy.cc"

# --- Data definitions ---

DYCR_EE_HISTS = [
    ("_pt",                 "Resolve_DYCREEpt",            100,  "0.",     "1000."),
    ("_leading_jet_pt",     "Resolve_DYCREEleadjetpt",     200,  "0.",     "2000."),
    ("_subleading_jet_pt",  "Resolve_DYCREEsubleadjetpt",  200,  "0.",     "2000."),
    ("_mlljj",              "Resolve_DYCREEmlljj",         800,  "0.",     "8000."),
    ("_leading_lep_pt",     "Resolve_DYCREEleadleppt",    2000,  "0.",     "2000."),
    ("_subleading_lep_pt",  "Resolve_DYCREEsubleadleppt", 2000,  "0.",     "2000."),
    ("_mass",               "Resolve_DYCREEmass",          100,  "0.",     "1000."),
    ("_eta",                "Resolve_DYCREEeta",           100,  "-5.",    "5."),
    ("_phi",                "Resolve_DYCREEphi",           100,  "-3.14",  "3.14"),
    ("_leading_jet_eta",    "Resolve_DYCREEleadjeteta",    100,  "-2.5",   "2.5"),
    ("_leading_jet_phi",    "Resolve_DYCREEleadjetphi",    100,  "-3.14",  "3.14"),
    ("_subleading_jet_eta", "Resolve_DYCREEsubleadjeteta", 100,  "-2.5",   "2.5"),
    ("_subleading_jet_phi", "Resolve_DYCREEsubleadjetphi", 100,  "-3.14",  "3.14"),
    ("_j1j2_mass",          "Resolve_DYCREEj1j2mass",      100,  "0.",     "1000."),
    ("_j1j2_pt",            "Resolve_DYCREEj1j2pt",        100,  "0.",     "1000."),
    ("_j1j2_eta",           "Resolve_DYCREEj1j2eta",       100,  "-5.",    "5."),
    ("_j1j2_phi",           "Resolve_DYCREEj1j2phi",       100,  "-3.14",  "3.14"),
    ("_l1j1j2_mass",        "Resolve_DYCREEl1j1j2mass",    200,  "0.",     "2000."),
    ("_l1j1j2_pt",          "Resolve_DYCREEl1j1j2pt",      200,  "0.",     "2000."),
    ("_l1j1j2_eta",         "Resolve_DYCREEl1j1j2eta",     100,  "-5.",    "5."),
    ("_l1j1j2_phi",         "Resolve_DYCREEl1j1j2phi",     100,  "-3.14",  "3.14"),
    ("_l2j1j2_mass",        "Resolve_DYCREEl2j1j2mass",    200,  "0.",     "2000."),
    ("_l2j1j2_pt",          "Resolve_DYCREEl2j1j2pt",      200,  "0.",     "2000."),
    ("_l2j1j2_eta",         "Resolve_DYCREEl2j1j2eta",     100,  "-5.",    "5."),
    ("_l2j1j2_phi",         "Resolve_DYCREEl2j1j2phi",     100,  "-3.14",  "3.14"),
    ("_mlljj_pt",           "Resolve_DYCREEmlljjpt",       200,  "0.",     "2000."),
    ("_mlljj_eta",          "Resolve_DYCREEmlljjeta",      100,  "-5.",    "5."),
    ("_mlljj_phi",          "Resolve_DYCREEmlljjphi",      100,  "-3.14",  "3.14"),
    ("_leading_lep_eta",    "Resolve_DYCREEleadlepeta",    100,  "-2.5",   "2.5"),
    ("_leading_lep_phi",    "Resolve_DYCREEleadlepphi",    100,  "-3.14",  "3.14"),
    ("_subleading_lep_eta", "Resolve_DYCREEsubleadlepeta", 100,  "-2.5",   "2.5"),
    ("_subleading_lep_phi", "Resolve_DYCREEsubleadlepphi", 100,  "-3.14",  "3.14"),
    ("_jetnum",             "Resolve_DYCREEjetnum",         20,  "0.",     "20."),
    ("_punum",              "Resolve_DYCREEpunum",          80,  "0.",     "80."),
    ("_pv",                 "Resolve_DYCREEpv",             80,  "0.",     "80."),
    ("_pvgood",             "Resolve_DYCREEpvgood",         80,  "0.",     "80."),
]

DYCR_EE_REGIONS = [
    ("is_Resolved_DY_EE",             "Obj_PU_Corr_DYCR_Resolved_EE"),
    ("is_Resolved_DY_EE_OS",          "Obj_PU_Corr_DYCR_Resolved_EE_OS"),
    ("is_Resolved_DY_EE_OS_2e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_OS_2e_tight"),
    ("is_Resolved_DY_EE_OS_1e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_OS_1e_tight"),
    ("is_Resolved_DY_EE_OS_0e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_OS_0e_tight"),
    ("is_Resolved_DY_EE_SS",          "Obj_PU_Corr_DYCR_Resolved_EE_SS"),
    ("is_Resolved_DY_EE_SS_2e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_SS_2e_tight"),
    ("is_Resolved_DY_EE_SS_1e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_SS_1e_tight"),
    ("is_Resolved_DY_EE_SS_0e_tight", "Obj_PU_Corr_DYCR_Resolved_EE_SS_0e_tight"),
]

DYCR_MM_HISTS = [
    ("_pt",                 "Resolve_DYCRMMpt",            100,  "0.",     "1000."),
    ("_leading_jet_pt",     "Resolve_DYCRMMleadjetpt",     200,  "0.",     "2000."),
    ("_subleading_jet_pt",  "Resolve_DYCRMMsubleadjetpt",  200,  "0.",     "2000."),
    ("_mlljj",              "Resolve_DYCRMMmlljj",         800,  "0.",     "8000."),
    ("_leading_lep_pt",     "Resolve_DYCRMMleadleppt",    2000,  "0.",     "2000."),
    ("_subleading_lep_pt",  "Resolve_DYCRMMsubleadleppt", 2000,  "0.",     "2000."),
    ("_mass",               "Resolve_DYCRMMmass",          100,  "0.",     "1000."),
    ("_eta",                "Resolve_DYCRMMeta",           100,  "-5.",    "5."),
    ("_phi",                "Resolve_DYCRMMphi",           100,  "-3.14",  "3.14"),
    ("_leading_jet_eta",    "Resolve_DYCRMMleadjeteta",    100,  "-2.5",   "2.5"),
    ("_leading_jet_phi",    "Resolve_DYCRMMleadjetphi",    100,  "-3.14",  "3.14"),
    ("_subleading_jet_eta", "Resolve_DYCRMMsubleadjeteta", 100,  "-2.5",   "2.5"),
    ("_subleading_jet_phi", "Resolve_DYCRMMsubleadjetphi", 100,  "-3.14",  "3.14"),
    ("_j1j2_mass",          "Resolve_DYCRMMj1j2mass",      100,  "0.",     "1000."),
    ("_j1j2_pt",            "Resolve_DYCRMMj1j2pt",        100,  "0.",     "1000."),
    ("_j1j2_eta",           "Resolve_DYCRMMj1j2eta",       100,  "-5.",    "5."),
    ("_j1j2_phi",           "Resolve_DYCRMMj1j2phi",       100,  "-3.14",  "3.14"),
    ("_l1j1j2_mass",        "Resolve_DYCRMMl1j1j2mass",    200,  "0.",     "2000."),
    ("_l1j1j2_pt",          "Resolve_DYCRMMl1j1j2pt",      200,  "0.",     "2000."),
    ("_l1j1j2_eta",         "Resolve_DYCRMMl1j1j2eta",     100,  "-5.",    "5."),
    ("_l1j1j2_phi",         "Resolve_DYCRMMl1j1j2phi",     100,  "-3.14",  "3.14"),
    ("_l2j1j2_mass",        "Resolve_DYCRMMl2j1j2mass",    200,  "0.",     "2000."),
    ("_l2j1j2_pt",          "Resolve_DYCRMMl2j1j2pt",      200,  "0.",     "2000."),
    ("_l2j1j2_eta",         "Resolve_DYCRMMl2j1j2eta",     100,  "-5.",    "5."),
    ("_l2j1j2_phi",         "Resolve_DYCRMMl2j1j2phi",     100,  "-3.14",  "3.14"),
    ("_mlljj_pt",           "Resolve_DYCRMMmlljjpt",       200,  "0.",     "2000."),
    ("_mlljj_eta",          "Resolve_DYCRMMmlljjeta",      100,  "-5.",    "5."),
    ("_mlljj_phi",          "Resolve_DYCRMMmlljjphi",      100,  "-3.14",  "3.14"),
    ("_leading_lep_eta",    "Resolve_DYCRMMleadlepeta",    100,  "-2.5",   "2.5"),
    ("_leading_lep_phi",    "Resolve_DYCRMMleadlepphi",    100,  "-3.14",  "3.14"),
    ("_subleading_lep_eta", "Resolve_DYCRMMsubleadlepeta", 100,  "-2.5",   "2.5"),
    ("_subleading_lep_phi", "Resolve_DYCRMMsubleadlepphi", 100,  "-3.14",  "3.14"),
    ("_jetnum",             "Resolve_DYCRMMjetnum",         20,  "0.",     "20."),
    ("_punum",              "Resolve_DYCRMMpunum",          80,  "0.",     "80."),
    ("_pv",                 "Resolve_DYCRMMpv",             80,  "0.",     "80."),
    ("_pvgood",             "Resolve_DYCRMMpvgood",         80,  "0.",     "80."),
]

DYCR_MM_REGIONS = [
    ("is_Resolved_DY_MM",             "Obj_PU_Corr_DYCR_Resolved_MM"),
    ("is_Resolved_DY_MM_OS",          "Obj_PU_Corr_DYCR_Resolved_MM_OS"),
    ("is_Resolved_DY_MM_OS_tight",    "Obj_PU_Corr_DYCR_Resolved_MM_OS_tight"),
    ("is_Resolved_DY_MM_OS_not_tight","Obj_PU_Corr_DYCR_Resolved_MM_OS_not_tight"),
    ("is_Resolved_DY_MM_SS",          "Obj_PU_Corr_DYCR_Resolved_MM_SS"),
    ("is_Resolved_DY_MM_SS_tight",    "Obj_PU_Corr_DYCR_Resolved_MM_SS_tight"),
    ("is_Resolved_DY_MM_SS_not_tight","Obj_PU_Corr_DYCR_Resolved_MM_SS_not_tight"),
]

SR_EE_HISTS = [
    ("_ll_pt",              "Resolve_SREEpt",              8000, "0.",     "8000."),
    ("_leading_jet_pt",     "Resolve_SREEleadjetpt",       8000, "0.",     "8000."),
    ("_subleading_jet_pt",  "Resolve_SREEsubleadjetpt",    8000, "0.",     "8000."),
    ("_mlljj",              "Resolve_SREEmlljj",           8000, "0.",     "8000."),
    ("_leading_lep_pt",     "Resolve_SREEleadleppt",       2000, "0.",     "2000."),
    ("_subleading_lep_pt",  "Resolve_SREEsubleadleppt",    2000, "0.",     "2000."),
    ("_mass",               "Resolve_SREEmass",             100, "0.",     "2000."),
    ("_eta",                "Resolve_SREEeta",              100, "-5.",    "5."),
    ("_phi",                "Resolve_SREEphi",              100, "-3.14",  "3.14"),
    ("_leading_jet_eta",    "Resolve_SREEleadjeteta",       100, "-2.5",   "2.5"),
    ("_leading_jet_phi",    "Resolve_SREEleadjetphi",       100, "-3.14",  "3.14"),
    ("_subleading_jet_eta", "Resolve_SREEsubleadjeteta",    100, "-2.5",   "2.5"),
    ("_subleading_jet_phi", "Resolve_SREEsubleadjetphi",    100, "-3.14",  "3.14"),
    ("_j1j2_mass",          "Resolve_SREEj1j2mass",         100, "0.",     "1000."),
    ("_j1j2_pt",            "Resolve_SREEj1j2pt",           100, "0.",     "1000."),
    ("_j1j2_eta",           "Resolve_SREEj1j2eta",          100, "-5.",    "5."),
    ("_j1j2_phi",           "Resolve_SREEj1j2phi",          100, "-3.14",  "3.14"),
    ("_l1j1j2_mass",        "Resolve_SREEl1j1j2mass",       800, "0.",     "8000."),
    ("_l1j1j2_pt",          "Resolve_SREEl1j1j2pt",         200, "0.",     "2000."),
    ("_l1j1j2_eta",         "Resolve_SREEl1j1j2eta",        100, "-5.",    "5."),
    ("_l1j1j2_phi",         "Resolve_SREEl1j1j2phi",        100, "-3.14",  "3.14"),
    ("_l2j1j2_mass",        "Resolve_SREEl2j1j2mass",       800, "0.",     "8000."),
    ("_l2j1j2_pt",          "Resolve_SREEl2j1j2pt",         200, "0.",     "2000."),
    ("_l2j1j2_eta",         "Resolve_SREEl2j1j2eta",        100, "-5.",    "5."),
    ("_l2j1j2_phi",         "Resolve_SREEl2j1j2phi",        100, "-3.14",  "3.14"),
    ("_mlljj_pt",           "Resolve_SREEmlljjpt",          800, "0.",     "8000."),
    ("_mlljj_eta",          "Resolve_SREEmlljjeta",         100, "-5.",    "5."),
    ("_mlljj_phi",          "Resolve_SREEmlljjphi",         100, "-3.14",  "3.14"),
    ("_leading_lep_eta",    "Resolve_SREEleadlepeta",       100, "-2.5",   "2.5"),
    ("_leading_lep_phi",    "Resolve_SREEleadlepphi",       100, "-3.14",  "3.14"),
    ("_subleading_lep_eta", "Resolve_SREEsubleadlepeta",    100, "-2.5",   "2.5"),
    ("_subleading_lep_phi", "Resolve_SREEsubleadlepphi",    100, "-3.14",  "3.14"),
    ("_jetnum",             "Resolve_DYSREEjetnum",          20, "0.",     "20."),
    ("_punum",              "Resolve_DYSREEpunum",           80, "0.",     "80."),
    ("_pv",                 "Resolve_DYSREEpv",              80, "0.",     "80."),
    ("_pvgood",             "Resolve_DYSREEpvgood",          80, "0.",     "80."),
]

SR_EE_REGIONS = [
    ("is_Resolved_SR_EE",             "Obj_PU_Corr_SR_Resolved_EE"),
    ("is_Resolved_SR_EE_SS",          "Obj_PU_Corr_SR_Resolved_EE_SS"),
    ("is_Resolved_SR_EE_SS_2e_tight", "Obj_PU_Corr_SR_Resolved_EE_SS_2e_tight"),
    ("is_Resolved_SR_EE_SS_1e_tight", "Obj_PU_Corr_SR_Resolved_EE_SS_1e_tight"),
    ("is_Resolved_SR_EE_SS_0e_tight", "Obj_PU_Corr_SR_Resolved_EE_SS_0e_tight"),
    ("is_Resolved_SR_EE_OS",          "Obj_PU_Corr_SR_Resolved_EE_OS"),
    ("is_Resolved_SR_EE_OS_2e_tight", "Obj_PU_Corr_SR_Resolved_EE_OS_2e_tight"),
    ("is_Resolved_SR_EE_OS_1e_tight", "Obj_PU_Corr_SR_Resolved_EE_OS_1e_tight"),
    ("is_Resolved_SR_EE_OS_0e_tight", "Obj_PU_Corr_SR_Resolved_EE_OS_0e_tight"),
]

SR_MM_HISTS = [
    ("_ll_pt",              "Resolve_SRMMpt",              8000, "0.",     "8000."),
    ("_leading_jet_pt",     "Resolve_SRMMleadjetpt",       8000, "0.",     "8000."),
    ("_subleading_jet_pt",  "Resolve_SRMMsubleadjetpt",    8000, "0.",     "8000."),
    ("_mlljj",              "Resolve_SRMMmlljj",           8000, "0.",     "8000."),
    ("_leading_lep_pt",     "Resolve_SRMMleadleppt",       2000, "0.",     "2000."),
    ("_subleading_lep_pt",  "Resolve_SRMMsubleadleppt",    2000, "0.",     "2000."),
    ("_mass",               "Resolve_SRMMmass",             100, "0.",     "2000."),
    ("_eta",                "Resolve_SRMMeta",              100, "-5.",    "5."),
    ("_phi",                "Resolve_SRMMphi",              100, "-3.14",  "3.14"),
    ("_leading_jet_eta",    "Resolve_SRMMleadjeteta",       100, "-2.5",   "2.5"),
    ("_leading_jet_phi",    "Resolve_SRMMleadjetphi",       100, "-3.14",  "3.14"),
    ("_subleading_jet_eta", "Resolve_SRMMsubleadjeteta",    100, "-2.5",   "2.5"),
    ("_subleading_jet_phi", "Resolve_SRMMsubleadjetphi",    100, "-3.14",  "3.14"),
    ("_j1j2_mass",          "Resolve_SRMMj1j2mass",         100, "0.",     "1000."),
    ("_j1j2_pt",            "Resolve_SRMMj1j2pt",           100, "0.",     "1000."),
    ("_j1j2_eta",           "Resolve_SRMMj1j2eta",          100, "-5.",    "5."),
    ("_j1j2_phi",           "Resolve_SRMMj1j2phi",          100, "-3.14",  "3.14"),
    ("_l1j1j2_mass",        "Resolve_SRMMl1j1j2mass",       800, "0.",     "8000."),
    ("_l1j1j2_pt",          "Resolve_SRMMl1j1j2pt",         200, "0.",     "2000."),
    ("_l1j1j2_eta",         "Resolve_SRMMl1j1j2eta",        100, "-5.",    "5."),
    ("_l1j1j2_phi",         "Resolve_SRMMl1j1j2phi",        100, "-3.14",  "3.14"),
    ("_l2j1j2_mass",        "Resolve_SRMMl2j1j2mass",       800, "0.",     "8000."),
    ("_l2j1j2_pt",          "Resolve_SRMMl2j1j2pt",         200, "0.",     "2000."),
    ("_l2j1j2_eta",         "Resolve_SRMMl2j1j2eta",        100, "-5.",    "5."),
    ("_l2j1j2_phi",         "Resolve_SRMMl2j1j2phi",        100, "-3.14",  "3.14"),
    ("_mlljj_pt",           "Resolve_SRMMmlljjpt",          800, "0.",     "8000."),
    ("_mlljj_eta",          "Resolve_SRMMmlljjeta",         100, "-5.",    "5."),
    ("_mlljj_phi",          "Resolve_SRMMmlljjphi",         100, "-3.14",  "3.14"),
    ("_leading_lep_eta",    "Resolve_SRMMleadlepeta",       100, "-2.5",   "2.5"),
    ("_leading_lep_phi",    "Resolve_SRMMleadlepphi",       100, "-3.14",  "3.14"),
    ("_subleading_lep_eta", "Resolve_SRMMsubleadlepeta",    100, "-2.5",   "2.5"),
    ("_subleading_lep_phi", "Resolve_SRMMsubleadlepphi",    100, "-3.14",  "3.14"),
    ("_jetnum",             "Resolve_DYSRMMjetnum",          20, "0.",     "20."),
    ("_punum",              "Resolve_DYSRMMpunum",           80, "0.",     "80."),
    ("_pv",                 "Resolve_DYSRMMpv",              80, "0.",     "80."),
    ("_pvgood",             "Resolve_DYSRMMpvgood",          80, "0.",     "80."),
]

SR_MM_REGIONS = [
    ("is_Resolved_SR_MM",             "Obj_PU_Corr_SR_Resolved_MM"),
    ("is_Resolved_SR_MM_SS",          "Obj_PU_Corr_SR_Resolved_MM_SS"),
    ("is_Resolved_SR_MM_SS_tight",    "Obj_PU_Corr_SR_Resolved_MM_SS_tight"),
    ("is_Resolved_SR_MM_SS_not_tight","Obj_PU_Corr_SR_Resolved_MM_SS_not_tight"),
    ("is_Resolved_SR_MM_OS",          "Obj_PU_Corr_SR_Resolved_MM_OS"),
    # Note: OS_tight and OS_not_tight use "ResolvedMM" (no underscore) in original
    ("is_Resolved_SR_MM_OS_tight",    "Obj_PU_Corr_SR_ResolvedMM_OS_tight"),
    ("is_Resolved_SR_MM_OS_not_tight","Obj_PU_Corr_SR_ResolvedMM_OS_not_tight"),
]

FLAV_EM_HISTS = [
    ("_ll_pt",              "Resolve_FlavCRpt",            1000, "0.",     "1000."),
    ("_leading_jet_pt",     "Resolve_FlavCRleadjetpt",     2000, "0.",     "2000."),
    ("_subleading_jet_pt",  "Resolve_FlavCRsubleadjetpt",  2000, "0.",     "2000."),
    ("_mlljj",              "Resolve_FlavCRmlljj",         8000, "0.",     "8000."),
    ("_leading_lep_pt",     "Resolve_FlavCRleadleppt",     2000, "0.",     "2000."),
    ("_subleading_lep_pt",  "Resolve_FlavCRsubleadleppt",  2000, "0.",     "2000."),
    ("_mass",               "Resolve_FlavCRmass",           100, "0.",     "2000."),
    ("_eta",                "Resolve_FlavCReta",            100, "-5.",    "5."),
    ("_phi",                "Resolve_FlavCRphi",            100, "-3.14",  "3.14"),
    ("_leading_jet_eta",    "Resolve_FlavCRleadjeteta",     100, "-2.5",   "2.5"),
    ("_leading_jet_phi",    "Resolve_FlavCRleadjetphi",     100, "-3.14",  "3.14"),
    ("_subleading_jet_eta", "Resolve_FlavCRsubleadjeteta",  100, "-2.5",   "2.5"),
    ("_subleading_jet_phi", "Resolve_FlavCRsubleadjetphi",  100, "-3.14",  "3.14"),
    ("_j1j2_mass",          "Resolve_FlavCRj1j2mass",       100, "0.",     "1000."),
    ("_j1j2_pt",            "Resolve_FlavCRj1j2pt",         100, "0.",     "1000."),
    ("_j1j2_eta",           "Resolve_FlavCRj1j2eta",        100, "-5.",    "5."),
    ("_j1j2_phi",           "Resolve_FlavCRj1j2phi",        100, "-3.14",  "3.14"),
    ("_l1j1j2_mass",        "Resolve_FlavCRl1j1j2mass",     800, "0.",     "8000."),
    ("_l1j1j2_pt",          "Resolve_FlavCRl1j1j2pt",       200, "0.",     "2000."),
    ("_l1j1j2_eta",         "Resolve_FlavCRl1j1j2eta",      100, "-5.",    "5."),
    ("_l1j1j2_phi",         "Resolve_FlavCRl1j1j2phi",      100, "-3.14",  "3.14"),
    ("_l2j1j2_mass",        "Resolve_FlavCRl2j1j2mass",     800, "0.",     "8000."),
    ("_l2j1j2_pt",          "Resolve_FlavCRl2j1j2pt",       200, "0.",     "2000."),
    ("_l2j1j2_eta",         "Resolve_FlavCRl2j1j2eta",      100, "-5.",    "5."),
    ("_l2j1j2_phi",         "Resolve_FlavCRl2j1j2phi",      100, "-3.14",  "3.14"),
    ("_mlljj_pt",           "Resolve_FlavCRmlljjpt",        800, "0.",     "8000."),
    ("_mlljj_eta",          "Resolve_FlavCRmlljjeta",       100, "-5.",    "5."),
    ("_mlljj_phi",          "Resolve_FlavCRmlljjphi",       100, "-3.14",  "3.14"),
    ("_leading_lep_eta",    "Resolve_FlavCRleadlepeta",     100, "-2.5",   "2.5"),
    ("_leading_lep_phi",    "Resolve_FlavCRleadlepphi",     100, "-3.14",  "3.14"),
    ("_subleading_lep_eta", "Resolve_FlavCRsubleadlepeta",  100, "-2.5",   "2.5"),
    ("_subleading_lep_phi", "Resolve_FlavCRsubleadlepphi",  100, "-3.14",  "3.14"),
    ("_jetnum",             "Resolve_FlavCRjetnum",          20, "0.",     "20."),
    ("_punum",              "Resolve_FlavCRpunum",           80, "0.",     "80."),
    ("_pv",                 "Resolve_FlavCRpv",              80, "0.",     "80."),
    ("_pvgood",             "Resolve_FlavCRpvgood",          80, "0.",     "80."),
]

# Flav EM: sub-conditions (M_tight_E_tight etc.) all use the same prefix as the
# parent OS_tight or SS_tight condition (intentional in original code)
FLAV_EM_REGIONS = [
    ("is_Resolved_Flav_EM",                    "Obj_PU_Corr_Flav_CR_Resolved_EM"),
    ("is_Resolved_Flav_EM_OS",                 "Obj_PU_Corr_Flav_CR_Resolved_EM_OS"),
    ("is_Resolved_Flav_EM_OS_tight",           "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_tight"),
    ("is_Resolved_Flav_EM_OS_not_tight",       "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_not_tight"),
    ("is_Resolved_Flav_EM_OS_M_tight_E_tight",     "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_tight"),
    ("is_Resolved_Flav_EM_OS_M_tight_E_not_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_tight"),
    ("is_Resolved_Flav_EM_OS_M_not_tight_E_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_tight"),
    ("is_Resolved_Flav_EM_OS_M_not_tight_E_not_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_OS_tight"),
    ("is_Resolved_Flav_EM_SS",                 "Obj_PU_Corr_Flav_CR_Resolved_EM_SS"),
    ("is_Resolved_Flav_EM_SS_tight",           "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_tight"),
    ("is_Resolved_Flav_EM_SS_not_tight",       "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_not_tight"),
    ("is_Resolved_Flav_EM_SS_M_tight_E_tight",     "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_tight"),
    ("is_Resolved_Flav_EM_SS_M_tight_E_not_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_tight"),
    ("is_Resolved_Flav_EM_SS_M_not_tight_E_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_tight"),
    ("is_Resolved_Flav_EM_SS_M_not_tight_E_not_tight", "Obj_PU_Corr_Flav_CR_Resolved_EM_SS_tight"),
]

# --- Code generation ---

def gen_block(hists, regions, outer_indent):
    """Generate the lambda+loop replacement block."""
    I0 = " " * outer_indent        # block open/close
    I1 = " " * (outer_indent + 4)  # auto fill / for loop
    I2 = " " * (outer_indent + 8)  # lambda body FillHist calls
    I3 = " " * (outer_indent + 8)  # region entries in initializer_list

    lines = []
    lines.append(f"{I0}{{")
    lines.append(f"{I1}auto fill = [&](const char* pfx) {{")
    for (suf, var, n, xmin, xmax) in hists:
        lines.append(f'{I2}FillHist(syst_name + "/" + pfx + "{suf}", {var}, final_weight, {n}, {xmin}, {xmax});')
    lines.append(f"{I1}}};")

    # Compute max condition length for alignment
    max_cond_len = max(len(cond) for cond, _ in regions)

    lines.append(f"{I1}for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{{")
    for (cond, pfx) in regions:
        padding = " " * (max_cond_len - len(cond))
        lines.append(f'{I3}{{{cond},{padding} "{pfx}"}},')
    lines.append(f"{I1}}}) {{ if (cond) fill(pfx); }}")
    lines.append(f"{I0}}}")
    return "\n".join(lines)

# --- Groups: (hists, regions, mc_start, mc_end, data_start, data_end, mc_outer_indent, data_outer_indent) ---
# Line numbers are 1-indexed (as in the file)

GROUPS = [
    # (hists, regions, mc_start, mc_end, data_start, data_end, mc_indent, data_indent)
    (DYCR_EE_HISTS,  DYCR_EE_REGIONS,  2651, 2992, 6291, 6632, 12, 8),
    (DYCR_MM_HISTS,  DYCR_MM_REGIONS,  2993, 3258, 6633, 6898, 12, 8),
    (SR_EE_HISTS,    SR_EE_REGIONS,    3260, 3601, 6900, 7241, 12, 8),
    (SR_MM_HISTS,    SR_MM_REGIONS,    3602, 3868, 7242, 7508, 12, 8),
    (FLAV_EM_HISTS,  FLAV_EM_REGIONS,  3871, 4440, 7511, 8080, 12, 8),
]

# --- Apply replacements ---

with open(FILE, 'r') as f:
    lines = f.readlines()

print(f"File has {len(lines)} lines")

# Collect all replacements (as (start_0idx, end_0idx, new_text))
# Process in reverse order so indices remain valid
replacements = []
for (hists, regions, mc_s, mc_e, da_s, da_e, mc_ind, da_ind) in GROUPS:
    mc_block = gen_block(hists, regions, mc_ind)
    da_block = gen_block(hists, regions, da_ind)
    replacements.append((mc_s - 1, mc_e - 1, mc_block))
    replacements.append((da_s - 1, da_e - 1, da_block))

# Sort by start index descending (process from end of file to start)
replacements.sort(key=lambda x: x[0], reverse=True)

for (s, e, new_text) in replacements:
    print(f"Replacing lines {s+1}-{e+1} ({e-s+1} lines) with {len(new_text.splitlines())} lines")
    # Verify the first line of the original matches what we expect
    orig_first = lines[s].rstrip()
    print(f"  Original first line: {repr(orig_first)}")
    new_lines = [l + "\n" for l in new_text.splitlines()]
    lines[s:e+1] = new_lines

print(f"\nFile now has {len(lines)} lines")

with open(FILE, 'w') as f:
    f.writelines(lines)

print("Done! File written.")
