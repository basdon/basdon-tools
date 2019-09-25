Small tool to convert some input formats to .map files used by basdon-fly.

Compiling
---------
gcc -Wall -s -x c -ansi -std=c89 -o cnvrt cnvrt.c

Usage
-----
(please dos2unix input.pwn first)
sed 's/ //g' < input.pwn | ./cnvrt output.map

Input formats
-------------
- *.pwn
	lines with: CreateObject(ID,X,Y,Z,RX,RY,RZ);

