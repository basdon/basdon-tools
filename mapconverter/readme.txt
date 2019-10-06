Small tool to convert some input formats to .map format used by basdon-fly.

Compiling
---------
gcc -Wall -s -x c -std=c89 -m32 -o cnvrt cnvrt.c

Usage
-----
cat inputfile.pwn | ./cnvrt output.map

exitcode is non-zero when something failed

Input formats
-------------
- *.pwn
	whitespaces are ignored
	lines with:
		RemoveBuildingForPlayer(playerid,ID,X,Y,Z,R);
		CreateObject(ID,X,Y,Z,RX,RY,RZ);

