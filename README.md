# cvgl
Simple C++ library for OpenCV-OpenGL interoperability  

This repository is mainly developed under Visual Studio 2017/Windows 10. To support further development in different platform, I am mainly working with cmake-based build. However, several libraries are still complex to handle, so I keep them as prebuilt binaries. This may bother the smooth porting to other platforms.  
Any suggestion or push request are welcomed.  

## Dependencies

### external: header-only libraries

These are included in this repository as git submodule.

- [GLM](https://glm.g-truc.net): 0.9.8  

### [3rdParty](https://github.com/stnoh/3rdParty): mandatory prebuilt binaries

These are also managed by git submodule, but it contains prebuilt binaries due to build complexity.  
Due to this, this repository does not support various OS environment.  

- GLFW: 3.3.0
- GLEW: 2.1.0
- OpenCV: 2.4.13.6
- AntTweakBar: 1.16b

### optional: optional prebuilt binaries

[TODO]
