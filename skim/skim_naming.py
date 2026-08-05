"""
Alias / path conventions shared by submit_skim.py and register_skim.py.

These mirror python/sampleManager.py:180-248 exactly, because SKNano.py resolves
a skimmed sample by looking the alias up in Sample/Skim/skimTreeInfo.json and
then taking xsec / sumW / sumsign from the ORIGINAL PD's CommonSampleInfo.json
entry (SKNano.py:290-296). Get the naming wrong and either the lookup fails or,
worse, the job silently normalises to the wrong sample.
"""

import os


def split_alias(alias):
    """('TTLL_powheg', None, True)  |  ('EGamma0_C', ...) -> ('EGamma0', 'C', False)

    Data aliases carry a trailing period: either a single capital letter ('_C')
    or the 2023 style '_C_v1'.
    """
    parts = alias.split("_")
    if len(parts) >= 2:
        # 2023 style: ..._C_v1
        if (len(parts[-2]) == 1 and parts[-2].isupper()
                and parts[-1].startswith("v")):
            return "_".join(parts[:-2]), f"{parts[-2]}_{parts[-1]}", False
        # older style: ..._C
        if len(parts[-1]) == 1 and parts[-1].isupper():
            return "_".join(parts[:-1]), parts[-1], False
    return alias, None, True


def skim_name(suffix, pd):
    return f"Skim_{suffix}_{pd}"


def output_dir(outbase, era, suffix, alias, user):
    """Where tree_<i>.root files go, matching SKNano.py's skimming-mode layout."""
    pd, period, is_mc = split_alias(alias)
    base = os.path.join(outbase, era, "MC" if is_mc else "DATA", "Skim", user,
                        skim_name(suffix, pd))
    return base if is_mc else os.path.join(base, f"Period{period}")


def json_name(suffix, alias):
    """Filename (without .json) under Sample/Skim/ for this alias."""
    pd, period, is_mc = split_alias(alias)
    return skim_name(suffix, pd) if is_mc else f"{skim_name(suffix, pd)}_{period}"
