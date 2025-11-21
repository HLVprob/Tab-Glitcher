cmake_minimum_required(VERSION 3.16)
project(TabGlitcher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let's turn on the modern Windows stuff
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)

# Where ImGui lives
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/imgui")

# All our source files go here
set(SOURCES
    src/main_imgui.cpp
    src/process_manager.cpp
    src/hotkey_manager.cpp
    TabGlitcher.rc
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# And the headers
set(HEADERS
    src/process_manager.h
    src/hotkey_manager.h
    ${IMGUI_DIR}/backends/imgui_impl_win32.h
    ${IMGUI_DIR}/backends/imgui_impl_dx11.h
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

# Tell CMake where to find the headers
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(src)

# Time to build the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link the necessary Windows libraries
target_link_libraries(${PROJECT_NAME} 
    user32
    kernel32
    gdi32
    comctl32
    dwmapi
    uxtheme
    d3d11
    d3dcompiler
    dxgi
)

# Make it a proper Windows app (no ugly console window popping up)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/MANIFEST:NO"
)
