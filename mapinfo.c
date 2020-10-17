#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MODELS 20000

char *modelNames[MAX_MODELS];
static char names[MAX_MODELS * 20];
static char *nameptr;
static char *files[] = {
	"data/maps/country/countn2.ide",
	"data/maps/country/countrye.ide",
	"data/maps/country/countryN.ide",
	"data/maps/country/countryS.ide",
	"data/maps/country/countryW.ide",
	"data/maps/country/counxref.ide",
	"data/maps/generic/barriers.ide",
	"data/maps/generic/dynamic.ide",
	"data/maps/generic/dynamic2.ide",
	"data/maps/generic/multiobj.ide",
	"data/maps/generic/procobj.ide",
	"data/maps/generic/vegepart.ide",
	"data/maps/interior/gen_int1.ide",
	"data/maps/interior/gen_int2.ide",
	"data/maps/interior/gen_int3.ide",
	"data/maps/interior/gen_int4.ide",
	"data/maps/interior/gen_int5.ide",
	"data/maps/interior/gen_intb.ide",
	"data/maps/interior/int_cont.ide",
	"data/maps/interior/int_LA.ide",
	"data/maps/interior/int_SF.ide",
	"data/maps/interior/int_veg.ide",
	"data/maps/interior/propext.ide",
	"data/maps/interior/props.ide",
	"data/maps/interior/props2.ide",
	"data/maps/interior/savehous.ide",
	"data/maps/interior/stadint.ide",
	"data/maps/LA/LAe.ide",
	"data/maps/LA/LAe2.ide",
	"data/maps/LA/LAhills.ide",
	"data/maps/LA/LAn.ide",
	"data/maps/LA/LAn2.ide",
	"data/maps/LA/LAs.ide",
	"data/maps/LA/LAs2.ide",
	"data/maps/LA/LAw.ide",
	"data/maps/LA/LAw2.ide",
	"data/maps/LA/LaWn.ide",
	"data/maps/LA/LAxref.ide",
	"data/maps/leveldes/leveldes.ide",
	"data/maps/leveldes/levelmap.ide",
	"data/maps/leveldes/levelxre.ide",
	"data/maps/leveldes/seabed.ide",
	"data/maps/SF/SFe.ide",
	"data/maps/SF/SFn.ide",
	"data/maps/SF/SFs.ide",
	"data/maps/SF/SFSe.ide",
	"data/maps/SF/SFw.ide",
	"data/maps/SF/SFxref.ide",
	"data/maps/txd.ide",
	"data/maps/vegas/vegasE.ide",
	"data/maps/vegas/VegasN.ide",
	"data/maps/vegas/VegasS.ide",
	"data/maps/vegas/VegasW.ide",
	"data/maps/vegas/vegaxref.ide",
	"data/maps/veh_mods/veh_mods.ide",
	"SAMP/SAMP.ide",
	"SAMP/CUSTOM.ide",
	"",
};

// ide 'loader' copied from yugecin/samp-mapedit
static
int ide_load_file(char *filename)
{
	FILE *f;
	char buf[512];
	char inobj;
	int pos;
	int modelid;
	int amount;

	amount = 0;
	if ((f = fopen(filename, "r"))) {
		inobj = 0;
nextline:
		if (!fgets(buf, sizeof(buf), f)) {
			goto done;
		}
		if (buf[0] == '#' || buf[0] == 0 || buf[0] == ' ') {
			;
		} else if (buf[0] == 'o' && buf[1] == 'b' && buf[2] == 'j') {
			inobj = 1;
		} else if (buf[0] == 't' && buf[1] == 'o' &&
			buf[2] == 'b' && buf[3] == 'j')
		{
			inobj = 1;
		} else if (buf[0] == 'a' && buf[1] == 'n' &&
			buf[2] == 'i' && buf[3] == 'm')
		{
			inobj = 1;
		} else if (buf[0] == 'e' && buf[1] == 'n' && buf[2] == 'd') {
			inobj = 0;
		} else if (inobj) {
			amount++;
			modelid = atoi(buf);
			if (modelid < 0 || MAX_MODELS <= modelid) {
				printf("model id %d in file %s oob\n", modelid, filename);
				goto nextline;
			}
			if (nameptr - names > sizeof(names) - 30) {
				puts("ide name pool depleted");
				goto nextline;
			}
			pos = 0;
			while (buf[pos++] != ',');
			while (buf[pos] == ' ') pos++;
			modelNames[modelid] = nameptr;
			sprintf(nameptr, "%05d:_", modelid);
			nameptr += 7;
nextchr:
			*nameptr = buf[pos];
			if (*nameptr == ',' || *nameptr == ' ') {
				*nameptr = 0;
				nameptr++;
			} else {
				nameptr++;
				pos++;
				goto nextchr;
			}
		}
		goto nextline;
done:
		fclose(f);
	} else {
		printf("failed to read %s\n", filename);
	}
	return amount;
}

void ide_load(char *sadir)
{
	char file[500];
	int i, num;

	memset(modelNames, 0, sizeof(modelNames));
	nameptr = names;
	i = num = 0;
	while (files[i][0]) {
		sprintf(file, "%s/%s", sadir, files[i++]);
		num += ide_load_file(file);
	}
}

