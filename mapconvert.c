/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_DISTANCE_DRAW 1500
#define DEFAULT_DISTANCE_STREAMIN 2500
#define DEFAULT_DISTANCE_STREAMOUT 4000

#pragma pack(push,1)
#define MAX_OBJECTS 1000
#define MAX_REMOVES 1000

struct OBJECT {
	unsigned short objdata_size;
	unsigned int model;
	float x, y, z, rx, ry, rz;
	float drawdistance;
	unsigned char nocameracol;
	short attached_object_id;
	short attached_vehicle_id;
	unsigned char num_materials;
};

static struct OBJECT objects[MAX_OBJECTS];
static unsigned short num_objects;

struct OBJECT_MATERIAL_TEXTURE {
	unsigned short model_id;
	unsigned char txdname_len;
	char txdname[64];
	unsigned char texture_len;
	char texture[64];
	unsigned int color;
};

static char object_texture_slot_used[MAX_OBJECTS][16];
static struct OBJECT_MATERIAL_TEXTURE object_texture_data[MAX_OBJECTS][16];

struct REMOVE {
	unsigned short model_id;
	float x, y, z;
	float radius;
};

static struct REMOVE removes[MAX_REMOVES];
static unsigned short num_removes;

#define MAX_VARIABLES 100

struct VARIABLE {
	char name[32];
	int value;
};
static struct VARIABLE variables[MAX_VARIABLES];
static int num_variables;

static char strpool[20000], *s = strpool;

#define IDENTIFIER_CHAR 1
#define IDENTIFIER_CHAR_NOTFIRST 2
#define NUMBER_CHAR 4
#define HEX_NUMBER_CHAR 8
static unsigned char charmap[255];

#define MAX_TOKENS 50
#define T_IDENTIFIER 1
#define T_INT 2
#define T_FLOAT 3
#define T_STRING 4
#define T_LPAREN 5
#define T_RPAREN 6
#define T_EQ 7
#define T_COMMA 8
#define T_MINUS 9
__attribute__((unused))
static const char *token2string[] = {
	"<invalid>",
	"identifier",
	"int",
	"float",
	"string",
	"lparen",
	"rparen",
	"eq",
	"comma",
	"minus",
};
struct TOKEN {
	unsigned char type;
	union {
		char *identifier_value;
		int int_value;
		float float_value;
		char *string_value;
	} data;
};
static struct TOKEN tokens[MAX_TOKENS];
static int num_tokens;
static int line_number;

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

static
struct VARIABLE *register_variable(char *name)
{
	if (num_variables >= MAX_VARIABLES) {
		printf("max variables reached\n");
		exit(1);
	}
	strcpy(variables[num_variables].name, name);
	variables[num_variables].value = 0;
	return &variables[num_variables++];
}

static
struct VARIABLE *find_variable(char *name)
{
	int i;

	for (i = 0; i < num_variables; i++) {
		if (!strcmp(name, variables[i].name)) {
			return &variables[i];
		}
	}
	printf("%d: non-existing variable: %s\n", line_number, name);
	exit(1);
}

static
int arg_value_int(struct TOKEN **args, int position)
{
	if (args[position]->type == T_IDENTIFIER) {
		/*TODO(?) float variables*/
		return find_variable(args[position]->data.identifier_value)->value;
	}
	if (args[position]->type != T_INT) {
		printf("%d: expected arg %d as int but is %c\n", line_number, position, args[position]->type);
		exit(1);
	}
	return args[position]->data.int_value;
}

static
float arg_value_float(struct TOKEN **args, int position)
{
	/*TODO(?) float variables*/
	if (args[position]->type != T_FLOAT) {
		printf("%d: expected arg %d as float but is %c\n", line_number, position, args[position]->type);
		exit(1);
	}
	return args[position]->data.float_value;
}

static
void arg_value_str(char *dest, struct TOKEN **args, int position)
{
	if (args[position]->type != T_STRING) {
		printf("%d: expected arg %d as string but is %c\n", line_number, position, args[position]->type);
		exit(1);
	}
	strcpy(dest, args[position]->data.string_value);
}

