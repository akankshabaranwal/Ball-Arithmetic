# Ball Arithmetic
Team 25: _Akanksha Baranwal, Gyorgy Rethy, Jiaqi Chen, Kouroche Bouchiat_

## Setup
### Linux (Ubuntu 18.04)
First you will need to clone the project:  
$ `git clone git@gitlab.inf.ethz.ch:COURSE-ASL/asl21/team25.git ball-arithmetic`

Next you need to install flint, get the arb sources, compile and install them.
All of this can be done manually or with the supplied script:  
$ `sudo ./install.sh`

Now you can build the project:  
$ `mkdir -p build && cd build`  
$ `cmake ..`  
$ `cmake --build .`

### Windows
TODO

## Benchmarking
To run benchmarks you will need to add benchamrking code to src/benchmark.c.
There are convenience wrappers for:
1. Turning off TurboBoost:  
    $ `./benchmark-mode.sh`  
   This will turn off frequency scaling on your CPU and set the frequency to the
   highest sustained frequency your CPU can handle.

2. Running benchmarks:  
   $ `./run-benchmarks.sh {iterations} {out}`  
   This will run our benchmarking code $iterations times (default is 5) and saves
   it to $out (default is benchmark.log). You do need to supply to file extension.
   Please keep in mind that these are positional arguments and should be used as such.
   BE AWARE it truncates the file it will output to before running, so make sure you don't 
   point it to important files.
   This script also assumes that your benchmarking binary is ./benchmark.bin.
   I personally created a symlink to my actual binary so that I can have my builds wherever I want them to be. :)  
   $ `ln -s your/build/target benchmark.bin`  
   Example: $ `./run-benchmark.sh 10 ./logs/portable_mul`  
   will run the benchmarking code 10 times and save the results in ./logs/portable_mul.log.

3. Plotting:  
   $ `python3 plot_benchmark.py {in}`  
   This will create plots from the file given in $in will output a plot to $in.png. 
   The default file is again benchmark.log and will produce a benchmark.png.  
   Example: $ `python3 plot_benchmark.py ./logs/portable_mul`  
   will generate a plot from the benchmark file we generated in the previous step 
   and put it into ./logs/portable_mul