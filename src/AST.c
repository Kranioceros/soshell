#include "AST.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *new_ccall(const char op, Commands *comms) {
  printf("new_ccall called\n");
  assert(comms != NULL);

  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = TyCompoundCall;
  node->value.ccall.op = op;
  node->value.ccall.comms = comms;
  node->value.ccall.stdin_to = NULL;
  node->value.ccall.stdout_to = NULL;

  return node;
}

ASTNode *new_scall(const char *program_name, Params *params) {
  printf("new_scall called\n");
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = TySimpleCall;
  node->value.scall.program_name = program_name;

  if (params) {
    node->value.scall.params = params;
  } else {
    node->value.scall.params = NULL;
  }

  node->value.scall.stdout_to = NULL;
  node->value.scall.stdin_to = NULL;

  return node;
}

void stdout_to_file(ASTNode *node, const char *s) {
  printf("stdout_to_file called\n");
  assert(node != NULL);
  assert(node->type == TySimpleCall);

  node->value.scall.stdout_to = s;
}

void stdin_to_file(ASTNode *node, const char *s) {
  printf("stdin_to_file called\n");
  assert(node != NULL);
  assert(node->type == TySimpleCall);

  node->value.scall.stdin_to = s;
}

Params *new_param(const char *s) {
  printf("new_param called\n");
  assert(s != NULL);
  Params *p = malloc(sizeof(Params));
  p->count = 1;
  p->s[0] = s;

  return p;
}

void add_param(Params *params, const char *s) {
  printf("add_param called\n");
  assert(params != NULL);
  assert(params->count < PARAMS_MAX);

  params->s[params->count++] = s;
}

Commands *new_comm(ASTNode *node) {
  printf("new_comm called\n");
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);

  Commands *comms = malloc(sizeof(Commands));
  comms->count = 1;
  comms->c[0] = node;

  return comms;
}

void add_comm(Commands *comms, ASTNode *node) {
  printf("add_comm called\n");
  assert(comms != NULL);
  assert(comms->count < PARAMS_MAX);
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);

  comms->c[comms->count++] = node;
}
/**** BORRADO ****/
// Funcion auxiliar para borrar string si no es NULL
void freeif(const char *p) {
  if (p) {
    free((void *)p);
  }
}
void free_node(ASTNode *node) {
  assert(node != NULL);
  switch (node->type) {
  case TySimpleCall: {
    SimpleCall *call = &node->value.scall;
    freeif(call->program_name);
    freeif(call->stdin_to);
    freeif(call->stdout_to);
    if (call->params) {
      free_param(call->params);
    }
    break;
  }

  case TyCompoundCall: {
    CompoundCall *call = &node->value.ccall;
    freeif(call->stdin_to);
    freeif(call->stdout_to);
    free_commands(call->comms);
    break;
  }
  }
}

void free_param(Params *param) {
  assert(param != NULL);
  for (int i = 0; i < param->count; ++i) {
    freeif(param->s[i]);
  }
}

void free_commands(Commands *comms) {
  assert(comms != NULL);
  for (int i = 0; i < comms->count; ++i) {
    free_node(comms->c[i]);
  }
}

/**** DEBUGGING ****/

void print_node(ASTNode *node, int ind) {
  assert(node != NULL);
  switch (node->type) {
  case TySimpleCall:
    print_scall(node, ind);
    break;
  case TyCompoundCall:
    print_ccall(node, ind);
    break;
  }
}

void print_ccall(ASTNode *node, int ind) {
  assert(node != NULL);
  assert(node->type == TyCompoundCall);
  CompoundCall *call = &node->value.ccall;
  printf("%*sCompoundCall {\n", ind, "");
  printf("%*sop: %c\n", ind + 2, "", call->op);
  print_comms(call->comms, ind + 2);
  printf("%*sstdout_to: %s\n", ind + 2, "",
         call->stdout_to ? call->stdout_to : "NULL");
  printf("%*sstdin_to: %s\n", ind + 2, "",
         call->stdin_to ? call->stdin_to : "NULL");
  printf("%*s}\n", ind, "");
}

void print_scall(ASTNode *node, int ind) {
  assert(node != NULL);
  assert(node->type == TySimpleCall);
  SimpleCall *call = &node->value.scall;
  printf("%*sSimpleCall {\n", ind, "");
  printf("%*sprogram_name: %s\n", ind + 2, "", call->program_name);
  print_params(call->params, ind + 2);
  printf("%*sstdout_to: %s\n", ind + 2, "",
         call->stdout_to ? call->stdout_to : "NULL");
  printf("%*sstdin_to: %s\n", ind + 2, "",
         call->stdin_to ? call->stdin_to : "NULL");
  printf("%*s}\n", ind, "");
}

void print_comms(Commands *comms, int ind) {
  printf("%*sCommands {\n", ind, "");
  if (comms == NULL || comms->count == 0) {
    printf("%*s%s\n", ind + 2, "", "~None~\n");
  } else {
    for (int i = 0; i < comms->count; ++i) {
      print_node(comms->c[i], ind + 2);
    }
  }
  printf("%*s}\n", ind, "");
}

void print_params(Params *params, int ind) {
  printf("%*sParameters {\n", ind, "");
  if (params == NULL || params->count == 0) {
    printf("%*s%s\n", ind + 2, "", "~None~\n");
  } else {
    for (int i = 0; i < params->count; ++i) {
      printf("%*s%s\n", ind + 2, "", params->s[i]);
    }
  }
  printf("%*s}\n", ind, "");
}