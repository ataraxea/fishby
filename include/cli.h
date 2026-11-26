#ifndef FISHBY_CLI_H
#define FISHBY_CLI_H

#define MAX_NAME_LENGTH 24

typedef enum {
    CLI_ERROR = -1,
    CLI_RUN = 0,
    CLI_EXIT = 1
} CliResult;

typedef struct {
    char name[MAX_NAME_LENGTH + 1];

    int fps;
    int no_color;
    int no_save;
} CliOptions;

CliResult cli_parse(
    CliOptions *options,
    int argc,
    char **argv
);

void cli_print_help(void);
void cli_print_version(void);

#endif
