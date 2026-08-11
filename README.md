# Vers REFORGER

Travaux préparatoires à la migration d'OMTK (OFCRA Mission ToolKit) d'Arma 3 vers Arma Reforger.

> **État : en construction active.** Trois modules (`kill_logger`, `score_board`, `warm_up`) sont construits et testés en jeu. Le reste du dépôt est de l'analyse documentaire, pas encore vérifiée en Workbench. Voir [Fiabilité](#fiabilité) avant de vous appuyer sur un détail technique, et [Ce qui reste à faire](#ce-qui-reste-à-faire) pour l'état complet, module par module.

---

## Le préalable : le nombre de joueurs

À trancher avant tout le reste, y compris avant d'écrire une ligne de code.

L'OFCRA réunit couramment **100 à 130 joueurs** par session publique, avec des pics historiques au-delà de 200. Or Arma Reforger **plafonne à 128 joueurs** par configuration serveur, la plupart des scénarios étant calibrés pour 32 à 64. Les retours de la communauté décrivent des serveurs à 128 déjà en difficulté, le moteur répliquant l'ensemble de l'état de jeu — joueurs, véhicules, structures, projectiles.

À cela s'ajoute que l'optimisation courante consistant à désactiver l'IA côté serveur pour libérer du CPU n'est pas utilisable telle quelle : l'OFCRA emploie de l'IA non combattante (civils, otages) sur certaines missions. L'impact reste faible vu les volumes en jeu, mais la marge n'est pas là.

**Aucun portage de module ne résout ce problème.** Tant qu'il n'est pas tranché, le reste de ce dépôt décrit comment migrer OMTK, pas si la migration est jouable pour un format à 100+ joueurs. Les pistes à explorer : mesurer le comportement réel d'un serveur modé à cette échelle, envisager des formats à effectifs réduits, ou attendre les évolutions du moteur.

---

## Le constat de départ

Ce n'est pas un portage, c'est une réécriture. Le SQF n'existe pas sous Enfusion, `@RHSmod` y devient
RHS: Status Quo — un mod distinct, plus jeune et au catalogue bien plus restreint (voir « À définir ») —,
et l'éditeur Eden est remplacé par le World Editor. Ce qui survit, c'est le **découpage fonctionnel**
d'OMTK — pas son code.

Trois particularités de l'OFCRA orientent tout ce qui suit, et ne correspondent pas aux usages
pour lesquels Reforger est documenté : **aucun respawn**, **aucune IA combattante** (seulement
des civils et otages, occasionnellement), et une **mission inédite chaque semaine** plutôt qu'un
mode de jeu persistant. Les récapitulatifs signalent au cas par cas ce que ça rend sans objet.

### OMTK aujourd'hui

![Synoptique OMTK sur Arma 3](docs/img/synoptique-arma3.svg)

Un point d'entrée unique (`init.sqf` / `description.ext`), deux dépendances externes, et onze modules
dont presque tous sont du script maintenu par l'OFCRA.

### Ce que ça deviendrait sur Reforger

![Synoptique OMTK sur Reforger](docs/img/synoptique-reforger.svg)

Le code couleur est le même sur les deux schémas : vert = code maintenu par l'OFCRA, gris = fourni
par le moteur ou un tiers. La comparaison est le résultat principal de cette analyse — seuls **trois
modules sur onze** demandent réellement d'écrire du code.

### Index des modules

