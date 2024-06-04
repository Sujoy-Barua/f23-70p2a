/**
 * Project 2
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static inline int isNumber(char *);

typedef struct {
    char label[7];
    int lineNum;
} Labels;

typedef struct {
    int text;
    int data;
    int symbol;
    int relo;
} Header;

typedef struct {
    char type[2];
    int opcode;
    int regA;
    int regB;
    int regDest;
    int offset;
} Text;

typedef struct {
    int fill;
} Data;

typedef struct {
    char label[7];
    char type[2];
    int offset; //from the start of the T/D section
} Symbol;

typedef struct {
    char opcode[6];
    char label[7];
    int offset; //from the start of the T/D section
} Reloc;




int 
main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    Labels labels[1000];
    Text text[1000]; //creaing array to store the instructions only
    //Data data[1000]; //creating array to store the data table (NOT NEEDED BECAUSE ORDER IS PRESERVED ALREADY WITH OLD CODE)
    Symbol symbol[1000]; //creating array to store the symbol table
    Reloc reloc[1000]; //creaing array ti store the relocation table

    int lineCounter = 0;
    int textCounter = 0;
    int dataCounter = 0;
    int symbolCounter = 0;
    int relocCounter = 0;

    int dataLineCounter = 0;
    int textLineCounter = 0;

    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        for (int i = 0; i < 1000; i++) {
            if (!strcmp(labels[i].label, label) && strlen(label) != 0) {
                exit(1);
            }
        }
        if (isNumber(label)) {
            exit(1);
        }
        if (strcmp(label, "")) {
            strcpy(labels[lineCounter].label, label);
            labels[lineCounter].lineNum = lineCounter;
            if (!strcmp(opcode, ".fill")) {
                
                if (isupper(label[0])) {
                    strcpy(symbol[symbolCounter].label, label);
                    strcpy(symbol[symbolCounter].type, "D");
                    symbol[symbolCounter].offset = dataLineCounter;
                    symbolCounter++;
                }
                dataLineCounter++;

            } else {
                
                if (isupper(label[0])) {
                    strcpy(symbol[symbolCounter].label, label);
                    strcpy(symbol[symbolCounter].type, "T");
                    symbol[symbolCounter].offset = textLineCounter;
                    symbolCounter++;
                }
                textLineCounter++;

            }
        } else if (!strcmp(opcode, ".fill")) {
            dataLineCounter++;
        } else {
            textLineCounter++;
        }
        lineCounter++;
    }

    /*for (int i = 0; i < 10; i++) {
        printf("%s\n", labels[i].label);
        printf("%d\n", labels[i].lineNum);
    }*/

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    
    rewind(inFilePtr);
    lineCounter = 0;
    relocCounter = 0;
    dataLineCounter = 0;
    //note: check that exit(1) exists for all instructions at the same place by doing ctrl + f = exit(1) and taking note
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        if (!strcmp(opcode, "lw")) {
            if (!(isNumber(arg0) && isNumber(arg1))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "I");
            text[lineCounter].opcode = 0b010;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)) {
                exit(1);
            }
            if ( isNumber(arg2) ) {
                text[lineCounter].offset = atoi(arg2);
                if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                    exit(1);
                }
            } else {

                reloc[relocCounter].offset = textCounter;
                strcpy(reloc[relocCounter].opcode, "lw");
                strcpy(reloc[relocCounter].label, arg2);
                relocCounter++;

                bool labelExists = false;
                for (int i = 0; i < 1000; i++) {
                    if (!strcmp(labels[i].label, arg2)) {
                        text[lineCounter].offset = labels[i].lineNum;
                        if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                            exit(1);
                        }
                        labelExists = true;
                        break; // Exit the loop once the label is found
                    }
                }
                if (labelExists == false && isupper(arg2[0])) {
                    bool symbolExists = false;
                    for (int i = 0; i < symbolCounter; i++) {
                        if (!strcmp(symbol[i].label, arg2)) {
                            symbolExists = true;
                            break;
                        }
                    }
                    if (symbolExists == false) {
                        text[lineCounter].offset = 0;
                        strcpy(symbol[symbolCounter].label, arg2);
                        strcpy(symbol[symbolCounter].type, "U");
                        symbol[symbolCounter].offset = 0;
                        symbolCounter++;
                    }
                } else if (labelExists == false) {
                    exit(1);
                }
            }
            textCounter++;
        } 
        else if (!strcmp(opcode, "add")) {
            if (!(isNumber(arg0) && isNumber(arg1) && isNumber(arg2))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "R");
            text[lineCounter].opcode = 0b000;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            text[lineCounter].regDest = atoi(arg2);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)
            || (text[lineCounter].regDest < 0 || text[lineCounter].regDest > 7)) {
                exit(1);
            }
            textCounter++;
        } 
        else if (!strcmp(opcode, "nor")) {
            if (!(isNumber(arg0) && isNumber(arg1) && isNumber(arg2))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "R");
            text[lineCounter].opcode = 0b001;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            text[lineCounter].regDest = atoi(arg2);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)
            || (text[lineCounter].regDest < 0 || text[lineCounter].regDest > 7)) {
                exit(1);
            }
            textCounter++;
        }
        else if (!strcmp(opcode, "sw")) {
            if (!(isNumber(arg0) && isNumber(arg1))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "I");
            text[lineCounter].opcode = 0b011;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)) {
                exit(1);
            }
            if ( isNumber(arg2) ) {
                text[lineCounter].offset = atoi(arg2);
                if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                    exit(1);
                }
            } else {

                reloc[relocCounter].offset = textCounter;
                strcpy(reloc[relocCounter].opcode, "sw");
                strcpy(reloc[relocCounter].label, arg2);
                relocCounter++;

                bool labelExists = false;
                for (int i = 0; i < 1000; i++) {
                    if (!strcmp(labels[i].label, arg2)) {
                        text[lineCounter].offset = labels[i].lineNum;
                        if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                            exit(1);
                        }
                        labelExists = true;
                        break; // Exit the loop once the label is found
                    }
                }
                if (labelExists == false && isupper(arg2[0])) {
                    bool symbolExists = false;
                    for (int i = 0; i < symbolCounter; i++) {
                        if (!strcmp(symbol[i].label, arg2)) {
                            symbolExists = true;
                            break;
                        }
                    }
                    if (symbolExists == false) {
                        text[lineCounter].offset = 0;
                        strcpy(symbol[symbolCounter].label, arg2);
                        strcpy(symbol[symbolCounter].type, "U");
                        symbol[symbolCounter].offset = 0;
                        symbolCounter++;
                    }
                } else if (labelExists == false) {
                    exit(1);
                }
            }
            textCounter++;
        }
        else if (!strcmp(opcode, "beq")) {
            if (!(isNumber(arg0) && isNumber(arg1))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "I");
            text[lineCounter].opcode = 0b100;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)) {
                exit(1);
            }
            if ( isNumber(arg2) ) {
                text[lineCounter].offset = atoi(arg2);
                if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                    exit(1);
                }
            } else {
                bool labelExists = false;
                for (int i = 0; i < 1000; i++) {
                    if (!strcmp(labels[i].label, arg2)) {
                        text[lineCounter].offset = labels[i].lineNum - lineCounter - 1;
                        if (text[lineCounter].offset < -32768 || text[lineCounter].offset > 32767) {
                            exit(1);
                        }
                        labelExists = true;
                        break; // Exit the loop once the label is found
                    }
                }
                if (labelExists == false) {
                    exit(1);
                }
            }
            textCounter++;
        }
        else if (!strcmp(opcode, "jalr")) {
            if (!(isNumber(arg0) && isNumber(arg1))) {
                exit(1);
            }
            strcpy(text[lineCounter].type, "J");
            text[lineCounter].opcode = 0b101;
            text[lineCounter].regA = atoi(arg0);
            text[lineCounter].regB = atoi(arg1);
            if ((text[lineCounter].regA < 0 || text[lineCounter].regA > 7) 
            || (text[lineCounter].regB < 0 || text[lineCounter].regB > 7)) {
                exit(1);
            }
            textCounter++;
        }
        else if (!strcmp(opcode, "halt")) {
            strcpy(text[lineCounter].type, "O");
            text[lineCounter].opcode = 0b110;
            textCounter++;
        }
        else if (!strcmp(opcode, "noop")) {
            strcpy(text[lineCounter].type, "O");
            text[lineCounter].opcode = 0b111;
            textCounter++;
        }
        else if (!strcmp(opcode, ".fill")) {

            strcpy(text[lineCounter].type, "D");
            if ( isNumber(arg0) ) {
                text[lineCounter].regA = atoi(arg0);
                if (text[lineCounter].regA < -2147483648 || text[lineCounter].regA > 2147483647) {
                    exit(1);
                }
            } else {
                reloc[relocCounter].offset = dataLineCounter;
                strcpy(reloc[relocCounter].opcode, ".fill");
                strcpy(reloc[relocCounter].label, arg0);
                relocCounter++;

                bool labelExists = false;
                for (int i = 0; i < 1000; i++) {
                    if (!strcmp(labels[i].label, arg0)) {
                        text[lineCounter].regA = labels[i].lineNum;
                        labelExists = true;
                        break; // Exit the loop once the label is found
                    }
                }
                if (labelExists == false && isupper(arg0[0])) {
                    bool symbolExists = false;
                    for (int i = 0; i < symbolCounter; i++) {
                        if (!strcmp(symbol[i].label, arg0)) {
                            symbolExists = true;
                            break;
                        }
                    }
                    if (symbolExists == false) {
                        text[lineCounter].regA = 0;
                        strcpy(symbol[symbolCounter].label, arg0);
                        strcpy(symbol[symbolCounter].type, "U");
                        symbol[symbolCounter].offset = 0;
                        symbolCounter++;
                    }
                } else if (labelExists == false) {
                    exit(1);
                }
            }
            dataCounter++;
            dataLineCounter++;
        } else {
            exit(1);
        }
        lineCounter++;
    }

    fprintf(outFilePtr, "%d %d %d %d\n", textCounter, dataCounter, symbolCounter, relocCounter);

    for (int i = 0; i < lineCounter; i++) {
        int result = 0;

        if (!strcmp(text[i].type, "R")) {
            result = (text[i].opcode << 22) + (text[i].regA << 19)
                + (text[i].regB << 16) + (text[i].regDest);
        } else if (!strcmp(text[i].type, "J")) {
            result = (text[i].opcode << 22) + (text[i].regA << 19)
                + (text[i].regB << 16);
        } else if (!strcmp(text[i].type, "I")) {
            if (text[i].offset < 0) {
                text[i].offset &= 0xFFFF; // Ensure 16-bit offset
            }
            result = (text[i].opcode << 22) + (text[i].regA << 19)
                + (text[i].regB << 16) + (text[i].offset);
        } else if (!strcmp(text[i].type, "O")) {
            result = (text[i].opcode << 22);
        } else {
            result = (text[i].regA);
        }

        fprintf(outFilePtr, "%d\n", result);
    }

    for (int i = 0; i < symbolCounter; i++) {
        fprintf(outFilePtr, "%s %s %d\n", symbol[i].label, symbol[i].type, symbol[i].offset);
    }

    for (int i = 0; i < relocCounter; i++) {
        fprintf(outFilePtr, "%d %s %s\n",  reloc[i].offset, reloc[i].opcode, reloc[i].label);
    }
    exit(0);
}

/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Treat a blank line as end of file.
    // Arguably, we could just ignore and continue, but that could
    // get messy in terms of label line numbers etc.
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(size_t line_idx = 0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                ++line_char_is_whitespace;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            ++nonempty_line;
            break;
        }
    }
    if(nonempty_line == 0) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}
