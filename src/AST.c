#include "AST.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**** CONSTRUCCION ****/

ASTNode *new_ccall(const char op, Commands *comms) {
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
  assert(node != NULL);
  assert(node->type == TySimpleCall);

  node->value.scall.stdout_to = s;
}

void stdin_to_file(ASTNode *node, const char *s) {
  assert(node != NULL);
  assert(node->type == TySimpleCall);

  node->value.scall.stdin_to = s;
}

Params *new_param(const char *s) {
  assert(s != NULL);
  Params *p = malloc(sizeof(Params));
  p->count = 1;
  p->s[0] = s;

  return p;
}

void add_param(Params *params, const char *s) {
  assert(params != NULL);
  assert(params->count < PARAMS_MAX);

  params->s[params->count++] = s;
}

Commands *new_comm(ASTNode *node) {
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);

  Commands *comms = malloc(sizeof(Commands));
  comms->count = 1;
  comms->c[0] = node;

  return comms;
}

void add_comm(Commands *comms, ASTNode *node) {
  assert(comms != NULL);
  assert(comms->count < PARAMS_MAX);
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);

  comms->c[comms->count++] = node;
}

void add_comm_begin(Commands *comms, ASTNode *node) {
  assert(comms != NULL);
  assert(comms->count < PARAMS_MAX);
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);

  comms->count++;
  // Desplazamos todos hacia adelante una posicion
  for (int i = comms->count - 1; i >= 0; --i) {
    comms->c[i] = comms->c[i - 1];
  }
  // Guardamos el nodo en la primera posicion
  comms->c[0] = node;
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

void _print_node(ASTNode *node, int ind) {
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
  printf("%*sptr: %x\n", ind + 2, "", node);
  printf("%*sop: %c\n", ind + 2, "", call->op);
  printf("%*sstdout_to: %s\n", ind + 2, "",
         call->stdout_to ? call->stdout_to : "NULL");
  printf("%*sstdin_to: %s\n", ind + 2, "",
         call->stdin_to ? call->stdin_to : "NULL");
  print_comms(call->comms, ind + 2);
  printf("%*s}\n", ind, "");
}

void print_scall(ASTNode *node, int ind) {
  assert(node != NULL);
  assert(node->type == TySimpleCall);
  SimpleCall *call = &node->value.scall;
  printf("%*sSimpleCall {\n", ind, "");
  printf("%*sptr: %x\n", ind + 2, "", node);
  printf("%*sprogram_name: %s\n", ind + 2, "", call->program_name);
  printf("%*sstdout_to: %s\n", ind + 2, "",
         call->stdout_to ? call->stdout_to : "NULL");
  printf("%*sstdin_to: %s\n", ind + 2, "",
         call->stdin_to ? call->stdin_to : "NULL");
  print_params(call->params, ind + 2);
  printf("%*s}\n", ind, "");
}

void print_comms(Commands *comms, int ind) {
  printf("%*sCommands {\n", ind, "");
  if (comms == NULL || comms->count == 0) {
    printf("%*s%s\n", ind + 2, "", "~None~\n");
  } else {
    for (int i = 0; i < comms->count; ++i) {
      _print_node(comms->c[i], ind + 2);
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

/**** NORMALIZADO ****/
ASTNode *remove_G(ASTNode *node) {
  assert(node != NULL);

  if (node->type != TyCompoundCall) {
    return node;
  }

  Commands *comms = node->value.ccall.comms;
  assert(comms);

  if (node->value.ccall.op == 'G') {
    assert(comms->count == 1);
    // Copiamos direccion de nodo hijo
    ASTNode *child = comms->c[0];

    // Eliminamos datos del nodo, pero no su hijo
    freeif(node->value.ccall.stdin_to);
    freeif(node->value.ccall.stdout_to);
    free(comms);
    free(node);

    return remove_G(child);
  } else {
    for (int i = 0; i < comms->count; ++i) {
      comms->c[i] = remove_G(comms->c[i]);
    }
    return node;
  }
}
