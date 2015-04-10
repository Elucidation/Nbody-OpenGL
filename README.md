# Play
---
This is a sandbox for practicing OpenGL using [GLFW](www.glfw.org)



## Compiling

Use CMake to generate the makefile with properly linked libraries in `build/` (If it doesn't exist create it using `mkdir build/`)

Then cd into build and call `cmake ..` to generate the makefile

    mkdir build/
    cd build/
    cmake ..

## Building
Then, build the executable using `make`


## Running
    ./play


## Troubleshooting

If you get the following error;

    ./play: error while loading shared libraries: libglfw.so.3: cannot open shared object file: No such file or directory

Run

    sudo ldconfig
