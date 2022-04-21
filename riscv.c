#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "linkedlist.h"
#include "hashtable.h"
#include "riscv.h"

/************** BEGIN HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ***************/
const int R_TYPE = 0;
const int I_TYPE = 1;
const int MEM_TYPE = 2;
const int U_TYPE = 3;
const int UNKNOWN_TYPE = 4;


/**
 * Return the type of instruction for the given operation
 * Available options are R_TYPE, I_TYPE, MEM_TYPE, UNKNOWN_TYPE
 */
static int get_op_type(char *op)
{
    const char *r_type_op[] = {"add", "sub", "and", "or", "xor", "slt", "sll", "sra"};
    const char *i_type_op[] = {"addi", "andi", "ori", "xori", "slti"};
    const char *mem_type_op[] = {"lw", "lb", "sw", "sb"};
    const char *u_type_op[] = {"lui"};
    for (int i = 0; i < (int)(sizeof(r_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(r_type_op[i], op) == 0)
        {
            return R_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(i_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(i_type_op[i], op) == 0)
        {
            return I_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(mem_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(mem_type_op[i], op) == 0)
        {
            return MEM_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(u_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(u_type_op[i], op) == 0)
        {
            return U_TYPE;
        }
    }
    return UNKNOWN_TYPE;
}
/*************** END HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ****************/

registers_t *registers;
// TODO: create any additional variables to store the state of the interpreter
hashtable_t *table;

void init(registers_t *starting_registers)
{
    registers = starting_registers;
    // TODO: initialize any additional variables needed for state
    table = ht_init(24062001);
}

// TODO: create any necessary helper functions
char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *strsep(char **stringp, const char *delim) {
    char *rv = *stringp;
    if (rv) {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0; }
    return rv;
}


int sign_extend_lb(int number) {
    if ((number & 0x00000080) != 0) {
        number = number | 0xffffff00;
    }
    return number;
}


void r_type(char *op, char *rd, char *rs1, char *rs2) {
    int a = registers->r[strtol(rs1, NULL, 10)];
    int b = registers->r[strtol(rs2, NULL, 10)];
    if (strcmp(op, "add") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a + b;
    } else if (strcmp(op, "sub") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a - b;
    } else if (strcmp(op, "and") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a & b;
    } else if (strcmp(op, "or") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a | b;
    } else if (strcmp(op, "xor") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a ^ b;
    } else if (strcmp(op, "slt") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a < b;
    } else if (strcmp(op, "sll") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a << b;
    } else if (strcmp(op, "sra") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a >> b;
    } 
}

void i_type(char *op, char *rd, char *rs1, char *imm) {
    int a = registers->r[strtol(rs1, NULL, 10)];
    int b = strtol(imm, NULL, 16);
    if (strcmp(op, "addi") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a + b;
    } else if (strcmp(op, "andi") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a & b;
    } else if (strcmp(op, "ori") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a | b;
    } else if (strcmp(op, "xori") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a ^ b;
    } else if (strcmp(op, "slti") == 0) {
        registers->r[strtol(rd, NULL, 10)] = a < b;
    } 
}

void mem_type(hashtable_t *table, char *op, char *rd, char *rs1, char *imm) {
    int offset = strtol(imm, NULL, 16);
    int address = registers->r[strtol(rs1, NULL, 10)] + offset;
    int rs1_value = registers->r[strtol(rd, NULL, 10)];
    int saved_register = rs1_value + offset;
    int saved_register_value = registers->r[rs1_value + offset];

    if (strcmp(op, "lw") == 0) {
        int result = ht_get(table, address);
        for (int i=0; i<4; i++) {
            result += ht_get(table, address+i) << (8*i);
        }
        registers->r[strtol(rd, NULL, 10)] = result;
    } else if (strcmp(op, "lb") == 0) {
        int result = ht_get(table, address);
        registers->r[strtol(rd, NULL, 10)] = result;
    } else if (strcmp(op, "sw") == 0) {
        for (int i=0; i<4; i++) {
            ht_add(table, address+i, (registers->r[saved_register] >> 8*i) & 0xff);
        }
        ht_add(table, address, registers->r[strtol(rd, NULL, 10)]);
    } else if (strcmp(op, "sb") == 0) {
        ht_add(table, address, registers->r[strtol(rd, NULL, 10)] & 0xff);
    } 
}

void u_type(char *op, char *rd, char *imm) {
    int imm_temp = strtol(imm, NULL, 16);
    int address = strtol(rd, NULL, 10);
    registers->r[address] = imm_temp << 12;
}
//-----------------------------------------------------------
void step(char *instruction)
{
    // Extracts and returns the substring before the first space character,
    // by replacing the space character with a null-terminator.
    // `instruction` now points to the next character after the space
    // See `man strsep` for how this library function works
    instruction = ltrim(instruction);
    char *op = strsep(&instruction, " ");
    // Uses the provided helper function to determine the type of instruction
    int op_type = get_op_type(op);
    // Skip this instruction if it is not in our supported set of instructions
    if (op_type == UNKNOWN_TYPE)
    {
        return;
    }

    char *rd = ltrim(strsep(&instruction, " ,"));

    // TODO: write logic for evaluating instruction on current interpreter state
    if (op_type == R_TYPE) {
        char *rs1 = strsep(&instruction, " ,");
        char *rs2 = strsep(&instruction, " ,");
        r_type(op, rd, rs1, rs2);
    } else if (op_type == I_TYPE) {
        char *rs1 = strsep(&instruction, " ,");
        char *imm = strsep(&instruction, " ,");
        i_type(op, rd, rs1, imm);
    } else if (op_type == MEM_TYPE) {
        char *imm = strsep(&instruction, " ,");
        char *rs1 = strsep(&instruction, " (");
        mem_type(table, op, rd, rs1, imm);
    } else {
        // u type
        char *imm = strsep(&instruction, " ");
        u_type(op, rd, imm);
    }
}
