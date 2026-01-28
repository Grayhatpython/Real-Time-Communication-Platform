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
- 로그 전용 스레드 및 I/O 전용 스레드 ( epoll Dispatcher ) 분리 
- 부하 테스트 및 병목 분석
- 패킷 직렬화 및 패킷 핸들러

# Build (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build gdb

### Optional: Monitoring Server Dependencies

> The following packages are required **only for the monitoring server**
> that uses ImGui with OpenGL.
> They are **not required for server and client deployments**.

```bash
sudo apt install -y libgl1-mesa-dev
sudo apt install -y libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev


cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build


./build/Server/Server
./build/DummyClient/DummyClient
./build/MonitoringServer/MonitoringServer
