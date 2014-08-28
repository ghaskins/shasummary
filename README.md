shasummary - A filesystem integrity validation tool
=

This tool allows you to monitor the integrity of a filesystem based on SHA1 sums. Think of it as a way to manage
[shasum](http://linux.die.net/man/1/shasum) recursively and across many files at once.  This involves the generation and persistence of the SHA1 information and the monitoring/reporting of changes detected via SHA1 comparison.

The tool is designed to operate in two modes: _generation_ and _verification_:
- **Generation mode:** recurses through a specified file hierarchy while computing SHA1 metadata and emitting it into a hidden sub-directory in each subdirectory discovered.
- **Verification mode:** will recompute the SHA1 for each file discovered and use it as a comparison against any previously stored SHA1 values.  Any differences in the filesystem between the stored metadata and computed value will result in a report (files added, removed, or changed) since the last update.  

The metadata is stored in standard files within standard (albeit hidden) subdirectories, providing
compatibility with standard tools like rsync or tar.  This is particularly helpful for ensuring that a
filesystem backup remains coherent over time since the backup can be verified independently from
the state of the source filesystem.

Usage
==

Building
===
```
greg:shasummary ghaskins$ make
Compiling (C++) main.cc to obj/x86_64/main.o
c++ -g --std=c++11 -DBOOST_FILESYSTEM_VERSION=3   -c -o obj/x86_64/main.o main.cc
Linking obj/x86_64/shasummary
c++ -g --std=c++11 -DBOOST_FILESYSTEM_VERSION=3   -o obj/x86_64/shasummary obj/x86_64/main.o  -lboost_program_options-mt -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt
```

Example
===
In this example, we will use the built-in "make testdata" facility to generate some test files to play around with

Generate the Test Data
====
```
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
```
Run the verifier on the baseline filesystem
====
```
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
C       "./obj/test/foo1.dat"
C       "./obj/test/foo2.dat"
C       "./obj/test/foo3.dat"
C       "./obj/test/foo4.dat"
```
The tool outputs the files that it discovered, prefixed with one of "C", "U", or "D" representing files that are Created, Updated, or Deleted, respectively, since we last ran.  In this case, we have never run the tool so all files are considered new.

Generate our metadata with "--generate"
====
**CAUTION: Running the generator will modify the filesystem.  Hidden directories (.shasummary) are created in each subdirectory being scanned to hold the SHA1 metadata**
```
greg:shasummary ghaskins$ ./obj/x86_64/shasummary --generate ./obj/test/
Using 8 threads
Generating sums in "./obj/test/"
C       "./obj/test/foo1.dat"
C       "./obj/test/foo2.dat"
C       "./obj/test/foo3.dat"
C       "./obj/test/foo4.dat"
```
We can observe that this results in the same report as before.  The difference is that the state of the filesystem was recorded such that future iterations will be based on the state of our current snapshot, as we can see below.
```
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
```
Note that the tool did not report any differences.  We have verified that the filesytem is in the same state as it was when the metadata was generated.

Make some artificial changes
====

First, delete a file
=====
```
greg:shasummary ghaskins$ rm ./obj/test/foo1.dat 
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
D       "./obj/test/foo1.dat"
```
Note that the deleted file is flagged as "D".

Next, change an existing file
=====
```
greg:shasummary ghaskins$ uuidgen > ./obj/test/foo2.dat 
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
U       "./obj/test/foo2.dat"
D       "./obj/test/foo1.dat"
```
Note that the tool has now flagged the removed file ("D") and the updated file ("U")

Finally, re-generate the metadata
=====
We can update the metadata with the new state of our filesystem to serve as the reference point.
```
greg:shasummary ghaskins$ ./obj/x86_64/shasummary --generate ./obj/test/
Using 8 threads
Generating sums in "./obj/test/"
U       "./obj/test/foo2.dat"
D       "./obj/test/foo1.dat"
greg:shasummary ghaskins$ ./obj/x86_64/shasummary ./obj/test/
Using 8 threads
Verifying sums in "./obj/test/"
```
Note that the final verification step indicates that there are no outstanding changes
