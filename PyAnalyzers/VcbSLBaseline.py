"""Thin PyROOT front-end for the compiled Vcb_SL baseline analysis."""

from ROOT import TString, Vcb_SL


class VcbSLBaseline:
    """Configure Vcb_SL to stop after baseline selection and basic histograms.

    Composition is intentional: the object running ``Loop`` is the concrete
    C++ ``Vcb_SL`` instance, so PyROOT never installs a per-event dispatcher.
    """

    def __init__(self):
        object.__setattr__(self, "_backend", Vcb_SL())

    def __getattr__(self, name):
        return getattr(self._backend, name)

    def __setattr__(self, name, value):
        setattr(self._backend, name, value)

    def initializePyAnalyzer(self):
        if not any(str(flag) == "BaselineOnly" for flag in self._backend.Userflags):
            self._backend.Userflags.emplace_back(TString("BaselineOnly"))
        self._backend.initializeAnalyzer()
