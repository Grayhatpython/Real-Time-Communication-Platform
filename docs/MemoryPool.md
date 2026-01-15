## Thread-Local Cache 기반 Memory Pool 설계 (TLS + Global Pool)

<img width="1241" height="1081" alt="스크린샷 2026-01-11 222140" src="https://github.com/user-attachments/assets/04c8f64b-3c71-4f66-9222-4e895671d507" />

본 메모리 풀은 **스레드 로컬 캐시(TLS: Thread Local Storage)** 와
**글로벌 메모리 풀(Global Pool)** 을 조합하여, 멀티스레드 환경에서의 메모리 할당/해제를 빠르게 처리하는 구조이다.

---

### ✅ 핵심 아이디어

- 각 스레드는 `thread_local` 영역에 **전용 메모리 버킷(= size class 별 캐시)** 을 보유한다.
- 자주 발생하는 작은 할당은 **TLS 캐시에서 락 없이 처리**한다.
- TLS 캐시에 블록이 부족하면 **Global Pool에서 배치(Refill)로 가져오며**, 이때만 락이 발생한다.
- Global Pool은 **size class(버킷)별 mutex**를 사용하여, 서로 다른 크기의 요청은 락 경쟁을 최소화한다.

---

### ✅ Size Class / Bucket 구성

- `S_CACHE_BUCKET_SIZE = 32` : size class(버킷) 개수
- `S_MIN_BLOCK_SIZE = 32`, `S_MAX_BLOCK_SIZE = 4096` 범위의 고정 크기 블록을 캐싱
- 요청 크기는 해당 구간 정책에 맞게 **라운딩(올림)** 되어 size class로 매핑된다.

예시(도식 기준):
- 32 ~ 256 바이트: **32B 단위**
  - 32, 64, 96, ..., 256
- 256 ~ 1024 바이트: **128B 단위**
  - 384, 512, ..., 1024
- 1024 ~ 4096 바이트: **512B 단위**
  - 1536, 2048, ..., 4096

> 참고: 4096B 초과 요청은 아직 처리하지 않음.

---

### ✅ Thread-Local Cache (TLS) 구조

각 스레드는 size class 별로 `Bucket`을 가진다.

- 각 `Bucket`은 같은 크기의 블록 포인터를 저장하는 캐시
- `S_MAX_CACHE_BLOCK_SIZE = 500` : **TLS 버킷 당 최대 캐시 개수(블록 개수)**

TLS 경로는 기본적으로 **락이 없다**(스레드 전용이므로).

---

### ✅ Global Pool 구조

Global Pool 역시 동일한 size class(버킷) 개념으로 **FreeList(중앙 캐시)** 를 유지한다.

- `S_MAX_FREELIST_COUNT = 32` : size class 개수
- `std::array<std::mutex, 32> locks;` 처럼 **버킷별 락**을 사용
- 자주 쓰이는 size class만 경쟁이 발생하며, 서로 다른 크기 요청끼리는 락을 공유하지 않는다.

---

## ✅ 할당(Allocate) 흐름

1) 사용자가 특정 크기의 메모리를 요청한다.  
2) 요청 크기를 size class로 매핑한다(라운딩/버킷 인덱스 계산).  
3) **TLS 버킷에 여유 블록이 있으면** → 락 없이 즉시 pop하여 반환한다.  
4) **TLS 버킷이 비어있으면** → Global Pool에 **배치 리필(Refill)** 을 요청한다.
   - `S_REFILL_BLOCK_CACHE_COUNT = 100` 개 단위로 가져와 TLS 버킷을 채운다.
   - 이 과정은 멀티스레드 경쟁이 가능하므로 **해당 size class mutex를 잠근다.**
5) 리필 후 TLS 버킷에서 블록을 반환한다(이후 같은 size class 요청은 락 없이 처리 가능).

---

## ✅ 해제(Free) 흐름

1) 해제 요청된 블록의 크기(size class)를 판별한다.  
2) **TLS 버킷에 여유가 있으면**(예: 현재 캐시 개수 < 500) → TLS에 push하여 보관한다(락 없음).  
3) **TLS 버킷이 이미 가득 찼다면** → Global Pool에 반환한다.
   - 이때는 멀티스레드 경쟁이 가능하므로 **해당 size class mutex를 잠근다.**

> 요약: “TLS에 담을 수 있으면 TLS로”, “넘치면 Global로” 정책

---

## ✅ 동시성(Concurrency) 관점 정리

- **fast path(대부분의 할당/해제)**: TLS에서 처리 → 락 없음
- **slow path(리필/오버플로 반환)**: Global Pool 접근 → size class별 락
- Global Pool이 버킷별 락을 가지므로,
  - 여러 스레드가 동시에 메모리를 요청하더라도 **요청 크기가 다르면 락 경쟁이 거의 없다**
  - 다만 **핫 사이즈(자주 쓰는 크기)** 에 요청이 몰리면 해당 버킷 락 경쟁이 집중될 수 있다.

---

## ✅ 개선/보완 아이디어 (TODO)

### 1) TLS 캐시 “과잉 보유” 메모리 회수 정책
현재 구조에서는 특정 스레드가 한 번 큰 트래픽을 겪고 나면,
TLS 버킷에 블록이 많이 남아 **장시간 사용되지 않는 메모리**가 발생할 수 있다.

개선 방향:
- 스레드 유휴/주기적 타이밍에 TLS 버킷을 스캔하여
- 일정 임계치 초과분을 Global Pool로 반환(또는 일부 해제)

### 2) 핫 size class 락 경쟁 완화
실제 워크로드에서 할당 크기는 특정 구간에 몰리는 경우가 많다.
그 결과 Global Pool의 일부 버킷만 지속적으로 락 경쟁이 발생할 수 있다.

개선 방향:
- 버킷을 더 세분화하거나(더 많은 size class)... 효과있을지는 모르겠음
- Refill/Return 배치 크기(현재는 100개로 고정)를 워크로드에 따라 동적으로 조정해, 핫 사이즈에서는 락 횟수를 줄이고 메모리 사용량을 안정화

---

## ✅ 사용 예시

```cpp
class Knight
{
public:
    Knight() = default;
    ~Knight() = default;

public:
    int hp = 3;
    int mp = 0;
};


for (int i = 0; i < 5; i++)
{
    serverCore::GThreadManager->Launch([]() {
        for (int i = 0; i < 100; i++)
        {
           auto k1 = serverCore::cnew<Knight>();
           serverCore::cdelete(k1);

           auto k2 = MakeShared<Knight>();
        }
        },"TestThread", false);
}

serverCore::GThreadManager->Join();
