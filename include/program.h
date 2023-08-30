#if !defined(PROGRAM_H_)
#define PROGRAM_H_

// Sections are a wrapper for OS segments that change the meaning of a declaration.
// Some sections (such as typedefs and constants) do not show up in the output of
// the final executable, but are used to declare names only needed for generating
// machine code (such as compile-time constants). Other sections, such as procedures
// and strdata directly translate into target machine code.
typedef enum Section {
	Section_Procedures,
	Section_Constants,
	Section_Typedefs,
	Section_Strdata,
} Section;

// The Lookup Table is mapping from a string to a global (if it exists).
// There are also separate arrays for each different *kind* of declaration,
// see the definition of the Program structure for more information.
// This could probably become a hash table at some point, but we haven't
// run into a situation where we needed one, yet.
typedef struct Lookup_Table_Entry {
	String_View name;    // The identifier used to define us
	Section     section; // What kind of entry are we?
	size_t      index;   // The index into the array for this section (see Program structure)
} Lookup_Table_Entry;

typedef struct Lookup_Table {
	Lookup_Table_Entry *items;
	size_t count;
	size_t capacity;
} Lookup_Table;

// 
typedef struct Procedure {
	IP start, end;
} Procedure;

typedef int Polyvalue; // @Unfinished: We need a way of storing any kind of value here.

typedef struct Constant {
	Polyvalue value;
} Constant;

// This is the beginning of a list of dynamic array types, used to
// store every single different kind of declaration in the user's
// program. Because we have a separate array for each type, it is
// super fast/cache-friendly to iterate all of one type and perform
// an action over it (the Fundamental Theory of Programming?). If
// you want a list of *all* declarations in the program, check out
// the definition of the Lookup_Table structure.
typedef struct Instructions {
	Instr *items;
	size_t count;
	size_t capacity;
} Instructions;

typedef struct Procedures {
	Procedure *items;
	size_t count;
	size_t capacity;
} Procedures;

typedef struct Constants {
	Constant *items;
	size_t count;
	size_t capacity;
} Constants;

typedef struct String_Table {
	char *items;
	size_t count;
	size_t capacity;
} String_Table;

// At last, we have the definition of the Program structure.
// Notice how a program is just a list of sections, where each
// section is a list of different kinds of declaration. This
// should look *very* close to actual assembly code.
typedef struct Program {
	Instructions instructions;
	Lookup_Table lookup_table;
	Type_Table   type_table;
	Procedures   procedures;
	Constants    constants;
	String_Table string_table;
} Program;

#endif // PROGRAM_H_
