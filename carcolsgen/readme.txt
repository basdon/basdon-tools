Very specific tool to get vehicle colors for usage in fly-plugin. Parses the
vehicles.ide and carcols.dat files of GTA:SA to collect all the possible
'normal' random color combinations for each vehicle.

Compile
-------
javac Carcols.java

Usage
-----
java -cp . Carcols "/path/to/vehicles.ide" "/path/to/carcols.dat"

Sample output
-------------

struct CARCOLDATA carcoldata[VEHICLE_MODEL_TOTAL] = {
	 { 8, 0 },
	 { 7, 16 },
	 (..)
};

char carcols[1912] = {
	101,1, 113,1, 123,1, 36,1, 4,1, 40,1, 62,1, 75,1,
	113,113, 41,41, 47,47, 52,52, 66,66, 74,74, 87,87,
	(..)
};

