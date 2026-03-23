// codegen.c
#include "chibicc.h"

static FILE *output_file;
static int depth;
static Obj *current_fn;

// CPU16 registers
static char *reg[] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
                      "R8", "R9", "R10", "R11", "R12", "R13", "BP", "SP"};
static char *argreg[] = {"R0", "R1", "R2", "R3"};
static char *callee_saved[] = {"R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11"};

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

__attribute__((format(printf, 1, 2)))
static void println(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(output_file, fmt, ap);
  va_end(ap);
  fprintf(output_file, "\n");
}

static int count(void) {
  static int i = 1;
  return i++;
}

static void push(char* r) {
  println("    PUSH %s", r);
  depth++;
}

static void pop(char *r) {
  println("    POP %s", r);
  depth--;
}

int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

// Compute the absolute address of a given node.
// It's an error if a given node does not reside in memory.
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    if (node->var->is_local) {
      // Local variable
      println("    LEA R0, [BP - %d]", node->var->offset);
    } else {
      // Global variable
      println("    MOV R0, %s", node->var->name);
    }
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  case ND_MEMBER:
    gen_addr(node->lhs);
    println("    ADD R0, %d", node->member->offset);
    return;
  }

  error_tok(node->tok, "not an lvalue");
}

// Load a value from where R0 is pointing to.
static void load(Type *ty) {
  if (ty->kind == TY_ARRAY || ty->kind == TY_STRUCT || ty->kind == TY_UNION) {
    return;
  }
  // All loads are 16-bit in this architecture
  println("    MOV R0, [R0]");
}

// Store R0 to an address that the stack top is pointing to.
static void store(Type *ty) {
  pop("R1"); // Pop address into R1

  if (ty->kind == TY_STRUCT || ty->kind == TY_UNION) {
      for (int i = 0; i < ty->size; i+=2) {
          println("    MOV R2, [R0 + %d]", i);
          println("    MOV [R1 + %d], R2", i);
      }
      return;
  }
  
  // All stores are 16-bit
  println("    MOV [R1], R0");
}


