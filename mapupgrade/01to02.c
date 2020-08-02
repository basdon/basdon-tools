#include <stdio.h>

int main(int argc, char **argv)
{
	FILE *in, *out;
#pragma pack(push,1)
	union {
		char buf[8];
		struct {
			int version;
			int numremoves;
			int numobjects;
		} data;
	} headerv1;
	union {
		char buf[28];
		struct {
			int version;
			int numremoves;
			int numobjects;
			int numzones;
			float streamin;
			float streamout;
			float drawdistance;
		} data;
	} headerv2;
	union {
		int model;
		int data[2];
		struct {
			int model;
			float x, y, z, rx, ry, rz;
			float drawdistance;
		} objectv1;
		struct {
			int model;
			float x, y, z, rx, ry, rz;
		} objectv2;
		struct {
			int model;
			float x, y, z, radius;
		} removev1;
		struct {
			int model;
			float x, y, z, radius;
		} removev2;
	} data;
#pragma pack(pop)

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

	headerv2.data.version = 0x0250414D;
	if (headerv2.buf[0] != 'M' || headerv2.buf[3] != 2) {
		puts("wrong endianness or structs are not packed");
		goto ret;
	}
	headerv2.data.numremoves = 0;
	headerv2.data.numobjects = 0;
	headerv2.data.numzones = 0;
	headerv2.data.streamin = 500.0f;
	headerv2.data.streamout = 600.0f;
	headerv2.data.drawdistance = 1500.0f;

	if (!fread(&headerv1, sizeof(headerv1), 1, in)) {
		goto corrupted;
	}
	headerv2.data.numremoves = headerv1.data.numremoves;
	headerv2.data.numobjects = headerv1.data.numobjects;

	fwrite(&headerv2, sizeof(headerv2), 1, out);

	if (headerv1.data.numremoves == 0) {
		goto skipremoves;
	}

	while (fread(&data.model, sizeof(data.model), 1, in)) {
		if (data.model < 0) {
			if (!fread(data.data + 1, sizeof(data.removev1) - sizeof(data.model), 1, in)) {
				goto corrupted;
			}
			data.model = -data.model;
			fwrite(&data.removev2, sizeof(data.removev2), 1, out);
		} else {
			if (!fread(data.data + 1, sizeof(data.objectv1) - sizeof(data.model), 1, in)) {
				goto corrupted;
			}
		}
	}

	if (!fseek(in, sizeof(headerv1), SEEK_SET)) {
		puts("failed to fseek");
		goto ret;
	}
skipremoves:

	while (fread(&data.model, sizeof(data.model), 1, in)) {
		if (data.model < 0) {
			if (!fread(data.data + 1, sizeof(data.removev1) - sizeof(data.model), 1, in)) {
				goto corrupted;
			}
		} else {
			if (!fread(data.data + 1, sizeof(data.objectv1) - sizeof(data.model), 1, in)) {
				goto corrupted;
			}
			fwrite(&data.objectv2, sizeof(data.objectv2), 1, out);
		}
	}

ret:
	fclose(in);
	fclose(out);
	return 0;
corrupted:
	puts("input file is corrupted");
	goto ret;
}
