#!/usr/bin/env python3
"""muon_HighPt.json.gz 의 키·입력변수·유효범위를 확인한다.

C++ 빌드 없이 도는 검증. MyCorrection 이 이 JSON 을 어떤 인자로 부르는지가
여기 적힌 계약과 어긋나면 런타임에 correctionlib 이 throw 한다.

실행:
    python3 scripts/checks/check_muon_corrections.py
"""
import os
import sys

import correctionlib

ERA_TAG = "Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15"

# (키, 운동량 입력 이름). RECO 만 p(전체 운동량)이고 나머지는 pt 다.
# 이 구분을 틀리면 조용히 잘못된 bin 의 SF 가 적용된다.
REQUIRED_KEYS = [
    ("NUM_HighPtID_DEN_GlobalMuonProbes", "pt"),
    ("NUM_GlobalMuons_DEN_TrackerMuonProbes", "p"),
    ("NUM_HLT_DEN_HighPtLooseRelIsoProbes", "pt"),
    ("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", "pt"),
]

# 네 키 모두 운동량 하한이 50 이다. C++ 쪽 clamp 가 이보다 낮으면 throw 한다.
MOMENTUM_MIN = 50.0
# eta 상한. 네 키 모두 2.4 이고 JSON 이 내부에서 abs 를 건다.
ETA_MAX = 2.4
VALTYPES = ("nominal", "systup", "systdown")


def main() -> int:
    repo = os.environ.get("SKNANO_HOME", os.getcwd())
    path = os.path.join(
        repo, "CMS_corrections", "MUO", ERA_TAG, "latest", "muon_HighPt.json.gz"
    )
    if not os.path.exists(path):
        print(f"FAIL: missing {path}")
        return 1

    cset = correctionlib.CorrectionSet.from_file(path)
    failures = []

    for key, momentum_input in REQUIRED_KEYS:
        if key not in cset:
            failures.append(f"{key}: missing from correction set")
            continue

        corr = cset[key]
        names = [inp.name for inp in corr.inputs]
        if momentum_input not in names:
            failures.append(
                f"{key}: expected momentum input {momentum_input!r}, got {names}"
            )
            continue

        # 대표값 평가. 세 valtype 이 모두 있어야 systematic 을 만들 수 있다.
        try:
            values = [corr.evaluate(1.0, 300.0, v) for v in VALTYPES]
        except Exception as exc:  # noqa: BLE001 - 진단 목적
            failures.append(f"{key}: evaluate raised {exc}")
            continue

        nominal, up, down = values
        if not 0.5 < nominal < 1.5:
            failures.append(f"{key}: implausible nominal {nominal}")
        elif not down <= nominal <= up:
            failures.append(
                f"{key}: variations do not bracket nominal "
                f"(down={down:.4f} nom={nominal:.4f} up={up:.4f})"
            )
        else:
            print(
                f"OK  {key} [{momentum_input}] "
                f"nom={nominal:.4f} up={up:.4f} down={down:.4f}"
            )

        # 하한 아래를 요청하면 실제로 throw 하는지 확인한다. C++ clamp 가
        # 왜 필요한지에 대한 근거이므로, throw 하지 않으면 clamp 전제가 틀린 것이다.
        try:
            corr.evaluate(1.0, MOMENTUM_MIN - 10.0, "nominal")
        except Exception:
            pass
        else:
            failures.append(
                f"{key}: evaluating below {MOMENTUM_MIN} unexpectedly succeeded; "
                "the C++ clamp assumption needs revisiting"
            )

        # eta 상한 밖도 마찬가지.
        try:
            corr.evaluate(ETA_MAX + 0.5, 300.0, "nominal")
        except Exception:
            pass
        else:
            failures.append(
                f"{key}: evaluating beyond |eta|={ETA_MAX} unexpectedly succeeded"
            )

    if failures:
        for line in failures:
            print(f"FAIL: {line}")
        return 1

    print(
        f"all {len(REQUIRED_KEYS)} muon HighPt keys OK "
        f"(momentum clamp {MOMENTUM_MIN}, |eta| clamp {ETA_MAX})"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
