#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MODELS 20000

char ides_loaded;
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

static
int ide_load(char *sadir)
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
	ides_loaded = 1;
	return num > 0;
}

#define MAX_COLORS 1000
static
int mapinfo(char *filename, char do_dump, char skip_materials)
{
	FILE *file;
	int num_object_models;
	int num_remove_models;
	int i, j;
	int modelusage[MAX_MODELS];
	int modelremoves[MAX_MODELS];
	int total_read_object_size;
	int num_objects_with_materials;
	int total_materials;
	float minx, miny, minz;
	float maxx, maxy, maxz;
	float midx, midy, midz;
	int zone_color[MAX_COLORS];
	int zone_color_usage[MAX_COLORS];
	int num_zone_colors;
#pragma pack(push,1)
	struct {
		int version;
		int numremoves;
		int numobjects;
		int objectdata_size;
		int numzones;
		float streamin;
		float streamout;
		float drawdistance;
	} header;
	struct {
		short objectdata_size;
		int model;
		float x, y, z, rx, ry, rz;
		float drawdistance;
		char no_camera_col;
		short attached_object;
		short attached_vehicle;
		char num_materials;
	} object;
	struct {
		int model;
		float x, y, z, radius;
	} remove;
	struct {
		float west, south, east, north;
		int color;
	} zone;
#pragma pack(pop)
	char bigbuffer[1000];
	int dword;
	short word;
	unsigned char byte;
	char material_type;

	memset(&modelusage, 0, sizeof(modelusage));
	memset(&modelremoves, 0, sizeof(modelremoves));

	if (!(file = fopen(filename, "rb"))) {
		puts("failed to open file for reading");
		return 1;
	}

	if (fread(&header, sizeof(header), 1, file) != 1) {
		goto corrupted;
	}

	puts("/*");
	printf("%s\n", filename);
	printf("  .map file version %08X\n", header.version);
	puts("");

	if (header.version != 0x0350414D) {
		puts("version unsupported");
		goto ret;
	}

	puts("header");
	printf("  version %08X\n", header.version);
	printf("  %d removes\n", header.numremoves);
	printf("  %d objects\n", header.numobjects);
	printf("  %d objectdata_size\n", header.objectdata_size);
	printf("  %d zones\n", header.numzones);
	printf("  streamdistance in  %.0f\n", header.streamin);
	printf("  streamdistance out %.0f\n", header.streamout);
	printf("  drawdistance %.0f\n", header.drawdistance);
	puts("");


	minx = miny = minz = 0x7F800000;
	maxx = maxy = maxz = -0x7F800000;
	midx = midy = midz = 0.0f;

	puts("*/");
	for (i = 0; i < header.numremoves; i++) {
		if (!fread(&remove, sizeof(remove), 1, file)) {
			printf("EOF while reading remove %i\n", i);
			goto corrupted;
		}
		if (remove.model != -1 && (remove.model < 611 || 19999 < remove.model)) {
			printf("// invalid remove model %d at index %d\n", remove.model, i);
		}
		if (do_dump) {
			printf("RemoveBuildingForPlayer(playerid,%d,%.4f,%.4f,%.4f,%.8f);\n",
				remove.model, remove.x, remove.y, remove.z, remove.radius);
		}
		modelremoves[remove.model]++;
	}
	puts("/*");

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

	puts("*/");
	total_materials = 0;
	num_objects_with_materials = 0;
	total_read_object_size = 0;
	for (i = 0; i < header.numobjects; i++) {
		if (!fread(&object, sizeof(object), 1, file)) {
			printf("EOF while reading object %i\n", i);
			goto corrupted;
		}
		if (object.model < 611 || 19999 < object.model) {
			printf("// invalid object model %d at index %d\n", object.model, i);
			if (object.model < 0 || 1999 < object.model) {
				goto corrupted;
			}
		}
		if (do_dump) {
			if (object.num_materials && !skip_materials) {
				printf("new o%d=", i);
			}
			printf("CreateObject(%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f);\n",
				object.model, object.x, object.y, object.z,
				object.rx, object.ry, object.rz, object.drawdistance);
		}
		total_read_object_size += object.objectdata_size;
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
		total_materials += object.num_materials;
		if (object.num_materials > 0) {
			num_objects_with_materials++;
			for (j = 0; j < object.num_materials; j++) {
				if (!fread(&material_type, sizeof(material_type), 1, file)) {
					printf("while reading object %d material %d type\n", i, j);
					goto corrupted;
				}
				if (!fread(&byte, sizeof(byte), 1, file)) {
					printf("while reading object %d material %d index\n", i, j);
					goto corrupted;
				}
				if (material_type == 1) {
					if (!fread(&word, sizeof(word), 1, file)) {
						printf("while reading object %d material %d model id\n", i, j);
						goto corrupted;
					}
					if (do_dump && !skip_materials) {
						printf("SetObjectMaterial(o%d,%d,%d", i, byte, word);
					}
					if (!fread(&byte, sizeof(byte), 1, file)) {
						printf("while reading object %d material %d txd len\n", i, j);
						goto corrupted;
					}
					if (!fread(bigbuffer, byte, 1, file)) {
						printf("while reading object %d material %d txd\n", i, j);
						goto corrupted;
					}
					if (do_dump && !skip_materials) {
						bigbuffer[byte] = 0;
						printf(",\"%s\"", bigbuffer);
					}
					if (!fread(&byte, sizeof(byte), 1, file)) {
						printf("while reading object %d material %d texture len\n", i, j);
						goto corrupted;
					}
					if (!fread(bigbuffer, byte, 1, file)) {
						printf("while reading object %d material %d texture\n", i, j);
						goto corrupted;
					}
					if (do_dump && !skip_materials) {
						bigbuffer[byte] = 0;
						printf(",\"%s\"", bigbuffer);
					}
					if (!fread(&dword, sizeof(dword), 1, file)) {
						printf("while reading object %d material %d color\n", i, j);
						goto corrupted;
					}
					if (do_dump && !skip_materials) {
						printf(",0x%08X);\n", dword);
					}
				} else {
					puts("unsupported material type");
					goto corrupted;
				}
			}
		}
	}
	puts("/*\n");

	if (total_read_object_size != header.objectdata_size) {
		printf("expected total object size of %d but read %d\n", header.objectdata_size, total_read_object_size);
		goto corrupted;
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
	if (header.numobjects) {
		printf("  middle: %.2f %.2f %.2f\n", midx, midy, midz);
		printf("  min: %.2f %.2f %.2f\n", minx, miny, minz);
		printf("  max: %.2f %.2f %.2f\n", maxx, maxy, maxz);
		printf("  area: %.2f %.2f %.2f\n", maxx - minx, maxy - miny, maxz - minz);
	}
	printf("  %d different object models used\n", num_object_models);
	printf("  %d objects with %d custom materials\n", num_objects_with_materials, total_materials);
	puts("");

	if (header.numobjects > 0 && ides_loaded) {
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
	puts("*/");

ret:
	fclose(file);
	return 0;
corrupted:
	printf("file data seems to be corrupted %s\n", filename);
	goto ret;
}

int main(int argc, char **argv)
{
	int i;
	char do_dump, nomats;
	char **filev;
	unsigned char filec;

	if (!argc) {
		goto printhelp;
	}
	do_dump = 0;
	nomats = 0;
	filev = alloca(sizeof(char*) * argc);
	filec = 0;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--sadir")) {
			if (i == argc - 1) {
				goto printhelp;
			}
			i++;
			if (!ide_load(argv[i])) {
				puts("bad game dir");
				return 1;
			}
		} else if (!strcmp(argv[i], "--dump")) {
			do_dump = 1;
		} else if (!strcmp(argv[i], "--nomats")) {
			nomats = 1;
		} else {
			filev[filec] = argv[i];
			filec++;
		}
	}
	if (!filec) {
		goto printhelp;
	}
	for (i = 0; i < filec; i++) {
		if (mapinfo(filev[i], do_dump, nomats)) {
			return 1;
		}
	}
	return 0;
printhelp:
	puts("mapinfo [--sadir /path/to/game/dir] [--dump] [--nomats] map1.map [map2.map] [...]");
	return 1;
}
