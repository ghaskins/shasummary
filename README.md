shasummary - A filesystem integrity validation tool
=

This tool allows you to monitor the integrity of a filesystem based on SHA1 sums.  It is designed to operate in
two modes: generation and verification.  Generation mode will emit SHA1 metadata into a hidden sub-directory.
Verification mode will check the current state of the filesystem with respect to the previously generated
metadata and report on any differences (files added, removed, or changed) since the last update.

This is particularly helpful for ensuring that a filesystem backup remains coherent over time.

Usage
==


Example
==
```
greg:shasummary ghaskins$ make
Compiling (C++) main.cc to obj/x86_64/main.o
c++ -g --std=c++11 -DBOOST_FILESYSTEM_VERSION=3   -c -o obj/x86_64/main.o main.cc
Linking obj/x86_64/shasummary
c++ -g --std=c++11 -DBOOST_FILESYSTEM_VERSION=3   -o obj/x86_64/shasummary obj/x86_64/main.o  -lboost_program_options-mt -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt
greg:shasummary ghaskins$ make testdata
Generating test data obj/test/foo1.dat
Generating test data obj/test/foo2.dat
Generating test data obj/test/foo3.dat
Generating test data obj/test/foo4.dat
greg:shasummary ghaskins$ tree -a obj/test/
obj/test/
|-- foo1.dat
|-- foo2.dat
|-- foo3.dat
`-- foo4.dat

0 directories, 4 files
greg:shasummary ghaskins$ ./obj/
test/   x86_64/ 
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
C       "./obj/test/foo1.dat"
C       "./obj/test/foo2.dat"
C       "./obj/test/foo3.dat"
C       "./obj/test/foo4.dat"
greg:shasummary ghaskins$ ./obj/x86_64/shasummary --generate ./obj/test/
Using 8 threads
Generating sums in "./obj/test/"
C       "./obj/test/foo1.dat"
C       "./obj/test/foo2.dat"
C       "./obj/test/foo3.dat"
C       "./obj/test/foo4.dat"
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
greg:shasummary ghaskins$ rm ./obj/test/foo1.dat 
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
D       "./obj/test/foo1.dat"
greg:shasummary ghaskins$ uuidgen > ./obj/
test/   x86_64/ 
greg:shasummary ghaskins$ uuidgen > ./obj/test/foo2.dat 
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Checkpoint
Using 8 threads
Verifying sums in "./obj/test/"
U       "./obj/test/foo2.dat"
D       "./obj/test/foo1.dat"
greg:shasummary ghaskins$ ./obj/x86_64/shasummary --generate ./obj/test/
Using 8 threads
Generating sums in "./obj/test/"
U       "./obj/test/foo2.dat"
D       "./obj/test/foo1.dat"
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
```