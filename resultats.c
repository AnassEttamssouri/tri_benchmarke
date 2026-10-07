#include "resultats.h"
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int enregistrer_mesure(const MesureBenchmark *mesure, void *fichier)
{
    FILE *f = fichier;
    if (!f) return 0;
    if (!mesure) return fflush(f) == 0;
    return fprintf(f, "%s;%d;%d;%s;%u;%u;%.12f\n", fichier_configuration(mesure->configuration),
                   mesure->taille, mesure->repetition, fichier_algorithme(mesure->algorithme),
                   (unsigned)mesure->graine_entree, (unsigned)mesure->graine_pivot, mesure->secondes) >= 0;
}

static FILE *ouvrir_sortie(const DossierExperience *d, const char *relative)
{
    char path[LONGUEUR_CHEMIN];
    if (!chemin(path, sizeof path, "%s/%s", d->resultats, relative)) return NULL;
    FILE *f = fopen(path, "w");
    if (!f) perror(path);
    return f;
}
int creer_dossier_experience(DossierExperience *d, const Parametres *p, uint32_t seed, int choice)
{
    time_t now = time(NULL);
    struct tm *date = localtime(&now);
    char base[24], path[LONGUEUR_CHEMIN];
    if (!date || !strftime(base, sizeof base, "%Y%m%d_%H%M%S", date) ||
        !assurer_dossier("resultats") || !assurer_dossier("graphiques")) return 0;
    int created = 0;
    for (int i = 0; i < 1000; ++i) {
        if (i == 0) snprintf(d->identifiant, sizeof d->identifiant, "%s", base);
        else snprintf(d->identifiant, sizeof d->identifiant, "%s_%03d", base, i);
        if (!chemin(d->resultats, sizeof d->resultats, "resultats/%s", d->identifiant) ||
            !chemin(d->graphiques, sizeof d->graphiques, "graphiques/%s", d->identifiant)) return 0;
        EtatDossier etat = creer_dossier(d->resultats);
        if (etat == DOSSIER_CREE) { created = 1; break; }
        if (etat != DOSSIER_EXISTANT) return 0;
    }
    if (!created || !assurer_dossier(d->graphiques)) return 0;
    const char *folders[] = {"benchmark", "cases"};
    for (int i = 0; i < 2; ++i) {
        if (!chemin(path, sizeof path, "%s/%s", d->resultats, folders[i]) || !assurer_dossier(path)) return 0;
    }
    const char *graphs[] = {"algorithm_benchmark", "cases", "configurations"};
    for (int i = 0; i < 3; ++i) {
        if (!chemin(path, sizeof path, "%s/%s", d->graphiques, graphs[i]) || !assurer_dossier(path)) return 0;
    }
    d->taille_max = p->taille_max;
    FILE *f = ouvrir_sortie(d, "informations.txt");
    if (!f) return 0;
    fprintf(f, "Application : tri_benchmark\nIdentifiant : %s\nGraine : %u\n", d->identifiant, (unsigned)seed);
    fprintf(f, "Repetitions : %d\nLimite lente : %d\nTaille maximale : %d\nChoix : %d\n",
            p->repetitions, p->limite_lente, p->taille_max, choice);
    fprintf(f, "Tailles :");
    for (int i = 0; i < NOMBRE_TAILLES && TAILLES[i] <= p->taille_max; ++i) fprintf(f, " %d", TAILLES[i]);
    fprintf(f, "\nTemps : secondes; minimum et moyenne arithmetique.\n"
               "Compteurs : un passage separe sur la repetition 1 (pas une moyenne).\n"
               "Comparaisons : valeurs entre elles ou valeur/pivot.\n"
               "Mouvements : ecritures de tableaux, tampon fusion inclus; un swap vaut 3.\n"
               "Generation/copies/verification exclues des temps et compteurs.\n"
               "Entrees : xorshift32, valeurs aleatoires de 0 a 1000000.\n"
               "Pivots : xorshift32-rejet-v1, etat separe; meme graine avant temps et comptage.\n"
               "Minimum chronometre : minimum des repetitions, pas meilleur cas theorique.\n"
               "Graines de chaque repetition dans mesures.csv; algorithme de derivation dans benchmark.c.\n"
               "non_mesure : test ignore au-dela de la limite lente.\n");
    InformationsMachine machine;
    informations_machine(&machine);
    fprintf(f, "Compilateur : GCC %s\nOptimisation : %s\nCPU : %s\nProcesseurs logiques : %lu\n",
            __VERSION__,
#ifdef __OPTIMIZE__
            "active (__OPTIMIZE__); build de livraison avec -O2",
#else
            "inactive",
#endif
            machine.cpu, machine.processeurs);
    if (machine.frequence > 0)
        fprintf(f, "Frequence du chronometre QPC : %" PRId64 " Hz\n", machine.frequence);
    else fputs("Frequence du chronometre QPC : indisponible\n", f);
    return fermer_sortie(f);
}

