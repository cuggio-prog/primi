/* primi.c — stampa i primi n numeri primi, uno per riga.
 *
 * Compilazione: gcc -O2 -Wall -Wextra -std=c11 -o primi primi.c -lm
 * Uso:          ./primi <n>
 *
 * Algoritmo: Criba di Eratostene solo sui numeri dispari (metà memoria e
 * metà lavoro), con limite superiore stimato dal teorema dei numeri primi
 * (per n >= 6: p_n < n * (ln n + ln ln n)), più un margine del 5% e una
 * rete di sicurezza che raddoppia il limite se non bastasse.
 */

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 10000000LL /* limite massimo accettato per n */

/* Convalida l'input: intero positivo in [1, MAX_N].
 * Ritorna 1 se valido (con il valore in *out), 0 altrimenti. */
static int parse_n(const char *s, long long *out)
{
    errno = 0;
    char *end;
    long long v = strtoll(s, &end, 10);

    if (errno != 0)          /* overflow */
        return 0;
    if (end == s || *end)    /* nessun numero o caratteri non numerici */
        return 0;
    if (v < 1 || v > MAX_N)  /* fuori intervallo */
        return 0;

    *out = v;
    return 1;
}

/* Limite superiore sicuro per l'n-esimo primo. */
static size_t upper_bound(long long n)
{
    if (n < 6)
        return 16; /* p_5 = 11 */
    double dn = (double)n;
    return (size_t)(1.05 * dn * (log(dn) + log(log(dn)))) + 16;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long long n;
    if (!parse_n(argv[1], &n)) {
        fprintf(stderr,
                "Errore: input non valido '%s' "
                "(atteso un intero positivo da 1 a %lld)\n",
                argv[1], MAX_N);
        return EXIT_FAILURE;
    }

    size_t bound = upper_bound(n);
    uint8_t *comp = NULL;

    /* Criba: l'indice i corrisponde al dispari 2*i + 3 (l'1 e il 2
     * sono gestiti a parte). Si ripete solo se il limite non bastasse. */
    for (;;) {
        size_t count = (bound - 1) / 2; /* numero di dispari in [3, bound] */

        comp = realloc(comp, count);
        if (!comp) {
            fprintf(stderr, "Errore: memoria insufficiente\n");
            return EXIT_FAILURE;
        }
        memset(comp, 0, count);

        for (size_t i = 0;; i++) {
            size_t p = 2 * i + 3;
            if (p > bound / p) /* p*p > bound: la cribra è completa */
                break;
            if (comp[i])
                continue; /* p già composto */
            /* Segna i multipli di p a partire da p^2, step 2p
             * (= p indici nello spazio dei soli dispari). */
            for (size_t j = (p * p - 3) / 2; j < count; j += p)
                comp[j] = 1;
        }

        size_t total = 1; /* il 2 */
        for (size_t i = 0; i < count; i++)
            total += !comp[i];

        if (total >= (size_t)n)
            break; /* limite sufficiente (il margine del 5% basta sempre) */

        if (bound > SIZE_MAX / 2) {
            fprintf(stderr, "Errore interno: limite di cribra troppo grande\n");
            return EXIT_FAILURE;
        }
        bound *= 2; /* rete di sicurezza: raddoppia e ricriba */
    }

    /* Stampa: il 2 e i dispari non segnati come composti. */
    size_t printed = 1;
    printf("2\n");
    for (size_t i = 0; printed < (size_t)n; i++) {
        if (!comp[i]) {
            printf("%zu\n", 2 * i + 3);
            printed++;
        }
    }

    free(comp);
    return EXIT_SUCCESS;
}
