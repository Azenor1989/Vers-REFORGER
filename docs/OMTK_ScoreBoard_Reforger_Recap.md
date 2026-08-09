# Récapitulatif — Portage du module `score_board` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `score_board` d'OMTK (Arma 3, SQF) gère aujourd'hui :
- la définition des objectifs par camp (`OMTK_SB_LIST_OBJECTIFS`),
- le calcul du score en temps réel,
- l'affichage du scoreboard en fin de mission (drapeaux BLUEFOR/REDFOR personnalisables).

Sur Arma Reforger, il n'existe pas de "module" équivalent à copier : la logique s'appuie sur des systèmes natifs du moteur Enfusion (scoring, tâches, réplication, UI, écran de fin), **modifiés** plutôt que réécrits de zéro.

**État d'avancement** : les sections 2 et 3 sont **confirmées en pratique** — code compilé et testé en jeu dans le Workbench (version 1.7.0.54). Les sections 4 et 5 (HUD, écran de fin) restent des hypothèses non testées.

---

## 2. Score par joueur (natif) et score par faction (notre ajout) — CONFIRMÉ EN PRATIQUE

### 2.1 Le natif : score par joueur

Reforger a déjà un système de score, mais il compte **par joueur**, pas par camp. La bonne classe à modder, confirmée par recherche de symbole dans le Script Editor puis compilation réussie, est :

```cpp
modded class SCR_BaseScoringSystemComponent
{
    override void AddKill(int playerId, int count = 1)
    {
        super.AddKill(playerId, count);
        // ...
    }
}
```

Deux corrections par rapport à nos hypothèses de départ :
- La classe à modder est **`SCR_BaseScoringSystemComponent`**, pas `SCR_ScoringSystemComponent`. Cette dernière existe aussi, mais elle **hérite** de la première et appelle sa version d'`AddKill` en interne (`SCR_ScoringSystemComponent.AddKill(instigator.GetInstigatorPlayerID())`, vu dans le vrai code source du jeu).
- La signature réelle est `AddKill(int playerId, int count = 1)` — deux paramètres, le second avec valeur par défaut. Notre première tentative avec un troisième paramètre `enemyId` ne compilait pas ("Overloading event ... is not allowed") : Enforce Script refuse une surcharge dont la signature ne correspond pas exactement à la méthode d'origine, il exige une vraie redéfinition.

### 2.2 Notre ajout : score par faction

OFCRA note un **camp** (BLUEFOR/REDFOR/GREENFOR), pas un joueur individuel. On ajoute donc, dans la même classe moddée, un suivi indépendant :

```cpp
protected ref map<string, int> m_mOMTK_FactionScores = new map<string, int>();

void AddFactionPoints(string factionKey, int points, string objectiveId = "")
{
    int current = 0;
    if (m_mOMTK_FactionScores.Contains(factionKey))
        current = m_mOMTK_FactionScores.Get(factionKey);

    int newScore = current + points;
    m_mOMTK_FactionScores.Set(factionKey, newScore);
    // + journalisation structurée (voir récap kill_logger)
}

int GetFactionScore(string factionKey) { /* ... */ }
```

**Testé en jeu** : deux kills successifs produisent `USSR = 1` puis `USSR = 2`, le score persiste correctement entre les événements. La faction du tueur est lue via `FactionAffiliationComponent.GetAffiliatedFaction().GetFactionKey()` sur son entité contrôlée — **confirmé**, ce getter existe et fonctionne tel quel.

---

## 3. Déclenchement par un vrai objectif (Scenario Framework) — CONFIRMÉ EN PRATIQUE

C'est la partie qui a demandé le plus de corrections successives. Ce qui suit est la version finale, validée par un objectif réel posé dans le World Editor et complété en jeu.

### 3.1 La vraie classe : `SCR_Task`, pas `SCR_TaskSystem` ni `SCR_BaseTask`

Le fichier source complet `SCR_Task.c` a été obtenu et lu intégralement. Faits confirmés :

