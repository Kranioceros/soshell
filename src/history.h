#include <readline/history.h>
#include <stdlib.h>
#define HISTORY_SIZE 64

char *history[HISTORY_SIZE];
int line_count = 0;

void history_add(char *line) {
  // La registramos
  add_history(line);

  // Borramos viejas lineas si es necesario
  if (line_count >= HISTORY_SIZE) {
    free(history[line_count]);
  }
  // La agregamos a history
  history[line_count++ % HISTORY_SIZE] = line;
}

void history_delete() {
  // Borramos las lineas aun almacenadas
  line_count = line_count > HISTORY_SIZE ? HISTORY_SIZE : line_count;
  for (int i = 0; i < line_count; ++i) {
    free(history[i]);
  }
}

void history_init() {
  using_history();
  stifle_history(HISTORY_SIZE);
}