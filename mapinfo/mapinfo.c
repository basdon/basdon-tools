#include <stdio.h>
#include <string.h>

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
	int modelusage[20000];
	int modelremoves[20000];
	int data_index;
	struct OBJECT {
		float x, y, z, rx, ry, rz, drawdistance;
	} object;
	struct REMOVEDBUILDING {
		float x, y, z, radius;
	} remove;

	memset(&modelusage, 0, sizeof(modelusage));
	memset(&modelremoves, 0, sizeof(modelremoves));

	if (argc < 2) {
		puts("please give .map file as argument");
		return 1;
	}

	if (!(file = fopen(argv[1], "rb"))) {
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
		}
		data_index++;
	}

	if (read_objects != num_objects) {
		printf("file header said %d objects but %d were found\n", num_objects, read_objects);
	}

	if (read_removes != num_removes) {
		printf("file header said %d removes but %d were found\n", num_removes, read_removes);
	}

	num_object_models = 0;
	num_remove_models = 0;
	for (i = 0; i < 20000; i++) {
		if (modelusage[i]) {
			num_object_models++;
		}
		if (modelremoves[i]) {
			num_remove_models++;
		}
	}
	printf("%d different object models used\n", num_object_models);
	printf("%d different remove models used\n", num_remove_models);

end:
	fclose(file);
	puts("\npress any key to exit");
	getch();

	return 0;
corrupted:
	puts("file data seems to be corrupted");
	goto end;
}
