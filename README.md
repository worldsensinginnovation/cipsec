**Welcome to SDRJD!**

GNU/Linux 
----------

 * Tested distributions: Ubuntu 16.04 LTS, Debian 8.0 "jessie"
 * Supported microprocessor architectures: 
   * i386: Intel x86 instruction set (32-bit microprocessors). 
   * amd64: also known as x86-64, the 64-bit version of the x86 instruction set, originally created by AMD and implemented by AMD, Intel, VIA and others.
   * armel: ARM embedded ABI, supported on ARM v4t and higher.
   * armhf: ARM hard float, ARMv7 + VFP3-D16 floating-point hardware extension + Thumb-2 instruction set and above. 
   * arm64: ARM 64 bits or ARMv8.
   * mips: MIPS architecture (big-endian, such as those manufactured by SGI).
   * mipsel: MIPS architecture (little-endian, such as Loongson 3).
   * mips64el: 64-bit version of MIPS architecture.
   * powerpc: the RISC 32-bit microprocessor architecture developed by IBM, Motorola (now Freescale) and Apple.
   * ppc64: 64-bit big-endian PowerPC architecture.
   * ppc64el: 64-bit little-endian PowerPC architecture.
   * s390x: IBM's S/390 architecture for mainframes.

Older distribution releases might work as well, but you will need GCC 4.7 or newer.

Before building SDRJD, you need to install all the required dependencies.

#### Alternative 1: Install dependencies using software packages ###############################################################

If you want to start building and running SDRJD as quick and easy as possible, the best option is to install all the required dependencies as binary packages. If you are using Debian 8, Ubuntu 14.10 or above, this can be done by copying and pasting the following line in a terminal:

~~~~~~ 
$ sudo apt-get install build-essential cmake git libboost-dev libboost-date-time-dev \
       libboost-system-dev libboost-filesystem-dev libboost-thread-dev libboost-chrono-dev \
       libboost-serialization-dev libboost-program-options-dev libboost-test-dev \
       liblog4cpp5-dev libuhd-dev gnuradio-dev gr-osmosdr libgflags-dev libgoogle-glog-dev
~~~~~~

#################################################################################################################################

#### Alternative 2: Manual installation of other required dependencies (not necessary if are installed using software packages)##

## Install GNU Radio using PyBOMBS

This option is adequate if you are interested in development, in working with the most recent versions of software dependencies, want more fine tuning on the installed versions, or simply in building everything from the scratch just for the fun of it. In such cases, we recommend to use PyBOMBS (Python Build Overlay Managed Bundle System), GNU Radio's meta-package manager tool that installs software from source, or whatever the local package manager is, that automatically does all the work for you. Please take a look at the configuration options and general PyBOMBS usage at https://github.com/gnuradio/pybombs. Here we provide a quick step-by-step tutorial.

First of all, install some basic packages:

$ sudo apt-get install git python-pip

Download, build and install PyBOMBS:

$ sudo pip install git+https://github.com/gnuradio/pybombs.git

Add some software recipes (i.e., instructions on how to install software dependencies):

$ pybombs recipes add gr-recipes git+https://github.com/gnuradio/gr-recipes.git
$ pybombs recipes add gr-etcetera git+https://github.com/gnuradio/gr-etcetera.git

Download, build and install GNU Radio, related drivers and some other extra modules into the directory /path/to/prefix (replace this path by your preferred one, for instance $HOME/sdr):

$ pybombs prefix init /path/to/prefix -a myprefix -R gnuradio-default

This will perform a local installation of the dependencies under /path/to/prefix, so they will not be visible when opening a new terminal. In order to make them available, you will need to set up the adequate environment variables:

$ cd /path/to/prefix
$ . ./setup_env.sh


## Install Gflags, a commandline flags processing module for C++ (https://github.com/gflags/gflags):

~~~~~~ 
$ wget https://github.com/gflags/gflags/archive/v2.1.2.tar.gz
$ tar xvfz v2.1.2.tar.gz
$ cd gflags-2.1.2
$ cmake -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_gflags_nothreads_LIB=OFF .
$ make
$ sudo make install
$ sudo ldconfig
~~~~~~ 

   

## Install Glog, a library that implements application-level logging (https://github.com/google/glog):

~~~~~~ 
$ wget https://github.com/google/glog/archive/v0.3.4.tar.gz 
$ tar xvfz v0.3.4.tar.gz 
$ cd glog-0.3.4
$ ./configure
$ make
$ sudo make install
$ sudo ldconfig
~~~~~~ 

#############################################################################################################################

#### Build and install SDRJD ################################################################################################

Go to SDRJD's build directory:

~~~~~~ 
$ cd sdrjd/build
~~~~~~ 

Configure and build the application:

~~~~~~ 
$ cmake ../
$ make
~~~~~~ 

By default, CMake will build the Release version, meaning that the compiler will generate a fast, optimized executable. This is the recommended build type when using a RF front-end and you need to attain real time. If working with a file (and thus without real-time constraints), you may want to obtain more information about the internals of the receiver, as well as more fine-grained logging. This can be done by building the Debug version, by doing:

~~~~~~ 
$ cmake -DCMAKE_BUILD_TYPE=Debug ../
$ make
~~~~~~ 

This will create four executables at sdrjd/install, namely ```sdrjd```, ```jammer_msg_rx```, ```volk_gnsssdr-config-info``` and ```volk_gnsssdr_profile```. 

You could be interested in creating the documentation by doing:

~~~~~~ 
$ make doc
~~~~~~ 

from the sdrjd/build folder. This will generate HTML documentation that can be retrieved pointing your browser of preference to build/docs/html/index.html. In order to generate the documentation you need to have installed doxygen, you can install it by doing:

~~~~~~ 
$ sudo apt-get install doxygen
~~~~~~ 

If a LaTeX installation is detected in your system,

~~~~~~ 
$ make pdfmanual
~~~~~~ 

will create a PDF manual at build/docs/SDRJD_manual.pdf. If you don't have LaTex, you can install it by doing:

~~~~~~ 
$ sudo apt-get install texlive-full
~~~~~~ 

Finally,

~~~~~~ 
$ make doc-clean
~~~~~~ 

will remove the content of previously-generated documentation.

SDRJD comes with a library which is a module of the Vector-Optimized Library of Kernels (so called VOLK_GNSSSDR) and a profiler that will build a config file for the best SIMD architecture for your processor. Run ```volk_gnsssdr_profile``` that is installed into ```$PREFIX/bin```. This program tests all known VOLK kernels for each architecture supported by the processor. When finished, it will write to ```$HOME/.volk_gnsssdr/volk_gnsssdr_config``` the best architecture for the VOLK function. This file is read when using a function to know the best version of the function to execute. It mimics GNU Radio's VOLK (http://libvolk.org/) library, so if you still have not run ```volk_profile```, this is a good moment to do so. 

