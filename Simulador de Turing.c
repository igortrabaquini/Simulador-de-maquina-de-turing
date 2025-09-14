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

char *read_file_to_str(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

int find_int(const char *s, const char *key, int *out) {
    const char *p = strstr(s, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    int val;
    if (sscanf(p, "%d", &val) == 1) { *out = val; return 1; }
    return 0;
}

int find_string_char(const char *s, const char *key, char *out) {
    const char *p = strstr(s, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\"') {
        p++;
        if (*p == '\\') p++;
        *out = *p;
        return 1;
    }
    if (*p) { *out = *p; return 1; }
    return 0;
}

int parse_finals(const char *s, int *arr, int *count) {
    const char *p = strstr(s, "\"final\"");
    if (!p) p = strstr(s, "\"finals\"");
    if (!p) return 0;
    p = strchr(p, '[');
    if (!p) return 0;
    p++;
    int c = 0;
    while (*p && *p != ']') {
        while (*p && (isspace((unsigned char)*p) || *p==',')) p++;
        if (!*p || *p==']') break;
        int val;
        if (sscanf(p, "%d", &val) == 1) {
            if (c < MAX_FINALS) arr[c++] = val;
            while (*p && *p != ',' && *p != ']') p++;
        } else break;
    }
    *count = c;
    return c>0;
}

int parse_transitions(const char *s, Transition *trans, int *count) {
    const char *p = strstr(s, "\"transitions\"");
    if (!p) return 0;
    p = strchr(p, '[');
    if (!p) return 0;
    p++;
    int c = 0;
    while (*p) {
        const char *obj = strchr(p, '{');
        if (!obj) break;
        const char *endobj = strchr(obj, '}');
        if (!endobj) break;
        size_t len = endobj - obj + 1;
        char *buf = malloc(len + 1);
        if (!buf) break;
        memcpy(buf, obj, len);
        buf[len] = '\0';
        Transition t;
        t.from = t.to = -1;
        t.read = t.write = t.dir = 0;
        find_int(buf, "\"from\"", &t.from);
        find_int(buf, "\"to\"", &t.to);
        find_string_char(buf, "\"read\"", &t.read);
        find_string_char(buf, "\"write\"", &t.write);
        char dirc = 0;
        if (find_string_char(buf, "\"dir\"", &dirc)) t.dir = dirc;
        else {
            const char *q = strstr(buf, "\"dir\"");
            if (q) {
                q = strchr(q, ':');
                if (q) {
                    q++;
                    while (*q && isspace((unsigned char)*q)) q++;
                    if (*q) t.dir = *q;
                }
            }
        }
        free(buf);
        if (t.from!=-1 && t.to!=-1 && t.read && t.write && (t.dir=='L' || t.dir=='R')) {
            if (c < MAX_TRANSITIONS) trans[c++] = t;
            else break;
        }
        p = endobj + 1;
    }
    *count = c;
    return c>0;
}

int is_final(TM *m, int state) {
    for (int i=0;i<m->finals_count;i++) if (m->finals[i]==state) return 1;
    return 0;
}

int find_transition(TM *m, int state, char read, Transition *out) {
    for (int i=0;i<m->transitions_count;i++) {
        if (m->transitions[i].from == state && m->transitions[i].read == read) {
            *out = m->transitions[i];
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s regras.json entrada.txt [saida.txt]\n", argv[0]);
        return 1;
    }
    const char *spec_path = argv[1];
    const char *input_path = argv[2];
    const char *output_path = (argc >= 4) ? argv[3] : "saida.txt";

    char *spec = read_file_to_str(spec_path);
    if (!spec) {
        fprintf(stderr, "Erro ao abrir %s\n", spec_path);
        return 1;
    }

    TM m;
    memset(&m, 0, sizeof(m));
    m.initial = 0;
    m.white = '_';
    parse_finals(spec, m.finals, &m.finals_count);
    find_int(spec, "\"initial\"", &m.initial);
    char whitec = 0;
    if (find_string_char(spec, "\"white\"", &whitec)) m.white = whitec;
    parse_transitions(spec, m.transitions, &m.transitions_count);
    free(spec);

    char *input = read_file_to_str(input_path);
    char *tape_content = NULL;
    if (!input) {
        tape_content = malloc(2);
        tape_content[0] = m.white;
        tape_content[1] = '\0';
    } else {
        char *nl = strchr(input, '\n');
        if (nl) *nl = '\0';
        if (strlen(input) == 0) {
            tape_content = malloc(2);
            tape_content[0] = m.white;
            tape_content[1] = '\0';
        } else {
            tape_content = strdup(input);
        }
        free(input);
    }

    size_t tape_size = INITIAL_TAPE_SIZE;
    char *tape = malloc(tape_size);
    if (!tape) { fprintf(stderr,"Memória insuficiente\n"); return 1; }
    for (size_t i=0;i<tape_size;i++) tape[i] = m.white;
    size_t center = tape_size / 2;
    size_t input_len = strlen(tape_content);
    for (size_t i=0;i<input_len;i++) tape[center + i] = tape_content[i];
    free(tape_content);

    long head = (long)center;
    long leftmost = head;
    long rightmost = head + (long)input_len - 1;
    int state = m.initial;

    unsigned long steps = 0;
    int accepted = 0;
    while (1) {
        if (is_final(&m, state)) { accepted = 1; break; }

        if (head < 0) {
            size_t new_size = tape_size * 2;
            char *newt = malloc(new_size);
            if (!newt) { fprintf(stderr,"Memória insuficiente\n"); return 1; }
            for (size_t i=0;i<new_size;i++) newt[i] = m.white;
            size_t shift = new_size - tape_size;
            memcpy(newt + shift, tape, tape_size);
            head += (long)shift;
            leftmost += (long)shift;
            rightmost += (long)shift;
            center += (long)shift;
            tape_size = new_size;
            free(tape);
            tape = newt;
        } else if ((size_t)head >= tape_size) {
            size_t new_size = tape_size * 2;
            char *newt = malloc(new_size);
            if (!newt) { fprintf(stderr,"Memória insuficiente\n"); return 1; }
            for (size_t i=0;i<new_size;i++) newt[i] = m.white;
            memcpy(newt, tape, tape_size);
            tape_size = new_size;
            free(tape);
            tape = newt;
        }

        char cur = tape[head];
        Transition t;
        if (!find_transition(&m, state, cur, &t)) {
            accepted = 0;
            break;
        }

        tape[head] = t.write;
        if (t.dir == 'R') head++;
        else head--;
        if (head < leftmost) leftmost = head;
        if (head > rightmost) rightmost = head;
        state = t.to;

        steps++;
    }

    long out_l = leftmost;
    long out_r = rightmost;
    if (out_l < 0) out_l = 0;
    if (out_r < 0) out_r = 0;
    if ((size_t)out_r >= tape_size) out_r = tape_size - 1;
    if ((size_t)out_l >= tape_size) out_l = tape_size - 1;

    while (out_l <= out_r && tape[out_l] == m.white) out_l++;
    while (out_r >= out_l && tape[out_r] == m.white) out_r--;
    FILE *fo = fopen(output_path, "w");
    if (!fo) { fprintf(stderr,"Erro ao criar %s\n", output_path); return 1; }
    if (out_l > out_r) {
        fputc(m.white, fo);
    } else {
        for (long i = out_l; i <= out_r; i++) fputc(tape[i], fo);
    }
    fputc('\n', fo);
    fclose(fo);

    free(tape);

    printf("%d\n", accepted ? 1 : 0);
    return 0;
}
