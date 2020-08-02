/*----------------------------------------------------------------------------*/
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *ofile;
	char line[512], *linep, *lineq;
	int linenum = 0;
	int result = 0;
#pragma pack(push,1)
	union {
		char buf[28];
		struct {
			int version;
			int numremoveobj;
			int numobj;
			int numgang;
			float streamin;
			float streamout;
			float drawdistance;
		} data;
	} memheader;
	union {
		char buf[28];
		struct {
			int model;
			float x, y, z, rx, ry, rz;
		} data;
	} memobject;
	union {
		char buf[20];
		struct {
			int model;
			float x, y, z, radius;
		} data;
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


	memheader.data.version = 0x0250414D;
	if (memheader.buf[0] != 'M' || memheader.buf[3] != 2) {
		puts("wrong endianness or structs are packed");
		result = 2;
		goto ret;
	}
	memheader.data.numremoveobj = -1;
	memheader.data.numobj = -1;
	memheader.data.numgang = -1;
	memheader.data.streamin = 500.0f;
	memheader.data.streamout = 600.0f;
	memheader.data.drawdistance = 500.0f;
	fwrite(memheader.buf, sizeof(memheader.buf), 1, ofile);
	memheader.data.numremoveobj = 0;
	memheader.data.numobj = 0;
	memheader.data.numgang = 0;

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
			&memobject.data.model,
			&memobject.data.x,
			&memobject.data.y,
			&memobject.data.z,
			&memobject.data.rx,
			&memobject.data.ry,
			&memobject.data.rz))
		{
			fwrite(memobject.buf, sizeof(memobject.buf), 1, ofile);
			memheader.data.numobj++;
		} else if (5 == sscanf(line,
			"RemoveBuildingForPlayer(playerid,%d,%f,%f,%f,%f);\n",
			&memremove.data.model,
			&memremove.data.x,
			&memremove.data.y,
			&memremove.data.z,
			&memremove.data.radius))
		{
			fwrite(memremove.buf, sizeof(memremove.buf), 1, ofile);
			memheader.data.numremoveobj++;
		} else {
			printf("invalid input on line %d: %s\n", linenum, line);
		}
	}

	if (fseek(ofile, 0, SEEK_SET)) {
		fputs("failed to rewrite header", stderr);
		result = 1;
	}
	fwrite(memheader.buf, sizeof(memheader.buf), 1, ofile);

ret:
	fclose(ofile);
	return result;
}
/*----------------------------------------------------------------------------*/
