set(LIB_NAME "Engine")

# file(GLOB_RECURSE ...) 명령어는 지정된 디렉토리(여기서는 현재 CMakeLists.txt가 있는 디렉토리의 "src" 폴더) 아래의
# 모든 하위 디렉토리까지 재귀적으로 탐색하여 ".cpp" 확장자를 가진 모든 파일을 찾아서 SOURCES 변수에 저장
# GLOB는 편리하지만, 소스 파일이 추가되거나 삭제되었을 때 CMake를 수동으로 다시 실행해야 할 수 있습
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/engine/*.h")

# 'LIB_NAME' (즉, "ServerCore")이라는 이름의 라이브러리를 생성
# STATIC 키워드는 이 라이브러리가 정적 라이브러리(.lib 또는 .a)로 빌드됨을 의미
# 이 라이브러리를 만들기 위해 위에서 찾은 소스 파일들(${SOURCES})과 헤더 파일들(${HEADERS})을 사용
add_library(${LIB_NAME} STATIC ${SOURCES} ${HEADERS})

# --- 미리 컴파일된 헤더(Precompiled Headers, PCH) 설정 ---
# PCH로 사용할 메인 헤더 파일을 지정
# 이 파일은 일반적으로 프로젝트의 include 디렉토리 내에 위치하며, 자주 사용되는 모든 헤더들을 포함
target_precompile_headers(${LIB_NAME} PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/engine/EnginePch.h>"
)

# 'LIB_NAME' (engine) 라이브러리가 포함할(include) 디렉토리 경로를 설정
# PUBLIC 키워드는 이 라이브러리를 사용하는 다른 타겟(예: APP_NAME "Server")도 이 인클루드 경로를 상속받아 사용할 수 있음을 의미
# ${CMAKE_CURRENT_SOURCE_DIR}는 현재 CMakeLists.txt 파일이 있는 디렉토리의 경로 -> 이 라이브러리의 "include" 폴더
target_include_directories(${LIB_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# =========================
# spdlog (GitHub) FetchContent
# =========================
include(FetchContent)

# 필요 시 옵션 고정 (원치 않으면 제거 가능)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

# 헤더온리로 사용
target_link_libraries(${LIB_NAME} PUBLIC spdlog::spdlog_header_only)
target_compile_definitions(${LIB_NAME} PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
# target_link_libraries(${LIB_NAME} PRIVATE spdlog::spdlog)