- La classe est **`SCR_Task`** (`GenericEntity`). Ni `SCR_TaskSystem` ni `SCR_BaseTask` ne sont les bonnes classes à cibler pour ce hook — ce sont des noms qu'on a essayés en premier et qui ne correspondaient pas (voir §3.3, historique des erreurs).
- L'état de succès est **`SCR_ETaskState.COMPLETED`**, pas `FINISHED` :
  ```cpp
  enum SCR_ETaskState { CREATED, ASSIGNED, PROGRESSED, COMPLETED, FAILED, CANCELLED }
  ```
- Une tâche a un **tableau** de factions propriétaires, pas une seule :
  ```cpp
  array<string> GetOwnerFactionKeys()  // ex. ["BLUEFOR", "REDFOR"] pour un objectif contesté
  ```
  Ça correspond exactement au format `"BLUEFOR+REDFOR"` d'`OMTK_SB_LIST_OBJECTIFS` — nativement supporté, sans bricolage.
- `GetTaskID()` renvoie une **`string`**, pas un entier.
- Le point d'abonnement propre est un `ScriptInvoker` **statique** exposé par un getter :
  ```cpp
  static SCR_TaskStateInvoker GetOnTaskStateChanged();
  // signature du callback : void MaMethode(SCR_Task task, SCR_ETaskState newState)
  ```

**Règle du système de tâches** (documentation officielle) : on ne lit les données d'une tâche que via les getters publics de `SCR_Task` ; toute écriture doit passer par `SCR_TaskSystem`, jamais en manipulant `SCR_Task`/`SCR_TaskData` directement. Notre code ne fait que lire (`GetOwnerFactionKeys`, `GetTaskID`) — conforme à cette règle.

### 3.2 Le code final

```cpp
modded class SCR_BaseGameModeComponent
{
    // Le ScriptInvoker SCR_Task.s_OnTaskStateChanged est STATIQUE — partagé par
    // toute la classe. Sans protection, chaque rechargement de partie dans le
    // Workbench ajoute un nouvel abonnement sans retirer les précédents.
    protected static bool s_bOMTK_TaskListenerRegistered = false;

    override void OnGameModeStart()
    {
        super.OnGameModeStart();
        if (!Replication.IsServer())
            return;

        if (s_bOMTK_TaskListenerRegistered)
            return; // déjà abonné, ne pas dupliquer

        SCR_Task.GetOnTaskStateChanged().Insert(OMTK_OnTaskStateChanged);
        s_bOMTK_TaskListenerRegistered = true;
    }

    protected void OMTK_OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
    {
        if (!task || newState != SCR_ETaskState.COMPLETED)
            return;

        array<string> ownerFactions = task.GetOwnerFactionKeys();
        if (!ownerFactions || ownerFactions.IsEmpty())
            return;

        string taskId = task.GetTaskID();
        SCR_BaseScoringSystemComponent scoring = SCR_BaseScoringSystemComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_BaseScoringSystemComponent));
        if (!scoring)
            return;

        foreach (string factionKey : ownerFactions)
            scoring.AddFactionPoints(factionKey, 1, taskId); // valeur de test — à remplacer par le vrai barème
    }
}
```

### 3.3 Historique des corrections (pour éviter de refaire les mêmes détours)

Trois versions ont échoué avant celle-ci — utile à savoir si on retombe sur un cas similaire ailleurs :

1. **`modded class SCR_Task` avec `override OnTaskStateChanged(SCR_TaskState state)`** — erreur `"Overloading event ... is not allowed"`. `SCR_Task` n'a pas cette méthode sous ce nom/cette forme.
2. **`SCR_BaseTaskManager.s_OnTaskFinished`** — trouvé dans un fichier source, mais ce fichier s'est révélé **entièrement commenté** dans la vraie version du jeu (chaque ligne préfixée `//`), donc classe inexistante à la compilation. Ne pas se fier à un fichier source sans vérifier qu'il n'est pas mort/remplacé.
3. **`modded class SCR_GameModeSFManager` avec `override OnTaskUpdate(SCR_BaseTask task, ...)`** — basé sur un usage réel (`SCR_GameModeSFManager.c`, non commenté), mais `SCR_BaseTask` n'existe pas non plus comme classe indépendante. Non testé jusqu'au bout car la version 4 (ci-dessus), obtenue en lisant le fichier `SCR_Task.c` complet, s'est avérée à la fois plus simple et correcte du premier coup.

