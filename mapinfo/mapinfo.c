#include <stdio.h>
#include <string.h>

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
	if (f = fopen(filename, "r")) {
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
				puts("ide name pool depleted\n");
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

int main(int argc, char **argv)
{
	FILE *file;
	int spec_version;
	int num_objects;
	int num_removes;
	int read_objects;
	int read_removes;
	int num_object_models;
	int num_remove_models;
	int i;
	int model;
	int modelusage[MAX_MODELS];
	int modelremoves[MAX_MODELS];
	float midx, midy;
	int data_index;
	char *mapfilepath;
	char *sadir;
	struct OBJECT {
		float x, y, z, rx, ry, rz, drawdistance;
	} object;
	struct REMOVEDBUILDING {
		float x, y, z, radius;
	} remove;

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
		puts("failed to read file");
		return 1;
	}

	if (fread(&spec_version, 4, 1, file) != 1) {
		goto corrupted;
	}

	printf(".map file version %d\n", spec_version);

	if (fread(&num_removes, 4, 1, file) != 1) {
		goto corrupted;
	}

	printf("remove count: %d\n", num_removes);

	if (fread(&num_objects, 4, 1, file) != 1) {
		goto corrupted;
	}

	printf("object count: %d\n", num_objects);

	data_index = 0;
	read_objects = 0;
	read_removes = 0;
	midx = 0.0f;
	midy = 0.0f;
	while (fread(&model, 4, 1, file)) {
		if (model < -19999 || 19999 < model) {
			printf("invalid model at index %d: %d\n", data_index, model);
			goto corrupted;
		}
		if (model < 0) {
			read_removes++;
			modelremoves[-model]++;
			if (fread(&remove, sizeof(remove), 1, file) != 1) {
				puts("unexpected EOF while reading removed building data");
				goto corrupted;
			}
		} else {
			read_objects++;
			modelusage[model]++;
			if (fread(&object, sizeof(object), 1, file) != 1) {
				puts("unexpected EOF while reading object data");
				goto corrupted;
			}
			midx += object.x;
			midy += object.y;
			printf("CreateObject(%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f);\n",
			model, object.x, object.y, object.z, object.rx, object.ry, object.rz, object.drawdistance);
		}
		data_index++;
	}
	if (read_objects > 0) {
		midx /= read_objects;
		midy /= read_objects;
	}

	if (read_objects != num_objects) {
		printf("file header said %d objects but %d were found\n", num_objects, read_objects);
	}

	if (read_removes != num_removes) {
		printf("file header said %d removes but %d were found\n", num_removes, read_removes);
	}

	printf("middle: %.2f %.2f\n", midx, midy);

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
	printf("%d different object models used\n", num_object_models);
	printf("%d different remove models used\n", num_remove_models);

	if (read_objects > 0 && sadir) {
		ide_load(sadir);
		for (i = 0; i < MAX_MODELS; i++) {
			if (modelusage[i]) {
				printf("%03dx %05d %s\n", modelusage[i], i, modelNames[i]);
			}
		}
	}

end:
	fclose(file);
	puts("\npress any key to exit");
	getch();

	return 0;
corrupted:
	puts("file data seems to be corrupted");
	goto end;
}
