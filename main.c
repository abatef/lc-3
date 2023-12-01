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
    OP_BR  = 0x0,    /* branch */
    OP_ADD = 0x1,    /* add */
    OP_LD  = 0x2,    /* load */
    OP_ST  = 0x3,    /* store */
    OP_JSR = 0x4,    /* jump register */
    OP_AND = 0x5,    /* bitwise AND */
    OP_LDR = 0x6,    /* load register */
    OP_STR = 0x7,    /* store register */
    OP_RTI = 0x8,    /* unused */
    OP_NOT = 0x9,    /* bitwise NOT */
    OP_LDI = 0xA,    /* load indirect */
    OP_STI = 0xB,    /* store indirect */
    OP_JMP = 0xC,    /* jump */
    OP_RES = 0xD,    /* reserved */
    OP_LEA = 0xE,    /* load effective address */
    OP_TRP = 0xF     /* trap */
};

uint16_t mem_read(uint16_t inst);
uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}
uint16_t update_flags(uint16_t r) {
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZER;
    } else if (reg[r] >> 15) {
        reg[R_COND] == FL_NEG;
    } else {
        reg[R_COND] == FL_POS;
    }
}

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
            {
                    uint16_t DR = (inst >> 9) & 0x7;
                    uint16_t SR1 = (inst >> 6) & 0x7;
                    uint16_t IMM_FL = (inst >> 5) & 0x1;
                    if (IMM_FL) {
                        uint16_t IMM = sign_extend(inst & 0x1f, 5);
                        reg[DR] = reg[SR1] + IMM;
                    } else {
                        uint16_t SR2 = (inst & 0x7);
                        reg[DR] = reg[SR1] + reg[SR2];
                    }
                update_flags(DR);
            }
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
            {

            }
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

