strace2ds-library - a bridge between strace and DataSeries trace format
=======================================================================

This library connects the [strace] code (written in C) to the [DataSeries] API
(written in C++). It acts as a glue so that C++ functions are made callable from
C code. Our fork of strace, [reanimator-strace], takes advantage of this library to
output traces in the DataSeries format.

Dependencies
------------

Currently, only Ubuntu 16 is officially supported.

- [Lintel] - general utility library for DataSeries
- [DataSeries] - data format for structured serial data
- [tcmalloc] - high-performance, multi-threaded `malloc()` implementation
- libtool
- libboost-dev (v1.58 only)
- libboost-thread-dev (v1.58 only)
- libboost-program-options-dev (v1.58 only)
- build-essential
- libxml2-dev
- zlib1g-dev

Build Instructions
------------------

### Manual Build

1. Install the following required programs and libraries:

   ```plaintext
   git cmake perl autoconf automake gcc g++ libtool libboost-dev libboost-thread-dev libboost-program-options-dev build-essential libxml2-dev zlib1g-dev
   ```

   On Ubuntu 16, all the above requirements are available through the APT
   package manager.

2. Install [Lintel] by running `cmake . && make && make install` at the root of
   the Lintel repository.

3. Install [DataSeries] by running `cmake . && make && make install` at the root
   of the DataSeries repository.

4. Install [tcmalloc] from the gperftools repository. See the gperftools
   [`INSTALL`](https://github.com/gperftools/gperftools/blob/master/INSTALL)
   file for detailed instructions.

5. In the strace2ds-library directory, run `autoreconf -v -i` to ensure build
   scripts are up-to-date.

6. Navigate to the `tables/` subdirectory and run `perl gen-xml-enums.pl` to
   update the .xml files.

7. Create a subdirectory named `BUILD/` in the strace2ds-library directory and
   navigate to it. Run `cp -r ../xml ./` to copy the xml folder into the build
   directory.

8. Run the command

   ```bash
   ../configure --enable-shared --disable-static
   ```

   If you have installed the other libraries in a nonstandard directory, you will need to change the `CPPFLAGS` and `LDFLAGS` environment variables before running `configure`. For example, if you have installed tcmalloc in `$HOME/tcmalloc`, you will need to run

   ```bash
   CPPFLAGS="-I$HOME/tcmalloc/include" LDFLAGS="-Xlinker -rpath=$HOME/tcmalloc/lib -L$HOME/tcmalloc/lib" ../configure --enable-shared --disable-static
   ```

   For more information on `CPPFLAGS` and `LDFLAGS`, run `../configure --help`.

9. Run `make && make install` to build and install strace2ds-library in `/usr/local`.

[strace]: https://strace.io
[DataSeries]: https://github.com/dataseries/dataseries
[reanimator-strace]: https://github.com/SNIA/reanimator-strace
[Lintel]: https://github.com/dataseries/lintel
[tcmalloc]: https://github.com/gperftools/gperftools
