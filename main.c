#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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

/* Condition Flags */
enum {
    FL_POS = 1 << 0,  /* POSITIVE */
    FL_ZER = 1 << 1,  /* ZERO */
    FL_NEG = 1 << 2   /* NEGATIVE */
};

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

uint16_t mem_read(uint16_t inst);

bool read_image(char *string);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("lc-3 [image-file] ...\n");
        exit(2);
    }
    for (int i = 0; i < argc; i++) {
        if (!read_image(argv[i])) {
            printf("failed to load image: %s\n", argv[i]);
            exit(1);
        }
    }
    reg[R_COND] = FL_ZER;

    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while (running) {
        uint16_t inst = mem_read(reg[R_PC]++);
        uint16_t op = inst >> 12;

        switch (op) {
            case OP_ADD:
                /* ADD */
            case OP_AND:
                /* AND */
            case OP_BR:
                /* BRANCH */
            case OP_JMP:
                /* JUMP */
            case OP_JSR:
                /* JSR */
            case OP_NOT:
                /* NOT */
            case OP_LD:
                /* LOAD */
            case OP_LDI:
                /* LOAD INDIRECT */
            case OP_LDR:
                /* LOAD REGISTER */
            case OP_LEA:
                /* LOAD EFFECTIVE ADDRESS */
            case OP_RES:
                /* RES */
            case OP_RTI:
                /* RTI */
            case OP_ST:
                /* ST */
            case OP_STI:
                /* STI */
            case OP_STR:
                /* STR */
            case OP_TRP:
                /* TRAP */
            default:
                break;
        }
    }
}

