#include <stdio.h>
#include <stdlib.h> // for atoi()
#include <string.h> // for strcmp()
#include "utils.h"

void init_symtab();
int passOne(FILE *, FILE *);

typedef struct
{
    char symbol[MAX_TOKEN_LENGTH];
    int value;
    int length;
    int address;
} littab_element;

littab_element LITTAB[MEMORY_SIZE];

int main()
{
    // input files
    FILE *input_file = fopen("input.txt", "r");

    // output files
    FILE *intermediate_file = fopen("intermediate.txt", "w");
    FILE *length = fopen("program_length.txt", "w");

    // Input file is an assembly program.
    // The program is written in a fixed format with fields
    // LABEL, OPCODE and OPERAND

    init_symtab();
    int program_length = passOne(input_file, intermediate_file);

    if (program_length == ERROR_VALUE)
        printf("Assembly failed.\n");
    else
        printf("Success!\n");

    fclose(input_file);
    fclose(intermediate_file);

    // Program length is in decimal.
    fprintf(length, "%d", program_length);
    fclose(length);
    return 0;
}

int passOne(FILE *input_file, FILE *intermediate_file)
{
    char label[MAX_TOKEN_LENGTH];
    char mnemonic[MAX_TOKEN_LENGTH];
    char operand[MAX_TOKEN_LENGTH];

    unsigned int START = 0;
    unsigned int LOCCTR = 0;
    unsigned int start_address;

    // read first line
    fscanf(input_file, "%s\t%s\t%x", label, mnemonic, &start_address);

    if (strcmp(mnemonic, "START") == 0)
    {
        START = start_address;
        LOCCTR = START;

        // First line doesn't require a location.
        fprintf(intermediate_file, "%s\t%s\t%x\n", label, mnemonic, start_address);

        // read next input line
        fscanf(input_file, "%s\t%s\t%s", label, mnemonic, operand);
    }
    else
    {
        printf("ERROR: Assembler expects START opcode in line 1.\n");
        return ERROR_VALUE;
    }

    while (strcmp(mnemonic, "END") != 0)
    {
        // if comment line, then go to next line
        if (strcmp(label, ".") == 0)
        {
            fscanf(input_file, "%s\t%s\t%s\n", label, mnemonic, operand);
            continue;
        }

        fprintf(intermediate_file, "%04x\t%s\t%s\t%s\n", LOCCTR, label, mnemonic, operand);
        // If there is a symbol in the LABEL field
        if (strcmp(label, EMPTY) != 0)
        {
            if (!symbol_search(label))
                insert_symbol_to_SYMTAB(label, LOCCTR);
            else
            {
                printf("ERROR: %s redefined at %x\n", label, LOCCTR);
                return ERROR_VALUE;
            }
        }

        // search OPTAB for opcode.
        int opcode_found = opcode_search(mnemonic);
        int instruction_format = opcode_instruction_format(mnemonic);

        if (opcode_found && instruction_format == 1)
            LOCCTR += 1;
        else if (opcode_found && instruction_format == 2)
            LOCCTR += 2;
        else if (opcode_found && instruction_format == 3)
            LOCCTR += 3;
        else if (opcode_found && instruction_format == 4)
            LOCCTR += 4;
        else if (strcmp(mnemonic, "WORD") == 0)
            LOCCTR += 3;
        else if (strcmp(mnemonic, "RESW") == 0)
            LOCCTR += (atoi(operand) * 3);
        else if (strcmp(mnemonic, "RESB") == 0)
            LOCCTR += (atoi(operand));
        else if (strcmp(mnemonic, "BYTE") == 0)
        {
            if (operand[0] == 'X')
                LOCCTR++;
            else if (operand[0] == 'C')
                LOCCTR += strlen(operand) - 3; // Subrtact 1 each for the character C, and the two quote marks.
        }
        else if (strcmp(mnemonic, "BASE") == 0)
        {
            fscanf(input_file, "%s\t%s\t%s\n", label, mnemonic, operand);
            continue;
        }
        else
        {
            printf("ERROR: Invalid OPCODE (%s) at %x.\n", mnemonic, LOCCTR);
            return ERROR_VALUE;
        }

        // Read next line
        fscanf(input_file, "%s\t%s\t%s", label, mnemonic, operand);
    }

    int program_length = LOCCTR - START;
    fprintf(intermediate_file, "%04x\t%s\t%s\t%s\n", 0000, "****", "END", "****");
    printf("Pass 1 of 2 of two completed successfully.\n");

    return program_length;
}

void init_symtab()
{
    // Erases contents of SYMTAB if it exists, else creates anew.
    FILE *symbol_table = fopen("SYMTAB.txt", "w");
    fclose(symbol_table);

    return;
}