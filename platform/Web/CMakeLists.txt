# SW Chosen for web browsers (WebAssembly)
#
# The web build runs the iPad/Android code (EDUKE32_IOS via NOTBLOOD_WEB in compat.h): touch
# controls, gl4es for the OpenGL renderer (here on WebGL), the classic renderer through
# SDL_Renderer otherwise. The game folder is /notblood, kept in the browser (IndexedDB);
# webbits.cpp implements iosbits.h, platform/Web/shell/ is the page around it.
# See platform/Web/README.md and platform/Web/build.sh.
#
#   emcmake cmake -S platform/Web -B build-web -DGL4ES_SOURCE_DIR=/path/to/gl4es
#   cmake --build build-web -j

cmake_minimum_required(VERSION 3.20)
project(SWChosenWeb LANGUAGES C CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure with emcmake (Emscripten)")
endif()

set(GL4ES_SOURCE_DIR "" CACHE PATH "Path to a gl4es source tree (with platform/iOS/gl4es/*.patch applied)")

get_filename_component(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(SRC "${ROOT}/source")

# Emscripten's SDL2, Ogg and Vorbis ports (compile and link flags)
set(PORTS -sUSE_SDL=2 -sUSE_OGG=1 -sUSE_VORBIS=1)

if(NOT GL4ES_SOURCE_DIR OR NOT EXISTS "${GL4ES_SOURCE_DIR}/src/gl/gl4es.c")
    message(FATAL_ERROR "Set GL4ES_SOURCE_DIR to a gl4es source tree (see platform/Android/fetch-deps.sh)")
endif()
file(GLOB GL4ES_SOURCES "${GL4ES_SOURCE_DIR}/src/gl/*.c" "${GL4ES_SOURCE_DIR}/src/gl/wrap/*.c" "${GL4ES_SOURCE_DIR}/src/gl/math/*.c")
list(APPEND GL4ES_SOURCES "${GL4ES_SOURCE_DIR}/src/glx/hardext.c" "${ROOT}/platform/Android/app/jni/gl4es_info.c")
add_library(nb_gl4es STATIC ${GL4ES_SOURCES})
target_compile_definitions(nb_gl4es PRIVATE NOX11 EGL_NO_X11 NOEGL STATICLIB NO_GBM NO_INIT_CONSTRUCTOR DEFAULT_ES=2)
target_include_directories(nb_gl4es PRIVATE "${GL4ES_SOURCE_DIR}/include" "${GL4ES_SOURCE_DIR}/src" "${GL4ES_SOURCE_DIR}/src/glx")
target_compile_options(nb_gl4es PRIVATE -std=gnu11 -O2 -w)

set(NB_DEFINES
    NOTBLOOD_WEB
    RENDERTYPESDL=1
    SDL_TARGET=2
    USE_OPENGL
    SDL_MAIN_HANDLED
    NOASM
    HAVE_VORBIS
    HAVE_XMP
    NDEBUG
)
set(NB_INCLUDES
    "${SRC}/build/include"
    "${SRC}/mact/include"
    "${SRC}/audiolib/include"
    "${SRC}/glad/include"
    "${SRC}/imgui/include"
    "${SRC}/libsmackerdec/include"
    "${SRC}/hmpplay/include"
)
set(NB_C_FLAGS -funsigned-char -fno-strict-aliasing -O2 ${PORTS}
    -fno-exceptions -fno-rtti -fno-threadsafe-statics -Wno-everything)

function(nb_library name)
    add_library(${name} STATIC ${ARGN})
    target_compile_definitions(${name} PRIVATE ${NB_DEFINES})
    target_include_directories(${name} PRIVATE ${NB_INCLUDES})
    target_compile_options(${name} PRIVATE ${NB_C_FLAGS})
    set_target_properties(${name} PROPERTIES C_STANDARD 11 CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
endfunction()

file(GLOB ENGINE_SOURCES "${SRC}/build/src/*.c" "${SRC}/build/src/*.cpp")
set(ENGINE_EXCLUDE
    build.cpp config.cpp sdlayer12.cpp sdlkeytrans.cpp
    startgtk.editor.cpp startwin.editor.cpp
    gtkbits.cpp dynamicgtk.cpp winbits.cpp winlayer.cpp rawinput.cpp wiibits.cpp
    polymer.cpp animvpx.cpp androidbits.cpp webbits.cpp
)
foreach(f ${ENGINE_EXCLUDE})
    list(REMOVE_ITEM ENGINE_SOURCES "${SRC}/build/src/${f}")
endforeach()

foreach(shader polymost1Frag polymost1Vert)
    set(shader_src "${SRC}/build/src/${shader}.glsl")
    set(shader_out "${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader}.glsl.cpp")
    file(READ "${shader_src}" shader_text)
    file(WRITE "${shader_out}" "extern char const *${shader};\nchar const *${shader} = R\"shader(${shader_text})shader\";\n")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${shader_src}")
    list(APPEND ENGINE_SOURCES "${shader_out}")
endforeach()
list(APPEND ENGINE_SOURCES "${SRC}/glad/src/glad.c")

file(GLOB ENGINE_C_SOURCES "${SRC}/build/src/*.c")
set_source_files_properties(${ENGINE_C_SOURCES} "${SRC}/glad/src/glad.c" PROPERTIES LANGUAGE CXX)

nb_library(nb_engine ${ENGINE_SOURCES})

nb_library(nb_imgui
    "${SRC}/imgui/src/imgui.cpp"
    "${SRC}/imgui/src/imgui_demo.cpp"
    "${SRC}/imgui/src/imgui_draw.cpp"
    "${SRC}/imgui/src/imgui_tables.cpp"
    "${SRC}/imgui/src/imgui_widgets.cpp"
    "${SRC}/imgui/src/imgui_impl_sdl2.cpp"
    "${SRC}/imgui/src/imgui_impl_opengl3.cpp"
)
target_compile_definitions(nb_imgui PRIVATE IMGUI_IMPL_OPENGL_ES2)

file(GLOB MACT_SOURCES "${SRC}/mact/src/*.cpp")
nb_library(nb_mact ${MACT_SOURCES})

file(GLOB AUDIOLIB_SOURCES "${SRC}/audiolib/src/*.cpp")
foreach(f music_external.cpp driver_directsound.cpp driver_winmm.cpp driver_alsa.cpp)
    list(REMOVE_ITEM AUDIOLIB_SOURCES "${SRC}/audiolib/src/${f}")
endforeach()
nb_library(nb_audiolib ${AUDIOLIB_SOURCES})
target_include_directories(nb_audiolib PRIVATE "${SRC}/libxmp-lite/include")

file(GLOB XMP_SOURCES "${SRC}/libxmp-lite/src/*.c")
set_source_files_properties(${XMP_SOURCES} PROPERTIES LANGUAGE CXX)
nb_library(nb_xmp ${XMP_SOURCES})
target_compile_definitions(nb_xmp PRIVATE HAVE_ROUND LIBXMP_CORE_PLAYER LIBXMP_NO_PROWIZARD LIBXMP_NO_DEPACKERS BUILDING_STATIC)
target_include_directories(nb_xmp PRIVATE "${SRC}/libxmp-lite/include/libxmp-lite")

nb_library(nb_smacker
    "${SRC}/libsmackerdec/src/BitReader.cpp"
    "${SRC}/libsmackerdec/src/FileStream.cpp"
    "${SRC}/libsmackerdec/src/HuffmanVLC.cpp"
    "${SRC}/libsmackerdec/src/LogError.cpp"
    "${SRC}/libsmackerdec/src/SmackerDecoder.cpp"
)

nb_library(nb_hmpplay
    "${SRC}/hmpplay/src/hmpplay.cpp"
    "${SRC}/hmpplay/src/fmmidi3.cpp"
)

file(GLOB GAME_SOURCES "${SRC}/sw/src/*.cpp")
# the editor (Wangulator) and the Windows/GTK startup windows
foreach(f bldscript.cpp brooms.cpp jbhlp.cpp jnstub.cpp startgtk.game.cpp startwin.game.cpp)
    list(REMOVE_ITEM GAME_SOURCES "${SRC}/sw/src/${f}")
endforeach()
list(APPEND GAME_SOURCES "${SRC}/build/src/webbits.cpp")

add_executable(swchosen ${GAME_SOURCES})
target_compile_definitions(swchosen PRIVATE ${NB_DEFINES} CHOSEN_NO_TOUCH_DEVICES)
target_include_directories(swchosen PRIVATE ${NB_INCLUDES} "${SRC}/sw/src")
target_compile_options(swchosen PRIVATE ${NB_C_FLAGS})
set_target_properties(swchosen PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON SUFFIX ".js")

# ASYNCIFY: the engine's loops never return to the browser; webbits.cpp yields once a frame
# (and while a loop only waits), so the page stays responsive and the browser can draw.
target_link_options(swchosen PRIVATE
    ${PORTS}
    -O2
    -sASYNCIFY=1
    -sASYNCIFY_STACK_SIZE=262144
    -sSTACK_SIZE=2097152
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=134217728
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=1
    -sGL_ENABLE_GET_PROC_ADDRESS=1
    -sFULL_ES2=1
    -sMODULARIZE=1
    -sEXPORT_NAME=SWChosen
    -sEXIT_RUNTIME=0
    -sFORCE_FILESYSTEM=1
    -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,callMain,ccall
    -sENVIRONMENT=web
    -sINVOKE_RUN=0
    -lidbfs.js
    -lwebsocket.js
)
target_link_libraries(swchosen PRIVATE
    -Wl,--start-group nb_engine nb_mact nb_audiolib nb_xmp nb_smacker nb_hmpplay nb_imgui nb_gl4es -Wl,--end-group)
