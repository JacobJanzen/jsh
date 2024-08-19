#include "config.h"
#include "grammar.h"
#include "src/grammar.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>

static void print_version()
{
    printf("%s version %d.%d\n", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR);
    printf("Copyright (C) 2024 Jacob Janzen\n");
    printf("License GPLv3+: GNU GPL version 3 or later "
           "<http://gnu.org/licenses.gpl.html>\n");
    printf("\n");
    printf(
        "This is free software; you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by the law\n");
}

static void print_help()
{
    printf("%s version %d.%d\n", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR);
    printf("Usage: %s [options] ...\n", PROJECT_NAME);
    printf("       %s [options] script-file ...\n", PROJECT_NAME);
    printf("\n");
    printf("Options:\n");
    printf("    --help\n");
    printf("      print this message and exit\n");
    printf("    --version\n");
    printf("      print the version and exit\n");
}

int main(int argc, char **argv)
{
    int c;
    char *run_command = NULL;

    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
        {"version", no_argument, 0, 0},
        {"help", no_argument, 0, 0},
        {0, 0, 0, 0},
    };

    while ((c = getopt_long(argc, argv, "c:?", long_options, &option_index)) !=
           -1) {
        switch (c) {
        case 0:
            if (strcmp(long_options[option_index].name, "version") == 0) {
                print_version();
                return 0;
            }
            if (strcmp(long_options[option_index].name, "help") == 0) {
                print_help();
                return 0;
            }
            break;
        case 'c':
            run_command = optarg;
            break;
        case '?':
            return 1;
        }
    }

    FILE *file;
    if (run_command) {
        file = fmemopen(run_command, strlen(run_command), "r");
    } else if (optind < argc) {
        file = fopen(argv[optind], "r");
        if (!file) {
            fprintf(stderr, "%s: ", argv[optind]);
            perror("");
            return 1;
        }
    } else {
        file = stdin;
    }

    struct lexer lex = {
        .stream = file,
        .prev = 0,
        .prevprev = 0,
    };
    grammar_parse(&lex);
    fclose(file);
    return 0;
}
