# VPopLib

VPopLib is shared object (Linux) or dll (Windows) version of the VarroaPop model which contains all the model 
data elements and logic without any dependencies on the user interface/API or the operating system file
system.  VPopLib can be compiled for Windows or Linux and is intended to be compatible with a number of user
programming languages including C/C++, Python, and R.  

This Git repo contains all the code for VPopLib as well as the CMakeLists.txt and CMakeSettings.json which will 
support the build on either Linux or Windows.  This document will describe building on Windows using 
Microsoft Visual Studio 2019 as well as building on a Linux system using cmake and make.  *(At this time, the 
Visual Studio-generated .so library runs substantially faster than one built from the same sources directly on
my Linux system.  I suspect that is due to default build configuration differences between the two systems.)*  

## Building VPopLib

VpopLib code can be found at this git repository https://github.com/RobertCurry/VPopLib.
### Windows
Clone the git repository to a folder on your Windows development system.  Using MS Visual Studio 2019, 
open the folder containing the local repository.  The .json file should have both windows and linux 
configurations that can be build and deployed from Visual Studio 
(you will have to set up the connection to your remote linux system).

### Linux
Create a folder on the Linux system – for example: MyProject  
`$ mkdir MyProject`  
Move to that folder and clone the git repository  
`$ git clone https://github.com/RobertCurry/VPopLib`  
Create a build directory and navigate to it.  
`$ mkdir build`  
`$ cd build`  
Run cmake and make  
`$ cmake ..`  
`$ make`  
A shared object file will be created in MyProject/build called liblibvpop.so which will be the 
library you want to link with the using program.  *(Note- the .so library build in this fashion is 
smaller than the one built with Visual Studio but runs 5x slower??  Take a look at make options 
for optimization.  Just to confirm, I moved the liblibvpop.so that was generated in visual studio for 
Linux and compiled the user program with this and it runs 5x faster.  The MSVC library is 20mb vs the cmake 
version is 3 mb.  So maybe the vs version statically links some dependencies?? And the linux version
 keeps them shared.)*

## Creating an executable to use VPopLib

The interface for VPopLib is contained in the header file vpoplib.h.  This provides all the access necessary to 
load the model parameters, execute a simulation run, and get results.  

### Windows

In a simple example, imagine a small c++ project called VPLibUser which uses VPopLib to load values and execute a 
simulation run.  In order to compile, vpoplib.h must be in the "include" path for VPLibUser.  This can be done in
Visual Studio by adding the directory containing vpoplib.h to the Additional Include Directories field of
Project Properties->C/C++->General.	Alternately, vpoplib.h can be moved to the project header file folder.
You have to idenfity the location of the libvpop.lib file in order to link the library with VPLibUser.  
This can be done through Project Properties->Linker->General where you can add the directory containing
libvpop.lib to the Additional Library Directories field.  You will also have to go to Project Properties->Linker->Input
and add libvpop.lib to the Additional Dependencies field.  Once your VPLibUser program is compiled, you will 
need to copy libvpop.dll to the same directory as your executable so it can be found at run time (there
are other approaches to this including putting the .dll in your PATH environment variable).

### Linux

To build VPLibUser with linux, you need to have the location of the library liblibvpop.so (note the additional 
"lib" at the beginning of the filename.  You also need to have the vpoplib.h file in the include path and as
on Windows, it is easiest to put it in the same directory as your VPopLibUser header files.  Once that has been
completed, the following command will compile and link VPopLibUser:  

`g++  -Wall -v -o test vpoplibuser.cpp -L"/home/stratpilot/projects/TestVPLibLinuxBld/MyProject/VPopLib/build" -llibvpop`

Note that the path inside the double quotes after -L is where liblibvpop.so is found on my Ubuntu machine.