static
int do_call(char *function, struct TOKEN **args, int num_args)
{
	int objectid;
	int materialindex;
	int txdlen, texturelen;

	if (!strcmp("CreateObject", function) || !strcmp("CreateDynamicObject", function)) {
		if (num_args < 7) {
			printf("%d: need at least 7 args for %s, got %d\n", line_number, function, num_args);
			exit(1);
		}
		if (num_objects >= MAX_OBJECTS) {
			printf("%d: MAX_OBJECTS reached\n", line_number);
			exit(1);
		}
		objects[num_objects].objdata_size = sizeof(struct OBJECT);
		objects[num_objects].model = arg_value_int(args, 0);
		objects[num_objects].x = arg_value_float(args, 1);
		objects[num_objects].y = arg_value_float(args, 2);
		objects[num_objects].z = arg_value_float(args, 3);
		objects[num_objects].rx = arg_value_float(args, 4);
		objects[num_objects].ry = arg_value_float(args, 5);
		objects[num_objects].rz = arg_value_float(args, 6);
		objects[num_objects].drawdistance = DEFAULT_DISTANCE_DRAW;
		objects[num_objects].nocameracol = 0;
		objects[num_objects].attached_object_id = -1;
		objects[num_objects].attached_vehicle_id = -1;
		objects[num_objects].num_materials = 0;
		return num_objects++;
	} else if (!strcmp("SetDynamicObjectMaterial", function)) {
		if (num_args != 6) {
			printf("%d: need at 6 args for %s, got %d\n", line_number, function, num_args);
			exit(1);
		}
		objectid = arg_value_int(args, 0);
		if (objectid < 0 || objectid >= num_objects) {
			printf("%d: calling %s on bad object id (%d, num_objects %d)\n", line_number, function, objectid, num_objects);
			exit(1);
		}
		materialindex = arg_value_int(args, 1);
		if (materialindex < 0 || materialindex > 15) {
			printf("%d: invalid materialindex %d\n", line_number, materialindex);
			exit(1);
		}
		object_texture_slot_used[objectid][materialindex] = 1;
		object_texture_data[objectid][materialindex].model_id = arg_value_int(args, 2);
		arg_value_str(object_texture_data[objectid][materialindex].txdname, args, 3);
		txdlen = strlen(object_texture_data[objectid][materialindex].txdname);
		object_texture_data[objectid][materialindex].txdname_len = txdlen;
		arg_value_str(object_texture_data[objectid][materialindex].texture, args, 4);
		texturelen = strlen(object_texture_data[objectid][materialindex].texture);
		object_texture_data[objectid][materialindex].texture_len = texturelen;
		object_texture_data[objectid][materialindex].color = (unsigned int) arg_value_int(args, 5);
		objects[objectid].num_materials++;
		objects[objectid].objdata_size += sizeof(short) + sizeof(char) * 2 + txdlen + texturelen;
		return 0;
	} else if (!strcmp("RemoveBuildingForPlayer", function)) {
		if (num_args < 6) {
			printf("%d: need at least 6 args for %s, got %d\n", line_number, function, num_args);
			exit(1);
		}
		if (num_removes >= MAX_REMOVES) {
			printf("%d: MAX_REMOVES reached\n", line_number);
			exit(1);
		}
		removes[num_removes].model_id = arg_value_int(args, 1);
		removes[num_removes].x = arg_value_float(args, 2);
		removes[num_removes].y = arg_value_float(args, 3);
		removes[num_removes].z = arg_value_float(args, 4);
		removes[num_removes].radius = arg_value_float(args, 5);
		return num_removes++;
	} else {
		printf("%d: unknown function: %s\n", line_number, function);
		exit(1);
	}
}

static
void inctoken()
{
	num_tokens++;
	if (num_tokens >= MAX_TOKENS) {
		printf("%d: too many tokens\n", line_number);
		exit(1);
	}
}

static
void combine_minus_prefix_with_number()
{
	int read, write;

	for (write = 0, read = 0; read < num_tokens; read++, write++) {
		if (tokens[read].type == T_MINUS && read < num_tokens - 1) {
			switch (tokens[read + 1].type) {
			/*TODO: negative variable read :)*/
			case T_INT:
				read++;
				tokens[read].data.int_value = -tokens[read].data.int_value;
				break;
			case T_FLOAT:
				read++;
				tokens[read].data.float_value = -tokens[read].data.float_value;
				break;
			}
		}
		if (write != read) {
			tokens[write] = tokens[read];
		}
	}
	num_tokens = write;

	for (read = 0; read < num_tokens; read++) {
		dprintf("%s\n", token2string[tokens[read].type]);
	}
}

