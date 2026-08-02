# Récapitulatif — Portage du module `dynamic_startup` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `dynamic_startup` d'OMTK (Arma 3, SQF) gère les marqueurs de carte et les points de spawn générés dynamiquement au lancement d'une mission. Sur Reforger, ce rôle n'est pas rempli par un module custom, mais par un système natif complet dédié : le **Scenario Framework**, couplé au **système de tâches** (classe `SCR_Task` — voir correction ci-dessous).

---

## 2. Le Scenario Framework — hiérarchie Area → Layer → Slot

Le système repose sur trois niveaux imbriqués, posés directement comme prefabs dans le monde (World Editor) :

- **Area** — une zone du monde regroupant un ensemble d'éléments à faire apparaître.
- **Layer** — une couche à l'intérieur d'une Area. Peut être un simple conteneur, ou une **LayerTask** qui gère la création et le cycle de vie d'une tâche/objectif.
- **Slot** — l'élément terminal :
  - un **Slot "basique"** fait apparaître un prefab précis (ex. un point de spawn, `SpawnPoint_US.et`) — c'est sa fonction principale, mais on peut aussi n'y attacher que des composants sans rien spawner (ex. un composant d'action pour l'IA) ;
  - un **SlotTask** travaille en synchronisation avec sa LayerTask parente pour représenter concrètement un objectif (ex. `SlotMove`, `SlotDestroy`, `SlotKill` pour des tâches spécialisées Move/Destroy/Kill).

Des **entités logiques** (`LogicCounter`, `LogicOR`, `LogicSwitch`) permettent de chaîner des conditions entre éléments — ex. déclencher une suite d'événements quand un certain nombre d'objectifs sont remplis.

---

## 3. Randomisation native — un vrai gain par rapport à SQF

Une Area peut définir **plusieurs types de tâches possibles**, dont seul un sous-ensemble sera sélectionné aléatoirement au lancement du scénario (exemple officiel : CombatOps Arland définit 4 types de tâches, 3 sont tirées au sort). C'est un comportement qu'on codait manuellement en SQF ; ici il est natif à la structure du Framework.

**Options utiles pour le développement/debug** :
- **Debug Areas** — forcer certaines zones (et éventuellement leur LayerTask) à apparaître systématiquement, plutôt que de dépendre de la randomisation, pour tester un cas précis.
- **Core Areas** — zones essentielles qui doivent toujours apparaître, indépendamment du tirage aléatoire des Debug Areas.
- **Dynamic Despawn** — désactivé par défaut ; une fois activé, permet de faire disparaître dynamiquement des éléments (utile pour la gestion de performance sur de longues sessions).

---

## 4. Le système de tâches (`SCR_Task`) — équivalent direct des objectifs OMTK

> **Correction** : ce document employait initialement `SCR_TaskSystem` comme nom de classe. Confirmé par compilation et test en jeu (voir récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md), §3), la vraie classe qui porte les propriétés et méthodes décrites ci-dessous est **`SCR_Task`**. `SCR_TaskSystem` existe bien côté moteur, mais son rôle est différent : c'est lui qui gère l'écriture/l'enregistrement des tâches (`RegisterTask`, `UnregisterTask`), jamais à manipuler directement — toute lecture passe par les getters de `SCR_Task` (`GetOwnerFactionKeys()`, `GetTaskID()`, `GetTaskState()`...).