FILE *ouvrir_mesures(const DossierExperience *d)
{
    FILE *f = ouvrir_sortie(d, "mesures.csv");
    if (f && fprintf(f, "configuration;taille;repetition;algorithme;graine_entree;graine_pivot;temps_s\n") < 0) {
        fclose(f); return NULL;
    }
    return f;
}

static void ecrire_valeur(FILE *f, const LigneResultat *row, int a)
{
    if (row && row->resultat.effectue[a]) fprintf(f, " %.12f", row->resultat.minimum[a]);
    else fprintf(f, " non_mesure");
}
static int ecrire_configuration(const DossierExperience *d, const LigneResultat *rows,
                                int count, Configuration c)
{
    char relative[128];
    snprintf(relative, sizeof relative, "%s.csv", fichier_configuration(c));
    FILE *f = ouvrir_sortie(d, relative);
    if (!f) return 0;
    fprintf(f, "taille");
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) fprintf(f, ";%s_min_s;%s_moyenne_s", fichier_algorithme((Algorithme)a), fichier_algorithme((Algorithme)a));
    fputc('\n', f);
    for (int i = 0; i < count; ++i) {
        if (rows[i].configuration != c) continue;
        const ResultatExperience *r = &rows[i].resultat;
        fprintf(f, "%d", rows[i].taille);
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (r->effectue[a]) fprintf(f, ";%.12f;%.12f", r->minimum[a], r->moyenne[a]);
            else fputs(";non_mesure;non_mesure", f);
        }
        fputc('\n', f);
    }
    return fermer_sortie(f);
}

int enregistrer_resultats(const DossierExperience *d, const LigneResultat *rows, int count)
{
    FILE *f = ouvrir_sortie(d, "resume.csv");
    if (!f) return 0;
    fprintf(f, "configuration;taille;algorithme;statut;minimum_s;moyenne_s\n");
    for (int i = 0; i < count; ++i) {
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (rows[i].configuration >= QUICK_EQUILIBRE && a != RAPIDE) continue;
            fprintf(f, "%s;%d;%s;", fichier_configuration(rows[i].configuration), rows[i].taille, fichier_algorithme((Algorithme)a));
            if (rows[i].resultat.effectue[a]) fprintf(f, "mesure;%.12f;%.12f\n", rows[i].resultat.minimum[a], rows[i].resultat.moyenne[a]);
            else fprintf(f, "non_mesure;non_mesure;non_mesure\n");
        }
    }
    if (!fermer_sortie(f)) return 0;
    f = ouvrir_sortie(d, "operations.csv");
    if (!f) return 0;
    fprintf(f, "configuration;taille;algorithme;statut;repetition_representative;graine_entree;graine_pivot;comparaisons;mouvements\n");
    for (int i = 0; i < count; ++i) {
        const ResultatExperience *r = &rows[i].resultat;
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (rows[i].configuration >= QUICK_EQUILIBRE && a != RAPIDE) continue;
            fprintf(f, "%s;%d;%s;", fichier_configuration(rows[i].configuration), rows[i].taille, fichier_algorithme((Algorithme)a));
            if (r->effectue[a]) fprintf(f, "mesure;1;%u;%u;%" PRIu64 ";%" PRIu64 "\n", (unsigned)r->graine_entree,
                                       (unsigned)r->graine_pivot, r->operations[a].comparaisons, r->operations[a].mouvements);
            else fputs("non_mesure;non_mesure;non_mesure;non_mesure;non_mesure;non_mesure\n", f);
        }
    }
    if (!fermer_sortie(f)) return 0;
    for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c)
        if (configuration_presente(rows, count, (Configuration)c) && !ecrire_configuration(d, rows, count, (Configuration)c)) return 0;
    if (configuration_presente(rows, count, ALEATOIRE)) {
        f = ouvrir_sortie(d, "benchmark/res.dat");
        if (!f) return 0;
        fputs("# Size Selection_Sort Bubble_Sort Insertion_Sort Quick_Sort Merge_Sort Heap_Sort (minimum seconds)\n", f);
        for (int i = 0; i < count; ++i) if (rows[i].configuration == ALEATOIRE) {
            fprintf(f, "%d", rows[i].taille);
            for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) ecrire_valeur(f, &rows[i], a);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    const Configuration order[] = { CROISSANT, ALEATOIRE, DECROISSANT, PRESQUE_TRIE, VALEURS_REPETEES };
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        int have_general = 0;
        for (int i = 0; i < count; ++i) if (rows[i].configuration < QUICK_EQUILIBRE) have_general = 1;
        if (!have_general) break;
        char relative[128];
        snprintf(relative, sizeof relative, "cases/%s_cases.dat", fichier_algorithme((Algorithme)a));
        f = ouvrir_sortie(d, relative);
        if (!f) return 0;
        fputs("# N Sorted Random Reversed Nearly_Sorted Ten_Repeated_Values (minimum seconds)\n", f);
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= d->taille_max; ++s) {
            fprintf(f, "%d", TAILLES[s]);
            for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c) ecrire_valeur(f, trouver(rows, count, TAILLES[s], order[c]), a);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    if (configuration_presente(rows, count, QUICK_EQUILIBRE)) {
        f = ouvrir_sortie(d, "cases/quick_sort_best_avg_worst_cases.dat");
        if (!f) return 0;
        fputs("# N Balanced_Partitions Random All_Equal (minimum seconds)\n", f);
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= d->taille_max; ++s) {
            fprintf(f, "%d", TAILLES[s]);
            for (int c = QUICK_EQUILIBRE; c <= QUICK_EGAUX; ++c) ecrire_valeur(f, trouver(rows, count, TAILLES[s], (Configuration)c), RAPIDE);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    return 1;
}