| Module OMTK | Nature du travail sur Reforger | Statut | Analyse |
|---|---|---|---|
| `score_board` | Code — scoring moddé, réplication, HUD, écran de fin | **Testé en jeu** | [Récapitulatif](docs/OMTK_ScoreBoard_Reforger_Recap.md) |
| `warm_up` | Code — minuteur autonome, zones par faction, invulnérabilité | **Testé en jeu** (voir réserves) | [Récapitulatif](docs/OMTK_WarmUp_Reforger_Recap.md) |
| `kill_logger` | Code — Instigator + écriture FileIO | **Testé en jeu** | [Récapitulatif](docs/OMTK_KillLogger_Reforger_Recap.md) |
| `infantry_loadouts` | Config — factions et classes par héritage de prefabs | Doc seulement, rien construit | [Récapitulatif](docs/OMTK_Loadouts_Reforger_Recap.md) |
| `dynamic_startup` | Config — Scenario Framework + `SCR_Task` | Doc seulement, rien construit | [Récapitulatif](docs/OMTK_DynamicStartup_Reforger_Recap.md) |
| `vehicles_cargos` | Config — `FillInitialStorages` sur les prefabs véhicules | Doc seulement, rien construit | [Récapitulatif](docs/OMTK_VehiclesCargos_Reforger_Recap.md) |
| `difficulty_check` · `IA_skills` · `ia_manager` | Config — `SCR_AIConfigComponent` sur les prefabs | Doc seulement + tension à clarifier (voir « À définir ») | [Récapitulatif](docs/OMTK_IASkills_Reforger_Recap.md) |
| `radio_settings` | Config — fréquences et clés dans la config de faction | Doc seulement, rien vérifié en jeu | [Récapitulatif](docs/OMTK_Radio_Reforger_Recap.md) |
| `radio_lock` | Déjà natif — chiffrement par faction au spawn | Doc seulement, rien vérifié en jeu | [Récapitulatif](docs/OMTK_Radio_Reforger_Recap.md) |
| `test_mode` | Déjà natif — Debug Areas, exécutables Diag, Remote Console | Doc seulement, procédure à écrire | [Récapitulatif](docs/OMTK_TestMode_Reforger_Recap.md) |
| `ui` (panneau admin) | À déterminer — probablement en grande partie remplacé par Game Master natif | Doc seulement, rien construit | [Récapitulatif](docs/OMTK_UI_AdminPanel_Reforger_Recap.md) |

Les outils compagnons disparaissent : `omtk-groups` devient de l'héritage de prefabs, `OMTK-loadouts`
devient `FillInitialStorages`. Plus rien à maintenir à côté du mod.

**Modules du dépôt OMTK jamais audités**, repérés en listant l'ensemble du dépôt source (voir
récapitulatif `ui`, §7) : `respawn_mode`, `view_distance`, `zeus_admins`, `rambo_warn`,
`uniform_lock`, `tactical_paradrop`, `vehicles_thermalimaging`, `map_exploration`, `3rd-parties`.
Plusieurs recoupent probablement déjà des modules documentés (voir tableau ci-dessus et récap `ui`,
§4.3 et §7) — à examiner avant de considérer l'audit terminé.

---

## Feuille de route

| # | Étape | État |
|---|---|---|
| 1 | Prise en main du Workbench (installation, tutoriels, samples officiels) | **fait** — Workbench installé, `SampleMod_ModdedScript` compilé et testé en jeu |
| 2 | Audit de l'existant et correspondance avec les systèmes natifs | **fait** — ce dépôt (voir aussi modules non audités ci-dessus) |
| 3 | Prototype minimal d'un module simple, testé de bout en bout | **fait** — `kill_logger` (morts, dégâts, positions, véhicules, connexions, journal structuré) |
| 4 | Construction module par module, dans l'ordre des dépendances | **en cours** — 3 des 11 modules construits et testés en jeu (`kill_logger`, `score_board`, `warm_up`) ; `infantry_loadouts` abandonné ; les 7 autres restent à construire — voir [Ce qui reste à faire](#ce-qui-reste-à-faire) |
| 5 | Assemblage dans un `GameMode` et mission-modèle réutilisable | à faire — un GameMode de test fonctionnel existe, à reconstruire proprement sur une base `GameModeSF` héritée, sans les résidus de test (voir récap `warm_up`, §6) |
| 6 | Test en conditions réelles avec des membres de l'OFCRA | à faire |
| 7 | Documentation de chaque composant pour permettre la contribution | à faire |

Le détail de ce qui a été validé en pratique (noms de classes/méthodes confirmés par compilation,
procédure World Editor, historique des corrections) est dans les récapitulatifs
[`kill_logger`](docs/OMTK_KillLogger_Reforger_Recap.md), [`score_board`](docs/OMTK_ScoreBoard_Reforger_Recap.md)
et [`warm_up`](docs/OMTK_WarmUp_Reforger_Recap.md), section par section.

