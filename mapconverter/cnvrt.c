/*----------------------------------------------------------------------------*/
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *ofile;
	char line[512];
	int linenum = 0;
	union {
		char buf[32];
		int version;
#pragma pack(push,1)
		struct {
			int model;
			float x, y, z, rx, ry, rz, drawdistance;
		} obj;
#pragma pack(pop)
	} mem;

	if (argc != 2) {
		puts("need one arg for outputfile");
		return 1;
	}

	ofile = fopen(argv[1], "wb");
	if (ofile == NULL) {
		printf("could not open output file %s", argv[1]);
		return 1;
	}

	mem.version = 1;
	fwrite(mem.buf, 4, 1, ofile);

	while (fgets(line, sizeof(line), stdin) != 0) {
		linenum++;
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
		} else {
			printf("invalid input on line %d", linenum);
		}
	}

	fclose(ofile);

	return 0;
}
/*----------------------------------------------------------------------------*/