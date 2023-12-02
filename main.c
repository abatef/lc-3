#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

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

/* TRAPS */
enum {
    TRP_GETC  = 0x20, /* get character from the keyboard */
    TRP_OUT   = 0x21, /* output a character */
    TRP_PUTS  = 0x22, /* output a string */
    TRP_IN    = 0x23, /* get character from the terminal */
    TRP_PUTSP = 0x24, /* output a byte string */
    TRP_HALT  = 0x25 /* halt the execution */
};

enum {
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};


uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t mem_read(uint16_t address) {
    if (address == MR_KBSR) {
        if (check_key()) {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        } else {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

void mem_write(uint16_t address, uint16_t val) {
    memory[address] = val;
}

uint16_t sign_extend(uint16_t x, int bit_c) {
    if ((x >> (bit_c - 1)) & 1) {
        x |= (0xFFFF << bit_c);
    }
    return x;
}

uint16_t zero_extend(uint16_t x, int bit_c) {
    return (x &= (0x00FF >> bit_c));
}

uint16_t update_flags(uint16_t r) {
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZER;
    } else if (reg[r] >> 15) {
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
    return reg[r];
}

uint16_t swap16(uint16_t n) {
    return ((n >> 8) | (n << 8));
}

void read_image_file(FILE *file) {
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = MAX_MEMORY - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while (read--) {
        *p = swap16(*p);
        p++;
    }
}

int read_image(char *image_path) {
    FILE *file = fopen(image_path, "rb");
    if (!file) { return 0; }
    read_image_file(file);
    fclose(file);
    return 1;
}

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt() {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    if (argc < 2) {
        printf("lc-3 [image-file] ...\n");
        exit(2);
    }
    for (int i = 1; i < argc; i++) {
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
                    uint16_t IMM = sign_extend(inst & 0x1F, 5);
                    reg[DR] = reg[SR1] + IMM;
                } else {
                    uint16_t SR2 = (inst & 0x7);
                    reg[DR] = reg[SR1] + reg[SR2];
                }
                update_flags(DR);
            }
                break;
            case OP_AND:
                /* AND */
            {
                uint16_t DR = (inst >> 9) & 0x7;
                uint16_t SR1 = (inst >> 6) & 0x7;
                uint16_t IMM_FL = (inst >> 5) & 0x1;
                if (IMM_FL) {
                    uint16_t IMM = sign_extend((inst & 0x1F), 5);
                    reg[DR] = reg[SR1] & IMM;
                } else {
                    uint16_t SR2 = (inst & 0x7);
                    reg[DR] = reg[SR1] & reg[SR2];
                }
                update_flags(DR);
            }
                break;
            case OP_BR:
                /* BRANCH */
            {
                uint16_t P = (inst >> 9) & 0x7;
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                if (P & reg[R_COND]) {
                    reg[R_PC] += PC_OFFSET;
                }
            }
                break;
            case OP_JMP:
                /* JUMP */
            {
                uint16_t LOC = (inst >> 6) & 0x7;
                reg[R_PC] = reg[LOC];
            }
                break;
            case OP_JSR:
                /* JUMP TO LOCATION SPECIFIED BY REGISTER */
            {
                uint16_t SR_FL = (inst >> 11) & 0x1;
                reg[R_R7] = reg[R_PC];
                if (SR_FL) {
                    uint16_t PC_OFFSET = (inst & 0x7FF);
                    reg[R_PC] += sign_extend(PC_OFFSET, 11);
                } else {
                    uint16_t BASE = (inst >> 6) & 0x7;
                    reg[R_PC] = reg[BASE];
                }
            }
                break;
            case OP_NOT:
                /* NOT */
            {
                uint16_t DR = (inst >> 9) & 0x7;
                uint16_t SR = (inst >> 6) & 0x7;
                reg[DR] = ~reg[SR];
                update_flags(DR);
            }
                break;
            case OP_LD:
                /* LOAD */
            {
                uint16_t DR = (inst >> 9) & 0x7;
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                reg[DR] = mem_read(reg[R_PC] + PC_OFFSET);
                update_flags(DR);
            }
                break;
            case OP_LDI:
                /* LOAD INDIRECT */
            {
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                uint16_t DR = (inst >> 9) & 0x7;
                reg[DR] = mem_read(mem_read(reg[R_PC] + PC_OFFSET));
                update_flags(DR);
            }
                break;
            case OP_LDR:
                /* LOAD REGISTER */
            {
                uint16_t DR = (inst >> 9) & 0x7;
                uint16_t BASE = (inst >> 6) & 0x7;
                uint16_t OFFSET6 = sign_extend((inst & 0x3F), 6);
                reg[DR] = mem_read(OFFSET6 + reg[BASE]);
                update_flags(DR);
            }
                break;
            case OP_LEA:
                /* LOAD EFFECTIVE ADDRESS */
            {
                uint16_t DR = (inst >> 9) & 0x7;
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                reg[DR] = PC_OFFSET + reg[R_PC];
                update_flags(DR);
            }
                break;

                /* RTI */
//                abort();
            case OP_ST:
                /* STORE */
            {
                uint16_t SR = (inst >> 9) & 0x7;
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                mem_write(reg[R_PC] + PC_OFFSET, reg[SR]);
            }
                break;
            case OP_STI:
                /* STORE INDIRECT */
            {
                uint16_t SR = (inst >> 9) & 0x7;
                uint16_t PC_OFFSET = sign_extend((inst & 0x1FF), 9);
                mem_write(mem_read(reg[R_PC] + PC_OFFSET), reg[SR]);
            }
                break;
            case OP_STR:
                /* STORE IN REGISTER */
            {
                uint16_t SR = (inst >> 9) & 0x7;
                uint16_t BASE = (inst >> 6) & 0x7;
                uint16_t OFFSET6 = sign_extend((inst & 0x3F), 6);
                mem_write(reg[BASE] + OFFSET6, reg[SR]);
            }
                break;
            case OP_TRP:
                /* TRAP */
            {
                reg[R_R7] = reg[R_PC];
                uint16_t TRP_VEC8 = (inst & 0xFF);
                switch (TRP_VEC8) {
                    case TRP_GETC:
                    {
                        reg[R_R0] = (uint16_t)getchar();
                        update_flags(R_R0);
                    }
                        break;
                    case TRP_PUTS:
                    {
                        uint16_t* c = memory + reg[R_R0];
                        while (*c) {
                            putc((char) *c, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                        break;
                    case TRP_OUT:
                    {
                        putc((char) reg[R_R0], stdout);
                        fflush(stdout);
                    }
                        break;
                    case TRP_IN:
                    {
                        printf("> ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);
                        reg[R_R0] = (uint16_t) c;
                        update_flags(R_R0);
                    }
                        break;
                    case TRP_PUTSP:
                    {
                        uint16_t* c = memory + reg[R_R0];
                        while (*c) {
                            char c1 = (*c) & 0xFF;
                            putc(c1, stdout);
                            char c2 = (*c) >> 8;
                            if (c2) putc(c2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                        break;
                    case TRP_HALT:
                    {
                        printf("Halting The Execution...\n");
                        running = 0;
                    }
                    default:
                        break;
                }
            }
            case OP_RES:
                /* RES */
            case OP_RTI:
                /* RTI */
            default:
                break;
        }
    }
    restore_input_buffering();
}