static
int sym_exec_tokens(int fromindex)
{
	struct VARIABLE *variable;
	struct TOKEN *call_args[MAX_TOKENS];
	int num_call_args;
	int i;

	if (tokens[fromindex].type == T_IDENTIFIER) {
		if (!strcmp("new", tokens[0].data.identifier_value)) {
			if (fromindex != 0) {
				printf("%d: 'new' as non-first token\n", line_number);
				exit(1);
			}
			if (num_tokens < 2) {
				printf("%d: loose 'new'\n", line_number);
				exit(1);
			}
			/*variable decl*/
			variable = register_variable(tokens[1].data.identifier_value);
			dprintf("registered variable %s\n", variable->name);
			if (num_tokens > 2) {
				return variable->value = sym_exec_tokens(2);
			}
			return 0;
		} else if (num_tokens > fromindex) {
			if (tokens[fromindex + 1].type == T_EQ) {
				/*variable assignm*/
				if (num_tokens == fromindex + 2) {
					printf("%d: line ending with EQ\n", line_number);
					exit(1);
				}
				variable = find_variable(tokens[fromindex].data.identifier_value);
				variable->value = sym_exec_tokens(fromindex + 2);
				dprintf("assigned variable %s to %d\n", variable->name, variable->value);
				return variable->value;
			} else if (tokens[fromindex + 1].type == T_LPAREN) {
				if (fromindex + 2 >= num_tokens) {
					printf("%d: unclosed functioncall\n", line_number);
					exit(1);
				}
				/*functioncall*/
				dprintf("functioncall %s\n", tokens[fromindex].data.identifier_value);
				num_call_args = 0;
				for (i = fromindex + 2; i < num_tokens; i++) {
					switch (tokens[i].type) {
					case T_IDENTIFIER:
					case T_INT:
					case T_FLOAT:
					case T_STRING:
						call_args[num_call_args++] = &tokens[i];
						break;
					default:
						printf("%d: unk token type %d as call arg\n", line_number, tokens[i].type);
						exit(1);
					}
					i++;
					if (i >= num_tokens) {
						printf("%d: call ends without RPAREN\n", line_number);
						exit(1);
					}
					switch (tokens[i].type) {
					case T_COMMA:
						continue;
					case T_RPAREN:
						break;
					default:
						printf("%d: unexpected token type %d after call arg (num_args %d)\n", line_number, tokens[i].type, num_call_args);
						exit(1);
					}
					break;
				}
				/*this is not really a parser*/
				if (tokens[i].type != T_RPAREN) {
					printf("%d: no ending RPAREN in functioncall (%d, num_args %d)\n", line_number, tokens[i].type, num_call_args);
					exit(1);
				}
				if (i != num_tokens - 1) {
					printf("%d: stuff after functioncall\n", line_number);
					exit(1);
				}
				return do_call(tokens[fromindex].data.identifier_value, call_args, num_call_args);
			}
		} else {
			/*variable access*/
			variable = find_variable(tokens[fromindex].data.identifier_value);
			dprintf("accessing variable %s (value %d)\n", variable->name, variable->value);
			return variable->value;
		}
	}
	printf("%d: starts with token %d\n", line_number, tokens[0].type);
	exit(1);
}