int enregistrer_dernier_dossier(const DossierExperience *d)
{
    FILE *f = fopen("resultats/dernier_run.txt", "w");
    if (!f) return 0;
    fprintf(f, "%s\n", d->identifiant);
    return fermer_sortie(f);
}
int charger_dernier_dossier(DossierExperience *d)
{
    FILE *f = fopen("resultats/dernier_run.txt", "r");
    if (!f) return 0;
    if (!fgets(d->identifiant, sizeof d->identifiant, f)) { fclose(f); return 0; }
    fclose(f);
    d->identifiant[strcspn(d->identifiant, "\r\n")] = 0;
    if (!d->identifiant[0]) return 0;
    for (int i = 0; d->identifiant[i]; ++i)
        if (!isdigit((unsigned char)d->identifiant[i]) && d->identifiant[i] != '_') return 0;
    if (!chemin(d->resultats, sizeof d->resultats, "resultats/%s", d->identifiant) ||
        !chemin(d->graphiques, sizeof d->graphiques, "graphiques/%s", d->identifiant)) return 0;
    d->taille_max = TAILLE_MAXIMALE;
    return 1;
}

int charger_resume(const DossierExperience *d, LigneResultat *rows, int *count)
{
    char path[LONGUEUR_CHEMIN], line[1024];
    int seen[NOMBRE_CAS * NOMBRE_TAILLES][NOMBRE_ALGORITHMES] = {{0}};
    *count = 0;
    if (!chemin(path, sizeof path, "%s/resume.csv", d->resultats)) return 0;
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Resume introuvable : %s\n", path); return 0; }
    int ok = fgets(line, sizeof line, f) &&
        strcmp(line, "configuration;taille;algorithme;statut;minimum_s;moyenne_s\n") == 0;
    while (ok && fgets(line, sizeof line, f)) {
        if (!strchr(line, '\n') && !feof(f)) { ok = 0; break; }
        line[strcspn(line, "\r\n")] = 0;
        char *fields[6], *part = line;
        for (int i = 0; i < 6; ++i) {
            fields[i] = part;
            char *sep = strchr(part, ';');
            if ((i < 5 && !sep) || (i == 5 && sep)) { ok = 0; break; }
            if (sep) { *sep = 0; part = sep + 1; }
            if (!fields[i][0]) { ok = 0; break; }
        }
        if (!ok) break;
        int c = -1, a = -1, n = 0;
        for (int i = 0; i < NOMBRE_CAS; ++i)
            if (!strcmp(fields[0], fichier_configuration((Configuration)i))) c = i;
        for (int i = 0; i < NOMBRE_ALGORITHMES; ++i)
            if (!strcmp(fields[2], fichier_algorithme((Algorithme)i))) a = i;
        char *end;
        errno = 0;
        long size = strtol(fields[1], &end, 10);
        if (!errno && !*end)
            for (int i = 0; i < NOMBRE_TAILLES; ++i) if (size == TAILLES[i]) n = (int)size;
        if (c < 0 || a < 0 || !n || (c >= QUICK_EQUILIBRE && a != RAPIDE)) { ok = 0; break; }
        int r;
        for (r = 0; r < *count; ++r) if (rows[r].configuration == (Configuration)c && rows[r].taille == n) break;
        if (r == *count) {
            if (*count == NOMBRE_CAS * NOMBRE_TAILLES) { ok = 0; break; }
            memset(&rows[r], 0, sizeof rows[r]);
            rows[r].configuration = (Configuration)c; rows[r].taille = n;
            ++*count;
        }
        if (seen[r][a]++) { ok = 0; break; }
        if (!strcmp(fields[3], "non_mesure")) {
            if (strcmp(fields[4], "non_mesure") || strcmp(fields[5], "non_mesure")) ok = 0;
        } else if (!strcmp(fields[3], "mesure")) {
            double values[2];
            for (int i = 0; i < 2; ++i) {
                errno = 0;
                values[i] = strtod(fields[4 + i], &end);
                if (errno || *end || !isfinite(values[i]) || values[i] < 0) ok = 0;
            }
            if (!ok || values[0] > values[1] + 1e-12) { ok = 0; break; }
            rows[r].resultat.effectue[a] = 1;
            rows[r].resultat.minimum[a] = values[0];
            rows[r].resultat.moyenne[a] = values[1];
        } else ok = 0;
    }
    if (ferror(f) || !*count) ok = 0;
    fclose(f);
    for (int r = 0; ok && r < *count; ++r) {
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a)
            if ((rows[r].configuration < QUICK_EQUILIBRE || a == RAPIDE) && !seen[r][a]) ok = 0;
        if (rows[r].configuration >= QUICK_EQUILIBRE)
            for (int c = QUICK_EQUILIBRE; c <= QUICK_EGAUX; ++c)
                if (!trouver(rows, *count, rows[r].taille, (Configuration)c)) ok = 0;
    }
    if (!ok) fprintf(stderr, "Resume vide, incomplet ou malforme : %s\n", path);
    return ok;
}