Une remarque sur l'ordre, toujours valable : la **réplication** (`RplProp` / `BumpMe` / `RplRpc`)
n'a aucun équivalent en SQF et conditionne la façon d'écrire chaque composant multijoueur — c'est
d'ailleurs ce qui explique une partie des détours rencontrés en construisant `score_board` et
`warm_up` (abonnements dupliqués à un événement statique, événements marqués obsolètes dans le
moteur, pièges de configuration d'entités — voir les récapitulatifs correspondants).

**Une règle apprise à la dure, valable pour tout le reste du projet** : un monde Reforger ne
tolère **qu'un seul** `SCR_BaseGameMode` actif — c'est documenté comme obligatoire côté moteur,
pas une simple bonne pratique. En avoir deux, même avec l'un désactivé via sa case "Disabled",
casse le démarrage de la partie de façon peu lisible (écran noir, doublons de tous les
gestionnaires de faction/loadout/radio). Seule la suppression pure et simple du GameMode en
trop résout le problème.

---

## Ce qui reste à faire

### Modules construits et testés — réserves restantes

**`warm_up`** — zones de confinement, téléportation sans mort, invulnérabilité et minuteur global
confirmés en jeu. Restent ouverts :
- vote des officiers pour écourter le warm-up (mécanisme non retrouvé côté Reforger)
- gel des véhicules / retrait de carburant pendant le warm-up (non abordé)
- assignation de faction pré-déterminée par identité de joueur (aucune piste testée — les camps
  OFCRA sont décidés hors jeu, contrairement aux écrans de sélection Reforger disponibles)
- extension de l'invulnérabilité/confinement aux IA jouables si elles sont encore utilisées (voir `ia_manager` ci-dessous)
- nettoyage des brouillons obsolètes dans `docs/drafts/` (approche `CanAdvanceState` abandonnée au profit du minuteur autonome)

**`score_board`** — score par joueur/faction et déclenchement par tâche du Scenario Framework
confirmés en jeu. Rien de bloquant identifié à ce jour au-delà des points listés dans son récapitulatif.

**`kill_logger`** — chaîne complète testée en jeu. Rien de bloquant identifié.

### Modules jamais construits (doc théorique seulement)

- **`dynamic_startup`** — construire une première Area/Layer/Slot minimale, tester la randomisation
  (Debug Areas vs Core Areas), résoudre la réservation de créneaux par escouade membre
- **`infantry_loadouts`** — construire un personnage de base OFCRA complet, tester une config de
  faction custom, évaluer la couverture réelle des assets RHS: Status Quo par rapport aux besoins
- **`IA_skills` / `difficulty_check` / `ia_manager`** — construire et tester `SCR_AIConfigComponent`
  sur un personnage OFCRA ; **clarifier avec l'OFCRA la tension `ia_manager`** (voir « À définir »)
- **`radio_lock` / `radio_settings`** — vérifier en jeu le chiffrement par faction, la capture de
  radio ennemie, et si le respawn sur manpack doit être désactivé (l'OFCRA ne respawn pas)
- **`vehicles_cargos`** — construire un véhicule OFCRA avec `FillInitialStorages` custom, tester
  la persistance "Save In Loadout"
- **`test_mode`** — pas un module à coder : écrire la procédure de test standard (Debug Areas,
  exécutables Diag, Remote Console) une fois éprouvée en conditions réelles
- **`ui` (panneau admin)** — vérifier ce que couvre déjà Game Master nativement avant d'écrire un
  seul widget custom ; trouver le composant de gestion des rôles/permissions admin ; trancher le
  sort de "Fix uniform Bug" (probablement obsolète) et du raccourci distance d'affichage

### Modules jamais audités

`respawn_mode`, `view_distance`, `zeus_admins`, `rambo_warn`, `uniform_lock`, `tactical_paradrop`,
`vehicles_thermalimaging`, `map_exploration`, `3rd-parties` — à confronter au dépôt source OMTK
avant de considérer l'audit terminé.

### Nouvelles strates de statistiques (pas dans OMTK d'origine)

Souhaitées par l'OFCRA, non implémentées : escouade membre, chef de camp, auteur de mission, rôle joueur.
S'y ajoutent des problèmes de données identifiés sur le corpus existant (champ Victory confondant
égalité réelle et absence de saisie, champ auteur en texte libre incohérent, armes véhicule sans `UIInfo`).

---

## Fiabilité

Ces documents ont été rédigés à partir du wiki Bohemia, du Dev Hub Arma Reforger, du code source
du jeu et de mods communautaires. Trois d'entre eux ont depuis été confirmés par compilation et
test en jeu réel dans le Workbench (version 1.7.0.54) : `kill_logger`, `score_board`, `warm_up` —
signalé au cas par cas dans leurs récapitulatifs par la mention « confirmé en pratique ». Tout le
reste n'a encore jamais tourné en Workbench. Les conséquences :

- les noms d'API (`SCR_*`, méthodes, attributs) peuvent avoir changé : certaines pages consultées
  datent des versions 1.1 ou 1.6, le jeu est en 1.7.x ;
- chaque récapitulatif se termine par une section « Ce qui reste à valider en pratique » ; c'est
  la partie la plus importante de chaque fichier tant qu'il n'est pas passé par le Workbench ;
- le récapitulatif radio est le plus fragile : il s'appuie surtout sur des guides communautaires,
  faute de documentation officielle sur le sujet.

Corriger ces documents contre le comportement réel du Workbench est l'objet de l'étape 4 — et
c'est ce qui s'est déjà produit plusieurs fois pour `kill_logger`, `score_board` et `warm_up` :
des noms d'API supposés se sont révélés faux à la compilation, corrigés ensuite dans les
récapitulatifs correspondants. `warm_up` en particulier a nécessité plusieurs itérations en
conditions de jeu réelles avant d'aboutir à une version stable — voir son récapitulatif pour
le détail des impasses rencontrées et pourquoi elles ont été abandonnées.

---

## Ressources

| Sujet | Lien |
|---|---|
| Wiki modding (catégorie racine) | https://community.bistudio.com/wiki/Category:Arma_Reforger/Modding |
| Dev Hub — Boot Camps et notes de version | https://reforger.armaplatform.com/dev-hub |
| Samples officiels Bohemia | https://github.com/BohemiaInteractive/Arma-Reforger-Samples |
| Premiers pas en Enforce Script | https://community.bistudio.com/wiki/Arma_Reforger:Scripting_First_Steps |
| OMTK actuel (Arma 3) | https://github.com/ofcrav2/omtk |
| Arma Reforger Explorer | https://arexplorer.zeroy.com/index.html |

---

## À définir

- **Dépendre de RHS ou non.** C'est la décision structurante du projet, à trancher tôt avec l'OFCRA.
  RHS: Status Quo existe sur Reforger et est activement développé, mais son catalogue reste loin de
  celui d'Arma 3, et son EULA impose des contraintes fortes à tout mod qui en dépend — publication
  publique obligatoire, licence non-dérivative, aucune monétisation. Détail complet dans le
  récapitulatif `infantry_loadouts`, §9.
- **Licence.** Non fixée. Si le projet dépend de RHS, la réponse est contrainte : une licence
  non-dérivative de type APL-ND, ce qui exclut toute licence permissive. Sinon, à aligner sur celle
  d'OMTK, et à vérifier contre l'Arma Public License si du code des samples Bohemia est réutilisé.
- **`.gitignore` Workbench.** Les fichiers de cache générés par l'outil ne sont pas encore connus —
  à compléter après usage prolongé du Workbench.
- **Assignation de faction pré-déterminée.** Le vrai usage OFCRA (camps décidés en amont, hors jeu)
  n'a pas encore d'équivalent testé côté Reforger — voir récap `warm_up`, §4.3 et §6.
- **IA combattante d'appoint (`ia_manager`).** Le module source `ia_manager` documente un usage
  d'IA combattante pour équilibrer les effectifs quand les camps sont déséquilibrés — ce qui
  contredit en partie le principe « aucune IA combattante » posé en tête de ce document (voir
  récapitulatif `IA_skills`, §9bis). À clarifier avec l'OFCRA : usage encore pratiqué aujourd'hui,
  ou abandonné au profit du seul usage non combattant (civils, otages) ?