/*tokenizer/parser/symbolic executor/whatever/idk/basic*/
static
int parse()
{
	unsigned char line[1000], tmp[1000], *t;
	unsigned char c, type;
	int pos;
	char is_int_hex;
	int i;

	for (i = 'a'; i <= 'z'; i++) {
		charmap[i] |= IDENTIFIER_CHAR | IDENTIFIER_CHAR_NOTFIRST;
	}
	for (i = 'A'; i <= 'Z'; i++) {
		charmap[i] |= IDENTIFIER_CHAR | IDENTIFIER_CHAR_NOTFIRST;
	}
	for (i = '0'; i <= '9'; i++) {
		charmap[i] |= IDENTIFIER_CHAR_NOTFIRST | NUMBER_CHAR;
	}
	for (i = 'a'; i <= 'f'; i++) {
		charmap[i] |= HEX_NUMBER_CHAR;
	}
	for (i = 'A'; i <= 'F'; i++) {
		charmap[i] |= HEX_NUMBER_CHAR;
	}
	charmap['_'] = IDENTIFIER_CHAR;
	charmap['@'] = IDENTIFIER_CHAR;

	line_number = 0;
	while (fgets((char*) line, sizeof(line), stdin)) {
		s = strpool;
		line_number++;
		dprintf("line %d\n", line_number);
		pos = -1;
		num_tokens = 0;

nextchar:
		c = line[++pos];
havenextchar:
		type = charmap[c];
		if (c == ' ' || c == '\t') {
			goto nextchar;
		}
		if (type & IDENTIFIER_CHAR) {
			tokens[num_tokens].type = T_IDENTIFIER;
			tokens[num_tokens].data.identifier_value = s;
			do {
				*(s++) = c;
				c = line[++pos];
			} while (charmap[c] & IDENTIFIER_CHAR_NOTFIRST);
			*(s++) = 0;
			dprintf("IDENTIFIER: %s\n", tokens[num_tokens].data.identifier_value);
			inctoken();
			goto havenextchar;
		}
		if (type & NUMBER_CHAR) {
			tokens[num_tokens].type = T_INT;
			t = tmp;
			is_int_hex = 0;
			for (;;) {
				*(t++) = c;
				c = line[++pos];
				if (c == '.') {
					if (tokens[num_tokens].type == T_FLOAT) {
						printf("%d:%d: unexpected . while parsing float\n", line_number, pos);
						exit(1);
					}
					tokens[num_tokens].type = T_FLOAT;
				} else if (c == 'x' || c == 'X') {
					if (t != tmp + 1) {
						printf("%d:%d: unexpected x while parsing num\n", line_number, pos);
						exit(1);
					}
					is_int_hex = 1;
				} else if (!(charmap[c] & NUMBER_CHAR)) {
					if (!((charmap[c] & HEX_NUMBER_CHAR) && is_int_hex)) {
						if (charmap[c] & (IDENTIFIER_CHAR | IDENTIFIER_CHAR_NOTFIRST)) {
							printf("%d:%d unexpected %d while parsing num\n", line_number, pos, c);
							exit(1);
						}
						break;
					}
				}
			}
			*t = 0;
			if (tokens[num_tokens].type == T_FLOAT) {
				tokens[num_tokens].data.float_value = atof((char*) tmp);
				dprintf("FLOAT: %f\n", tokens[num_tokens].data.float_value);
			} else if (is_int_hex) {
				tokens[num_tokens].data.int_value = 0;
				for (i = 2; i < 10; i++) {
					if (!tmp[i]) {
						break;
					}
					tokens[num_tokens].data.int_value <<= 4;
					if (tmp[i] > 'Z') {
						tokens[num_tokens].data.int_value |= 0xF & (10 + tmp[i] - 'a');
					} else if (tmp[i] > '9') {
						tokens[num_tokens].data.int_value |= 0xF & (10 + tmp[i] - 'A');
					} else {
						tokens[num_tokens].data.int_value |= 0xF & (tmp[i] - '0');
					}
				}
				dprintf("INT: %d (from hex %s)\n", tokens[num_tokens].data.int_value, tmp);
			} else {
				tokens[num_tokens].data.int_value = atoi((char*) tmp);
				dprintf("INT: %d\n", tokens[num_tokens].data.int_value);
			}
			inctoken();
			goto havenextchar;
		}
		if (c == '"') {
			tokens[num_tokens].type = T_STRING;
			tokens[num_tokens].data.string_value = s;
			/*not handling escape chars yet*/
			for (;;) {
				c = line[++pos];
				if (c == '\n') {
					printf("%d: unexpected EOL while parsing string\n", line_number);
					exit(1);
				}
				if (c == '"') {
					break;
				}
				*(s++) = c;
			}
			*(s++) = 0;
			dprintf("STRING: %s\n", tokens[num_tokens].data.string_value);
			inctoken();
			goto nextchar;
		}
		if (c == '(') {
			tokens[num_tokens].type = T_LPAREN;
			dprintf("LPAREN\n");
			inctoken();
			goto nextchar;
		}
		if (c == ')') {
			tokens[num_tokens].type = T_RPAREN;
			dprintf("RPAREN\n");
			inctoken();
			goto nextchar;
		}
		if (c == ',') {
			tokens[num_tokens].type = T_COMMA;
			dprintf("COMMA\n");
			inctoken();
			goto nextchar;
		}
		if (c == '-') {
			tokens[num_tokens].type = T_MINUS;
			dprintf("MINUS\n");
			inctoken();
			goto nextchar;
		}
		if (c == '=') {
			tokens[num_tokens].type = T_EQ;
			dprintf("EQ\n");
			inctoken();
			goto nextchar;
		}
		if (c == '\n') {
			if (!num_tokens) {
				continue;
			}
			printf("%d: doesn't end in ;\n", line_number);
			exit(1);
		}
		if (c != ';') {
			printf("%d:%d unk char %c\n", line_number, pos, c);
			exit(1);
		}
		do {
			pos++;
			c = line[pos];
		} while (c == ' ' || c == '\t');
		if (c != '\n') {
			printf("%d: stuff after ;\n", line_number);
			exit(1);
		}
		/*some patchwork*/
		combine_minus_prefix_with_number();
		/*do the line*/
		dprintf("executing line:\n");
		if (num_tokens) {
			sym_exec_tokens(0);
		}
		dprintf("\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *ofile;
	int result = 0;
	int i;
	unsigned char len, matidx;
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

	if (argc != 2) {
		puts("need one arg for outputfile");
		return 1;
	}

	memheader.version = 0x0350414D;
	if (((char*) &memheader)[0] != 'M' || ((char*) &memheader)[3] != 3) {
		puts("wrong endianness or structs are packed");
		return 2;
	}

	parse(argv[1]); /*may exit*/

	ofile = fopen(argv[1], "wb");
	if (ofile == NULL) {
		return 1;
	}

	memheader.numremoveobj = num_removes;
	memheader.numobj = num_objects;
	memheader.numgang = -1;
	memheader.streamin = DEFAULT_DISTANCE_STREAMIN;
	memheader.streamout = DEFAULT_DISTANCE_STREAMOUT;
	memheader.drawdistance = DEFAULT_DISTANCE_DRAW;
	fwrite(&memheader, sizeof(memheader), 1, ofile);
	memheader.objdata_size = 0;
	memheader.numgang = 0;

	for (i = 0; i < num_removes; i++) {
		fwrite(&removes[i], sizeof(struct REMOVE), 1, ofile);
	}

	for (i = 0; i < num_objects; i++) {
		memheader.objdata_size += objects[i].objdata_size;
		fwrite(&objects[i], sizeof(struct OBJECT), 1, ofile);
		if (objects[i].num_materials) {
			for (matidx = 0; matidx < 16; matidx++) {
				if (object_texture_slot_used[i][matidx]) {
					len = 1; /*material type*/
					fwrite(&len, sizeof(char), 1, ofile);
					fwrite(&matidx, sizeof(char), 1, ofile);
					fwrite(&object_texture_data[i][matidx].model_id, sizeof(short), 1, ofile);
					len = object_texture_data[i][matidx].txdname_len;
					fwrite(&len, 1, 1, ofile);
					fwrite(object_texture_data[i][matidx].txdname, len, 1, ofile);
					len = object_texture_data[i][matidx].texture_len;
					fwrite(&len, 1, 1, ofile);
					fwrite(object_texture_data[i][matidx].texture, len, 1, ofile);
					fwrite(&object_texture_data[i][matidx].color, sizeof(int), 1, ofile);
				}
			}
		}
	}

	if (fseek(ofile, 0, SEEK_SET)) {
		fputs("failed to rewrite header", stderr);
		fclose(ofile);
		return 1;
	}
	fwrite(&memheader, sizeof(memheader), 1, ofile);
	fclose(ofile);
	return result;
}
/*----------------------------------------------------------------------------*/
