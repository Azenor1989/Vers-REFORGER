# Vers REFORGER

Travaux préparatoires à la migration d'**OMTK** (OFCRA Mission ToolKit) d'Arma 3 vers **Arma Reforger**.

> **État : exploration documentaire.** Aucune ligne de code n'a encore été écrite, et rien n'a été
> testé dans l'Enfusion Workbench. Ce dépôt contient pour l'instant l'analyse préalable :
> qu'est-ce qui existe déjà nativement dans Reforger, qu'est-ce qu'il faudra construire,
> et quelle forme ça prendra. Voir [Fiabilité](#fiabilité) avant de vous appuyer sur un détail technique.

---

## Le préalable : le nombre de joueurs

**À trancher avant tout le reste, y compris avant d'écrire une ligne de code.**

L'OFCRA réunit couramment **100 à 130 joueurs** par session publique, avec des pics historiques
au-delà de 200. Or Arma Reforger **plafonne à 128 joueurs** par configuration serveur, la plupart
des scénarios étant calibrés pour 32 à 64. Les retours de la communauté décrivent des serveurs
à 128 déjà en difficulté, le moteur répliquant l'ensemble de l'état de jeu — joueurs, véhicules,
structures, projectiles.

À cela s'ajoute que l'optimisation courante consistant à désactiver l'IA côté serveur pour libérer
du CPU n'est pas utilisable telle quelle : l'OFCRA emploie de l'IA non combattante (civils, otages)
sur certaines missions. L'impact reste faible vu les volumes en jeu, mais la marge n'est pas là.

**Aucun portage de module ne résout ce problème.** Tant qu'il n'est pas tranché, le reste de ce dépôt
décrit comment migrer OMTK, pas si la migration est jouable pour un format à 100+ joueurs.
Les pistes à explorer : mesurer le comportement réel d'un serveur modé à cette échelle, envisager
des formats à effectifs réduits, ou attendre les évolutions du moteur.

---

## Le constat de départ

Ce n'est pas un portage, c'est une réécriture. Le SQF n'existe pas sous Enfusion, `@RHSmod` y devient
**RHS: Status Quo** — un mod distinct, plus jeune et au catalogue bien plus restreint (voir « À définir ») —,
et l'éditeur Eden est remplacé par le World Editor. Ce qui survit, c'est le **découpage fonctionnel**
d'OMTK — pas son code.

Trois particularités de l'OFCRA orientent tout ce qui suit, et ne correspondent pas aux usages
pour lesquels Reforger est documenté : **aucun respawn**, **aucune IA combattante** (seulement
des civils et otages, occasionnellement), et une **mission inédite chaque semaine** plutôt qu'un
mode de jeu persistant. Les récapitulatifs signalent au cas par cas ce que ça rend sans objet.

### OMTK aujourd'hui

![Synoptique d'OMTK sous Arma 3](docs/img/synoptique-arma3.svg)

Un point d'entrée unique (`init.sqf` / `description.ext`), deux dépendances externes, et onze modules
dont presque tous sont du script maintenu par l'OFCRA.

### Ce que ça deviendrait sur Reforger

![Synoptique révisé pour Arma Reforger](docs/img/synoptique-reforger.svg)

Le code couleur est le même sur les deux schémas : **vert = code maintenu par l'OFCRA**,
**gris = fourni par le moteur ou un tiers**. La comparaison est le résultat principal de cette analyse —
seuls trois modules sur onze demandent réellement d'écrire du code.

---

## Index des modules

| Module OMTK | Nature du travail sur Reforger | Analyse |
|---|---|---|
| `score_board` | Code — scoring moddé, réplication, HUD, écran de fin | [Récapitulatif](docs/OMTK_ScoreBoard_Reforger_Recap.md) |
| `warm_up` | Code — état `PREGAME` du GameMode | [Récapitulatif](docs/OMTK_WarmUp_Reforger_Recap.md) |
| `kill_logger` | Code — `Instigator` + écriture `FileIO` | [Récapitulatif](docs/OMTK_KillLogger_Reforger_Recap.md) |
| `infantry_loadouts` | Config — factions et classes par héritage de prefabs | [Récapitulatif](docs/OMTK_Loadouts_Reforger_Recap.md) |
| `dynamic_startup` | Config — Scenario Framework + `SCR_TaskSystem` | [Récapitulatif](docs/OMTK_DynamicStartup_Reforger_Recap.md) |
| `vehicles_cargos` | Config — `FillInitialStorages` sur les prefabs véhicules | [Récapitulatif](docs/OMTK_VehiclesCargos_Reforger_Recap.md) |
| `difficulty_check` · `IA_skills` | Config — `SCR_AIConfigComponent` sur les prefabs | [Récapitulatif](docs/OMTK_IASkills_Reforger_Recap.md) |
| `radio_settings` | Config — fréquences et clés dans la config de faction | [Récapitulatif](docs/OMTK_Radio_Reforger_Recap.md) |
| `radio_lock` | **Déjà natif** — chiffrement par faction au spawn | [Récapitulatif](docs/OMTK_Radio_Reforger_Recap.md) |
| `test_mode` | **Déjà natif** — Debug Areas, exécutables Diag, Remote Console | [Récapitulatif](docs/OMTK_TestMode_Reforger_Recap.md) |

Les outils compagnons disparaissent : `omtk-groups` devient de l'héritage de prefabs,
`OMTK-loadouts` devient `FillInitialStorages`. Plus rien à maintenir à côté du mod.

---

## Feuille de route

| # | Étape | État |
|---|---|---|
| 1 | Prise en main du Workbench (installation, tutoriels, samples officiels) | **fait** — Workbench installé, `SampleMod_ModdedScript` compilé et testé en jeu |
| 2 | Audit de l'existant et correspondance avec les systèmes natifs | **fait** — ce dépôt |
| 3 | Prototype minimal d'un module simple, testé de bout en bout | **fait** — `kill_logger` (morts, dégâts, positions, véhicules, connexions, journal structuré) |
| 4 | Construction module par module, dans l'ordre des dépendances | **en cours** — `score_board` construit et validé en jeu (score par faction + déclenchement par un vrai objectif du Scenario Framework) ; `infantry_loadouts` abandonné, l'OFCRA ne l'utilise plus ; reste `warm_up` |
| 5 | Assemblage dans un `GameMode` et mission-modèle réutilisable | à faire |
| 6 | Test en conditions réelles avec des membres de l'OFCRA | à faire |
| 7 | Documentation de chaque composant pour permettre la contribution | à faire |

Le détail de ce qui a été validé en pratique (noms de classes/méthodes confirmés par compilation,
procédure World Editor, historique des corrections) est dans les récapitulatifs
[`kill_logger`](docs/OMTK_KillLogger_Reforger_Recap.md) et
[`score_board`](docs/OMTK_ScoreBoard_Reforger_Recap.md), section par section.

Une remarque sur l'ordre, toujours valable : la **réplication** (`RplProp` / `BumpMe` / `RplRpc`)
n'a aucun équivalent en SQF et conditionne la façon d'écrire chaque composant multijoueur — c'est
d'ailleurs ce qui explique une partie des détours rencontrés en construisant `score_board`
(abonnements dupliqués à un événement statique, voir le récapitulatif correspondant).

---

## Fiabilité

Ces documents ont été rédigés à partir du wiki Bohemia, du Dev Hub Arma Reforger, du code source
du jeu et de mods communautaires — **sans jamais rien compiler ni exécuter**. Les conséquences :

- les noms d'API (`SCR_*`, méthodes, attributs) peuvent avoir changé : certaines pages consultées
  datent des versions 1.1 ou 1.6, le jeu est en 1.7.x ;
- chaque récapitulatif se termine par une section **« Ce qui reste à valider en pratique »** ;
  c'est la partie la plus importante de chaque fichier ;
- le récapitulatif radio est le plus fragile : il s'appuie surtout sur des guides communautaires,
  faute de documentation officielle sur le sujet.

Corriger ces documents contre le comportement réel du Workbench est l'objet de l'étape 1.

---

## Ressources

| Sujet | Lien |
|---|---|
| Wiki modding (catégorie racine) | <https://community.bistudio.com/wiki/Category:Arma_Reforger/Modding> |
| Dev Hub — Boot Camps et notes de version | <https://reforger.armaplatform.com/dev-hub> |
| Samples officiels Bohemia | <https://github.com/BohemiaInteractive/Arma-Reforger-Samples> |
| Premiers pas en Enforce Script | <https://community.bistudio.com/wiki/Arma_Reforger:Scripting_First_Steps> |
| OMTK actuel (Arma 3) | <https://github.com/OFCRA/OMTK> |

---

## À définir

- **Dépendre de RHS ou non.** C'est la décision structurante du projet, à trancher tôt avec
  l'OFCRA. **RHS: Status Quo** existe sur Reforger et est activement développé, mais son
  catalogue reste loin de celui d'Arma 3, et son EULA impose des contraintes fortes à tout mod
  qui en dépend — publication publique obligatoire, licence non-dérivative, aucune monétisation.
  Détail complet dans le [récapitulatif `infantry_loadouts`](docs/OMTK_Loadouts_Reforger_Recap.md), §9.
- **Licence.** Non fixée. Si le projet dépend de RHS, la réponse est contrainte : une licence
  non-dérivative de type APL-ND, ce qui exclut toute licence permissive. Sinon, à aligner sur
  celle d'OMTK, et à vérifier contre l'Arma Public License si du code des samples Bohemia
  est réutilisé.
- **`.gitignore` Workbench.** Les fichiers de cache générés par l'outil ne sont pas encore
  connus — à compléter après l'étape 1.
