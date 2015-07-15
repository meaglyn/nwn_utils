# nwn_utils
A collection of small utilities to help manage NWN custom content files and modules.

erf_tool is a copy of Grant Hughes' erf utility with a few bug fixes to help it work better with file names
contianing spaces and a few other minor fixes. This version is required to use the Project Q updater on Linux.
It still contains the build files for windows systems but I don't know if it still builds there. 

diff2da is a perl utility for cleaning up, renumbering, validating, extracting lines and columns from and 
diffing 2DA files. 

tlkcomp contains the nwntlkcomp program which is a small c program to convert TLK files to and from a simple text
format for easy editting and source control tracking. 

These are all released under the GPL v2.  All the code is Copyrighted by Meaglyn except the original portions of the
erf utility. 


You may also be interested in my fork of Niv's nwn_tools which has an updated nwnnsscomp compiler that fixes a few
bugs and will build with later (3.0 + ) bison versions. 
