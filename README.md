# a-star

The purpose of this project is to learn more about the A* algorithm. Also since it was made for a school assignment, there might be a slightly excessive amount of comments, but should anyone want to learn more about the birb3d game engine, I guess this is a good resource for that.

https://github.com/Toasterbirb/a-star/raw/refs/heads/main/example.mp4

## Building
Since the game engine (birb3d) is included as a submodule, the repo should be cloned recursively.

After that create a build directory and build the project with cmake and make. The `-j` flag controls the amount of threads used for the build process. Since this will also compile the game engine before building the actual project, it might take a moment with less threads. It also should be noted that the game engine has several build time dependencies and those need to be installed aswell. CMake will notify about missing dependencies if there are any.
```
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```
At the end there should be a `a-star` binary that can be run.
