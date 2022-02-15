cmake_minimum_required(VERSION 3.7)

project(Filament_PG)
set(CMAKE_CXX_STANDARD 17)

set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
set(FILAMENT_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/filament/include")

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${FILAMENT_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
#set(SDL2_LIBRARIES "filament/out/cmake-release/third_party/libsdl2/tnt/libsdl2.dylib")

set(FILAMENT_LIBS
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilament.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilamat.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libbackend.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libbluegl.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libbluevk.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilabridge.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilaflat.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libutils.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libgeometry.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libsmol-v.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libshaders.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libvkshaders.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libibl.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilament-iblprefilter.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libimage.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libfilameshio.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libmeshoptimizer.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libimageio.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libtinyexr.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libpng.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libz.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libastcenc.a"
        "${CMAKE_SOURCE_DIR}/filament/lib/x86_64/libEtcLib.a"
)

list(APPEND FILAMENT_LIBS "-framework Cocoa -framework QuartzCore -framework Metal -framework CoreVideo")

add_executable(helloquad src/helloquad.cpp src/NativeWindowHelperCocoa.mm)
target_link_libraries(helloquad ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(helloquad_color src/helloquad_color.cpp src/NativeWindowHelperCocoa.mm)
target_link_libraries(helloquad_color ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(helloquad_IBL src/helloquad_IBL.cpp src/NativeWindowHelperCocoa.mm src/IBL.cpp)
target_link_libraries(helloquad_IBL ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(hellocube_Standalone src/hellocube_Standalone.cpp src/NativeWindowHelperCocoa.mm src/IBL.cpp)
target_link_libraries(hellocube_Standalone ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(hellopbr_Standalone src/hellopbr_Standalone.cpp src/NativeWindowHelperCocoa.mm src/IBL.cpp)
target_link_libraries(hellopbr_Standalone ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(hellotexture_Standalone src/hellotexture_Standalone.cpp src/NativeWindowHelperCocoa.mm src/IBL.cpp)
target_link_libraries(hellotexture_Standalone ${SDL2_LIBRARIES} ${FILAMENT_LIBS})

add_executable(frameGenerator_Standalone src/frameGenerator_Standalone.cpp src/NativeWindowHelperCocoa.mm src/IBL.cpp)
target_link_libraries(frameGenerator_Standalone ${SDL2_LIBRARIES} ${FILAMENT_LIBS})


