%option noyywrap nodefault
%{
    #include "../src/parser.h"
    #include "sosh.tab.h"
    #include <assert.h>

    static YY_BUFFER_STATE buf = NULL;
    static char* line_eol = NULL;
%}
%%
[^ \t\n{}$=|><\[\]]+    { yylval.s = yytext;
                          return SIMPLE_STRING; }
\{([^\n{}])+            { yylval.s = yytext;
                          return BEGIN_DELIM_STRING; }
"[" |
"]" |
"}" |
"$" |
"=" |
"|" |
">" |
"<"                     { return yytext[0]; }
\n                      { return EOL; }
[ \t]                   { /* Ignoramos espacios en blanco */ }
%%
void read_from(const char* line) {
    assert(line != NULL);

    // Desalojamos la linea anterior si habia una
    if (line_eol) {
        free(line_eol);
    }

    // Vaciamos el buffer anterior si habia uno
    if (buf) {
        yy_delete_buffer(buf);
    }

    // Creamos una copia de line con un EOL al final
    size_t len = strlen(line);
    char *line_eol = malloc(len+2);
    strncpy(line_eol, line, len);
    line_eol[len] = '\n';
    line_eol[len+1] = '\0';

    buf = yy_scan_string(line_eol);

    yy_switch_to_buffer(buf);
}