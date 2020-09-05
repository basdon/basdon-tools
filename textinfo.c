#pragma pack(push, 1)

#define STATIC_ASSERT(E) typedef char __static_assert_[(E)?1:-1]
#define EXPECT_SIZE(S,SIZE) STATIC_ASSERT(sizeof(S)==(SIZE))

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FLAG_BOX 1
#define FLAG_LEFT 2
#define FLAG_RIGHT 4
#define FLAG_CENTER 8
#define FLAG_PROP 16

struct {
	char name[40];
	char flags;
	float letter_width;
	float letter_height;
	int font_colorABGR;
	float box_width;
	float box_height;
	int box_colorABGR;
	char shadow_size;
	char outline_size;
	int shadow_colorABGR;
	char font;
	char selectable;
	float x;
	float y;
	short preview_model;
	float preview_rot_x;
	float preview_rot_y;
	float preview_rot_z;
	float preview_zoom;
	short preview_col1;
	short preview_col2;
	short text_length;
	char text[800];
} text;
EXPECT_SIZE(text, 905);

int main(int argc, char **argv)
{
	FILE *file;
	int version;

	if (argc != 2) {
		puts("please give .text file as argument");
		return 1;
	}

	if (!(file = fopen(argv[1], "rb"))) {
		puts("failed to open file for reading");
		return 1;
	}

	if (fread(&version, 4, 1, file) != 1) {
		goto corrupted;
	}

	printf(".text file version %08X\n\n", version);

	if (version != 0x01545854) {
		puts("version unsupported");
		goto ret;
	}

	while (fread(&text, sizeof(text), 1, file)) {
		printf("text '%s'\n", text.name);
		printf("  flags %02X:", text.flags);
		if (text.flags & FLAG_BOX) {
			printf(" box");
		}
		if (text.flags & FLAG_LEFT) {
			printf(" left");
		}
		if (text.flags & FLAG_RIGHT) {
			printf(" right");
		}
		if (text.flags & FLAG_CENTER) {
			printf(" center");
		}
		if (text.flags & FLAG_PROP) {
			printf(" proportional");
		}
		printf("\n");
		if (text.selectable) {
			puts("  selectable");
		}
		printf("  position %f %f\n", text.x, text.y);
		printf("  font %d letter size %f %f abgr %08X\n", text.font, text.letter_width, text.letter_height, text.font_colorABGR);
		printf("  box size %f %f abgr %08X\n", text.box_width, text.box_height, text.box_colorABGR);
		printf("  shadow %d outline %d color %08x\n", text.shadow_size, text.outline_size, text.shadow_colorABGR);
		if (text.font == 5) {
			printf("  preview");
			printf("    model %d col %d %d\n", text.preview_model, text.preview_col1, text.preview_col2);
			printf("    angle %f %f %f\n", text.preview_rot_x, text.preview_rot_y, text.preview_rot_z);
			printf("    zoom %f\n", text.preview_zoom);
		}
		printf("  text(%d): %s\n", text.text_length, text.text);
		puts("");
	}

	if (!feof(file)) {
		puts("extra data at the end of the file");
		goto corrupted;
	}

ret:
	fclose(file);

	return 0;
corrupted:
	puts("file data seems to be corrupted");
	goto ret;
}
