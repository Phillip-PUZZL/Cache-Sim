Cache Simulator Project for COSC-6310.501

To run the build.bat script to build the application, you will need to have a working C compiler, CMake > 3.30,
and Ninja installed on your machine. Then you can simply run the batch script.

If you decide to build the project yourself, in order to use the run_tests.bat script you will need to ensure that
your build path creates the executable at "cmake-build-release/src/cache_sim.exe" and manually create the "output"
folder both at this project's root folder.

Both scripts should be run with the root folder of this project as the working directory.

All test outputs are placed into the "output" folder when running the run_tests.bat script.

Usage of the application can be seen using the -h flag.