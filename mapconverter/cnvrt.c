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
			float x, y, z, rx, ry, rz;
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
/*----------------------------------------------------------------------------*/
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
			fwrite(mem.buf, 4, 7, ofile);
		} else {
			printf("invalid input on line %d", linenum);
		}
	}

	fclose(ofile);

	return 0;
}
