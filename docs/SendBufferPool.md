# 📦 SendBuffer / SendBufferPool 설계 문서 (TLS Arena + Pool + 수명 보장)

`SendBuffer`는 송신 패킷을 만들 때 발생하는 잦은 동적 할당을 줄이기 위해 만든 **재사용 가능한 연속 버퍼(Arena)** 입니다.  
`SendBufferPool`은 `SendBuffer` 객체를 재활용하는 **풀(Pool)** 이고,  
`SendBufferArena`는 스레드 로컬(TLS)로 “현재 작업 중인 SendBuffer”를 관리해 **빠르게 Allocate** 할 수 있도록 구성했습니다.

> ✅ 핵심 포인트  
> - **할당 비용 감소**: 패킷마다 new/delete 대신 풀에서 재사용  
> - **수명 안전**: `shared_ptr`로 “송신이 끝날 때까지 버퍼 생존” 보장  
> - **TLS 기반 성능**: 각 스레드가 자기 CurrentSendBuffer를 갖고 빠르게 할당

---

## 🎯 목표 (Why)

### 1) Heap 할당 최소화
송신 패킷을 만들 때마다 `new` / `delete`가 반복되면 성능이 떨어지고 파편화가 발생합니다.  
→ 큰 버퍼를 재사용하면서 필요한 만큼만 잘라 쓰는 방식으로 최적화합니다.

### 2) 비동기 송신에서도 버퍼 수명 보장
네트워크 송신은 즉시 완료되지 않을 수 있습니다.  
→ “send가 끝나기 전에 버퍼가 풀로 반환되거나 해제”되면 Use-after-free가 발생합니다.  
→ `SendBufferSegment`(또는 그에 준하는 객체)가 `shared_ptr<SendBuffer>`를 잡아 수명을 보장합니다.

### 3) 멀티스레드 환경에서 충돌 최소화
각 스레드가 TLS `CurrentSendBuffer`를 사용하면 “버퍼 내부 쓰기 경쟁”이 크게 줄어듭니다.

---

## 🧩 구성 요소

아래 구성으로 동작합니다.

```text
(여러 스레드)
    │ Allocate()
    ▼
[SendBufferArena (TLS)]
    │ 부족하면 Swap()
    ▼
[Current SendBuffer]
    │
    │ (shared_ptr로 수명 보장)
    ├──────────────────────────▶ [SendBufferSegment]
    │
    └── 풀에서 가져옴 / 반납
          ▼
     [SendBufferPool]
```

---

## 1) SendBuffer (연속 버퍼 / Arena)

### 역할
- 내부에 큰 연속 버퍼(`vector<BYTE>`)를 갖고
- `_usedSize`를 증가시키며 `Allocate(size)`로 연속 영역을 제공합니다.

### 핵심 동작 개념
- 남은 공간이 충분하면 `buffer + used`를 반환하고 `used += size`
- 부족하면 `nullptr`

> 장점  
> - 메모리 할당 없이 pointer 연산만으로 빠르게 영역 확보  
> - 패킷 생성 비용 감소

---

## 2) SendBufferPool (재사용 풀)

### 역할
- `SendBuffer` 객체를 보관했다가 필요할 때 꺼내 쓰고(`Pop`)
- 사용이 끝나면 다시 풀로 반환(`Push`)합니다.

### ⭐ 중요한 설계: **custom deleter 기반 자동 반납**
`SendBuffer`는 보통 `shared_ptr`로 다루며, refcount가 0이 되는 순간 자동으로 풀로 돌아가게 설계합니다.

- `shared_ptr<SendBuffer>(new SendBuffer(), Push)` 같은 형태의 **커스텀 deleter**를 사용
- `shared_ptr`이 소멸 → deleter(`Push`) 호출 → 풀로 복귀

---

## 3) SendBufferArena (TLS CurrentSendBuffer)

### 역할
- `thread_local CurrentSendBuffer`를 유지합니다.
- 요청한 `size`를 CurrentSendBuffer에서 바로 할당합니다.
- 공간이 부족하면 `SwapSendBuffer()`로 새 버퍼를 가져옵니다.

### Allocate 흐름
1) TLS CurrentSendBuffer가 없거나 공간 부족 → `SwapSendBuffer()`
2) `CurrentSendBuffer->Allocate(size)` 시도
3) 결과 포인터를 `SendBufferSegment`로 포장해서 반환  
   - **Segment가 shared_ptr<SendBuffer>를 들고 있어 수명 보장**

---

## 🔄 수명(Lifetime) 흐름 상세

### ✅ 정상 흐름
1) 스레드가 `SendBufferArena::Allocate(n)` 호출
2) `SendBufferSegment`가 만들어지고 내부에 `shared_ptr<SendBuffer>`를 보관
3) 송신이 끝나면 Segment가 파괴되면서 refcount 감소
4) refcount가 0이 되는 시점에 deleter가 호출되어 SendBuffer는 풀로 반환

### 왜 안전한가?
- “메모리의 소유권”이 `shared_ptr`에 묶여있기 때문에,
- 송신(또는 다른 스레드로 넘겨진 작업)이 끝나기 전까지 버퍼가 해제되지 않습니다.

---

## 🧵 동시성(멀티스레드) 관점

- `CurrentSendBuffer`는 TLS이므로 **각 스레드가 자기 버퍼를 독립적으로 사용**
- 풀(`SendBufferPool`)은 여러 스레드 접근 가능하므로 내부 자료구조 보호(락/원자)가 필요합니다.

> ✅ 효과  
> - “패킷 작성(Allocate + write)” 구간에서 충돌 감소  
> - 풀은 Pop/Push에서만 경합

---

## 🧯 종료(Shutdown) 처리

서버 종료 시에는 “풀 객체가 이미 파괴됐는데 deleter가 풀로 반환하려는 상황”이 생길 수 있습니다.  
이를 막기 위해 종료 플래그(`S_SendBufferDeleterRelease` 같은)로 동작을 분기합니다.

- 종료 플래그 OFF: deleter가 풀로 반환
- 종료 플래그 ON: deleter가 `delete`로 직접 해제

> 권장 운영 규칙  
> - 모든 워커/송신 스레드 join 후  
> - 마지막에 `SendBufferPoolClear()` 같은 정리 함수를 호출

---

## ✅ 사용 예시

```cpp
#pragma pack(push, 1)
struct TestPacket : PacketHeader
{
    uint64 playerId;
    uint64 playerMp;
};
#pragma pack(pop)

// 1. SendBuffer 할당
auto segment = servercore::SendBufferArena::Allocate(sizeof(TestPacket));

// 2. 패킷 구성
TestPacket* testPacket = reinterpret_cast<TestPacket*>(segment->ptr);
testPacket->id = 3;
testPacket->playerId = 3;
testPacket->playerMp = 6;
testPacket->size = sizeof(TestPacket);

// 3. SendContext 생성 및 전송 요청
auto sendContext = std::make_shared<servercore::SendContext>();
sendContext->sendBuffer = segment->sendBuffer;
sendContext->wsaBuf.buf = reinterpret_cast<CHAR*>(segment->ptr);
sendContext->wsaBuf.len = static_cast<ULONG>(sizeof(TestPacket));

session->Send(sendContext);