static int decouper_champs(char *line, char **fields, int number)
{
    line[strcspn(line, "\r\n")] = 0;
    char *part = line;
    for (int i = 0; i < number; ++i) {
        fields[i] = part;
        char *sep = strchr(part, ';');
        if ((i < number - 1 && !sep) || (i == number - 1 && sep) || !part[0]) return 0;
        if (sep) { *sep = 0; part = sep + 1; }
        if (!fields[i][0]) return 0;
    }
    return 1;
}
static int lire_compteur(const char *text, uint64_t maximum, uint64_t *value)
{
    if (!text[0]) return 0;
    for (int i = 0; text[i]; ++i) if (!isdigit((unsigned char)text[i])) return 0;
    char *end;
    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno || *end || parsed > maximum) return 0;
    *value = (uint64_t)parsed;
    return 1;
}
int charger_resultats_analyse(const DossierExperience *d, LigneResultat *rows, int *count)
{
    if (!charger_resume(d, rows, count)) return 0;
    char path[LONGUEUR_CHEMIN], line[1024];
    if (!chemin(path, sizeof path, "%s/operations.csv", d->resultats)) return 0;
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Compteurs introuvables : %s\n", path); return 0; }
    int seen[NOMBRE_CAS * NOMBRE_TAILLES][NOMBRE_ALGORITHMES] = {{0}};
    int ok = fgets(line, sizeof line, f) && strcmp(line,
        "configuration;taille;algorithme;statut;repetition_representative;graine_entree;graine_pivot;comparaisons;mouvements\n") == 0;
    while (ok && fgets(line, sizeof line, f)) {
        if (!strchr(line, '\n') && !feof(f)) { ok = 0; break; }
        char *fields[9];
        if (!decouper_champs(line, fields, 9)) { ok = 0; break; }
        int c = -1, a = -1, r = -1;
        uint64_t size;
        for (int i = 0; i < NOMBRE_CAS; ++i)
            if (!strcmp(fields[0], fichier_configuration((Configuration)i))) c = i;
        for (int i = 0; i < NOMBRE_ALGORITHMES; ++i)
            if (!strcmp(fields[2], fichier_algorithme((Algorithme)i))) a = i;
        if (c < 0 || a < 0 || !lire_compteur(fields[1], TAILLE_MAXIMALE, &size)) { ok = 0; break; }
        for (int i = 0; i < *count; ++i)
            if (rows[i].configuration == (Configuration)c && rows[i].taille == (int)size) r = i;
        if (r < 0 || (c >= QUICK_EQUILIBRE && a != RAPIDE) || seen[r][a]++) { ok = 0; break; }
        ResultatExperience *result = &rows[r].resultat;
        if (!strcmp(fields[3], "non_mesure")) {
            if (result->effectue[a]) { ok = 0; break; }
            for (int i = 4; i < 9; ++i) if (strcmp(fields[i], "non_mesure")) ok = 0;
        } else if (!strcmp(fields[3], "mesure")) {
            uint64_t rep, input, pivot, comparisons, movements;
            if (!result->effectue[a] || !lire_compteur(fields[4], 1, &rep) || rep != 1 ||
                !lire_compteur(fields[5], UINT32_MAX, &input) || !input ||
                !lire_compteur(fields[6], UINT32_MAX, &pivot) || !pivot ||
                !lire_compteur(fields[7], UINT64_MAX, &comparisons) ||
                !lire_compteur(fields[8], UINT64_MAX, &movements)) { ok = 0; break; }
            if (result->graine_entree && (result->graine_entree != input || result->graine_pivot != pivot)) {
                ok = 0; break;
            }
            result->graine_entree = (uint32_t)input; result->graine_pivot = (uint32_t)pivot;
            result->operations[a] = (Operations){comparisons, movements};
        } else ok = 0;
    }
    if (ferror(f)) ok = 0;
    fclose(f);
    for (int r = 0; ok && r < *count; ++r)
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a)
            if ((rows[r].configuration < QUICK_EQUILIBRE || a == RAPIDE) && !seen[r][a]) ok = 0;
    if (!ok) fprintf(stderr, "Compteurs incomplets, malformes ou incompatibles avec le resume : %s\n", path);
    return ok;
}

