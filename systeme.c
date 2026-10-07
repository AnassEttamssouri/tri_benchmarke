#include "systeme.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>

static LARGE_INTEGER frequence_chrono;

void initialiser_console(void)
{
    SetConsoleOutputCP(CP_UTF8); SetConsoleCP(CP_UTF8);
}
int chrono_disponible(void)
{
    return frequence_chrono.QuadPart > 0 ||
        (QueryPerformanceFrequency(&frequence_chrono) && frequence_chrono.QuadPart > 0);
}
int chrono_instant(int64_t *instant)
{
    LARGE_INTEGER value;
    if (!QueryPerformanceCounter(&value)) return 0;
    *instant = value.QuadPart;
    return 1;
}
double chrono_secondes(int64_t debut, int64_t fin)
{
    return (double)(fin - debut) / (double)frequence_chrono.QuadPart;
}
void informations_machine(InformationsMachine *machine)
{
    SYSTEM_INFO information;
    GetSystemInfo(&information);
    const char *cpu = getenv("PROCESSOR_IDENTIFIER");
    snprintf(machine->cpu, sizeof machine->cpu, "%s", cpu ? cpu : "identifiant indisponible");
    machine->processeurs = (unsigned long)information.dwNumberOfProcessors;
    machine->frequence = chrono_disponible() ? frequence_chrono.QuadPart : 0;
}
EtatDossier creer_dossier(const char *path)
{
    if (CreateDirectoryA(path, NULL)) return DOSSIER_CREE;
    return GetLastError() == ERROR_ALREADY_EXISTS ? DOSSIER_EXISTANT : DOSSIER_ERREUR;
}
int remplacer_fichier(const char *source, const char *destination)
{
    return MoveFileExA(source, destination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}
int parcourir_fichiers(const char *dossier, const char *motif,
                      int (*visiter)(const char *, void *), void *contexte, int *nombre)
{
    char pattern[LONGUEUR_CHEMIN], path[LONGUEUR_CHEMIN];
    if (!chemin(pattern, sizeof pattern, "%s/%s", dossier, motif)) return 0;
    WIN32_FIND_DATAA found;
    HANDLE search = FindFirstFileA(pattern, &found);
    if (search == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
    }
    int ok = 1;
    do {
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (!chemin(path, sizeof path, "%s/%s", dossier, found.cFileName) || !visiter(path, contexte)) ok = 0;
        ++*nombre;
    } while (FindNextFileA(search, &found));
    if (GetLastError() != ERROR_NO_MORE_FILES) ok = 0;
    FindClose(search);
    return ok;
}

int chemin(char *out, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(out, size, format, args);
    va_end(args);
    return n >= 0 && (size_t)n < size;
}
int assurer_dossier(const char *path)
{
    if (CreateDirectoryA(path, NULL)) return 1;
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}
int fichier_existe(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
int fermer_sortie(FILE *f)
{
    int ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;
    return ok;
}

int trouver_gnuplot(char *out, size_t capacity)
{
    const char *custom = getenv("GNUPLOT_EXE");
    if (custom && custom[0]) return fichier_existe(custom) && chemin(out, capacity, "%s", custom);
    DWORD n = SearchPathA(NULL, "gnuplot.exe", NULL, (DWORD)capacity, out, NULL);
    if (n > 0 && n < capacity) return 1;
    const char *places[] = {"C:/Program Files/gnuplot/bin/gnuplot.exe", "C:/Program Files (x86)/gnuplot/bin/gnuplot.exe"};
    for (int i = 0; i < 2; ++i) if (fichier_existe(places[i])) return chemin(out, capacity, "%s", places[i]);
    return 0;
}

int executer_gnuplot(const char *exe, const char *script)
{
    char command[LONGUEUR_CHEMIN * 2 + 16];
    if (!chemin(command, sizeof command, "\"%s\" \"%s\"", exe, script)) return 0;
    STARTUPINFOA start;
    PROCESS_INFORMATION process;
    memset(&start, 0, sizeof start);
    start.cb = sizeof start;
    if (!CreateProcessA(exe, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &start, &process)) return 0;
    DWORD exit_code = 1;
    WaitForSingleObject(process.hProcess, INFINITE);
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hProcess); CloseHandle(process.hThread);
    return exit_code == 0;
}

int ouvrir_png(const char *png)
{
    char absolute[LONGUEUR_CHEMIN];
    DWORD n = GetFullPathNameA(png, sizeof absolute, absolute, NULL);
    if (!n || n >= sizeof absolute || !fichier_existe(absolute)) return 0;
    return (INT_PTR)ShellExecuteA(NULL, "open", absolute, NULL, NULL, SW_SHOWNORMAL) > 32;
}
