Searches for an entry in a GXT file of GTA:SA

Compile
-------
javac grabgxtvalue.java

Usage
-----
-Dgxtentry: entries separated with a comma, either in text format or numerical (base 10 or base16 (can have 0x prefix))
-Dgxtpath: path to gxt file, can be absolute or relative to working dir
-Ddebug: debug output

java -Dgxtentry=PARA,CIVI -Dgxtpath=american.gxt -cp ../ grabgxtvalue.grabgxtvalue

Sample output
-------------
found 'CIVI': Santa Flora
found 'PARA': Paradiso

Sample output with -Ddebug
--------------------------
'PARA' CRC is EDB88666
'CIVI' CRC is CBB06299
127 subtables
subtable 'MAIN' offset 00000600
subtable 'AMBULAE' offset 0002F918
subtable 'BCESAR2' offset 0002FB64
subtable 'BCESAR4' offset 0003008C
(ommitted output)
subtable 'MAIN' has 5534 entries
found 'CIVI': Santa Flora
found 'PARA': Paradiso
subtable 'AMBULAE' has 15 entries
subtable 'BCESAR2' has 25 entries
subtable 'BCESAR4' has 93 entries
(ommitted output)

