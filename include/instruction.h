#if !defined(INSTRUCTION_H_)
#define INSTRUCTION_H_

typedef enum Opcode {
	NOP = 0, // TODO: Make zero be PANIC so that we hit this if we forget to initialize
	PUSH_CONSTANT,
	INTEGER_ADD,
	INTEGER_PRINT,
} Opcode;

typedef struct Instr {
	int opcode;
	int operand[3];
} Instr;

typedef size_t IP; // Instruction Pointer

#endif // INSTRUCTION_H_
