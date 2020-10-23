#include <stdio.h>

int main(int argc, char **argv)
{
	int result;
	FILE *in, *out;
#pragma pack(push,1)
	struct {
		int version;
		int numremoves;
		int numobjects;
		int numzones;
		float streamin;
		float streamout;
		float drawdistance;
	} headerv2;
	struct {
		union {
			int num;
			char buf[4];
		} version;
		int numremoves;
		int numobjects;
		int objectdata_size;
		int numzones;
		float streamin;
		float streamout;
		float drawdistance;
	} headerv3;
	struct {
		short objectdata_size;
		struct {
			int model;
			float x, y, z, rx, ry, rz;
		} objectv2;
		float drawdistance;
		char no_camera_col;
		short attached_object;
		short attached_vehicle;
		char num_materials;
		/*Note that objectv3 can have more data, but there is none
		in this case because we're upgrading from objectv2.*/
	} objectv3;
#pragma pack(pop)
	char bigbuffer[1000];
	int i;
#define SIZEOF_REMOVE 20
#define SIZEOF_ZONE 20

	result = 1;

	if (argc < 3) {
		puts("need inputfile and outputfile");
		return 1;
	}

	if (!(in = fopen(argv[1], "rb"))) {
		printf("can't open file '%s' for reading\n", argv[1]);
		return 1;
	}
	if (!(out = fopen(argv[2], "wb"))) {
		printf("can't open file '%s' for writing\n", argv[2]);
		fclose(in);
		return 1;
	}

	headerv3.version.num = 0x0350414D;
	if (headerv3.version.buf[0] != 'M' || headerv3.version.buf[3] != 3) {
		puts("wrong endianness or structs are not packed");
		goto ret;
	}

	if (!fread(&headerv2, sizeof(headerv2), 1, in)) {
		puts("failed to read header");
		goto ret;
	}
	if (headerv2.version != 0x0250414D) {
		puts("input file is not v2");
		goto ret;
	}
	headerv3.numremoves = headerv2.numremoves;
	headerv3.numobjects = headerv2.numobjects;
	headerv3.objectdata_size = headerv2.numobjects * sizeof(objectv3);
	headerv3.numzones = headerv2.numzones;
	headerv3.streamin = headerv2.streamin;
	headerv3.streamout = headerv2.streamout;
	headerv3.drawdistance = headerv2.drawdistance;

	fwrite(&headerv3, sizeof(headerv3), 1, out);

	for (i = 0; i < headerv2.numremoves; i++) {
		if (!fread(bigbuffer, SIZEOF_REMOVE, 1, in)) {
			puts("eof while reading removes");
			goto ret;
		}
		fwrite(bigbuffer, SIZEOF_REMOVE, 1, out);
	}

	if (fseek(in, headerv2.numobjects * sizeof(objectv3.objectv2), SEEK_CUR)) {
		puts("failed to fseek to zones");
		goto ret;
	}

	for (i = 0; i < headerv2.numzones; i++) {
		if (!fread(bigbuffer, SIZEOF_ZONE, 1, in)) {
			puts("eof while reading removes");
			goto ret;
		}
		fwrite(bigbuffer, SIZEOF_ZONE, 1, out);
	}

	if (fseek(in, sizeof(headerv2) + headerv2.numremoves * SIZEOF_REMOVE, SEEK_SET)) {
		puts("failed to fseek to objects");
		goto ret;
	}

	for (i = 0; i < headerv2.numobjects; i++) {
		if (!fread(&objectv3.objectv2, sizeof(objectv3.objectv2), 1, in)) {
			puts("eof while reading objects");
			goto ret;
		}
		objectv3.objectdata_size = sizeof(objectv3);
		objectv3.drawdistance = headerv3.drawdistance;
		objectv3.no_camera_col = 0;
		objectv3.attached_object = -1;
		objectv3.attached_vehicle = -1;
		objectv3.num_materials = 0;
		fwrite(&objectv3, sizeof(objectv3), 1, out);
	}

	if (fseek(in, headerv2.numzones * SIZEOF_ZONE, SEEK_CUR)) {
		puts("failed to fseek to eof");
		goto ret;
	}

	if (fread(bigbuffer, 1, 1, in) || !feof(in)) {
		puts("more data at the end of the file");
		goto ret;
	}

	result = 0;

ret:
	if (result) {
		printf("failed for %s\n", argv[2]);
	}
	fclose(in);
	fclose(out);
	return result;
}
