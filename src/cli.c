#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "cli.h"
#include "version.h"

#define DEFAULT_FPS 60
#define MIN_FPS 10
#define MAX_FPS 120

static int valid_name(const char *name)
{
    size_t length = strlen(name);

    if (length == 0 || length > MAX_NAME_LENGTH)
        return 0;

    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)name[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-')) {
            return 0;
        }
    }

    return 1;
}

static int parse_fps(const char *value, int *fps)
{
    if (*value == '\0')
        return -1;

    for (const char *p = value; *p; p++) {
        if (*p < '0' || *p > '9')
            return -1;
    }

    char *end;

    errno = 0;

    long result = strtol(value, &end, 10);

    if (errno == ERANGE ||
        *end != '\0' ||
        result < MIN_FPS ||
        result > MAX_FPS) {
        return -1;
    }

    *fps = (int)result;

    return 0;
}

void cli_print_help(void)
{
    puts(
        "fishby - a tiny terminal aquarium\n"
        "\n"
        "Usage: fishby [OPTIONS]\n"
        "\n"
        "Options:\n"
        "  -h, --help       Show this help message\n"
        "  -v, --version    Show program version\n"
        "  --name NAME      Set fish name\n"
        "  --fps NUMBER     Set frame rate (10-120)\n"
        "  --no-color       Disable terminal colors\n"
        "  --no-save        Disable persistent state\n"
        "\n"
        "Controls:\n"
        "  F                Feed your fish\n"
        "  Q                Quit the aquarium\n"
        "\n"
        "No thoughts. Just blub."
    );
}

void cli_print_version(void)
{
    printf("fishby %s\n", FISHBY_VERSION);
}

CliResult cli_parse(
    CliOptions *options,
    int argc,
    char **argv
)
{
    *options = (CliOptions){
        .name = "fishby",
        .fps = DEFAULT_FPS,
        .no_color = 0,
        .no_save = 0
    };

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--help") == 0 ||
            strcmp(arg, "-h") == 0) {

            cli_print_help();
            return CLI_EXIT;

        } else if (strcmp(arg, "--version") == 0 ||
                   strcmp(arg, "-v") == 0) {

            cli_print_version();
            return CLI_EXIT;

        } else if (strcmp(arg, "--no-color") == 0) {

            options->no_color = 1;

        } else if (strcmp(arg, "--no-save") == 0) {

            options->no_save = 1;

        } else if (strcmp(arg, "--name") == 0) {

            if (i + 1 >= argc) {
                fprintf(stderr,
                    "fishby: --name requires a value\n");
                return CLI_ERROR;
            }

            const char *name = argv[++i];

            if (!valid_name(name)) {
                fprintf(stderr,
                    "fishby: invalid name "
                    "(1-24 ASCII letters, digits, _ or -)\n");

                return CLI_ERROR;
            }

            memcpy(
                options->name,
                name,
                strlen(name) + 1
            );

        } else if (strcmp(arg, "--fps") == 0) {

            if (i + 1 >= argc ||
                parse_fps(argv[++i], &options->fps) < 0) {

                fprintf(stderr,
                    "fishby: --fps must be between 10 and 120\n");

                return CLI_ERROR;
            }

        } else {

            fprintf(stderr,
                "fishby: unknown argument\n"
                "Try 'fishby --help' for usage.\n");

            return CLI_ERROR;
        }
    }

    return CLI_RUN;
}