**Leçon générale** : les résultats de recherche web/doc peuvent provenir de versions différentes du jeu ou de code désactivé. Le fichier source complet, lu en entier, reste la seule source vraiment fiable — mieux qu'un extrait ou qu'un nom deviné par analogie.

### 3.4 Mise en place côté World Editor — CONFIRMÉ EN PRATIQUE

Procédure suivie avec succès, à partir du tutoriel officiel *Scenario Framework Setup Tutorial* :

1. **Préalable, une fois par monde** : `Plugins > Game Mode Setup`, pointer *Template* vers `ScenarioFramework.conf`, scanner le monde, puis **"Create entities"** pour générer ce qui manque (`SCR_AIWorld`, gestionnaires de faction/loadout, et un GameMode nommé `GameModeSF` avec `SCR_GameModeSFManager` et une trentaine d'autres composants).
2. Créer une **`Area`**, contenant un **`Layer`** (marqué actif), contenant un **`LayerTaskKill`** (ou `LayerTaskMove`/`LayerTaskDestroy`/`LayerTaskDefend` selon le type d'objectif — voir récap `dynamic_startup`).
3. Dans `LayerTaskKill`, créer un **`SlotTask`** (composant `SCR_ScenarioFrameworkSlotTask`) — **un seul prefab de Slot générique**, pas une variante par type comme on le supposait au départ. C'est dans ses attributs *Asset* qu'on règle :
   - **`Object To Spawn`** — le prefab de la cible (ex. un personnage IA),
   - **`Faction Key`** — la faction propriétaire de la tâche.
4. **Depuis la 1.6, aucun branchement manuel n'est nécessaire** : poser le prefab dans le monde suffit, le système de tâches l'enregistre automatiquement.
5. Le Scenario Framework habille lui-même l'objectif généré (nom, description) sans code supplémentaire — observé en jeu : *"Eliminate target — Neutralize the High Value Target... Target: Machine-Gunner Assistant"*.

**Piège rencontré et résolu** : l'assistant *Game Mode Setup* a créé un **second** GameMode (`GameModeSF`) séparé de celui qui portait déjà nos composants OMTK (`GameMode_Editor_Full1`). Un seul GameMode est actif à la fois — il a fallu regrouper tous les composants (natifs et OMTK) sur `GameModeSF` puis supprimer l'ancien. `SCR_BaseScoringSystemComponent` s'est révélé **déjà présent nativement** sur `GameModeSF` (composant standard du GameMode) : notre `modded class` s'y applique automatiquement, sans rien ajouter à la main pour lui — seul `OMTK_KillLoggerComponent` (composant entièrement nouveau, pas une modification) a dû être ajouté explicitement.

### 3.4bis Divergence de conception à assumer — lu dans le vrai `library.sqf` d'OMTK

Le vrai code source (`github.com/ofcrav2/omtk/blob/master/omtk/score_board/library.sqf`, jamais lu en entier avant ce croisement) révèle qu'**OMTK ne calcule pas le score en temps réel**. `omtk_sb_compute_scoreboard` s'exécute **une seule fois, à la fin de la mission**, et évalue à cet instant précis l'état de chaque objectif de la liste (`sb_o`) — zones actuellement occupées, survivants actuels, etc.

Ce qu'on a construit côté Reforger (§3.2) est un **choix différent, pas un portage fidèle** : `AddFactionPoints` s'exécute immédiatement à chaque `SCR_ETaskState.COMPLETED`, donc le score existe en continu pendant la partie plutôt qu'en un seul instantané final.

Les deux se défendent :
- **Notre approche (temps réel)** permet un score affiché en direct (HUD, §4) et correspond mieux au type d'événement natif Reforger (`GetOnTaskStateChanged`).
- **L'approche d'origine (batch final)** garantit une évaluation cohérente de tous les objectifs au même instant — pas de risque qu'un objectif "gagné tôt" reste comptabilisé alors que la situation a tourné entre-temps pour un objectif de type `INSIDE`/`OUTSIDE` non verrouillé.

**Nuance importante** : OMTK ne fait pas *tout* en temps réel côté score final, mais il **verrouille bien certains objectifs en temps réel** — les types temporisés (`T_INSIDE`, `T_OUTSIDE`, `T_SURVIVAL`, `T_DESTRUCTION`) enregistrent leur résultat dans un tableau de drapeaux (`sb_f`) au moment précis de leur échéance, via `omtk_setFlagResult`. C'est exactement le mécanisme de **verrouillage horodaté** qu'on documentait comme manquant côté Reforger (§9) — sauf que dans OMTK, ce verrouillage alimente un drapeau consulté plus tard au calcul final, il ne pousse pas directement de points.

**À trancher avant d'aller plus loin** : reproduire fidèlement le modèle "instantané final + drapeaux verrouillés", ou assumer le modèle "temps réel" déjà construit et fonctionnel. Les deux sont légitimes ; ce n'est plus une inconnue technique mais un choix de conception à faire consciemment.

### 3.4ter Le type `ACTION` n'évalue rien — il lit une valeur déjà posée ailleurs

Contrairement aux autres types (`INSIDE`, `SURVIVAL`...), `ACTION` ne calcule aucune condition au moment de l'évaluation — il lit simplement une valeur déjà écrite par `omtk_setObjectiveResult`, appelée depuis `omtk_closeAction`. Ce dernier est déclenché par une **action interactible en jeu**, avec une **barre de progression affichée** (`dialog_action_progress.hpp`, un dialogue dédié jamais vu avant ce croisement) : le joueur interagit, la barre se remplit sur une durée configurée, et c'est ce geste qui fixe le résultat — pas une condition de zone ou de survie.

Sur Reforger, ça correspondrait à une action déclenchable sur une entité (voir le mécanisme d'action déjà évoqué pour les officiers dans le récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md), §4.3), avec une barre de progression native si l'UI le permet — à vérifier.

### 3.4quater `DIFF` confirmé par le code

Le mode `DIFF` (déjà supposé, jamais vérifié) est confirmé tel quel : pour remporter une zone contestée, la faction doit avoir plus d'unités dans la zone que **les deux** autres camps à la fois, pas seulement l'un d'eux. Le code le vérifie littéralement par deux comparaisons combinées (`ET` logique), une par camp adverse.

### 3.4quinquies Un second système d'identifiant, distinct des joueurs

Le mode `OMTK_ID` (parfois nommé `MT_ID` dans le code — un des deux noms est probablement un résidu, à clarifier si on retrouve la bonne variable) sert à cibler des **entités précises** liées à un objectif — typiquement un VIP à protéger ou éliminer — via une variable `mt_id` posée sur l'unité elle-même. C'est un mécanisme **différent** de l'`OMTK_ID` du joueur documenté dans le récap [`kill_logger`](OMTK_KillLogger_Reforger_Recap.md) (qui contournait l'écrasement du nom de variable par le pseudo) : celui-ci identifie un objectif-cible, pas un joueur. Les deux portent malheureusement le même nom dans le code d'origine — à ne pas confondre en portant la logique.

### 3.5 Test de bout en bout réussi

Séquence observée dans la console, après correction du bug d'abonnement dupliqué (§3.2) :

```
[OMTK] Abonnement à SCR_Task.GetOnTaskStateChanged() effectué.
[OMTK] Abonnement à s_OnTaskStateChanged déjà actif, ignoré.   (répété aux rechargements suivants)
...
[OMTK] TEST tâche complétée — faction=USSR id=LayerTaskKill1
[OMTK] SCORE FACTION — USSR = 2 (+1, objectif=LayerTaskKill1)
```

Une seule ligne de complétion pour un seul kill réel de la cible ciblée par le Slot — confirmé après le correctif, avant lequel un seul événement déclenchait le callback jusqu'à 25 fois en boucle (un abonnement dupliqué à chaque rechargement du Workbench, le `ScriptInvoker` étant statique donc partagé, jamais réinitialisé).

---

## 4. Affichage en direct (HUD) — NON TESTÉ, hypothèse de départ

- L'UI Reforger est faite de **Widgets** organisés en hiérarchies dans des fichiers `.layout`.
- Un **Script Component** (`ScriptedWidgetComponent`) attaché au widget du scoreboard lirait la valeur de `GetFactionScore()` et mettrait à jour l'affichage via `Widget.FindAnyWidget()`.
- **Bonne pratique** : un seul composant "source de vérité" à la racine du prefab HUD.
- **HUD Slotting** : le système redistribue automatiquement les éléments HUD compatibles vers les menus (carte, inventaire) ou les cache.

Rien de cette section n'a encore été compilé ni testé — à traiter comme la suite logique une fois le score par faction jugé fiable.

---

## 5. Écran de fin de mission — NON TESTÉ, hypothèse de départ

- Le **End Screen** se configure dans l'éditeur ; pour du full custom, hériter de `SCR_BaseGameOverScreenInfo` et surcharger `SCR_GameOverScreenContentUIComponent`.
- **`SCR_GameModeEndData`** : structure sérialisable déjà répliquée nativement (faction gagnante, joueur gagnant...).
- Point d'attention non résolu : compatibilité **Game Master**, où plusieurs factions gagnantes sont possibles.
- Les popups intermédiaires pourraient utiliser le système générique de **Configurable Dialog** (`SCR_ConfigurableDialogUi`).

---

## 5bis. Correction : le vainqueur est bien transmis à la chaîne de statistiques

Le récapitulatif [`kill_logger`](OMTK_KillLogger_Reforger_Recap.md), §6.5, affirmait que l'agrégat manquant le plus important côté statistiques était le vainqueur de mission, absent de la base. **C'est à corriger** : le vrai code source montre que `omtk_sb_compute_scoreboard` envoie explicitement `[_winner, score_ouest, score_est]` à `statslogger_fnc_mission_end`, puis déclenche `statslogger_fnc_export`, dès lors que le plugin `STATSLOGGER` est présent.

**La donnée part donc bien de la source.** Si le site `aar.ofcra.org` n'affiche pas de vainqueur agrégé, le problème est en aval — stockage en base, ou affichage — pas dans l'instrumentation du jeu. Ça change la nature du travail : il ne s'agit pas d'ajouter un événement manquant côté `kill_logger`, mais de vérifier ce que devient cette donnée une fois reçue côté serveur de statistiques. Cette correction devra être répercutée dans le récapitulatif `kill_logger`.

---

## 6. Chaîne complète (mise à jour)

```
Pendant la partie :
  Kill détecté (kill_logger, OnPlayerKilled)
      → scoring.AddKill(killerId)        [natif, par joueur]
      → scoring.AddFactionPoints(...)    [notre ajout, par faction]   ← CONFIRMÉ

  Tâche du Scenario Framework complétée (SCR_ETaskState.COMPLETED)
      → OMTK_OnTaskStateChanged(task, state)
      → scoring.AddFactionPoints(factionKey, points, taskId) pour chaque faction propriétaire  ← CONFIRMÉ

  (à faire) → widget HUD mis à jour via GetFactionScore()

Fin de partie (hypothèse, non testée) :
  Score final → SCR_GameModeEndData (répliqué nativement)
      → SCR_GameOverScreenContentUIComponent (notre logique custom)
      → notre layout final (drapeaux, mise en page OFCRA)
```

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Réplication (concepts) | `community.bistudio.com/wiki/Arma_Reforger:Multiplayer_Scripting` |
| Modding par override/super | `community.bistudio.com/wiki/Arma_Reforger:Scripting_Modding` |
| Sample de code réel | `github.com/BohemiaInteractive/Arma-Reforger-Samples` -> `SampleMod_ModdedScript` |
| Code source réel du module `score_board` (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/score_board` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` -- utile pour vérifier un nom de classe/méthode avant de coder, mais certains fichiers indexés sont commentés/désactivés dans la vraie version : vérifier par compilation |
| Scenario Framework (concepts) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework` |
| Scenario Framework (tutoriel pas à pas) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework_Setup_Tutorial` |
| Task System Usage (règles lecture/écriture) | `community.bistudio.com/wiki/Arma_Reforger:Task_System_Usage` |
| Changements du système de tâches en 1.6 | `reforger.armaplatform.com/news/modding-update-october-7-2025` |
| UI / HUD (concepts) | Modding Boot Camp #4 -- `reforger.armaplatform.com/news/modding-boot-camp-4-user-interface-and-hud` |
| Dialogues génériques | `community.bistudio.com/wiki/Arma_Reforger:Dialog_Configuration_Tutorial` |
| Écran de fin de mission | `community.bistudio.com/wiki/Arma_Reforger:End_Screen_Creation` |
| Setup général de game mode | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |

---

## 8. Ce qui reste à faire

**Confirmé et stable** : score par joueur (natif), score par faction (notre ajout), déclenchement par un objectif réel du Scenario Framework, protection contre les abonnements dupliqués.

**Reste à faire, dans un ordre suggéré** :
- Remplacer la valeur de points fixe (`1`) par le vrai barème OFCRA (`OMTK_SB_LIST_OBJECTIFS` : valeurs différenciées par objectif, communes vs par camp).
- Retirer le déclenchement de test « chaque kill donne 1 point » dans `AddKill` (utile pour valider la tuyauterie, pas la vraie règle).
- Implémenter le **verrouillage horodaté par objectif** (voir §9) -- aucun équivalent natif trouvé à ce jour.
- Câbler l'affichage HUD (§4) -- non testé.
- Câbler l'écran de fin de mission (§5) -- non testé, y compris la compatibilité Game Master multi-factions gagnantes.
- Vérifier le comportement de `RplCondition` si un jour la réplication custom devient nécessaire au-delà de ce que `AddKill`/le score natif gèrent déjà.

---

## 9. Contexte OFCRA : structure réelle des objectifs

Le barème d'une mission OFCRA est plus riche que le simple couple objectif/point. Sur une mission type de 90 minutes, on trouve :

**Trois familles d'objectifs**
- **Communs aux deux camps** -- typiquement le contrôle de zones (une zone d'éoliennes, une usine, une ville), chacune valant 2 à 3 points.
- **Propres au BLUFOR** -- défendre des installations, empêcher une action adverse.
- **Propres au REDFOR** -- le miroir : détruire ces mêmes installations, réussir l'action.

**Des points différenciés** : chaque objectif porte sa propre valeur, y compris à l'intérieur d'un lot (« 3 installations à défendre, 1 point chacune »).

**Des verrouillages échelonnés** : les objectifs ne se ferment pas tous à la fin. Sur une mission de 90 minutes, certains se verrouillent à 60 minutes, d'autres à 75, les derniers seulement à l'expiration du temps. Une fois verrouillé, l'état d'un objectif est figé quoi qu'il arrive ensuite sur le terrain.

**La survie comme objectif** : « le chef de camp a survécu » vaut des points, pour chaque camp.

**Ce que ça implique pour l'implémentation, mis à jour avec les vrais noms confirmés (§3) :**

- La notion de **faction(s) propriétaire(s)** d'une tâche (`SCR_Task.GetOwnerFactionKeys()`, tableau de chaînes) couvre nativement la distinction commun / BLUFOR / REDFOR -- y compris le cas d'un objectif appartenant aux deux camps à la fois.
- Le **verrouillage horodaté par objectif** n'a toujours pas d'équivalent natif identifié : c'est la part la plus spécifique à écrire, sous forme d'un état « figé » atteint à un instant configuré par objectif (probablement en comparant le temps de mission -- voir récap `kill_logger` -- à un seuil défini par tâche), et non à la fin de partie.
- Le score final doit distinguer **objectif rempli** (`SCR_ETaskState.COMPLETED`) et **objectif verrouillé sur un échec** (`FAILED` ou verrouillage manuel), deux états terminaux différents qu'il faudra traiter séparément dans `OMTK_OnTaskStateChanged`.

---

*Document mis à jour après tests réels en Workbench (version 1.7.0.54) -- §2 et §3 confirmés en pratique, §4 et §5 restent des hypothèses non testées (voir §8).*
