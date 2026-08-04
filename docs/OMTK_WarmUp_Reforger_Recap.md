# Récapitulatif — Portage du module `warm_up` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `warm_up` d'OMTK (Arma 3, SQF) gère la période d'attente avant le démarrage effectif d'une mission. **Corrigé après lecture du vrai code source** (`github.com/ofcrav2/omtk/tree/master/omtk/warm_up`) : ce n'est **pas** une attente d'un nombre de joueurs, mais un minuteur configuré à l'avance (paramètre de mission `OMTK_MODULE_WARM_UP`, 30 s à 15 min), avec un seul raccourci possible — un vote des officiers des deux camps.

Fonctionnalités réelles du module (source : `README.md` et `main.sqf`/`library.sqf` du dépôt OMTK) :
- décompte affiché en temps réel aux joueurs ;
- joueurs invulnérables et simulation désactivée brièvement au démarrage ;
- véhicules avec moteur bloqué (gel du contact, pas retrait du carburant dans la version actuelle du code, malgré ce que dit le README) ;
- zone de restriction autour du point d'apparition, avec téléportation après 5 s hors zone ;
- **les officiers des deux camps** (classes définies dans `OMTK_WU_CHIEF_CLASSES`) peuvent déclarer leur camp prêt ; une fois **les deux** prêts, le temps restant est ramené à ~10 secondes (`omtk_wu_fn_launch_game`, calcul exact : `0.002777 h × 3600 ≈ 10 s`) ;
- le raccourci ne peut jamais **rallonger** le temps restant — l'appel est ignoré si le minuteur est déjà sous le seuil.

Sur Reforger, ce rôle correspond à l'état natif **`PREGAME`** de la machine à états du GameMode.

---

## 2. La machine à états native : `SCR_EGameModeState`

Reforger structure chaque partie en trois phases, gérées par le GameMode :

```
PREGAME → GAME → POSTGAME (scoreboard, vote du prochain scénario)
```

La transition vers `POSTGAME` se fait via `EndGameMode()`, qui prend en paramètre `SCR_GameModeEndData` — la même structure documentée dans le récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md).

### 2.1 La vraie méthode à surcharger — CONFIRMÉ EN PRATIQUE

Pas `OnGameStateChanged()` comme supposé initialement : le bon point d'accroche pour décider **si** l'état `PREGAME` peut se terminer est **`CanAdvanceState()`**, présente sur `SCR_BaseGameModeStateComponent` et ses trois filles (`SCR_PreGameGameModeStateComponent`, `SCR_GameGameModeStateComponent`, `SCR_PostGameGameModeStateComponent`).

**Signature confirmée par compilation** (source : `arexplorer.zeroy.com`, version 1.7.0.54, vérifiée en tête de page) :

```cpp
bool CanAdvanceState(SCR_EGameModeState nextState)
```

Un paramètre, pas zéro — la première tentative sans paramètre échouait avec `"Overloading event ... is not allowed"`, exactement comme pour `OnPlayerKilled` sur `kill_logger`.

**Testé en jeu** : le composant modifié doit être **explicitement attaché** au GameMode dans le World Editor — comme pour `OMTK_KillLoggerComponent`, `modded class` seul ne suffit pas si le composant natif de base n'est pas déjà posé sur l'entité. Une fois attaché, le hook se déclenche à chaque frame tant que la partie reste en `PREGAME` :

```
[OMTK] TEST CanAdvanceState (PREGAME) appelé — vers=1 résultat natif=false
```

**Point non résolu** : dans le code source lu sur arexplorer, `CanAdvanceState()` de la classe de base renvoie toujours `true` sans condition. Or le test en jeu montre `résultat natif=false` en continu — donc soit le Scenario Framework (`SCR_GameModeSFManager`, déjà présent sur `GameModeSF`) surcharge ce point avec sa propre logique avant nous, soit un autre mécanisme entre en jeu. **À vérifier avant de chaîner un appel à `super.CanAdvanceState()`** dans une implémentation finale — voir §4.3.

### 2.2 Démarrage manuel par un administrateur — hors périmètre de ce hook

La documentation officielle précise que `CanAdvanceState()` *"Does not apply to manual `StartGameMode()` call from the authority!"* — un lancement manuel par l'admin serveur contourne entièrement ce mécanisme. Rien à coder pour ce cas : c'est déjà natif.

---

## 3. Options serveur complémentaires

| Option | Rôle |
|---|---|
| `-autoreload <secondes>` | Relance automatiquement le scénario en fin de session, sans couper le serveur — utile pour enchaîner des sessions OFCRA |
| `operating.lobbyPlayerSync` (config serveur) | Synchronise l'état des joueurs en lobby |

---

## 4. Implémentation — étapes validées et brouillons en cours

### 4.1 Étape 1 — preuve que le hook se déclenche (CONFIRMÉ EN PRATIQUE)

```cpp
modded class SCR_PreGameGameModeStateComponent
{
    override bool CanAdvanceState(SCR_EGameModeState nextState)
    {
        bool result = super.CanAdvanceState(nextState);
        Print("[OMTK] TEST CanAdvanceState (PREGAME) appelé — vers=" + nextState + " résultat natif=" + result, LogLevel.NORMAL);
        return result;
    }
}
```

