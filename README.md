/* tm_simulator.c
   Compilar:
     gcc -O2 -o tm_simulator tm_simulator.c
   Uso:
     ./tm_simulator regras.json entrada.txt saida.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_TAPE_SIZE 4096
#define MAX_TRANSITIONS 10000
#define MAX_FINALS 256

typedef struct {
    int from;
    int to;
    char read;
    char write;
    char dir; /* 'L' ou 'R' */
} Transition;

typedef struct {
    int initial;
    int finals[MAX_FINALS];
    int finals_count;
    char white;
    Transition transitions[MAX_TRANSITIONS];
    int transitions_count;
} TM;

/* ---- Funções utilitárias de parsing JSON simplificado ---- */
char *read_file_to_str(const char *path);
int find_int(const char *s, const char *key, int *out);
int find_string_char(const char *s, const char *key, char *out);
int parse_finals(const char *s, int *arr, int *count);
int parse_transitions(const char *s, Transition *trans, int *count);

/* ---- Funções da Máquina de Turing ---- */
int is_final(TM *m, int state);
int find_transition(TM *m, int state, char read, Transition *out);

/* ---- Função principal ---- */
int main(int argc, char **argv) {
    /* ... (código completo, igual ao enviado anteriormente) ... */
}
