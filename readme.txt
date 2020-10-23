generally exitcodes are non-zero when something failed

mapcombine
==========

combines two mapfiles

$ mapcombine master.map additional.map

master file will be overriden with the merged data

mapconvert
==========

convert some input formats to mapfile

$ cat inputfile.pwn | ./mapconvert output.map

input formats:
- *.pwn
	whitespaces are ignored
	lines with:
		RemoveBuildingForPlayer(playerid,ID,X,Y,Z,R);
		CreateObject(ID,X,Y,Z,RX,RY,RZ);

mapinfo
=======

shows info

$ mapinfo a.map

mapupgr01to02
=============

upgrades a mapfile from v1 to v2

$ mapupgr01to02 from.map to.map

mapupgr02to03
=============

upgrades a mapfile from v2 to v3

$ mapupgr02to03 from.map to.map

textinfo
========

shows info

$ textinfo a.text
