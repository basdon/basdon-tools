#include <stdio.h>
#include <string.h>

struct header {
	int version;
	int numremoves;
	int numobjects;
	int numzones;
	float streamin;
	float streamout;
	float drawdistance;
};
struct header h1, h2, h3;
struct {
	int model;
	float x, y, z, rx, ry, rz;
} object;
struct {
	int model;
	float x, y, z, radius;
} removebuild;
struct {
	float west, south, east, north;
	int color;
} gangzone;

char newdata[sizeof(h3) + sizeof(object) * 2000 + sizeof(removebuild) * 2000 + sizeof(gangzone) * 2048];
char *datapos = newdata;

int main(int argc, char **argv)
{
	FILE *master, *add;
	int i;

	if (argc < 3) {
		puts("give master file and additional file as args");
		return 1;
	}

	if (!(master = fopen(argv[1], "rb"))) {
		puts("failed to open master file for reading");
		return 1;
	}

	if (!(add = fopen(argv[2], "rb"))) {
		fclose(master);
		puts("failed to open additional file for reading");
		return 1;
	}

	if (fread(&h1, sizeof(h1), 1, master) != 1 || fread(&h2, sizeof(h2), 1, add) != 1) {
		goto fail;
	}

	if (h1.version != 0x0250414D || h2.version != 0x0250414D) {
		puts("version unsupported");
		goto fail;
	}

	h3 = h1;
	h3.numremoves = h1.numremoves + h2.numremoves;
	h3.numobjects = h1.numobjects + h2.numobjects;
	h3.numzones = h1.numzones + h2.numzones;
	memcpy(datapos, &h3, sizeof(h3));
	datapos += sizeof(h3);

	for (i = 0; i < h1.numremoves; i++) {
		if (!fread(&removebuild, sizeof(removebuild), 1, master)) {
			goto fail;
		}
		memcpy(datapos, &removebuild, sizeof(removebuild));
		datapos += sizeof(removebuild);
	}

	for (i = 0; i < h2.numremoves; i++) {
		if (!fread(&removebuild, sizeof(removebuild), 1, add)) {
			goto fail;
		}
		memcpy(datapos, &removebuild, sizeof(removebuild));
		datapos += sizeof(removebuild);
	}

	for (i = 0; i < h1.numobjects; i++) {
		if (!fread(&object, sizeof(object), 1, master)) {
			goto fail;
		}
		memcpy(datapos, &object, sizeof(object));
		datapos += sizeof(object);
	}

	for (i = 0; i < h2.numobjects; i++) {
		if (!fread(&object, sizeof(object), 1, add)) {
			goto fail;
		}
		memcpy(datapos, &object, sizeof(object));
		datapos += sizeof(object);
	}

	for (i = 0; i < h1.numzones; i++) {
		if (!fread(&gangzone, sizeof(gangzone), 1, master)) {
			goto fail;
		}
		memcpy(datapos, &gangzone, sizeof(gangzone));
		datapos += sizeof(gangzone);
	}

	for (i = 0; i < h2.numzones; i++) {
		if (!fread(&gangzone, sizeof(gangzone), 1, add)) {
			goto fail;
		}
		memcpy(datapos, &gangzone, sizeof(gangzone));
		datapos += sizeof(gangzone);
	}

	fclose(master);
	fclose(add);

	if (!(master = fopen(argv[1], "wb"))) {
		puts("failed to open master file for writing");
		return 1;
	}

	if (!fwrite(newdata, datapos - newdata, 1, master)) {
		puts("failed to write master file?");
	}

	fclose(master);
	return 0;

fail:
	puts("failed, check both maps with mapinfo to see if they're valid");
	fclose(master);
	fclose(add);
	return 1;
}
