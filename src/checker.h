#ifndef CHECKER_H
#define CHECKER_H
#include "AST.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum TyRedirError {
  NO_REDIRERR,
  ERR_STDIN,
  ERR_STDOUT,
} TyRedirError;

typedef struct RedirError {
  TyRedirError type;
  const char *program;
} RedirError;

const char *get_stdin_to(ASTNode *node) {
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);
  const char *stdin_to;
  switch (node->type) {
  case TySimpleCall:
    stdin_to = node->value.scall.stdin_to;
    node->value.scall.stdin_to = NULL;
    return stdin_to;
  case TyCompoundCall:
    stdin_to = node->value.ccall.stdin_to;
    node->value.ccall.stdin_to = NULL;
    return stdin_to;
  }
}

int check_redirections(ASTNode *node, RedirError *err);

const char *get_stdout_to(ASTNode *node) {
  assert(node != NULL);
  assert(node->type == TySimpleCall || node->type == TyCompoundCall);
  const char *stdout_to;
  switch (node->type) {
  case TySimpleCall:
    stdout_to = node->value.scall.stdout_to;
    node->value.scall.stdout_to = NULL;
    return stdout_to;
  case TyCompoundCall:
    stdout_to = node->value.ccall.stdout_to;
    node->value.ccall.stdout_to = NULL;
    return stdout_to;
  }
}

int check_redirections_pipe(ASTNode *node, RedirError *err) {
  debugf("check_redirections_pipe called on node: %x\n", node);
  assert(node != NULL);
  assert(node->type == TyCompoundCall);
  assert(node->value.ccall.op == '|');
  assert(node->value.ccall.comms);
  Commands *comms = node->value.ccall.comms;
  assert(comms->count > 1);

  for (int i = 0; i < comms->count; ++i) {
    check_redirections(comms->c[i], err);
  }

  ASTNode *first_child = comms->c[0];
  ASTNode *last_child = comms->c[comms->count - 1];

  if (get_stdout_to(first_child) != NULL) {
    // Asumimos que nodo es simple_call
    err->type = ERR_STDOUT;
    err->program = first_child->value.scall.program_name;
    debugf("check_redirections_pipe: Redireccionando salida estandar en primer "
           "comando de pipeline\n");
    return -1;
  }

  for (int i = 1; i < comms->count - 1; ++i) {
    ASTNode *middle_child = comms->c[i];
    int err_stdin = get_stdin_to(middle_child) != NULL;
    int err_stdout = get_stdout_to(middle_child) != NULL;
    if (err_stdin || err_stdout) {
      // Asumimos que nodo es simple_call
      err->type = err_stdin ? ERR_STDIN : ERR_STDOUT;
      err->program = middle_child->value.scall.program_name;
      debugf("check_redirections_pipe: Redireccionando entrada o salida "
             "estandar de un comando en medio de la pipeline\n",
             i);
      return -1;
    }
  }

  if (get_stdin_to(last_child) != NULL) {
    // Asumimos que nodo es simple_call
    err->type = ERR_STDIN;
    err->program = last_child->value.scall.program_name;
    debugf("check_redirections_pipe: Redireccionando entrada estandar en "
           "ultimo comando de pipeline\n");
    return -1;
  }

  node->value.ccall.stdin_to = get_stdin_to(first_child);
  node->value.ccall.stdout_to = get_stdout_to(last_child);

  return 0;
}

int check_redirections(ASTNode *node, RedirError *err) {
  assert(node != NULL);
  if (node->type != TyCompoundCall) {
    return 0;
  }

  CompoundCall *ccall = &node->value.ccall;
  int status;

  switch (ccall->op) {
  case '|':
    status = check_redirections_pipe(node, err);
    return status;
  default:
    debugf("Error: Operador desconocido en checker!\n");
  }
  return 0;
}

void print_redirerror(RedirError *err) {
  assert(err);
  switch (err->type) {
  case ERR_STDIN:
    printf(
        "Error de redirección en subcomando '%s': No se puede redireccionar la "
        "entrada estandar de un subcomando de una pipeline salvo que sea el "
        "primero\n",
        err->program);
    break;

  case ERR_STDOUT:
    printf(
        "Error de redirección en subcomando '%s': No se puede redireccionar la "
        "salida estandar de un subcomando de una pipeline salvo que sea el "
        "ultimo\n",
        err->program);
    break;
  default:
    printf("Error desconocido de redireccion\n");
  }
}

#endif