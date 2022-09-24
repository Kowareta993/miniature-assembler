#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEM_SIZE 8192
#define BUF_SIZE 1024
#define INST_SIZE 8192
#define TABLE_SIZE 8192


#define UNKOWN_SYMBOL   -2
#define OFFSET_OVERFLOW -3
#define UNKOWN_OPCODE -4


char *error_msgs[] = {
        "",
        "",
        "unknown symbol",
        "offset out of bound",
        "unknown opcode"
};

typedef struct {
    char *instruction;
    char *fields;
    int line;
} Command;

typedef struct {
    char *label;
    int address;
} Symbol;

Symbol *symbol_table;

int is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\n' || ch == EOF;
}

int is_command(char *str) {
    char *commands[] = {"add", "sub", "slt", "or", "nand",
                        "addi", "slti", "ori", "lui", "lw",
                        "sw", "beq", "jalr", "j", "halt",
                        ".fill", ".space"};
    for (int i = 0; i < 17; i++)
        if (strcmp(commands[i], str) == 0)
            return 1;
    return 0;
}

int read_table(char *label) {
    int idx = 0;
    while (symbol_table[idx].label != NULL) {
        if (strcmp(symbol_table[idx].label, label) == 0)
            return symbol_table[idx].address;
        idx++;
    }
    return -1;
}

void free_resources(Command **commands) {
    if (commands) {
        int idx = 0;
        Command *inst = commands[idx++];
        while (inst != NULL) {
            free(inst->instruction);
            free(inst->fields);
            free(inst);
            inst = commands[idx++];
        }
        free(commands);
    }
    if (symbol_table) {
        int idx = 0;
        while (symbol_table[idx].label != NULL) {
            free(symbol_table[idx].label);
            idx++;
        }
        free(symbol_table);
        symbol_table = NULL;
    }
}

Command **read_commands(FILE *input) {
    char ch;
    char buffer[BUF_SIZE];
    int cmd_idx = 0;
    int token_idx = 0;
    int table_idx = 0;
    int offset = 0;
    int i = 0;
    int comment = 0;
    int line = 1;
    int added = 0;
    int labeled = 0;
    Command **commands = (Command **) malloc(INST_SIZE * sizeof(Command *));
    commands[0] = NULL;
    while ((ch = fgetc(input))) {
        if (comment) {
            if (ch == '\n') {
                line++;
                comment = 0;
                if (added) {
                    token_idx = 0;
                    cmd_idx++;
                    commands[cmd_idx] = NULL;
                    added = 0;
                }
            }
            continue;

        }
        if (is_space(ch)) {
            if (i > 0) {
                buffer[i] = '\0';
                i = 0;
                if (commands[cmd_idx] == NULL) {
                    commands[cmd_idx] = (Command *) malloc(sizeof(Command));
                    commands[cmd_idx]->line = line;
                    added = 1;
                    labeled = 0;
                }
                if (token_idx == 0) {
                    if (is_command(buffer)) {
                        commands[cmd_idx]->instruction = (char *) malloc((strlen(buffer) + 1) * sizeof(char));
                        strcpy(commands[cmd_idx]->instruction, buffer);
                        token_idx++;
                    } else {
                        if (labeled) {
                            printf("syntax error in line %d\n", line);
                            free_resources(commands);
                            return NULL;
                        }
                        if (read_table(buffer) != -1) {
                            printf("symbol %s defined before\n", buffer);
                            free_resources(commands);
                            return NULL;
                        }
                        symbol_table[table_idx].address = cmd_idx + offset;
                        symbol_table[table_idx].label = malloc((strlen(buffer) + 1) * sizeof(char));
                        strcpy(symbol_table[table_idx++].label, buffer);
                        symbol_table[table_idx].label = NULL;
                        labeled = 1;
                    }
                } else if (token_idx == 1) {
                    commands[cmd_idx]->fields = (char *) malloc((strlen(buffer) + 1) * sizeof(char));
                    strcpy(commands[cmd_idx]->fields, buffer);
                    if (strcmp(commands[cmd_idx]->instruction, ".space") == 0) {
                        offset += atoi(commands[cmd_idx]->fields) - 1;
                    }
                    token_idx++;
                } else {
                    printf("syntax error in line %d\n", line);
                    free_resources(commands);
                    return NULL;
                }
                if (ch == '\n') {
                    token_idx = 0;
                    cmd_idx++;
                    commands[cmd_idx] = NULL;
                    added = 0;
                }
            }
            if (ch == '\n') {
                line++;
                if (added) {
                    token_idx = 0;
                    cmd_idx++;
                    commands[cmd_idx] = NULL;
                    added = 0;
                }
            }
            if (ch == EOF)
                break;
            continue;
        }
        if (ch == '#') {
            comment = 1;
            continue;
        }
        buffer[i++] = ch;
    }
    return commands;
}


int is_rtype(char *instruction) {
    char *instructions[] = {"add", "sub", "slt", "or", "nand"};
    for (int i = 0; i < 5; i++)
        if (strcmp(instructions[i], instruction) == 0)
            return 1;
    return 0;
}

