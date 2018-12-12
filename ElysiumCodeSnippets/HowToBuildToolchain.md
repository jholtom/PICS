The MSP430 architecture is fully supported by GCC, you just need to build a cross-compiler toolchain for it. 
I recommend that you create a new directory in which to perform these steps.

Start by downloading the sources for binutils (version 2.27), GCC (version 6.2), and GDB (version 7.12) from GNU. 

```
wget https://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.gz
wget https://ftp.gnu.org/gnu/gdb/gdb-7.12.tar.gz
```

Also download the sources for Newlib (version 2.5).

```
wget ftp://sourceware.org/pub/newlib/newlib-2.5.0.tar.gz
```

Extract them all.

```
tar -zxf binutils-2.27.tar.gz
tar -zxf gcc-6.2.0.tar.gz
tar -zxf gdb-7.12.tar.gz
tar -zxf newlib-2.5.0.tar.gz
```

Now we'll build each of these programs, targeting them to the msp430-elf backend.

Some terms you should be familiar with. A PREFIX is the location where the programs will be installed after being compiled. You get to choose your own PREFIX, but you should make sure to add it to your PATH environment variable so that later build steps can find files from earlier build steps. The HOST is the machine the toolchain is being **run on** - this can usually be autodetected. The TARGET is the architecture the toolchain will **compile for** - this is msp430-elf in this case. We will pass PREFIX and TARGET to configure below to make sure that our toolchain is configured correctly.

Before you start these steps, you'll want to make sure that your Linux machine has the required packages installed to build these programs. Check the package documentation for a complete list of the dependencies.

Now we start compiling. We start with binutils. In addition to the prefix and target, we also pass --disable-nls, which disables native language support, speeding up compilation for native English speakers.

```
export PREFIX=/home/awygle/toolchain/install # for example
export TARGET=msp430-elf
export PATH=$PREFIX/bin:$PATH # add $PREFIX/bin to your PATH variable
mkdir build-binutils # all of these programs expect out-of-tree builds
cd build-binutils
../binutils-2.27/configure --prefix=$PREFIX --target=$TARGET --disable-nls
make all # you can use 'make -j4 all' to run four compilation threads in parallel if you like
make install
```

Next we build the **host** parts of GCC. GCC needs to be built in two parts, **host** and **target**. The **host** section includes all the parts of GCC which only depend on the host machine, while the **target** section includes target-specific code generation. We build the **target** section after Newlib as there are some inter-dependencies. In addition to prefix and target we pass several flags to GCC's configure script. --without-headers tells GCC that we don't have a C runtime library for the target yet. --with-newlib tells GCC that we're planning to build Newlib as the C library for the target. --enable-languages chooses which of the languages that GCC supports to build. Only 'c' is necessary but we build c++ as well just in case.

```
cd .. # return to working directory
mkdir build-gcc
cd build-gcc
../gcc-6.3.0/configure --prefix=$PREFIX --target=$TARGET --without-headers --with-newlib --enable-languages=c,c++
make -j4 all-host
make install-host
```

The next bit to build is Newlib but there is one change we need to make to the source first. Newlib unfortunately doesn't properly detect whether to use the hardware multiplier for the MSP430. The Elysium uses an MSP430FR5969 which has an 'f5series' hardware multiplier. Newlib by default will build without hardware multiply support. Since we want to use the hardware multiplier, we'll change a line in newlib-2.5.0/newlib/configure.host.

```
cd ..
vim newlib-2.5.0/newlib/configure.host # change line 236 to say '-mhwmult=f5series' instead of '-mhwmult=none'
mkdir build-newlib
cd build-newlib
../newlib-2.5.0/configure --prefix=$PREFIX --target=$TARGET
make -j4
make install
```

Now that we've built Newlib we're ready to build the target-specific parts of GCC.

```
cd ../build-gcc
make all-target
make install-target
```

Finally, we build GDB so that we can debug MSP430 programs. We enable Python scripting for GDB to enable running some interesting tests later on.

```
cd ..
mkdir build-gdb
cd build-gdb
../gdb-7.12.1/configure --prefix=$PREFIX --target=$TARGET --with-python
make -j4
make install
```

That concludes the cross-compile toolchain but there are still a couple of utilities that are needed to work with the MSP in a useful way. The first of these is TI's MSP debug stack. It can be found at http://www.ti.com/tool/mspds - you want the MSPDS-OPEN-SOURCE package. Download the .zip (slac460s.zip) to your working directory and extract it.

```
mkdir MSPDebugStack_OS_Package
cd MSPDebugStack_OS_Package
unzip ../slac460s.zip
```

The debug stack depends on hidapi, which can be found on Github at https://github.com/downloads/signal11/hidapi/hidapi-0.7.0.zip . Download this to your working directory as well, then extract it in the MSPDebugStack_OS_Package folder under ThirdParty.

```
cd ..
wget https://github.com/downloads/signal11/hidapi/hidapi-0.7.0.zip
cd MSPDebugStack_OS_Package/ThirdParty/
unzip ../../hidapi-0.7.0.zip
```

hidapi needs a small change to the Makefile to build properly. Open hidapi-0.7.0/linux/Makefile in your text editor and find the line that reads: 
```
LIBS      = `pkg-config libusb-1.0 libudev --libs`
```
and add '-pthread' to the end so that it reads
```
 LIBS      = `pkg-config libusb-1.0 libudev --libs` -pthread
 ```
 Now build hidapi:
  ```
 cd hidapi-0.7.0/linux
 make
 ```
 And copy the resulting binary and the hidapi.h header to their proper locations
 ```
 cp hid-libusb.o ../../lib64 # or ../../lib if you're on a 32-bit machine
 cp ../hidapi/hidapi.h ../../include
 ```
 Now we can build the debug stack. The debug stack Makefile doesn't respect prefixes so we just copy the .so file into place.
 ```
 cd ../../../ # to MSPDebugStack_OS_Package
 make -j4
 cp libmsp430.so $PREFIX/lib/
 ```
 
The final utility which needs to be built is called mspdebug. It serves as a gdb agent allowing gdb to control the MSP directly. While the 0.24 release will work, we use the current Git version because it has better error reporting. Check it out from Github at https://github.com/dlbeer/mspdebug.git. We use the LDFLAGS environment variable to tell the compiled binary where to find libmsp430.so.
 
```
cd ..
git clone https://github.com/dlbeer/mspdebug.git
cd mspdebug
LDFLAGS=-Wl,-rpath,$PREFIX/lib make # if you didn't set the PREFIX environment variable above you'll need to set it here
make install
```
 
Lastly, you'll need to download the msp430 support files from TI, which can be found at http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html (msp430-gcc-support-files-1.198.zip as of this writing). Extract the files and enter the 'include' folder. Copy all .h files to $PREFIX/msp430-elf/include/ and all .ld files to $PREFIX/msp430-elf/lib/, as shown:

```
unzip msp430-gcc-support-files-1.198.zip
cd msp430-gcc-support-files/include
cp *.h $PREFIX/msp430-elf/include
cp *.ld $PREFIX/msp430-elf/lib
```

Now you have the toolchain and all the necessary utilities to start developing for the MSP430 architecture.
```