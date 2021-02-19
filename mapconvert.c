/*----------------------------------------------------------------------------*/
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *ofile;
	char line[512], *linep, *lineq;
	int linenum = 0;
	int result = 0;
#pragma pack(push,1)
	struct {
		int version;
		int numremoveobj;
		int numobj;
		int objdata_size;
		int numgang;
		float streamin;
		float streamout;
		float drawdistance;
	} memheader;
	struct {
		short objdata_size;
		int model;
		float x, y, z, rx, ry, rz;
		float drawdistance;
		char nocameracol;
		short attached_object_id;
		short attached_vehicle_id;
		char num_materials;
	} memobject;
	struct {
		int model;
		float x, y, z, radius;
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


	memheader.version = 0x0350414D;
	if (((char*) &memheader)[0] != 'M' || ((char*) &memheader)[3] != 3) {
		puts("wrong endianness or structs are packed");
		result = 2;
		goto ret;
	}
	memheader.numremoveobj = -1;
	memheader.numobj = -1;
	memheader.objdata_size = -1;
	memheader.numgang = -1;
	memheader.streamin = 2500.0f;
	memheader.streamout = 4000.0f;
	memheader.drawdistance = 1500.0f;
	fwrite(&memheader, sizeof(memheader), 1, ofile);
	memheader.numremoveobj = 0;
	memheader.numobj = 0;
	memheader.objdata_size = 0;
	memheader.numgang = 0;

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
			&memobject.model,
			&memobject.x,
			&memobject.y,
			&memobject.z,
			&memobject.rx,
			&memobject.ry,
			&memobject.rz))
		{
			memobject.objdata_size = sizeof(memobject);
			memobject.drawdistance = memheader.drawdistance;
			memobject.nocameracol = 0;
			memobject.attached_object_id = -1;
			memobject.attached_vehicle_id = -1;
			memobject.num_materials = 0;
			fwrite(&memobject, sizeof(memobject), 1, ofile);
			memheader.objdata_size += memobject.objdata_size;
			memheader.numobj++;
		} else if (5 == sscanf(line,
			"RemoveBuildingForPlayer(playerid,%d,%f,%f,%f,%f);\n",
			&memremove.model,
			&memremove.x,
			&memremove.y,
			&memremove.z,
			&memremove.radius))
		{
			fwrite(&memremove, sizeof(memremove), 1, ofile);
			memheader.numremoveobj++;
		} else {
			printf("invalid input on line %d: %s\n", linenum, line);
		}
	}

	if (fseek(ofile, 0, SEEK_SET)) {
		fputs("failed to rewrite header", stderr);
		result = 1;
	}
	fwrite(&memheader, sizeof(memheader), 1, ofile);

ret:
	fclose(ofile);
	return result;
}
/*----------------------------------------------------------------------------*/
