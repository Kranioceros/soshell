#ifndef CHECKER_H
#define CHECKER_H
#include "AST.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum TyError { RedirectionError } TyError;

typedef struct Error {
  TyError type;
  int pos;
  int len;
} Error;

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

int check_redirections(ASTNode *node, Error *err);

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

int check_redirections_pipe(ASTNode *node, Error *err) {
  printf("check_redirections_pipe called on node: %x\n", node);
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
    err->type = RedirectionError;
    printf("Failed on first_child stdout_to\n");
    return -1;
  }

  for (int i = 1; i < comms->count - 1; ++i) {
    ASTNode *middle_child = comms->c[i];
    if (get_stdin_to(middle_child) || get_stdout_to(middle_child)) {
      err->type = RedirectionError;
      printf("Failed on child %d stdin_to or stdout_to\n", i);
      return -1;
    }
  }

  if (get_stdin_to(last_child) != NULL) {
    err->type = RedirectionError;
    printf("Failed on last_child stdin_to\n");
    return -1;
  }

  node->value.ccall.stdin_to = get_stdin_to(first_child);
  node->value.ccall.stdout_to = get_stdout_to(last_child);

  return 0;
}

int check_redirections(ASTNode *node, Error *err) {
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
    printf("Error: Operador desconocido en checker!\n");
  }
  return 0;
}

#endif