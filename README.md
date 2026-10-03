# Tri benchmark

Application console Windows en C : selection, bulles, insertion, rapide a pivot
aleatoire, fusion et tas. L'experience specifique du rapide conserve le pivot
milieu sur des partitions equilibrees et le pivot aleatoire sur les entrees
aleatoires et toutes egales.

## Compiler et lancer

Dans PowerShell, depuis ce dossier :

```powershell
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8 main.c tri.c graphique.c -o tri_benchmark.exe
.\tri_benchmark.exe
```

Fermer une execution precedente avant de recompiler. GCC doit etre disponible
dans PATH (installation actuelle : `C:\TDM-GCC-64\bin\gcc.exe`).

Si Windows affiche « An Application Control policy has blocked this file »,
la compilation a pu reussir mais Smart App Control/la politique de signature
bloque l'executable non signe. Il faut un executable approuve par cette politique
(par exemple signe avec un certificat reconnu). La compilation et les tests ne
modifient aucun reglage de securite Windows.

## Menu et parametres

1. Tous les tests : cinq configurations plus l'experience specifique du rapide.
2. Une configuration : aleatoire, croissante, decroissante, presque triee (environ
   1 % d'echanges), dix valeurs repetees ou l'experience specifique du rapide.
3. Reglages : repetitions, limite lente, taille maximale des tests complets.
4. Complexites theoriques.
5. Regeneration des graphiques du dernier run valide, sans refaire les mesures.
6. Quitter.

Tailles : 100, 500, 1000, 5000, 10000, 20000, 50000, 100000, 500000, 1000000.
Un test selectionne inclut toutes les tailles jusqu'au maximum choisi.
Defauts : **20 repetitions**, **1 million d'elements**, **limite lente 20000**.
Les repetitions sont modifiables de 1 a 100 et la limite de 1 a 1000000.
Les reglages restent en memoire pour la session et sont enregistres avec chaque run.

La limite s'applique a selection/bulles/insertion et au rapide sur dix valeurs
repetees ou toutes egales : sa partition en deux groupes peut etre quadratique.
Les mesures ignorees sont `non_mesure`, jamais des temps nuls. Le programme
affiche l'algorithme, la configuration, la taille et la repetition avant chaque
mesure; les grands tests peuvent prendre du temps.

## Mesures et compteurs

Chaque repetition cree une nouvelle entree; tous les algorithmes recoivent une
copie du meme tableau. `QueryPerformanceCounter` mesure uniquement le tri en
**secondes**. Generation, copies, allocation du tampon fusion, verification,
affichage, ecriture et comptage sont exclus. Chaque sortie de tri est verifiee.
Les graines d'entree (xorshift32) et de pivot (`srand`/`rand` du compilateur utilise)
sont conservees : les entrees et operations sont reproductibles avec ce build,
mais les temps dependent du materiel et de la charge du systeme.

Les minimums et les moyennes arithmetiques utilisent toutes les repetitions.
Les graphiques conservent les **minimums**, comme l'application initiale.
Les compteurs correspondent a **un passage separe sur la repetition 1**, pas a
une moyenne : comparaisons entre elements ou avec le pivot; mouvements lors des
ecritures de tableaux (tampon fusion inclus), avec trois mouvements par swap,
meme si les deux positions coincident. Les variables locales et les tests
d'indices ne sont pas comptes. Le code des tris est compile deux fois depuis
les memes definitions; la version chronometree ne contient aucun compteur.

## Fichiers conserves

- `resultats/<run-id>/mesures.csv` : temps individuels et graines.
- `resume.csv` : minimums, moyennes et statut par taille/cas/algorithme.
- `operations.csv` : compteurs, repetition representative et graines.
- `informations.txt` : parametres, tailles et conventions de mesure.
- `<configuration>.csv` : comparaisons des six algorithmes.
- `benchmark/res.dat` : comparaison globale sur l'entree aleatoire.
- `cases/*_cases.dat` : comparaison des configurations par algorithme et
  experience specifique du rapide.
- `graphiques/<run-id>/` : scripts `.plt` et PNG dans `algorithm_benchmark`,
  `cases` et `configurations`; vues completes, rapides et tailles jusqu'a 5000.

Les CSV utilisent `;` et un point decimal. Chaque run a un dossier distinct,
meme lorsque plusieurs commencent dans la meme seconde.
`resultats/dernier_run.txt` reference le dernier run complet. Les anciens dossiers
`results` et `plots` sont conserves tels quels et ne sont plus ecrases.

Gnuplot est cherche via `GNUPLOT_EXE`, PATH puis les installations Windows usuelles.
Exemple de chemin personnalise pour la session PowerShell :

```powershell
$env:GNUPLOT_EXE = 'C:\Program Files\gnuplot\bin\gnuplot.exe'
```

Si Gnuplot est absent, les donnees et scripts restent disponibles. Utiliser le
menu 5 apres installation/configuration, ou lancer les scripts depuis ce dossier :

```powershell
gnuplot "graphiques/<run-id>/algorithm_benchmark/algorithm_benchmark.plt"
```
