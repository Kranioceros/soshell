#ifndef SPAWN_H
#define SPAWN_H
#include "AST.h"

typedef enum TySpawnError {
  NO_ERR,
  ERR_ACCESO_EXEC = 126,
  ERR_NOEXISTE_EXEC = 127,
  ERR_ACCESO_IN = 3,
  ERR_NOEXISTE_IN,
  ERR_ACCESO_OUT,
  ERR_NOEXISTE_OUT,
  ERR_PIPE,
  ERR_DESCONOCIDO,
} TySpawnError;

typedef struct SpawnError {
  TySpawnError type;
  const char *str;
} SpawnError;

int run_scall(SimpleCall *call, int *status_code, SpawnError *err);
int run_ccall(CompoundCall *call, int *status_code, SpawnError *err);
void print_spawn_error(SpawnError *err);

#endif