#include "chibicc.h"

StringArray include_paths;
bool opt_fcommon = true;
bool opt_fpic = false;
char *base_file;

// Required by preprocess.c
bool file_exists(char *path) {
  struct stat st;
  return !stat(path, &st);
}

static void add_default_include_paths(char *argv0) {
  strarray_push(&include_paths, format("%s/include", dirname(strdup(argv0))));
  strarray_push(&include_paths, "/usr/local/include");
  strarray_push(&include_paths, "/usr/include/x86_64-linux-gnu");
  strarray_push(&include_paths, "/usr/include");
}

static void define(char *str) {
  char *eq = strchr(str, '=');
  if (eq)
    define_macro(strndup(str, eq - str), eq + 1);
  else
    define_macro(str, "1");
}

int main(int argc, char **argv) {
  init_macros();
  add_default_include_paths(argv[0]);

  char *input_file = NULL;

  // Extremely basic argument parsing
  for (int i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "-I", 2)) {
      strarray_push(&include_paths, argv[i] + 2);
      continue;
    }
    if (!strcmp(argv[i], "-I")) {
      strarray_push(&include_paths, argv[++i]);
      continue;
    }
    if (!strncmp(argv[i], "-D", 2)) {
      define(argv[i] + 2);
      continue;
    }
    if (!strcmp(argv[i], "-D")) {
      define(argv[++i]);
      continue;
    }
    
    // Ignore all other flags (like -o, -S, -c, etc.)
    if (argv[i][0] == '-')
      continue;

    // The first non-flag argument is our input file
    input_file = argv[i];
  }

  if (!input_file)
    error("no input file");

  base_file = input_file;

  // 1. Read and Tokenize
  Token *tok = tokenize_file(input_file);
  if (!tok)
    error("%s: %s", input_file, strerror(errno));

  // 2. Preprocess (#includes, #defines)
  tok = preprocess(tok);
  
  // 3. Parse into AST
  Obj *prog = parse(tok);

  // 4. Generate Assembly DIRECTLY TO STANDARD OUTPUT!
  codegen(prog, stdout);

  return 0;
}