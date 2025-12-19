#!/usr/bin/env bash

JSON="/data6/Users/yeonjoon/SKNANOAnalyzer_NanoV15/data/Run3_v15_Run2_v15/2024/Sample/CommonSampleInfo.json"
SCRIPT="ModellingPatch.py"
DEFAULT_WORKERS=48
TT_WORKERS=8
ERA=2024

# isMC == 1 인 항목들의 "키"만 뽑아서 순차 실행
jq -r 'to_entries[] | select(.value.isMC == 1) | .key' "$JSON" | while read -r KEY; do
    # KEY 안에 "TT"가 들어 있으면 worker 줄이기
    if [[ "$KEY" == *TT* ]]; then
        WORKERS="$TT_WORKERS"
    else
        WORKERS="$DEFAULT_WORKERS"
    fi

    echo "==== running $KEY (workers=$WORKERS) ===="
    python "$SCRIPT" --PD "$KEY" --era "$ERA" --workers "$WORKERS"

    # 실패하면 멈추고 싶으면 아래 주석 해제
    # if [ $? -ne 0 ]; then
    #     echo "Error running $KEY, stopping."
    #     exit 1
    # fi
done

echo "all done."