// Generate code for a given node.
static void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NULL_EXPR:
    return;
  case ND_NUM:
    println("    MOV R0, %ld", node->val);
    return;
  case ND_NEG:
    gen_expr(node->lhs);
    println("    NEG R0");
    return;
  case ND_VAR:
    gen_addr(node);
    load(node->ty);
    return;
  case ND_MEMBER:
    gen_addr(node);
    load(node->ty);
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    load(node->ty);
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    push("R0");
    gen_expr(node->rhs);
    store(node->ty);
    return;
  case ND_MEMZERO: {
    println("    LEA R0, [BP - %d]", node->var->offset);
    println("    MOV R1, 0");
    int words = node->var->ty->size / 2;
    if (words == 0) words = 1;
    for (int i = 0; i < words; i++) {
        println("    MOV R2, R0");
        if (i > 0) println("    ADD R2, %d", i);
        println("    MOV [R2], R1");
    }
    return;
  }
  case ND_STMT_EXPR:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_COMMA:
    gen_expr(node->lhs);
    gen_expr(node->rhs);
    return;
  case ND_CAST:
    gen_expr(node->lhs);
    if (node->ty->size == 1) {
        if (node->ty->is_unsigned) {
            println("    AND R0, 0x00FF");
        } else {
            println("    CBW R0"); 
        }
    }
    return;
  case ND_COND: {
    int c = count();
    gen_expr(node->cond);
    println("    CMP R0, 0");
    println("    JMPE .L.else.%d", c);
    gen_expr(node->then);
    println("    JMP .L.end.%d", c);
    println(".L.else.%d:", c);
    gen_expr(node->els);
    println(".L.end.%d:", c);
    return;
  }
  case ND_NOT:
    gen_expr(node->lhs);
    println("    CMP R0, 0");
    println("    JMPE .L.not.true.%d", count());
    println("    MOV R0, 0");
    println("    JMP .L.not.end.%d", count());
    println(".L.not.true.%d:", count()-1);
    println("    MOV R0, 1");
    println(".L.not.end.%d:", count()-1);
    return;
  case ND_BITNOT:
    gen_expr(node->lhs);
    println("    NOT R0");
    return;
  case ND_LOGAND: {
    int c = count();
    gen_expr(node->lhs);
    println("    CMP R0, 0");
    println("    JMPE .L.false.%d", c);
    gen_expr(node->rhs);
    println("    CMP R0, 0");
    println("    JMPE .L.false.%d", c);
    println("    MOV R0, 1");
    println("    JMP .L.end.%d", c);
    println(".L.false.%d:", c);
    println("    MOV R0, 0");
    println(".L.end.%d:", c);
    return;
  }
  case ND_LOGOR: {
    int c = count();
    gen_expr(node->lhs);
    println("    CMP R0, 0");
    println("    JMPNE .L.true.%d", c);
    gen_expr(node->rhs);
    println("    CMP R0, 0");
    println("    JMPNE .L.true.%d", c);
    println("    MOV R0, 0");
    println("    JMP .L.end.%d", c);
    println(".L.true.%d:", c);
    println("    MOV R0, 1");
    println(".L.end.%d:", c);
    return;
  }
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen_expr(arg);
      push("R0");
      nargs++;
    }

    // Pop arguments into registers
    for (int i = nargs - 1; i >= 0; i--) {
      if (i < 4) // First 4 args go into R0-R3
        pop(argreg[i]);
      else // Rest were passed on stack
        ; // leave on stack
    }

    println("    CALL %s", node->lhs->var->name);

    // Caller cleans up the stack
    if (nargs > 4)
      println("    ADD SP, %d", (nargs - 4) * 2);

    return;
  }
  }

  // Binary operators
  gen_expr(node->rhs);
  push("R0");
  gen_expr(node->lhs);
  pop("R1");

  switch (node->kind) {
  case ND_ADD:
    println("    ADD R0, R1");
    return;
  case ND_SUB:
    println("    SUB R0, R1");
    return;
  case ND_MUL:
    println("    MUL R0, R1");
    return;
  case ND_DIV:
    println("    DIV R0, R1");
    return;
  case ND_MOD:
    println("    MOD R0, R1");
    return;
  case ND_BITAND:
    println("    AND R0, R1");
    return;
  case ND_BITOR:
    println("    OR R0, R1");
    return;
  case ND_BITXOR:
    println("    XOR R0, R1");
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    println("    CMP R0, R1");
    
    char *inst;
    if (node->kind == ND_EQ) inst = "JMPE";
    else if (node->kind == ND_NE) inst = "JMPNE";
    else if (node->kind == ND_LT) inst = "JMPL";
    else if (node->kind == ND_LE) inst = "JMPLE";
    
    int c = count();
    println("    %s .L.true.%d", inst, c);
    println("    MOV R0, 0");
    println("    JMP .L.end.%d", c);
    println(".L.true.%d:", c);
    println("    MOV R0, 1");
    println(".L.end.%d:", c);
    return;
  case ND_SHL:
    println("    SHL R0, R1");
    return;
  case ND_SHR:
    println("    SHR R0, R1");
    return;
  }

  error_tok(node->tok, "invalid expression");
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_IF: {
    int c = count();
    gen_expr(node->cond);
    println("    CMP R0, 0");
    println("    JMPE .L.else.%d", c);
    gen_stmt(node->then);
    println("    JMP .L.end.%d", c);
    println(".L.else.%d:", c);
    if (node->els)
      gen_stmt(node->els);
    println(".L.end.%d:", c);
    return;
  }
  case ND_FOR: {
    int c = count();
    if (node->init)
      gen_stmt(node->init);
    println(".L.begin.%d:", c);
    if (node->cond) {
      gen_expr(node->cond);
      println("    CMP R0, 0");
      println("    JMPE %s", node->brk_label);
    }
    gen_stmt(node->then);
    println("%s:", node->cont_label);
    if (node->inc)
      gen_expr(node->inc);
    println("    JMP .L.begin.%d", c);
    println("%s:", node->brk_label);
    return;
  }
  case ND_DO: {
    int c = count();
    println(".L.begin.%d:", c);
    gen_stmt(node->then);
    println("%s:", node->cont_label);
    gen_expr(node->cond);
    println("    CMP R0, 0");
    println("    JMPNE .L.begin.%d", c);
    println("%s:", node->brk_label);
    return;
  }
  case ND_SWITCH:
    gen_expr(node->cond);
    for (Node *n = node->case_next; n; n = n->case_next) {
      println("    CMP R0, %ld", n->begin);
      println("    JMPE %s", n->label);
    }

    if (node->default_case)
      println("    JMP %s", node->default_case->label);

    println("    JMP %s", node->brk_label);
    gen_stmt(node->then);
    println("%s:", node->brk_label);
    return;
  case ND_CASE:
    println("%s:", node->label);
    gen_stmt(node->lhs);
    return;
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_GOTO:
    println("    JMP %s", node->unique_label);
    return;
  case ND_LABEL:
    println("%s:", node->unique_label);
    gen_stmt(node->lhs);
    return;
  case ND_RETURN:
    if (node->lhs)
      gen_expr(node->lhs);
    println("    JMP .L.return.%s", current_fn->name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
  case ND_ASM:
     println("    %s", node->asm_str);
     return;
  }

  error_tok(node->tok, "invalid statement");
}

static void assign_lvar_offsets(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_function)
      continue;

    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += var->ty->size;
      offset = align_to(offset, var->align);
      var->offset = offset;
    }
    fn->stack_size = align_to(offset, 2); // Align stack to 2 bytes
  }
}

static void emit_data(Obj *prog) {
    println("    ; Data Section");
    for (Obj *var = prog; var; var = var->next) {
        if (var->is_function)
            continue;
        
        println("%s:", var->name);

        if (var->init_data) {
            for (int i = 0; i < var->ty->size; i++) {
                println("    DB %d", var->init_data[i]);
            }
        } else {
            println("    DW 0 ; zero-initialized");
        }
    }
}

static void emit_text(Obj *prog) {
  println("    ; Text Section");
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_function || !fn->is_definition)
      continue;

    println("\n; Function: %s", fn->name);
    println("%s:", fn->name);
    current_fn = fn;

    println("    ENTER %d", fn->stack_size);

    // Save callee-saved registers
    for(int i = 0; i < sizeof(callee_saved) / sizeof(*callee_saved); i++)
        println("    PUSH %s", callee_saved[i]);

    // Copy arguments from registers to stack
    int i = 0;
    for (Obj *var = fn->params; var && i < 4; var = var->next, i++) {
        println("    MOV [BP - %d], %s", var->offset, argreg[i]);
    }

    // Emit code
    gen_stmt(fn->body);
    assert(depth == 0);

    // Epilogue
    println(".L.return.%s:", fn->name);
    
    // Restore callee-saved registers
    for(int i = sizeof(callee_saved) / sizeof(*callee_saved) - 1; i >= 0; i--)
        println("    POP %s", callee_saved[i]);

    println("    LEAVE");
    println("    RET");
  }
}

void codegen(Obj *prog, FILE *out) {
  output_file = out;

  println("    JMP main");

  assign_lvar_offsets(prog);
  emit_data(prog);
  emit_text(prog);
}