#define MAX_COLORS 1000
int main(int argc, char **argv)
{
	FILE *file;
	int num_object_models;
	int num_remove_models;
	int i, j;
	int modelusage[MAX_MODELS];
	int modelremoves[MAX_MODELS];
	float minx, miny, minz;
	float maxx, maxy, maxz;
	float midx, midy, midz;
	int zone_color[MAX_COLORS];
	int zone_color_usage[MAX_COLORS];
	int num_zone_colors;
	char *mapfilepath;
	char *sadir;
	struct {
		int version;
		int numremoves;
		int numobjects;
		int numzones;
		float streamin;
		float streamout;
		float drawdistance;
	} header;
	struct {
		int model;
		float x, y, z, rx, ry, rz;
	} object;
	struct {
		int model;
		float x, y, z, radius;
	} remove;
	struct {
		float west, south, east, north;
		int color;
	} zone;

	memset(&modelusage, 0, sizeof(modelusage));
	memset(&modelremoves, 0, sizeof(modelremoves));

	if (argc == 3) {
		sadir = argv[1];
		mapfilepath = argv[2];
	} else if (argc > 1) {
		sadir = 0;
		mapfilepath = argv[1];
	} else {
		puts("please give .map file as argument");
		return 1;
	}

	if (!(file = fopen(mapfilepath, "rb"))) {
		puts("failed to open file for reading");
		return 1;
	}

	if (fread(&header, sizeof(header), 1, file) != 1) {
		goto corrupted;
	}

	printf(".map file version %08X\n", header.version);

	if (header.version != 0x0250414D) {
		puts("version unsupported");
		goto ret;
	}

	puts("header");
	printf("  version %08X\n", header.version);
	printf("  %d removes\n", header.numremoves);
	printf("  %d objects\n", header.numobjects);
	printf("  %d zones\n", header.numzones);
	printf("  streamdistance in  %.0f\n", header.streamin);
	printf("  streamdistance out %.0f\n", header.streamout);
	printf("  drawdistance %.0f\n", header.drawdistance);
	puts("");


	minx = miny = minz = 0x7F800000;
	maxx = maxy = maxz = -0x7F800000;
	midx = midy = midz = 0.0f;

	for (i = 0; i < header.numremoves; i++) {
		if (!fread(&remove, sizeof(remove), 1, file)) {
			printf("EOF while reading remove %i\n", i);
			goto corrupted;
		}
		if (remove.model != -1 && (remove.model < 611 || 19999 < remove.model)) {
			printf("invalid remove model %d at index %d\n", remove.model, i);
		}
		modelremoves[remove.model]++;
	}

	for (i = 0; i < header.numobjects; i++) {
		if (!fread(&object, sizeof(object), 1, file)) {
			printf("EOF while reading object %i\n", i);
			goto corrupted;
		}
		if (object.model < 611 || 19999 < object.model) {
			printf("invalid object model %d at index %d\n", object.model, i);
		}
		modelusage[object.model]++;
		midx += object.x;
		midy += object.y;
		midz += object.z;
		if (object.x < minx) minx = object.x;
		if (object.y < miny) miny = object.y;
		if (object.z < minz) minz = object.z;
		if (object.x > maxx) maxx = object.x;
		if (object.y > maxy) maxy = object.y;
		if (object.z > maxz) maxz = object.z;
	}

	num_zone_colors = 0;
	for (i = 0; i < header.numzones; i++) {
		if (!fread(&zone, sizeof(zone), 1, file)) {
			printf("EOF while reading zone %i\n", i);
			goto corrupted;
		}
		for (j = 0; j < num_zone_colors; j++) {
			if (zone_color[j] == zone.color) {
				zone_color_usage[j]++;
				goto nextzone;
			}
			if (num_zone_colors == MAX_COLORS) {
				printf("too many zone colors\n");
				goto corrupted;
			}
		}
		zone_color[num_zone_colors] = zone.color;
		zone_color_usage[num_zone_colors] = 1;
		num_zone_colors++;
nextzone:
		;
	}

	if (fread(&header, 1, 1, file)) {
		puts("extra data at end of file");
		goto corrupted;
	}

	if (header.numobjects > 0) {
		midx /= header.numobjects;
		midy /= header.numobjects;
		midz /= header.numobjects;
	}

	num_object_models = 0;
	num_remove_models = 0;
	for (i = 0; i < MAX_MODELS; i++) {
		if (modelusage[i]) {
			num_object_models++;
		}
		if (modelremoves[i]) {
			num_remove_models++;
		}
	}

	puts("removes");
	printf("  %d different remove models used\n", num_remove_models);
	puts("");

	puts("objects");
	printf("  middle: %.2f %.2f %.2f\n", midx, midy, midz);
	printf("  min: %.2f %.2f %.2f\n", minx, miny, minz);
	printf("  max: %.2f %.2f %.2f\n", maxx, maxy, maxz);
	printf("  %d different object models used\n", num_object_models);
	puts("");

	if (header.numobjects > 0 && sadir) {
		ide_load(sadir);
		puts("object models");
		for (i = 0; i < MAX_MODELS; i++) {
			if (modelusage[i]) {
				printf("  %03dx %s\n", modelusage[i], modelNames[i]);
			}
		}
	}

	puts("zones");
	printf("  num: %d\n", header.numzones);
	puts("  colors:");
	while (num_zone_colors) {
		j = 0;
		for (i = 0; i < num_zone_colors; i++) {
			if (zone_color_usage[i] > zone_color_usage[j]) {
				j = i;
			}
		}
		printf("    %08x: %dx\n", zone_color[j], zone_color_usage[j]);
		num_zone_colors--;
		zone_color_usage[j] = zone_color_usage[num_zone_colors];
		zone_color[j] = zone_color[num_zone_colors];
	}
	puts("");

ret:
	fclose(file);
	return 0;
corrupted:
	puts("file data seems to be corrupted");
	goto ret;
}
