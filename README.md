# NFC LP Table

NU40DK + ILI9341 + RC522 + DFPlayer Mini용 독립 LP 플레이어 프로젝트입니다.
기존 TFT 가사 데모·가사 편집기·AAF 도구는 포함하지 않습니다.
이 폴더 전체를 옮겨도 원래 프로젝트의 파일을 읽지 않습니다.

## 주요 기능

- NFC 스티커의 UID를 미리 등록한 곡/음원 번호/커버와 연결
- 앨범 커버·노래 제목·가수·진행 상태·음량 표시
- 실제 MP3 분석에 기반한 선형 크기 스펙트럼 20대역 막대
- P1.05 및 화면 버튼으로 재생/일시정지, P1.06/07 및 화면 +/-로 음량
- UID 인식 시 커버·곡명을 표시하고 재생 버튼 입력 대기 (자동 재생 없음)
- 재생 전 마지막 UID 인식 후 1분간 재인식이 없으면 선택 초기화 및 기본 화면 복귀
- 재생·일시정지 중 UID 인식 공백은 무시하며 버튼으로 이어 재생
- 다른 등록 UID 인식 시 기존 재생을 정지하고 새 곡 선택 후 버튼 입력 대기
- 곡 종료 시 선택과 진행 위치를 초기화하고 텍스트 기본 화면으로 복귀
- 미등록 UID는 표시/출력하고 재생하지 않음
- 이전/다음 곡·셔플·반복 버튼 없음

개인 음원·커버·UID 카탈로그와 생성된 펌웨어·프리뷰·빌드·복구 파일은 저장소에 포함하지 않습니다.
버튼 입력 코드는 있으나 실제 버튼·모터는 아직 추가되지 않았고, 모터 제어도 미구현입니다.
새 LP 펌웨어는 보드에 업로드하지 않았습니다.

## 폴더

```
firmware/lp_player/        업로드할 lp_player.ino + lp_assets.h
projects/lp-player/
  catalog.json            모든 UID와 곡 등록표
  covers/                 앨범 커버 원본
  audio/                  원본 음원
  sd-card/mp3/             microSD에 복사할 0001.mp3, 0002.mp3...
  preview/                화면 PNG·동작 GIF
  build/                  컴파일 결과·로그
projects/recovery/        기존 설치 펌웨어 복구용 바이너리(런타임 미사용)
tools/lp-player/           화면·재생 상태·오디오·터치·스펙트럼 원본
tools/*lp*                등록·생성·검사·테스트·프리뷰·컴파일 도구
tests/                    신호 분석·UID·버튼·오디오·렌더링 검증
config/                   배선 정보와 복수 등록 예시
```

생성된 .ino/.h를 직접 고치면 다음 생성 때 덮어씁니다. 원본은 tools/lp-player/
아래에서 수정하고 다시 생성하세요. 커버·스펙트럼·등록표 변경은 기존 출력물을
무효화하며, lp:check가 최신 여부를 검사합니다.

## 개발 환경

현재 macOS 기준입니다. Python 3.11+, Node.js 18+, ffmpeg/ffprobe, C++ 컴파일러,
Arduino CLI와 NU40DK 코어(nucode:nrf52:nu40dk)가 필요합니다. Python 환경은 이
프로젝트 안에 새로 만듭니다. 기존 프로젝트의 .venv는 사용하지 않습니다.

```sh
python3.11 -m venv .venv
.venv/bin/python -m pip install -r tools/lp-player/requirements-lock.txt
arduino-cli lib install 'Adafruit GFX Library' 'Adafruit ILI9341' MFRC522@1.4.12
```

이미 설치된 NU40 코어/Arduino 라이브러리 같은 시스템 개발 도구는 공유할 수
있습니다. 에셋 생성 시 macOS의 Apple SD Gothic Neo/Menlo 폰트를 사용합니다.
생성된 펌웨어에는 글자/이미지가 포함되므로 실행 시 폰트 파일은 필요 없습니다.

## 곡 추가

새로 clone한 경우 등록표는 첫 `lp:add` 실행 시 생성됩니다. 최소 한 곡을 등록한 뒤
생성·테스트·컴파일하세요. 음원과 커버는 직접 준비해야 합니다.

아래 UID와 파일 경로를 실제 값으로 바꾸세요.

```sh
npm run lp:add -- \
  --id song-two \
  --uid 04AABBCCDDEEFF \
  --audio "/absolute/path/song.mp3" \
  --cover "/absolute/path/cover.jpg" \
  --title "노래 제목" \
  --artist "가수명"
npm run lp:list
```

원본 파일을 전용 audio/covers 폴더에 복사하고, 다음 빈 MP3 번호를 배정합니다.
UID·곡 ID·번호 중복을 확인하며, 기존 곡 수정은 --replace를 명시해야 합니다.
등록표 백업이 자동 생성되고 원본 음원/이미지는 이동·삭제하지 않습니다.
별도 기기 등록 메뉴는 없으며 제작 전에 PC에서 등록하는 구조입니다.

## 생성·검증·컴파일

프로젝트 최상위에서 실행합니다.

```sh
npm run lp:generate
npm run lp:test
npm run lp:preview
npm run lp:compile
```

`npm run lp:sd`는 생성과 함께 SD 복사용 mp3 폴더를 로컬에 준비합니다.
DFPlayer 전원을 끄고 microSD를 컴퓨터 카드리더기에 연결한 뒤,
sd-card/mp3 폴더를 FAT16/FAT32 카드 최상위로 직접 복사하세요.
어떤 도구도 물리 SD를 자동 포맷/복사하거나 보드에 자동 업로드하지 않습니다.

업로드는 별도로 요청/실행하며, 연결된 대상과 복구 경로를 먼저 확인해야 합니다.
기존 장치 설치본의 복구 zip과 기록은 원래 로컬 작업 폴더의 projects/recovery/에
보존되어 있으며 Git 저장소에는 포함하지 않습니다.

## 하드웨어 검증 범위

프리뷰는 펌웨어와 같은 C++ 렌더러의 결과지만 실제 TFT 속도/터치/소리를 검증하지
않습니다. 스펙트럼은 MP3를 미리 FFT 분석한 데이터이며 실시간 DFPlayer 출력 측정이
아닙니다. 진행 시각은 UART 명령 시각 기준 추정으로 실제 오디오 지연은 확인이
필요합니다. 곡별 커버/스펙트럼은 Flash 용량을 사용하므로 5~6곡 실제 데이터를
넣은 뒤 최종 용량을 확인하세요. 배선은 docs/WIRING.md를 보세요.

일시정지 후 LP만 제거하는 상황은 별도 유무 센서 없이 구분할 수 없습니다.
현재 곡 종료는 등록된 음원 길이 기준이며 DFPlayer 완료 응답 처리, 실제 모터 제어와
회전 중 RFID 진단 로그는 후속 작업입니다.
