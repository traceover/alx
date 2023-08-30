#define NOBUILD_IMPLEMENTATION
#include "include/nobuild.h"

#define WARNINGS "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-std=c99"
#define DEFINES  "-D_GNU_SOURCE"
#define INCLUDES "-Iinclude", "-Isource"

#if defined(_WIN32)
#define PROGRAM "alx.exe"
#else
#define PROGRAM "alx-linux"
#endif

int main(int argc, char **argv)
{
	GO_REBUILD_URSELF(argc, argv);
	CMD("clang", WARNINGS, DEFINES, INCLUDES, "-o", PROGRAM, "source/main.c");
	return 0;
}

// Additional Notes
// -D_GNU_SOURCE is required when using std=c99 to define strsignal() in nobuild.h
