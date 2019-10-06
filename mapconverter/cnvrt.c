/*----------------------------------------------------------------------------*/
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *ofile;
	char line[512], *linep, *lineq;
	int linenum = 0;
#pragma pack(push,1)
	union {
		char buf[32];
		int version;
		struct {
			int model;
			float x, y, z, rx, ry, rz, drawdistance;
		} obj;
	} mem;
	union {
		char buf[32];
		struct {
			int model;
			float x, y, z, radius;
		} obj;
	} memremove;
#pragma pack(pop)

	if (argc != 2) {
		puts("need one arg for outputfile");
		return 1;
	}

	ofile = fopen(argv[1], "wb");
	if (ofile == NULL) {
		printf("could not open output file %s\n", argv[1]);
		return 1;
	}

	mem.version = 1;
	fwrite(mem.buf, 4, 1, ofile);

	while (fgets(line, sizeof(line), stdin) != 0) {
		linenum++;
		/* remove spaces*/
		linep = lineq = line;
		while (1) {
			if (*linep != ' ') {
				if (!(*lineq = *linep)) {
					break;
				}
				lineq++;
			}
			linep++;
		}
		if (7 == sscanf(line,
			"CreateObject(%d,%f,%f,%f,%f,%f,%f);\n",
			&mem.obj.model,
			&mem.obj.x,
			&mem.obj.y,
			&mem.obj.z,
			&mem.obj.rx,
			&mem.obj.ry,
			&mem.obj.rz))
		{
			mem.obj.drawdistance = 0.0f;
			if (19121 <= mem.obj.model && mem.obj.model <= 19127) {
				/*more drawdistance for bollardlights*/
				mem.obj.drawdistance = 500.0f;
			}
			fwrite(mem.buf, 4, 8, ofile);
		} else if (5 == sscanf(line,
			"RemoveBuildingForPlayer(playerid,%d,%f,%f,%f,%f);\n",
			&memremove.obj.model,
			&memremove.obj.x,
			&memremove.obj.y,
			&memremove.obj.z,
			&memremove.obj.radius))
		{
			memremove.obj.model = -memremove.obj.model;
			fwrite(memremove.buf, 4, 5, ofile);
		} else {
			printf("invalid input on line %d: %s\n", linenum, line);
		}
	}

	fclose(ofile);

	return 0;
}
/*----------------------------------------------------------------------------*/
