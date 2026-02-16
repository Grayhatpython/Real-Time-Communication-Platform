# 프로젝트 이름을 'client'로 설정
# 이 변수는 나중에 빌드될 실행 파일의 이름으로 사용
set(APP_NAME "client")

# --- ImGui 관련 설정 ---
# ImGui 라이브러리 소스 파일이 위치한 디렉토리를 IMGUI_DIR 변수로 설정
# ${CMAKE_SOURCE_DIR}는 현재 CMake 프로젝트의 최상위 소스 디렉토리 경로
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)

# ImGui 라이브러리에 필요한 모든 소스 파일들을 IMGUI_SOURCES 변수에 명시적으로 나열
set(IMGUI_SOURCES 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)
# --- ImGui 관련 설정 ---

# src 디렉토리의 모든 .cpp 파일을 SOURCES 변수에 담기
file(GLOB_RECURSE SOURCES "src/*.cpp")

# include 디렉토리의 모든 .h 파일을 HEADERS 변수에 담기
file(GLOB_RECURSE HEADERS "include/*.h")

# 소스 파일과 헤더 파일을 함께 타겟에 추가
add_executable(${APP_NAME} ${IMGUI_SOURCES} ${SOURCES} ${HEADERS})

# --- 미리 컴파일된 헤더(Precompiled Headers, PCH) 설정 ---
# PCH로 사용할 메인 헤더 파일을 지정
# 이 파일은 일반적으로 프로젝트의 include 디렉토리 내에 위치하며, 자주 사용되는 모든 헤더들을 포함
target_precompile_headers(${APP_NAME} PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/Pch.h>"
)

# 생성된 실행 파일(`APP_NAME` = client)에 필요한 라이브러리들을 링크(연결)
# PRIVATE 키워드는 이 라이브러리들이 `APP_NAME`이라는 타겟 자체에만 필요하고,
# 이 타겟에 의존하는 다른 타겟들에게는 노출되지 않음을 의미
target_link_libraries(${APP_NAME} PRIVATE 
    Engine
    Network
    glfw                    # GLFW 라이브러리
    ${OPENGL_LIBRARIES}     # CMake의 find_package(OpenGL) 명령으로 찾아진 OpenGL 관련 라이브러리들
)

target_link_libraries(${APP_NAME} PRIVATE nlohmann_json::nlohmann_json)

# 타겟에 인클루드 디렉토리 추가
target_include_directories(${APP_NAME} PRIVATE
    ${IMGUI_DIR}                                # ImGui 라이브러리의 헤더 파일 경로
    ${OPENGL_INCLUDE_DIR}                       # CMake의 find_package(OpenGL)로 찾아진 OpenGL 헤더 파일 경로
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
