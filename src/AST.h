#ifndef AST_H
#define AST_H

#define PARAMS_MAX 32
#define COMMANDS_MAX 32

#ifdef DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(args...)                                                        \
  do {                                                                         \
  } while (0)
#endif

/** ESTRUCTURAS DE DATOS **/

typedef enum NodeType { TySimpleCall, TyCompoundCall } NodeType;

struct ASTNode;
typedef struct ASTNode ASTNode;

typedef struct Params {
  int count;
  const char *s[PARAMS_MAX];
} Params;

typedef struct Commands {
  int count;
  ASTNode *c[COMMANDS_MAX];
} Commands;

typedef struct SimpleCall {
  const char *program_name;
  Params *params;        // NULL si no tiene parametros
  const char *stdout_to; // NULL si la salida es stdout
  const char *stdin_to;  // NULL si la entrada es stdin
} SimpleCall;

typedef struct CompoundCall {
  char op;               // `|` si une sus comandos con tuberias
                         // `G` si solo agrupa sus comandos
  Commands *comms;       // Los comandos estan listados en orden inverso!
                         // No deberia ser NULL
  const char *stdout_to; // NULL si la salida es stdout
  const char *stdin_to;  // NULL si la entrada es stdin
} CompoundCall;

typedef union ASTNodeValue {
  SimpleCall scall;
  CompoundCall ccall;
} ASTNodeValue;

typedef struct ASTNode {
  NodeType type;
  ASTNodeValue value;
} ASTNode;

/**** CONSTRUCCION DEL AST ****/

// Crea nueva llamada compuesta
ASTNode *new_ccall(const char op, Commands *comms);
// Crea nueva llamada simple, params puede ser NULL si la llamada no tiene
// parametros
ASTNode *new_scall(const char *program_name, Params *params);
// Redirige la salida estandar a un archivo
void stdout_to_file(ASTNode *node, const char *s);
// Redirige la entrada estandar a un archivo
void stdin_to_file(ASTNode *node, const char *s);

// Crea nuevo bloque de parametros, con `s` como primer parametro
Params *new_param(const char *s);
// Agrega parametro `s` a bloque de parametros `params`
void add_param(Params *params, const char *s);

// Crea nuevo bloque de comandos, con `c` como primer comando
Commands *new_comm(ASTNode *node);
// Agrega comando `node` a bloque de parametros `comms`
void add_comm(Commands *comms, ASTNode *node);
// Agrega comando `node` a bloque de parametros `comms` al principio
void add_comm_begin(Commands *comms, ASTNode *node);

/**** NORMALIZADO DEL AST ****/
// Borra ccalls con operador `G`, producto de agrupar innecesariamente usando []
// Devuelve puntero a nuevo arbol
ASTNode *remove_G(ASTNode *node);

/**** BORRADO DEL AST ****/

void free_node(ASTNode *node);
void free_param(Params *param);
void free_commands(Commands *comms);

/**** FUNCIONES PARA DEBUGGING ****/

void print_node(ASTNode *node, int ind);
void print_scall(ASTNode *node, int ind);
void print_ccall(ASTNode *node, int ind);
void print_params(Params *params, int ind);
void print_comms(Commands *comms, int ind);

#endif