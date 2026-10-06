# Tri benchmark

Application console Windows en C : selection, bulles, insertion, rapide a pivot
aleatoire, fusion et tas. L'experience specifique du rapide conserve le pivot
milieu sur des partitions equilibrees et le pivot aleatoire sur les entrees
aleatoires et toutes egales.

## Compiler et lancer

Dans PowerShell, depuis ce dossier :

```powershell
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -finput-charset=UTF-8 -fexec-charset=UTF-8 main.c tri.c graphique.c -o tri_benchmark.exe -lshell32
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
2. Test au choix : aleatoire, croissante, decroissante, presque triee (environ
   1 % d'echanges), dix valeurs repetees ou l'experience specifique du rapide.
3. Parametres : repetitions, limite lente, taille maximale des tests complets.
4. Complexites theoriques.
5. Afficher les courbes : ouvrir les graphiques du dernier run complet,
   sans refaire les mesures (voir les choix ci-dessous).
6. Histogramme : choisir une configuration et une taille disponibles dans le
   dernier run complet. Barres groupees du minimum et de la moyenne en secondes;
   les trois cas et strategies de pivot sont compares pour l'experience rapide.
   Une vue rapide/fusion/tas est ajoutee si une barre lente depasse cinq fois
   la plus grande moyenne des algorithmes rapides. Le PNG principal est ouvert
   avec la visionneuse Windows; les chemins restent affiches en cas d'echec.
7. Quitter (la fin de l'entree ferme aussi le menu proprement).

Dans **5. Afficher les courbes** :

- **1. Un algorithme** : choisir le tri, puis une configuration disponible ou
  toutes les configurations pour ouvrir son graphique existant.
- **2. Tous les algorithmes** : choisir une configuration et comparer les six
  tris ensemble sur un seul graphique.
- **3. Comparer deux algorithmes** : choisir une configuration et deux tris
  distincts.
- **4. Comparer plusieurs algorithmes** : choisir une configuration et 3 a 6
  tris distincts.
- **5. Experience du tri rapide** : partitions equilibrees (pivot milieu),
  entree aleatoire et toutes valeurs egales (pivot aleatoire).
- **6. Regenerer les graphiques** : refaire les courbes, histogrammes et
  comparaisons personnalisees deja crees, sans relancer les tris.
- **0. Retour** : revenir au menu principal.

Les comparaisons sur une configuration proposent toutes les tailles disponibles
ou seulement celles jusqu'a 5000. L'axe vertical reste le **temps minimum en
secondes**; les exclusions restent des points absents. Les couleurs des
algorithmes sont conservees dans chaque comparaison.
Les PNG existants s'ouvrent avec Windows, meme sans Gnuplot. Une image manquante
ou une nouvelle comparaison est generee depuis les donnees sauvegardees.
Les chemins sont affiches si la generation ou la visionneuse echoue.
La logique de l'application est en C; Gnuplot reste le moteur de rendu.

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
Les graines d'entree (xorshift32) et de pivot (etat xorshift32 separe, version
`xorshift32-rejet-v1`) sont conservees. Le tirage avec rejet couvre toute
la partition sans biais modulo. Les entrees et operations sont reproductibles
avec cette version,
mais les temps dependent du materiel et de la charge du systeme.

Les minimums et les moyennes arithmetiques utilisent toutes les repetitions.
Le minimum des temps n'est pas le meilleur cas theorique : il reste une mesure
sur la configuration choisie. Le cas moyen theorique n'est pas non plus la
moyenne de vingt temps sur des entrees triees ou identiques.
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
- `informations.txt` : parametres, tailles, conventions, version du generateur
  de pivots, version GCC, optimisation, identifiant CPU, processeurs logiques
  et frequence du chronometre.
- `<configuration>.csv` : comparaisons des six algorithmes.
- `benchmark/res.dat` : comparaison globale sur l'entree aleatoire.
- `cases/*_cases.dat` : comparaison des configurations par algorithme et
  experience specifique du rapide.
- `graphiques/<run-id>/` : scripts `.plt` et PNG dans `algorithm_benchmark`,
  `cases` et `configurations`; vues completes, rapides et tailles jusqu'a 5000.
- `graphiques/<run-id>/histogrammes/` : donnees `.dat`, scripts `.plt` et PNG
  demandes via le menu 6. Les mesures exclues sont affichees comme non mesurees.
- `graphiques/<run-id>/comparaisons/` : scripts `.plt` et PNG des selections
  personnalisees, nommes selon configuration, algorithmes choisis et vue.
  Les scripts relisent les fichiers de mesures existants, sans les modifier.

Les CSV utilisent `;` et un point decimal. Chaque run a un dossier distinct,
meme lorsque plusieurs commencent dans la meme seconde.
`resultats/dernier_run.txt` reference le dernier run complet. Les anciens dossiers
`results` et `plots` sont archives sans perte dans `archives/version_initiale/`.
Les resultats dates anterieurs restent en place. La comparaison globale
aleatoire est la figure de reference; seule sa copie complete redondante n'est
plus produite (les vues aleatoires rapides et petites tailles restent).

Gnuplot est cherche via `GNUPLOT_EXE`, PATH puis les installations Windows usuelles.
Exemple de chemin personnalise pour la session PowerShell :

```powershell
$env:GNUPLOT_EXE = 'C:\Program Files\gnuplot\bin\gnuplot.exe'
```

Si Gnuplot est absent, les donnees et scripts restent disponibles. Utiliser le
menu 5 puis **6. Regenerer les graphiques** apres installation/configuration,
ou lancer les scripts depuis ce dossier :

```powershell
gnuplot "graphiques/<run-id>/algorithm_benchmark/algorithm_benchmark.plt"
```

## Structure

- `main.c` : menu, saisie, reglages et orchestration des experiences.
- `tri.c` / `tri.h` : generation, tris, compteur representatif et chronometrage.
- `graphique.c` / `graphique.h` : sauvegardes, courbes, histogrammes et visionneuse.

Reference : Sedgewick et Wayne, *Algorithms*, 4e edition,
[tableau des comparaisons](https://algs4.cs.princeton.edu/cheatsheet/) et
[hypotheses du tri rapide](https://algs4.cs.princeton.edu/23quicksort/).
Fusion est Theta(n log n) ici; tas a une borne O(n log n) mais devient Theta(n)
sur une entree toute egale avec l'arret de descente de cette implementation.
Le rapide conserve sa partition en deux groupes : tous les elements egaux
produisent exactement n(n-1)/2 comparaisons, contrairement au traitement
des egalites du rapide du livre.
