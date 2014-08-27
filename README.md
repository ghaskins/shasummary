shasummary - A filesystem integrity validation tool
==========

This tool allows you to monitor the integrity of a filesystem based on SHA1 sums.  It is designed to operate in
two modes: generation and verification.  Generation mode will emit SHA1 metadata into a hidden sub-directory.
Verification mode will check the current state of the filesystem with respect to the previously generated
metadata and report on any differences (files added, removed, or changed) since the last update.

This is particularly helpful for ensuring that a filesystem backup remains coherent over time.  
