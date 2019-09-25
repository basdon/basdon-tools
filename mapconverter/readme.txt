Small tool to convert some input formats to .map format used by basdon-fly.

Compiling
---------
gcc -Wall -s -x c -ansi -std=c89 -o cnvrt cnvrt.c

Usage
-----
(please dos2unix inputfile first)
sed 's/ //g' < inputfile | ./cnvrt output.map

Input formats
-------------
- *.pwn
	lines with: CreateObject(ID,X,Y,Z,RX,RY,RZ);

