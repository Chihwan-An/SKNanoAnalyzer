#ifndef JetTaggingParameter_h
#define JetTaggingParameter_h

#include <cstddef>
#include <iostream>
#include <TString.h>

#include "AnalysisException.h"

using namespace std;

namespace JetTagging
{

    // Jet Tagging parameters
    enum class JetFlavTagger {
        DeepJet,
        ParticleNet,
        ParT
    };
    enum class JetFlavTaggerScoreType
    {
        B,
        CvB,
        CvL,
        QvG,
        CvNotB,
        TauVJet,
        Ele,
        Mu,
        SvCB,
        SvUDG,
        probB,
        probBB,
        probUDG
    };
    enum class JetFlavTaggerWP
    {
        Loose,
        Medium,
        Tight,
        VeryTight,
        SuperTight
    };
    enum class JetTaggingSFMethod {
        comb,
        mujets,
        shape,
        wp
    };
    enum class FatJetTagger{
        ParticleNetWithMass,
        ParticleNet,
        ParTWithMass,
        ParT
    };
    enum class FatJetTaggerScoreType{
        //gloparT
        QCD,
        TopbWev,
        TopbWmv,
        TopbWq,
        TopbWqq,
        TopbWtauhv,
        WvsQCD,
        XWW3q,
        XWW4q,
        XWWqqev,
        XWWqqmv,
        Xbb,
        Xcc,
        Xcs,
        Xqq,
        Xtauhtaue,
        Xtauhtauh,
        Xtauhtaum,
        //globalParTwithMass
        TopvsQCD,
        //WvsQCD,
        ZvsQCD,
        //particleNetWithMass
        H4qvsQCD,
        HbbvsQCD,
        HccvsQCD,
        //TvsQCD,
        //WvsQCD,
        //ZvsQCD,
        //particleNet
        //QCD,
        QCD0HF,
        QCD1HF,
        QCD2HF,
        WVsQCD,
        XbbVsQCD,
        XccVsQCD,
        XggVsQCD,
        XqqVsQCD,
        XteVsQCD,
        XtmVsQCD,
        XttVsQCD,
        MassCorr,
        MassCorrGeneric,
        MassCorrX2p
    };
    //string version of JetTaggingSFMethod
    inline TString GetJetTaggingSFMethodStr(JetTaggingSFMethod method) {
        switch (method)
        {
        case JetTaggingSFMethod::comb:
            return "comb";
        case JetTaggingSFMethod::mujets:
            return "mujets";
        case JetTaggingSFMethod::shape:
            return "shape";
        case JetTaggingSFMethod::wp:
            return "wp";
        default:
            throw SKNano::LogicError("[JetTagging::GetJetTaggingSFMethodStr] No such method");
        }
    };

    inline TString GetTaggerCorrectionLibStr(JetFlavTagger tagger)
    {
        switch (tagger)
        {
        case JetFlavTagger::DeepJet:
            return "deepJet";
        case JetFlavTagger::ParticleNet:
            return "particleNet";
        case JetFlavTagger::ParT:
            return "UParTAK4";
        default:
            throw SKNano::LogicError("[JetTagging::GetTaggerCorrectionLibStr] No such tagger");
        }
    };
    inline TString GetTaggerCorrectionWPStr(JetFlavTaggerWP wp)
    {
        switch (wp)
        {
        case JetFlavTaggerWP::Loose:
            return "L";
        case JetFlavTaggerWP::Medium:
            return "M";
        case JetFlavTaggerWP::Tight:
            return "T";
        case JetFlavTaggerWP::VeryTight:
            return "XT";
        case JetFlavTaggerWP::SuperTight:
            return "XXT";
        default:
            throw SKNano::LogicError("[JetTagging::GetTaggerCorrectionWPStr] No such wp");
        }
    };
    inline TString GetScoreTypeStr(JetFlavTaggerScoreType type)
    {
        switch (type)
        {
        case JetFlavTaggerScoreType::B:
            return "B";
        case JetFlavTaggerScoreType::CvB:
            return "CvB";
        case JetFlavTaggerScoreType::CvL:
            return "CvL";
        case JetFlavTaggerScoreType::QvG:
            return "QvG";
        case JetFlavTaggerScoreType::CvNotB:
            return "CvNotB";
        case JetFlavTaggerScoreType::TauVJet:
            return "TauVJet";
        case JetFlavTaggerScoreType::Ele:
            return "Ele";
        case JetFlavTaggerScoreType::Mu:
            return "Mu";
        case JetFlavTaggerScoreType::SvCB:
            return "SvCB";
        case JetFlavTaggerScoreType::SvUDG:
            return "SvUDG";
        case JetFlavTaggerScoreType::probB:
            return "probB";
        case JetFlavTaggerScoreType::probBB:
            return "probBB";
        case JetFlavTaggerScoreType::probUDG:
            return "probUDG";
        default:
            throw SKNano::LogicError("[JetTagging::GetScoreTypeStr] No such score type");
        }
    };

    constexpr std::size_t JetFlavTaggerScoreCount = static_cast<std::size_t>(JetFlavTaggerScoreType::probUDG) + 1;

    inline constexpr std::size_t JetFlavTaggerScoreIndex(JetFlavTaggerScoreType type)
    {
        return static_cast<std::size_t>(type);
    }
    