static void ecrire_estimation(FILE *f, const EstimationCroissance *estimate)
{
    fprintf(f, "%d;%s;", estimate->points, nom_modele(estimate->modele));
    if (estimate->points >= 3 && isfinite(estimate->dispersion))
        fprintf(f, "%.6f;%.6f;", estimate->exposant, estimate->dispersion);
    else fputs("non_evalue;non_evalue;", f);
    const char *status = estimate->points < 3 ? "donnees_insuffisantes" :
                         estimate->modele == MODELE_INDETERMINE ? "aucun_modele_retenu" :
                         estimate->dispersion > DISPERSION_INCERTAINE ? "incertain" : "estimation";
    fprintf(f, "%s;", status);
}
int enregistrer_analyse(const DossierExperience *d, const AnalyseComplexite *analyses, int count)
{
    char destination[LONGUEUR_CHEMIN], temporary[LONGUEUR_CHEMIN];
    if (count <= 0 || count > NOMBRE_ANALYSES_MAX || !analyses ||
        !chemin(destination, sizeof destination, "%s/complexite.csv", d->resultats) ||
        !chemin(temporary, sizeof temporary, "%s/complexite.csv.tmp", d->resultats)) return 0;
    FILE *f = fopen(temporary, "w");
    if (!f) { perror(temporary); return 0; }
    fputs("configuration;algorithme;reference_theorique;points_operations;modele_operations;"
          "exposant_operations;dispersion_operations;statut_operations;points_temps;modele_temps;"
          "exposant_temps;dispersion_temps;statut_temps;verification_comparaisons_exactes;"
          "points_exacts;ecarts_exacts\n", f);
    for (int i = 0; i < count; ++i) {
        const AnalyseComplexite *a = &analyses[i];
        fprintf(f, "%s;%s;%s;", fichier_configuration(a->configuration),
                fichier_algorithme(a->algorithme), a->reference);
        ecrire_estimation(f, &a->operations); ecrire_estimation(f, &a->temps);
        fprintf(f, "%s;%d;%d\n", nom_verification(a->verification), a->points_exacts, a->ecarts_exacts);
    }
    if (!fermer_sortie(f)) { remove(temporary); fprintf(stderr, "Echec d'ecriture : %s\n", destination); return 0; }
    if (!remplacer_fichier(temporary, destination)) {
        remove(temporary); fprintf(stderr, "Impossible de remplacer : %s\n", destination); return 0;
    }
    return 1;
}