Compilé et testé en jeu — voir §2.1.

### 4.2 Étape 2 — minuteur fixe (rédigée, pas encore testée en jeu)

Remplace le résultat natif par une vraie comparaison de temps écoulé contre une durée configurable (`[Attribute]`), fidèle au paramètre `OMTK_MODULE_WARM_UP` d'origine — **pas** un nombre de joueurs. Un champ `m_fOMTK_ForcedRemainingSeconds` est prévu, non câblé, pour l'étape 3 (vote des officiers).

### 4.3 Brouillons soumis par Luis — analyse

Deux fichiers ont été rédigés en parallèle de l'étape 2 ci-dessus : `OMTK_WarmUpComponent.c` (minuteur + raccourci) et `OMTK_ReadyAction.c` (action radiale pour le vote). **Ce sont des brouillons, non compilés ni testés à ce stade.** Points relevés à la lecture :

**Ce qui est fidèle à OMTK :**
- Le raccourci à 10 secondes (`+ 10000.0` ms) correspond exactement au calcul réel d'`omtk_wu_fn_launch_game` (`0.002777 h × 3600 ≈ 10 s`).
- La garde qui empêche de rallonger (`if (newEndTime < m_fWarmUpEndTime)`) reproduit bien le comportement d'origine, qui ignore l'appel si le temps restant est déjà sous le seuil.

**Trois points à corriger avant de tester :**

1. **`super.CanAdvanceState(nextState)` en tête de méthode risque d'annuler toute la logique.** Si la classe de base renvoie toujours `true` sans condition (voir §2.1), alors `if (super.CanAdvanceState(nextState)) return true;` ferait sortir la fonction dès le premier appel, rendant le minuteur situé en dessous inatteignable. C'est précisément l'inconnue non résolue du §2.1 — **à tester en isolation** (afficher la valeur de `super.CanAdvanceState()` seule, sans rien faire d'autre) avant de s'y fier dans une version qui compte dessus.

2. **Aucune vérification de `Replication.IsServer()`.** Ni `CanAdvanceState`, ni `ShortenWarmUp`, ni `OMTK_ReadyAction.PerformAction` ne distinguent client et serveur. Le vrai OMTK sépare strictement les deux (`if (isServer) then {...}`, `remoteExec`, `publicVariable`) précisément pour que la fin du warm-up soit décidée une seule fois, par l'autorité, et pas recalculée indépendamment par chaque client. Tel quel, `m_fWarmUpEndTime` est une variable membre non répliquée : rien ne garantit qu'un raccourci déclenché par un joueur atteigne la bonne instance du composant côté serveur.

3. **Le vote des officiers n'est pas fidèle à la règle d'origine, sur deux points :**
   - Aucune vérification de rôle — n'importe quel joueur peut appeler `ShortenWarmUp()`, alors qu'OMTK restreint l'action aux classes définies dans `OMTK_WU_CHIEF_CLASSES`.
   - Un seul camp suffit à raccourcir, alors que la règle réelle exige que **les officiers des deux camps** soient prêts (*"Once both are [ready], the warmup is canceled"*).

   De plus, dans `CanBeShownScript`, `FindComponent(SCR_PreGameGameModeStateComponent)` réussira en permanence puisque ce composant reste attaché au GameMode tout au long de la partie — sa seule présence ne dit rien sur l'état courant. Il faudrait comparer l'état actif du GameMode à `PREGAME`, pas seulement vérifier que le composant existe.

**Prochaine étape suggérée** : lever l'inconnue du point 1 en isolation, puis corriger les points 2 et 3 avant tout test en jeu.

---

## 5. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du module OMTK (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/warm_up` |
| Setup général de game mode (états PREGAME/GAME/POSTGAME) | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |
| Paramètres de démarrage serveur (`-autoreload`, etc.) | `community.bistudio.com/wiki/Arma_Reforger:Startup_Parameters` |
| Configuration serveur (`lobbyPlayerSync`) | `low.ms/knowledgebase/arma-reforger-server-configuration` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` — utile pour vérifier un nom de classe/méthode avant de coder ; a permis de corriger la signature de `CanAdvanceState` |

---

## 6. Ce qui reste à valider en pratique (Workbench requis)

- Lever l'inconnue sur `super.CanAdvanceState()` (§2.1 et §4.3, point 1) — condition bloquante avant toute version finale.
- Corriger et tester les brouillons `OMTK_WarmUpComponent.c` et `OMTK_ReadyAction.c` (réplication serveur, restriction aux officiers, exigence des deux camps).
- Trouver l'équivalent Reforger de l'ajout d'une action à des classes d'unité spécifiques (les officiers), et de la vérification de l'état courant du GameMode (pas seulement la présence d'un composant).
- Tester le comportement de `-autoreload` sur un serveur de test.
- Vérifier si la zone de restriction (téléportation après 5 s hors zone) et le gel des véhicules ont un équivalent natif ou doivent être réécrits.

---

*Document mis à jour après lecture du code source réel d'OMTK et tests partiels en Workbench (version 1.7.0.54) — §2.1 confirmé en pratique, §4.2 rédigée mais non testée, §4.3 en cours de correction (voir §6).*