- Fonctionne en tandem avec le Scenario Framework, mais peut aussi être utilisé indépendamment.
- **S'enregistre automatiquement** : poser un prefab de tâche dans le monde suffit, pas d'initialisation manuelle à écrire.
- Chaque tâche définit :
  - un **Task ID** (identifiant unique),
  - des **infos UI** (nom, description, icône, jeu d'icônes),
  - un **état initial** (Task State),
  - un **niveau de propriété** (Task Ownership),
  - un **niveau de visibilité** (Task Visibility) et un **niveau de visibilité UI** (liste de tâches, carte, ou les deux),
  - des **clés de faction propriétaires** (Owner Faction Keys) et des **IDs de groupes propriétaires** (Owner Group IDs).

C'est cette dernière propriété qui correspond directement à notre logique BLUEFOR/REDFOR par objectif dans `OMTK_SB_LIST_OBJECTIFS` — chaque tâche peut être limitée à une faction ou un groupe précis, nativement.

---

## 5. Le mode Game Master — équivalent le plus proche d'un "éditeur en jeu"

Il n'y a pas d'éditeur façon Eden pour poser une mission à l'avance de la même manière. À la place, un **mode en jeu** (accessible depuis le menu pause) bascule sur une vue tactique du dessus, avec :
- placement de troupes et de véhicules à la volée ou via des présets sauvegardés,
- définition des **factions jouables**,
- création de **points de spawn par faction**,
- un menu d'**objectifs** adressés aux joueurs (l'IA les ignore — à ne pas confondre avec les **waypoints**, qui sont l'équivalent pour les groupes IA).

Le Workbench reste disponible en parallèle pour une construction plus poussée et persistante (via le Scenario Framework), plutôt qu'une session Game Master live.

---

## 6. Ce que ça implique pour notre portage

`dynamic_startup` ne serait plus un composant de script autonome générant des marqueurs par code, mais une **combinaison de prefabs Scenario Framework** (Area/Layer/Slot) posés dans le World Editor, associés à des tâches (`SCR_Task`) enregistrées via `SCR_TaskSystem`. Le lien avec les autres modules déjà documentés :
- les **Slots de spawn** utilisent les mêmes prefabs `SpawnPoint_*.et` que ceux vus dans le récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md) ;
- la **faction propriétaire** d'une tâche s'appuie sur les mêmes clés de faction que `SCR_FactionManager` ;
- le **résultat des tâches** (complétées/échouées) alimenterait le score répliqué via `[RplProp]`, comme documenté dans le récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md) ;
- un point de spawn peut aussi être **dynamique et porté par un joueur** — une radio manpack vivante sert de point de respawn d'équipe en mode Conflict (voir récap [`radio_lock`/`radio_settings`](OMTK_Radio_Reforger_Recap.md), §4). **Sans objet pour l'OFCRA**, qui joue sans respawn (voir §9) ; noté ici pour mémoire si un autre usage se présentait.

Encore une fois : moins de script à écrire, plus de structure déclarative (prefabs + config) à organiser correctement.

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Scenario Framework (concepts) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework` |
| Tutoriel de mise en place pas à pas | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework_Setup_Tutorial` |
| Système de tâches en détail | `community.bistudio.com/wiki/Arma_Reforger:Task_System_Usage` |
| Mode Game Master | `community.bistudio.com/wiki/Arma_Reforger:Game_Master` |
| Changements majeurs 1.1.0 (IA/Waypoints) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework_Update_Plugin` |
| Wiki d'exemples (Workshop) | `reforger.armaplatform.com/workshop/64F6CE0D8811D211-ScenarioFrameworkWiki/scenarios` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` — utile pour vérifier un nom de classe/méthode avant de coder ; voir la correction `SCR_Task` vs `SCR_TaskSystem` ci-dessus, découverte grâce à lui |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Construire une première Area/Layer/Slot minimale avec un unique point de spawn, avant d'ajouter la logique de tâche.
- Tester la randomisation (Debug Areas vs Core Areas) sur un scénario à plusieurs objectifs possibles.
- ~~Vérifier comment le résultat d'une tâche se propage concrètement vers le score~~ — **fait**, voir récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md), §3 : abonnement à `SCR_Task.GetOnTaskStateChanged()`, testé en jeu.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*

---

## 9. Contexte OFCRA : pas de respawn, insertion initiale uniquement

L'OFCRA joue **sans aucun respawn** : un joueur tué reste hors jeu jusqu'à la fin de la mission. Tout ce qui touche au respawn dans Reforger — `SCR_RespawnSystemComponent` en cours de partie, respawn sur radio manpack, minuteurs de vague — est donc **hors périmètre**.

Ce qui compte est l'**insertion initiale**, et elle est plus structurée qu'un point de départ unique. Sur une mission type, chaque escouade démarre à un emplacement qui lui est propre : hors de l'île principale pour le commandement, les hélicoptères et plusieurs escouades ; sur l'île au nord ou au sud pour les autres. Des groupes de véhicules sont rattachés à chacun de ces emplacements.

**Ce que ça implique concrètement :**

- Un `Slot` du Scenario Framework par emplacement de départ, avec le prefab de point d'insertion et les véhicules associés.
- Pas de logique de réapparition à écrire, ce qui **simplifie nettement** le portage par rapport à ce que laissait supposer la documentation générale de Reforger, très orientée Conflict.
- En revanche, une contrainte que Conflict ne connaît pas : les créneaux sont **réservés par escouade membre** (indicatifs entre crochets dans les listes de slots). Le mécanisme de sélection de rôle doit pouvoir refléter ces réservations.
