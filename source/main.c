#include <stdio.h>
#include <string.h>

#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"
#define SV_IMPLEMENTATION
#include "sv.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"

#include "tokenize.c"

static Arena gpa = {0}; // General-purpose arena
static Arena tmp = {0}; // Temporary       arena

int main(int argc, char **argv)
{
	char *program = shift_args(&argc, &argv);
	(void)program;
	(void)tmp;

	if (argc < 1) {
		PANIC("missing input file path command-line argument");
	}
	
	char *input_path_cstr = shift_args(&argc, &argv);

	String_View contents;
	if (arena_slurp_file(&gpa, input_path_cstr, &contents) != 0) {
		PANIC("failed reading file '%s': %s", input_path_cstr, strerror(errno));
	}
	
	// Tokenize the file and display each token
	Token token;
	String_View input = contents;
	while (parse_token(&input, &token)) {
		printf("%d: "SV_Fmt"\n", token.type, SV_Arg(token.text));
	}

	INFO("successfully read input file");
	return 0;
}
