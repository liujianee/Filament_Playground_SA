# Filament_Playground_SA

## Intro:  
This repo is a playground for Google's Filament rendering engine. Filament is lightweighted, fast, and very elegant in my opinion. I'll provide some standalone Filament samples in this repo. The samples rely on the released filament libraries, and I tried to keep the dependencies minimum. I hope in this way the Filament learners will find it easy to get started in their Filament journey.


## Environment:
The samples here should be working properly on MacOS. The default backend is Metal, while I've also tried OpenGL and Vulkan, which work fine.  I've also tried some samples inside a Vulkan docker container on a remote headless server (without display), and for this part I'll explain more in the future.


Before looking into the codes and building samples in this repo, I strongly suggest readers reading the official Filament repo first, and building the official examples from sources. By doing that readers can get familar with the libraries, dependencies (if any), and necessary assets used in this repo.


## Notes for CMakeLists:

### SDL2
SDL2 is used to create native windows and listen to some events in the Filament samples. In the official Filament repo, a SDL2 skeleton was implemented in FilamentAPP, which might look a bit complicated for new learners. So in this repo, I re-organized the SDL2 calls in each sample, making the samples "standalone" for easier understanding. 

My way of organization may not be optimal, it's just my attempts in disentangling the whole process. Suggestions for improvement are alway welcome.

1. Copy all the SDL2 header files into local dir for convenience. In CMakeLists.txt, these lines are added:
~~~
set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
include_directories(${SDL2_INCLUDE_DIRS})
~~~
Thess headers can be found in [here](https://github.com/google/filament/tree/main/third_party/libsdl2/include). (Or from a saperate SDL2 installtion I believe.)

2. 
