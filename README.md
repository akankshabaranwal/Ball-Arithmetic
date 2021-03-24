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