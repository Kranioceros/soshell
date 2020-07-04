#ifndef SPAWN_H
#define SPAWN_H
#include "AST.h"

typedef enum TySpawnError {
  ERR_ACCESO_EXEC = 1,
  ERR_NOEXISTE_EXEC,
  ERR_ACCESO_IN,
  ERR_NOEXISTE_IN,
  ERR_ACCESO_OUT,
  ERR_NOEXISTE_OUT,
  ERR_DESCONOCIDO,
} TySpawnError;

TySpawnError run_scall(SimpleCall *call, int *status_code);
void print_spawn_error(TySpawnError type, ASTNode *node);

#endif