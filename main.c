#include <stdio.h>
#include <stdint.h>

#define MAX_MEMORY (1 << 16)
uint16_t memory[MAX_MEMORY];

/* Registers */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,   /* Program Counter */
    R_COND, /* Conditional */
    R_COUNT  /* Number of Registers */
};

/* Register Storage */
uint16_t reg[R_COUNT];

/* Opcodes */
enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise AND */
    OP_NOT,    /* bitwise NOT */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved */
    OP_LEA,    /* load effective address */
    OP_TRP     /* trap */
};

/* Condition Flags */
enum {
    FL_POS = 1 << 0,  /* POSITIVE */
    FL_ZER = 1 << 1,  /* ZERO */
    FL_NEG = 1 << 2   /* NEGATIVE */
};

