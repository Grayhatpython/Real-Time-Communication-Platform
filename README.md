# 🐧 epoll 기반 Linux Network Server (WIP)

본 프로젝트는 **Linux 환경에서 epoll 기반 네트워크 서버를 직접 설계·구현 중인 개인 학습 프로젝트**입니다.  
Windows IOCP 모델과 Linux epoll 모델의 차이를 비교·이해하고,  
**epoll 특성에 맞는 이벤트 디스패처와 서버 루프 구조를 단계적으로 구현**하는 것을 목표로 하고 있습니다.

> 🚧 **Work In Progress**  
> 현재 서버 코어 구조 및 epoll 이벤트 분기 로직을 중심으로 설계·구현을 진행 중입니다.

---

## 🎯 프로젝트 목적

- 커널 이벤트 통지 + 사용자 공간 제어 중심 설계
- recv / send 시점을 서버 로직이 명확히 통제
- 고부하 환경에서도 예측 가능한 흐름 유지
- 멀티스레드 확장을 고려한 구조

---

## 🧠 epoll 기반 서버 설계 개요 (진행 중)

### 1️⃣ epoll_ctl – 이벤트 관심사 등록
- 소켓 생성 후 epoll 인스턴스에 등록
- 기본적으로 `EPOLLIN | EPOLLET` 기반 설계
- 필요 시 송신 상황에 따라 `EPOLLOUT` 동적 등록

### 2️⃣ epoll_wait – 이벤트 대기
- 커널이 소켓 상태 변화를 감지할 때까지 블로킹
- 이벤트 발생 시 준비된 소켓 목록 반환
- 타임아웃 및 종료 조건 처리 가능

### 3️⃣ Event Dispatch – 이벤트 분기 처리
- epoll_wait 결과를 기반으로 이벤트 유형 분기
- 단순 recv/send 호출이 아닌 서버 로직 중심 디스패치

### 4️⃣ recv / send – 실제 I/O 수행
- Non-blocking 소켓 기반 직접 I/O 수행
- EAGAIN / EWOULDBLOCK 처리 필수
- 부분 송신/수신을 고려한 버퍼 설계

### 🔹 현재 구현 상태
- [ v ]epoll 기반 이벤트 루프 구조
- [ v ]non-blocking socket 처리
- [ v ]이벤트 디스패처 구조
- [ v ]송수신 처리
- [ v ]로그 전용 스레드 및 I/O 전용 스레드 ( epoll Dispatcher ) 분리 
- 패킷 직렬화 및 패킷 핸들러
- 부하 테스트 및 병목 분석

---

#### 🧪 테스트

1. 서버 프로세스 실행
2. Logger 및 Thread Local 초기화
3. NetworkCore 초기화 및 포트 리스닝 시작
4. Dispatcher(NetworkDispatch) 스레드 기동
5. 테스트 클라이언트 접속 시도
6. Session 생성 및 `ProcessConnect()` 처리 확인

---

#### 📄 실행 로그

<img width="1554" height="126" alt="스크린샷 2026-01-29 232813" src="https://github.com/user-attachments/assets/2445d8d7-cedd-41e8-ab43-814fe96adcbd" />

<img width="1556" height="130" alt="스크린샷 2026-01-29 232728" src="https://github.com/user-attachments/assets/bdb1e0ef-f69b-4755-9f69-d2636c228947" />

---

#### 🧠 로그 해석

| 로그 키워드 | 의미 |
|---|---|
| `Logger initialized` | spdlog 로거 초기화 완료 |
| `[Main] Initialize Thread Local` | 메인 스레드 Thread Local 초기화 완료 |
| `Network Core Initialized` | 네트워크 코어 구성 완료 |
| `Server port:8000 Started` | 서버 리스닝 시작(포트 8000) |
| `[NetworkDispatch] Thread Started` | Dispatcher 스레드 정상 기동 |
| `Session is Connected` | 클라이언트 접속 및 세션 생성/연결 처리 성공 |

---

#### ✅ 검증 결과 요약

- 서버가 정상적으로 기동되고 Logger/ThreadLocal 초기화가 수행됨
- Main 스레드와 NetworkDispatch 스레드가 분리되어 동작함
- 클라이언트 접속 이벤트가 Dispatcher 스레드에서 처리되며 `ProcessConnect()`까지 정상 수행됨
- async_logger 환경에서도 스레드 컨텍스트가 구분되어(태그/Thread ID) 동작 흐름을 추적 가능함

---

#### 📝 비고 (운영/확장 포인트)

- 접속/종료 이벤트는 **INFO**로 기록하여 운영 중 생명주기 파악이 가능하도록 설계
- 비정상 종료(예: unexpected disconnect)는 **WARN/ERROR**로 분리 예정
- 추후 검증 항목 확장:
  - 송수신(FlushSend/Recv) 이벤트 흐름 로그 추가
  - 부하 테스트 시 큐 적체/지연 징후 WARN 기준 정의

---

✨ External Libraries (Summary)
- spdlog
  : ServerCore 전반의 로깅을 담당하며, 네트워크/세션/스레드 생명주기 로그를 기록한다.
    async_logger 기반으로 멀티스레드 환경에서도 안정적인 로그 처리를 목표로 설계중

- Dear ImGui
  : 향후 모니터링 서버에서 실제 서버의 상태(세션 수, 트래픽, 내부 큐 상태 등)를
    실시간 UI 형태로 시각화하기 위한 디버그/모니터링 UI 용도로 사용 예정이다.
    (현재 단계에서는 Core 구조 설계 및 연동을 고려한 준비 상태)

- OpenGL / GLFW
  : 서버 기능 검증을 위한 클라이언트 테스트 및 시각적 확인 용도로 사용할 계획이다.
    렌더링 및 입력 처리는 테스트 목적에 한정하며,
    현재는 ServerCore 구현에 집중하고 있어 추후 단계에서 적용 예정이다.

---

✨ Build (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build gdb

### Optional: Monitoring Server Dependencies ( Opengl - Imgui )
### Optional: Game Client Dependencies ( SDL2 : TEMP)

> The following packages are required **only for the monitoring server**
> that uses ImGui with OpenGL.
> They are **not required for server and client deployments**.

```bash
sudo apt install -y libsdl2-dev
sudo apt install -y libsdl2-image-dev

sudo apt install -y libgl1-mesa-dev
sudo apt install -y libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build


./build/apps/gameserver/gameserver
./build/apps/gameclient/gameclient
