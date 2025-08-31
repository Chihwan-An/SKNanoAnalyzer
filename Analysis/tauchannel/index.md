---
layout: post
title: Tau Channel
date: 2025-08-13 13:11:00 +0900
categories: lrsm tauchannel
permalink: /categories/lrsm/tauchannel/
heading: "Tau Channel"
subheading: "LRSM Tau Channel Analysis"
---

# Tau Channel

LRSM에서 tau lepton을 포함하는 채널 분석을 다룹니다.

## 분석 개요
Heavy neutrino가 tau lepton과 함께 생성되는 과정을 연구합니다.
- **W_R → τN** decay mode 분석
- Tau identification과 reconstruction
- Hadronic vs leptonic tau decay 구분

## 주요 특징
### Signal Process
- **pp → W_R → τ + N_R** 생성 과정  
- **N_R → l + W_R^* → l + jj** 붕괴
- Final state: **τ + l + jj**

### Tau Decay Modes
- **Hadronic tau**: τ → hadrons + ν_τ (~65%)
- **Leptonic tau**: τ → l + ν_l + ν_τ (~35%)
- **Mixed channels**: τ_h + μ, τ_h + e

## 분석 전략  
### Tau Identification
- **Hadron-Plus-Strips (HPS)**: CMS tau algorithm
- **Decay mode finding**: 1-prong, 3-prong 구분
- **Isolation requirements**: ΔR < 0.5 cone
- **Anti-electron/muon**: Fake tau rejection

### Event Selection
- At least one identified tau (p_T > 20 GeV)
- One additional isolated lepton
- Two or more jets from W_R decay
- Missing transverse energy from neutrinos

## 배경 사건
### Main Backgrounds
- **Z → ττ**: Irreducible background
- **W + jets**: Jet → τ fake rate
- **QCD multijet**: Jet → lepton fake
- **Diboson**: WW, WZ, ZZ production

### Background Estimation
- **Fake factor method**: Data-driven τ fake estimation
- **MC-based**: Z → ττ normalization
- **ABCD method**: QCD background estimation

## 체계적 불확실성
- Tau energy scale (3-5%)
- Tau identification efficiency (5-8%)
- Jet energy scale and resolution
- Luminosity and cross-section uncertainties