int is_itype(char *instruction) {
    char *instructions[] = {"addi", "ori", "slti", "lui", "lw", "sw", "beq", "jalr"};
    for (int i = 0; i < 8; i++)
        if (strcmp(instructions[i], instruction) == 0)
            return 1;
    return 0;
}

int is_jtype(char *instruction) {
    char *instructions[] = {"j", "halt"};
    for (int i = 0; i < 2; i++)
        if (strcmp(instructions[i], instruction) == 0)
            return 1;
    return 0;
}


int opcode(char *instruction) {
    char *instructions[] = {"add", "sub", "slt", "or", "nand",
                            "addi", "slti", "ori", "lui", "lw",
                            "sw", "beq", "jalr", "j", "halt"};
    for (int i = 0; i < 15; i++)
        if (strcmp(instructions[i], instruction) == 0)
            return i;
    return -1;
}


int is_numeric(const char *str) {
    int idx = 0;
    if (str[idx] == '-')
        idx++;
    while (str[idx] != '\0') {
        char ch = str[idx];
        if (ch < '0' || ch > '9')
            return 0;
        idx++;
    }
    return 1;
}

int translate_instruction(Command *command) {
    if (is_rtype(command->instruction)) {
        int rd = atoi(strtok(command->fields, ","));
        int rs = atoi(strtok(NULL, ","));
        int rt = atoi(strtok(NULL, ","));
        return opcode(command->instruction) * 1024 * 1024 * 16 + rs * 1024 * 1024 + rt * 1024 * 64 + rd * 1024 * 4;
    }
    if (is_itype(command->instruction)) {
        int rt = atoi(strtok(command->fields, ","));
        int rs = 0;
        if (strcmp("lui", command->instruction) != 0) {
            rs = atoi(strtok(NULL, ","));
        }
        int offset = 0;
        if (strcmp("jalr", command->instruction) != 0) {
            char *label = strtok(NULL, ",");
            offset = read_table(label);
            if (offset == -1) {
                if (is_numeric(label))
                    offset = atoi(label);
                else
                    return UNKOWN_SYMBOL;
            }
        }
        if (offset >= 1024 * 32 || offset < -1024 * 32)
            return OFFSET_OVERFLOW;
        return opcode(command->instruction) * 1024 * 1024 * 16 + rs * 1024 * 1024 + rt * 1024 * 64 + offset;
    }
    if (is_jtype(command->instruction)) {
        int offset = 0;
        if (strcmp("halt", command->instruction) != 0) {
            char *label = strtok(command->fields, ",");
            offset = read_table(label);
            if (offset == -1) {
                if (is_numeric(label))
                    offset = atoi(label);
                else
                    return UNKOWN_SYMBOL;
            }
        }
        if (offset >= 1024 * 32 || offset < -1024 * 32)
            return OFFSET_OVERFLOW;
        return opcode(command->instruction) * 1024 * 1024 * 16 + offset;
    }
    return UNKOWN_OPCODE;
}

int translate(Command **commands, FILE *output) {
    if (commands == NULL) {
        printf("no commands\n");
        return 0;
    }
    int idx = 0;
    while (commands[idx] != NULL) {
        if (commands[idx]->instruction == NULL) {
            printf("syntax error in line %d\n", commands[idx]->line);
            return 0;
        }
        int code = translate_instruction(commands[idx]);
        if (code < 0) {
            if (strcmp(".fill", commands[idx]->instruction) == 0) {
                char *label = strtok(commands[idx]->fields, ",");
                int offset = read_table(label);
                if (offset == -1) {
                    if (is_numeric(label))
                        offset = atoi(label);
                    else {
                        printf("syntax error in line %d (%s)\n", commands[idx]->line, error_msgs[- UNKOWN_SYMBOL]);
                        return 0;
                    }
                }
                fprintf(output, "%d\n", offset);
            } else if (strcmp(".space", commands[idx]->instruction) == 0) {
                int size = atoi(commands[idx]->fields);
                for (; size > 0; size--)
                    fprintf(output, "%d\n", 0);
            } else {
                printf("syntax error in line %d (%s)\n", commands[idx]->line, error_msgs[-code]);
                return 0;
            }
        } else {
            fprintf(output, "%d\n", code);
        }
        idx++;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("incorrect use of program. example usage: assembler test.as test.mc\n");
        return 1;
    }
    FILE *code = fopen(argv[1], "r");
    if (code == NULL) {
        printf("cannot open input file\n");
        return 1;
    }
    FILE *out = fopen(argv[2], "w");
    if (out == NULL) {
        printf("cannot open output file\n");
        return 1;
    }
    symbol_table = malloc(TABLE_SIZE * sizeof(Symbol));
    symbol_table[0].label = NULL;
    Command **commands = read_commands(code);
    if (commands == NULL) {
        free_resources(commands);
        return 1;
    }

    fclose(code);
    if (!translate(commands, out)) {
        fclose(out);
        free_resources(commands);
        return 1;
    }
    fclose(out);
    free_resources(commands);
    return 0;
}