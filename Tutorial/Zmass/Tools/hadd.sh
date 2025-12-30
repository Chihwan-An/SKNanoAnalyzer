#!/bin/bash

DATASTREAM=("Muon0" "Muon1")
BASE_PATH="/gv0/Users/$USER/SKNanoOutput/DY/2023"

# 파일 경로 배열 생성
input_files_DATA=()
for stream in "${DATASTREAM[@]}"; do
    input_files_DATA+=("${BASE_PATH}/${stream}.root")
done

# VV 샘플 직접 지정
VV_samples=("WW_pythia" "WZ_pythia" "ZZ_pythia")
VV=()
for mc in "${VV_samples[@]}"; do
    VV+=("${BASE_PATH}/${mc}.root")
done

# ST 샘플 직접 지정
ST_samples=("ST_sch_top_Lep" "ST_tW_top_Semilep" "ST_tW_antitop_Semilep" "ST_tW_top_Lep" "ST_tW_antitop_Lep")
ST=()
for mc in "${ST_samples[@]}"; do
    ST+=("${BASE_PATH}/${mc}.root")
done

# DATA
hadd -j 8 ${BASE_PATH}/Muon.root "${input_files_DATA[@]}"

# MC

## VV
hadd -j 8 ${BASE_PATH}/VV.root "${VV[@]}" 

## ST
hadd -j 8 ${BASE_PATH}/ST.root "${ST[@]}"