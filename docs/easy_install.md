# SKNanoAnalyzer 간편 설치 가이드 (NanoAODv15)

[문서 목록](README.md)

## 빠른 시작

이미 micromamba가 설치되어 있다면:

```bash
# 1. 저장소 클론
git clone --recurse-submodules git@github.com:CMSSNU/SKNanoAnalyzer.git
cd SKNanoAnalyzer

# 2. Nano15 환경 생성 (ROOT 6.40.02)
micromamba create -n Nano15 -f docs/Nano-linux-64.lock -y

# 3. 설정 파일 생성
cp config/config.default config/config.$USER
# config/config.$USER 파일을 편집하여 아래 내용 수정:
#   [PACKAGE] mamba
#   [SINGULARITY_IMAGE] /your/path/to/Nano15.sif

# 4. setup.sh에서 환경 이름 수정
sed -i 's/micromamba activate Nano$/micromamba activate Nano15/' setup.sh

# 5. 환경 설정 및 빌드
source setup.sh
./scripts/build.sh --clean
```

---

## 상세 설치 과정

### 1. micromamba 설치

micromamba가 없다면 먼저 설치합니다.

```bash
"${SHELL}" <(curl -L micro.mamba.pm/install.sh)
```

설치 중 질문에 대한 답변:
- **Micromamba binary folder?**: `/data6/Users/$USER/micromamba_bin` (홈 디렉토리 사용 금지)
- **Init shell?**: `Y`
- **Configure conda-forge?**: `Y`
- **Prefix location?**: `/data6/Users/$USER/micromamba` (홈 디렉토리 사용 금지)

터미널 재시작 후 확인:
```bash
micromamba --version
```

### 2. Nano15 환경 생성

```bash
cd SKNanoAnalyzer
micromamba create -n Nano15 -f docs/Nano-linux-64.lock -y
```

설치되는 주요 패키지:
- ROOT 6.40.02
- GCC 13.3.0
- Python 3.12
- correctionlib, onnxruntime 등

### 3. 설정 파일 수정

`config/config.$USER` 파일 생성 및 편집:

```bash
cp config/config.default config/config.$USER
```

필수 설정 항목:
```
[PACKAGE] mamba
[SINGULARITY_IMAGE] /data6/Users/$USER/Nano15.sif
```

선택 설정 (텔레그램 알림):
```
[TOKEN_TELEGRAMBOT] your_bot_token
[USER_CHATID] your_chat_id
```

### 4. setup.sh 수정

환경 이름을 Nano15로 변경:

```bash
sed -i 's/micromamba activate Nano$/micromamba activate Nano15/' setup.sh
```

### 5. 빌드

```bash
source setup.sh
./scripts/build.sh --clean
```

또는 수동 빌드:
```bash
source setup.sh
mkdir -p build/redhat && cd build/redhat
cmake ../.. -DTorch_DIR=$SKNANO_HOME/external/libtorch/share/cmake/Torch \
            -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME/install/redhat
make -j8
make install
```

---

## Singularity 이미지 생성

배치 작업 실행을 위해 Singularity 이미지가 필요합니다.

### SKNanoAnalyzer.def 파일 생성

```def
Bootstrap: docker
From: almalinux:9

%labels
    DESCRIPTION "SKNanoAnalyzer environment with Nano15"

%post
    set -eux
    dnf -y install zsh bzip2
    mkdir -p /opt/conda && cd /opt/conda
    curl -Ls https://micro.mamba.pm/api/micromamba/linux-64/latest | tar -xvj bin/micromamba
    echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh
    echo 'export MAMBA_ROOT_PREFIX=/opt/conda' >> /etc/profile.d/conda.sh
    mkdir -p /opt/conda/envs
    ln -sfn /data6/Users/$USER/micromamba/envs/Nano15 /opt/conda/envs/Nano15 || true
    chown -R root:users /opt/conda && chmod -R 775 /opt/conda

%environment
    export CONDA_PREFIX=/opt/conda/envs/Nano15
    export PATH=$CONDA_PREFIX/bin:/opt/conda/bin:$PATH
    export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$CONDA_PREFIX/lib64:${LD_LIBRARY_PATH}
    export PYTHONNOUSERSITE=1

%runscript
    exec /bin/zsh
```

### 이미지 빌드

```bash
apptainer build Nano15.sif SKNanoAnalyzer.def
```

---

## 사용법

### 환경 활성화

```bash
cd /path/to/SKNanoAnalyzer
source setup.sh
```

### 작업 제출

```bash
SKNano.py -a ExampleRun -i DYJets -e 2022EE -n 10
```

### 테스트

```bash
python scripts/test.py
```

---

## 문제 해결

### RoccoR CMakeLists.txt 오류

ROOT 6.40에서 발생하는 호환성 문제:
```bash
cp templates/RoccoR/CMakeLists.txt external/RoccoR/
# external/RoccoR/CMakeLists.txt에서 target_link_libraries를 수정:
# target_link_libraries(RoccoR ${ROOT_LIBRARIES})
# -> target_link_libraries(RoccoR PUBLIC ${ROOT_LIBRARIES})
```

### git submodule 오류

gitlab.cern.ch SSH 접근 권한이 없는 경우, 기존에 클론된 디렉토리에서 복사:
```bash
cp -r /path/to/existing/external/RoccoR external/
cp -r /path/to/existing/external/jsonpog-integration external/
```

### libhwy 누락

```bash
micromamba install -n Nano15 libhwy -c conda-forge -y
```

---

## 디렉토리 구조

```
SKNanoAnalyzer/
├── build/redhat/          # 빌드 디렉토리
├── install/redhat/lib/    # 설치된 라이브러리
├── config/config.$USER    # 사용자 설정
├── external/              # 외부 의존성 (RoccoR, jsonpog 등)
├── docs/                  # 문서
└── scripts/               # 유틸리티 스크립트
```

---

## 환경 요약

| 항목 | 값 |
|------|-----|
| micromamba 환경 | Nano15 |
| ROOT 버전 | 6.40.02 |
| Python 버전 | 3.12 |
| C++ 표준 | C++20 |
| GCC 버전 | 13.3.0 |
