# Filament_Playground_SA

<img src="https://github.com/liujianee/Filament_Playground_SA/blob/main/build/suzanne.gif" width="80%">

## Intro:  
This repo is a playground for Google's Filament rendering engine. Filament is lightweighted, fast, and very elegant in my opinion. I'll provide some standalone Filament samples in this repo. The samples rely on the released filament libraries, and I tried to keep the dependencies minimum. I hope in this way the Filament learners will find it easy to get started in their Filament journey.


## Environment:
The samples here should be working properly on MacOS. The default backend is Metal, while I've also tried OpenGL and Vulkan, which work fine.  I've also tried some samples inside a Vulkan docker container on a remote headless server (without display), and for this part I'll explain more in the future.


Before looking into the codes and building samples in this repo, I strongly suggest readers reading the official Filament repo first, and building the official examples from sources. By doing that readers can get familar with the libraries, dependencies (if any), and necessary assets used in this repo.


## Notes for CMakeLists:

### SDL2
SDL2 is used to create native windows and listen to some events in the Filament samples. In the official Filament repo, a SDL2 skeleton was implemented in FilamentAPP, which might look a bit complicated for new learners. So in this repo, I reorganized the SDL2 calls in each sample, making the samples "standalone" for easier understanding. 

My way of organization may not be optimal, it's just my attempts in disentangling the whole process. Suggestions for improvement are always welcome.

1. Copy all the SDL2 header files into local dir for convenience. These header files can be found in [here](https://github.com/google/filament/tree/main/third_party/libsdl2/include). (Or from a saperate SDL2 installtion I believe.). In CMakeLists.txt, these lines are added:
~~~
set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
include_directories(${SDL2_INCLUDE_DIRS})
~~~


2. Add SDL2 libraries:
~~~
find_package(SDL2 REQUIRED)
~~~
alterantively, we can directly find the SDL2 dynamic lib file and add it; (the dynamic lib file is from the Filament building outputs.)
~~~
set(SDL2_LIBRARIES "${FILAMENT_DIR}/filament/out/cmake-release/third_party/libsdl2/tnt/libsdl2.dylib")
~~~


### Filament

1. Download a released version of Filament, and extract to ./filament; the file structure is as below:
~~~
filament
│   README.md   
└───bin
│   └───assets
│       │   ...
└───include
│   └───backend
│       │   ...
└───lib
│   └───x86_64
~~~

2. Add necessary static library files into CMakeLists; (if any .a file is missing in the released bundle, try to find it in the Filament building outputs.)
~~~
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
~~~


### MacOS GL libs and frameworks
1. To run the samples on MacOs, we have to add the followinng frameworks:
~~~
list(APPEND FILAMENT_LIBS "-framework Cocoa -framework QuartzCore -framework Metal -framework CoreVideo")
~~~

2. Note in offical Filament repo, there is an objectivee-C++ file with functions to setup the NativeWinow, we need to add this file for proper display on MacOS. I just simply add it as a source to the CMake add_executable; (Btw, there are NativeWindowHelperLinux.cpp and NativeWindowHelperWindows.cpp for Linux and Windows, respectively.)
~~~
add_executable(helloquad src/helloquad.cpp src/NativeWindowHelperCocoa.mm)
~~~

3. link the libraries
~~~
target_link_libraries(helloquad ${SDL2_LIBRARIES} ${FILAMENT_LIBS})
~~~

## Build and Run
1. Following below for the building:
~~~
mkdir build
cd build
cmake ..
make
~~~

2. Try the executable as you wish.
