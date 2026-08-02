# Récapitulatif — Portage du module `warm_up` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `warm_up` d'OMTK (Arma 3, SQF) gère la période d'attente avant le démarrage effectif d'une mission (attente de joueurs, décompte). Sur Reforger, ce rôle correspond directement à un état natif de la machine à états de partie du GameMode.

---

## 2. La machine à états native : `SCR_EGameModeState`

Reforger structure chaque partie en trois grandes phases, gérées par le GameMode :

```
PREGAME  (ex. attendre n joueurs)  →  GAME  →  POSTGAME (scoreboard, vote du prochain scénario)
```

- Ce cycle est piloté par **`SCR_EGameModeState`** et la méthode **`OnGameStateChanged()`**.
- L'état **PREGAME** est explicitement pensé pour des cas comme "attendre n joueurs" avant de démarrer — la fonction même de notre `warm_up` actuel.
- La transition vers **POSTGAME** se fait via **`EndGameMode()`**, qui prend en paramètre une instance de **`SCR_GameModeEndData`** — la même structure déjà documentée dans le récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md). `warm_up` et `score_board` s'appuient donc sur la même mécanique d'état de jeu, à ses deux extrémités (début et fin de partie).

---

## 3. Options serveur complémentaires

- **`-autoreload <secondes>`** — relance automatiquement le scénario à la fin d'une session sans couper le serveur ; pratique pour enchaîner des sessions OFCRA sans intervention manuelle.
- **`operating.lobbyPlayerSync`** (config serveur) — synchronise l'état des joueurs en lobby ; pertinent si notre warm-up doit afficher un décompte ou une liste de joueurs connectés en temps réel.

---

## 4. Ce que ça implique pour notre portage

`warm_up` se construirait comme une **implémentation de l'état PREGAME** sur notre GameMode custom (probablement via `modded`, même logique que pour `score_board`, plutôt que codé entièrement de zéro) :
1. Définir une condition d'attente (ex. nombre minimum de joueurs connectés).
2. Afficher un décompte ou un statut via HUD (déjà documenté dans le récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md), §4).
3. Déclencher la transition vers l'état GAME une fois la condition remplie.

Pas besoin de réinventer un système de minuterie manuel comme en SQF — l'état PREGAME et sa transition font partie du cycle de vie natif du GameMode.

---

## 5. Ressources de référence

| Sujet | Lien |
|---|---|
| Setup général de game mode (états PREGAME/GAME/POSTGAME) | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |
| Paramètres de démarrage serveur (`-autoreload`, etc.) | `community.bistudio.com/wiki/Arma_Reforger:Startup_Parameters` |
| Configuration serveur (`lobbyPlayerSync`) | `low.ms/knowledgebase/arma-reforger-server-configuration` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` — utile pour vérifier un nom de classe/méthode avant de coder |

---

## 6. Ce qui reste à valider en pratique (Workbench requis)

- Construire un premier GameMode custom avec une condition PREGAME simple (ex. 2 joueurs minimum) et vérifier la transition vers GAME.
- Tester l'affichage d'un décompte HUD pendant la phase PREGAME.
- Vérifier le comportement de `-autoreload` sur un serveur de test pour l'enchaînement automatique de sessions.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §6).*