    inline TString GetFatJetTaggerCorrectionLibStr(FatJetTagger tagger)
    {
        switch (tagger)
        {
        case FatJetTagger::ParticleNetWithMass:
            return "particleNetWithMass";
        case FatJetTagger::ParticleNet:
            return "particleNet";
        case FatJetTagger::ParTWithMass:
            return "robustParticleTransformerWithMass";
        case FatJetTagger::ParT:
            return "robustParticleTransformer";
        default:
            throw SKNano::LogicError("[JetTagging::GetFatJetTaggerCorrectionLibStr] No such tagger");
        }
    };

    inline TString GetFatJetScoreTypeStr(FatJetTaggerScoreType type)
    {
        switch (type)
        {
        case FatJetTaggerScoreType::QCD:
            return "QCD";
        case FatJetTaggerScoreType::TopbWev:
            return "TopbWev";
        case FatJetTaggerScoreType::TopbWmv:
            return "TopbWmv";
        case FatJetTaggerScoreType::TopbWq:
            return "TopbWq";
        case FatJetTaggerScoreType::TopbWqq:
            return "TopbWqq";
        case FatJetTaggerScoreType::TopbWtauhv:
            return "TopbWtauhv";
        case FatJetTaggerScoreType::WvsQCD:
            return "WvsQCD";
        case FatJetTaggerScoreType::XWW3q:
            return "XWW3q";
        case FatJetTaggerScoreType::XWW4q:
            return "XWW4q";
        case FatJetTaggerScoreType::XWWqqev:
            return "XWWqqev";
        case FatJetTaggerScoreType::XWWqqmv:
            return "XWWqqmv";
        case FatJetTaggerScoreType::Xbb:
            return "Xbb";
        case FatJetTaggerScoreType::Xcc:
            return "Xcc";
        case FatJetTaggerScoreType::Xcs:
            return "Xcs";
        case FatJetTaggerScoreType::Xqq:
            return "Xqq";
        case FatJetTaggerScoreType::Xtauhtaue:
            return "Xtauhtaue";
        case FatJetTaggerScoreType::Xtauhtauh:
            return "Xtauhtauh";
        case FatJetTaggerScoreType::Xtauhtaum:
            return "Xtauhtaum";
        case FatJetTaggerScoreType::TopvsQCD:
            return "TopvsQCD";
        case FatJetTaggerScoreType::ZvsQCD:
            return "ZvsQCD";
        case FatJetTaggerScoreType::H4qvsQCD:
            return "H4qvsQCD";
        case FatJetTaggerScoreType::HbbvsQCD:
            return "HbbvsQCD";
        case FatJetTaggerScoreType::HccvsQCD:
            return "HccvsQCD";
        case FatJetTaggerScoreType::QCD0HF:
            return "QCD0HF";
        case FatJetTaggerScoreType::QCD1HF:
            return "QCD1HF";
        case FatJetTaggerScoreType::QCD2HF:
            return "QCD2HF";
        case FatJetTaggerScoreType::WVsQCD:
            return "WVsQCD";
        case FatJetTaggerScoreType::XbbVsQCD:
            return "XbbVsQCD";
        case FatJetTaggerScoreType::XccVsQCD:
            return "XccVsQCD";
        case FatJetTaggerScoreType::XggVsQCD:
            return "XggVsQCD";
        case FatJetTaggerScoreType::XqqVsQCD:
            return "XqqVsQCD";
        case FatJetTaggerScoreType::XteVsQCD:
            return "XteVsQCD";
        case FatJetTaggerScoreType::XtmVsQCD:
            return "XtmVsQCD";
        case FatJetTaggerScoreType::XttVsQCD:
            return "XttVsQCD";
        case FatJetTaggerScoreType::MassCorr:
            return "massCorr";
        case FatJetTaggerScoreType::MassCorrGeneric:
            return "massCorrGeneric";
        case FatJetTaggerScoreType::MassCorrX2p:
            return "massCorrX2p";
        default:
            throw SKNano::LogicError("[JetTagging::GetFatJetScore] No such score type");
        }
    }
    inline TString GetJetFlavTaggerScoreTypeStr(JetFlavTaggerScoreType type)
    {
        switch (type)
        {
        case JetFlavTaggerScoreType::B:
            return "B";
        case JetFlavTaggerScoreType::CvB:
            return "CvB";
        case JetFlavTaggerScoreType::CvL:
            return "CvL";
        case JetFlavTaggerScoreType::QvG:
            return "QvG";
        case JetFlavTaggerScoreType::CvNotB:
            return "CvNotB";
        case JetFlavTaggerScoreType::TauVJet:
            return "TauVJet";
        case JetFlavTaggerScoreType::Ele:
            return "Ele";
        case JetFlavTaggerScoreType::Mu:
            return "Mu";
        case JetFlavTaggerScoreType::SvCB:
            return "SvCB";
        case JetFlavTaggerScoreType::SvUDG:
            return "SvUDG";
        case JetFlavTaggerScoreType::probB:
            return "probB";
        case JetFlavTaggerScoreType::probBB:
            return "probBB";
        case JetFlavTaggerScoreType::probUDG:
            return "probUDG";
        default:
            throw SKNano::LogicError("[JetTagging::GetJetFlavTaggerScoreTypeStr] No such score type");
        }
    }
}

#endif
