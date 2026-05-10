#include "chibicc.h"

StringArray include_paths;
bool opt_fcommon = true;
bool opt_fpic = false;
char *base_file;

bool file_exists(char *path) {
    struct stat st;
    return !stat(path, &st);
}

static void add_default_include_paths(char *argv0) {
    strarray_push(&include_paths, format("%s/include", dirname(strdup(argv0))));
    strarray_push(&include_paths, "/usr/local/include");
    strarray_push(&include_paths, "/usr/include");
}

int main(int argc, char **argv) {
    char *input_file = NULL;
    char *output_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-o")) {
            output_file = argv[++i];
            continue;
        }
        if (!strncmp(argv[i], "-I", 2)) {
            strarray_push(&include_paths, argv[i] + 2);
            continue;
        }
        if (argv[i][0] == '-' && argv[i][1] != '\0') continue;
        input_file = argv[i];
    }

    if (!input_file) error("no input file");

    base_file = input_file;
    add_default_include_paths(argv[0]);
    init_macros();

    Token *tok = tokenize_file(input_file);
    if (!tok) error("%s: %s", input_file, strerror(errno));

    tok = preprocess(tok);
    Obj *prog = parse(tok);

    // Handle output redirection
    FILE *out = stdout;
    if (output_file) {
        out = fopen(output_file, "w");
        if (!out) error("cannot open output file: %s", output_file);
    }

    codegen(prog, out);

    if (output_file) fclose(out);
    return 0;
}