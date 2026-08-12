# Récapitulatif — Portage du module `ui` / panneau admin (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

**Module découvert tardivement** : absent de l'audit initial des neuf premiers récapitulatifs. Le dépôt OMTK contient un dossier `ui/` complet, jamais examiné jusqu'ici — ainsi que plusieurs autres modules encore non audités (voir §7).

---

## 1. Contexte

`omtk/ui/pauseScreenMenu.sqf` (25 Ko) ajoute un panneau de contrôle à l'écran pause du jeu. Ce n'est **pas** l'affichage joueur envisagé dans le récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md) §4 (HUD en direct pendant la partie) — c'est un **outil d'administration en jeu**, distinct et complémentaire.

Deux populations d'utilisateurs, avec des droits différents :

- **Tout joueur** : deux outils en libre-service, créés *avant* toute vérification de droits — « Fix uniform Bug » (contournement d'un bug Arma 3 de réinitialisation d'équipement, probablement sans objet sur Reforger — voir §6) et un raccourci de distance d'affichage à trois niveaux.
- **Administrateurs seulement** : tout le reste du panneau, derrière une double condition — `admin_uids` (liste blanche d'identifiants Steam) **ou** `serverCommandAvailable "#kick"` (droits serveur natifs).

---

## 2. Fonctions par joueur ciblé

Deux listes déroulantes — **WHO** (qui) et **WHERE** (destination) — groupées par camp (BLUEFOR/OPFOR/INDEPENDENT/civil), avec un champ de filtre texte qui rafiltre en direct (boucle `while` à 0,2 s).

| Bouton | Fonction appelée | Rôle |
|---|---|---|
| Teleport player | `omtk_teleport_unit` | Téléporte le joueur sélectionné (WHO) vers un autre joueur (WHERE) |
| Warn lonewolf | `omtk_warn_unit` | Avertit un joueur isolé de son escouade |
| Toggle Safety | `omtk_toggle_safety_unit` | Bascule un état de sécurité (invulnérabilité ?) par joueur |
| Respawn | `omtk_respawn_unit` | Force la réapparition d'un joueur |
| Full heal | `omtk_heal_unit` | Soigne intégralement |
| Reset Inventory | `omtk_reset_unit` | Réinitialise l'équipement |

Toutes ces actions passent par `remoteExec`, exécutées côté serveur à partir de l'écran admin d'un client.

---

## 3. Contrôle collectif (« OMTK Buttons »)

| Bouton | Fonction appelée | Lien avec d'autres modules déjà documentés |
|---|---|---|
| End Warm-up | `omtk_wu_fn_launch_game` (portée `2`, tous les clients) | **PORTÉ ET TESTÉ EN JEU** — voir récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §3.6. Pas un vote d'officiers : un déclencheur admin unique (`RpcAsk_AdminEndWarmup`), le même chemin de code que le minuteur automatique. Bouton dédié construit ci-dessous (§4.5). |
| Show Scoreboard | `omtk_sb_compute_scoreboard` **puis** `omtk_sb_start_mission_end` | Deux fonctions distinctes, appelées dans cet ordre — le calcul du score et le déclenchement de fin de mission sont **deux étapes séparées** dans OMTK d'origine. À vérifier si notre implémentation `score_board` (récap §3) doit refléter cette séparation. |
| Export Ocap/Stats | `statslogger_fnc_export` + événement `ocap_exportData` (CBA) | Confirme la porte d'entrée réelle de la chaîne AAR documentée dans [`kill_logger`](OMTK_KillLogger_Reforger_Recap.md) — c'est cet appel qui déclenche l'export final |
| Remove AIs | `omtk_delete_playableAiunits` | Touche le module `ia_manager`, non encore audité (voir §7) |
| Freeze AIs | `omtk_disable_aibehaviour` | Idem |
| Enable Dmg ALL | `omtk_enable_playerdamage` | — |
| Enable/Disable Safety ALL | `omtk_enable_safety` / `omtk_disable_safety` | — |
| Enable/Disable Sim ALL | `omtk_sim_disableplayerSim` + `omtk_sim_disablevehiclesim` / leurs inverses | Recoupe l'invulnérabilité et simulation désactivée du warm-up (récap `warm_up` §1), mais exposé ici comme **override manuel admin** plutôt qu'automatique |
| Show Player Count | `omtk_show_player_count` | — |
| Show Time Left | `omtk_show_time_left` | Affichage du décompte warm-up à la demande |
| export list | `omtk\table_forum.sqf` | Export au format forum — un troisième format de sortie, en plus d'OCAP et des statistiques web, jamais mentionné avant |

### Découverte notable : le contrôle par faction a été abandonné même côté OMTK

Le code contient un bloc entier **commenté** (`buttonSimulation_EnableBlue/Red/Green`) pour activer la simulation faction par faction, remplacé par un contrôle global unique (`Enable/Disable Sim ALL`). C'est une confirmation, depuis le code source lui-même, de la même limite de camps figés qu'on a identifiée en migrant vers Reforger : même les développeurs d'origine d'OMTK ont renoncé au contrôle par couleur de camp. Sur Reforger, où les factions sont des identifiants libres (voir récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md) §2), un contrôle par faction redeviendrait trivial à réintroduire si souhaité — mais rien n'indique que ce soit un besoin réel de l'OFCRA, puisque même l'ancien système y avait renoncé.

---

## 4. Ce que ça implique pour le portage

### 4.1 Couverture Game Master — en grande partie clarifiée

Comparatif établi à partir de la doc officielle Bohemia (*Arma Reforger:Game Master*) :

**Couvert nativement** (menu radial sur une entité, ou toolbar) :
- **Teleport player** → action native "Teleport Player"
- **Full heal** → action native "Heal"
- **Remove AIs** (mortes) → "Clear Destroyed Entities" — ne retire que les entités déjà détruites, pas des IA vivantes qu'on voudrait retirer volontairement (différent de `omtk_delete_playableAiunits`)

**Pas d'équivalent natif trouvé** :
- Toggle Safety / Enable Dmg ALL / Enable-Disable Sim ALL par joueur ou par camp — il existe un bouton toolbar "pause de la simulation" (ajouté en 1.2), mais c'est un pause global du jeu, pas une bascule ciblée façon OMTK
- Freeze AIs (spécifiquement, sans tout mettre en pause)
- Reset Inventory, Warn lonewolf, Respawn forcé, Show Player Count, Show Time Left, Export OCAP/stats, export forum

**Conclusion** : le trio téléportation/soin/nettoyage des morts peut s'appuyer sur Game Master tel quel — pas besoin de les reconstruire. Tout le reste (bascules collectives, exports, compteurs, respawn forcé) reste à construire, comme prévu à l'origine. Un point de détail reste non vérifié par la doc seule (portée exacte de la table "Possible/Not Possible" de Game Master, contenu non extrait) — à confirmer en jeu si un doute survient sur un cas précis, sans que ça bloque la suite.

### 4.2 Ce qui reste probablement nécessaire malgré tout

- Le déclenchement admin de fin de warm-up et de calcul de score — spécifique à OMTK, sans équivalent Game Master. **Fin de warm-up : fait (§4.5).**
- L'export vers la chaîne AAR (OCAP/statistiques) — action métier propre à l'OFCRA.
- Les bascules collectives (dégâts/sécurité/simulation pour tous) — à vérifier si Game Master les couvre déjà ou non.

### 4.3 Système de permissions — RÉSOLU

**`SCR_PlayerListedAdminManagerComponent`** (doc officielle Script API), attaché au GameMode. Lit la liste `admins` du `config.json` du serveur — l'équivalent direct d'`admin_uids`. Confirmé par compilation et testé en jeu de bout en bout (voir récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §3.6) : le rejet "pas admin" fonctionne ; la confirmation "admin reconnu" reste à tester sur un serveur réel (liste vide en Workbench solo).

```cpp
SCR_PlayerListedAdminManagerComponent adminMgr = SCR_PlayerListedAdminManagerComponent.GetInstance();
bool isAdmin = adminMgr.IsPlayerOnAdminList(playerId);
```

Utilisé dans `OMTK_AdminEndWarmupAction.c` (`CanBeShownScript`, affichage restreint aux admins) et dans `RpcAsk_AdminEndWarmup` côté serveur (double vérification, défense en profondeur).

### 4.4 Deux outils en libre-service, un statut différent

- **« Fix uniform Bug »** répare un bug de réinitialisation d'objets propre à l'inventaire Arma 3. Le système d'inventaire d'Enfusion étant entièrement différent, ce bug précis n'a probablement pas de raison d'exister sur Reforger — **à ne pas porter tel quel**, mais à garder en tête si un bug équivalent apparaît en pratique.
- **Le raccourci de distance d'affichage** accessible à tout joueur (pas seulement aux admins) est une fonctionnalité de confort à part entière, indépendante du reste du panneau admin.

### 4.5 Premier bouton construit — End Warm-up, CONFIRMÉ COMPILÉ

`OMTK_AdminEndWarmupAction.c` : une `ScriptedUserAction` dont `CanBeShownScript()` vérifie `IsPlayerOnAdminList()` — le bouton n'apparaît que pour un vrai admin, contrairement au bouton de test (`OMTK_TEST_AdminEndWarmupAction.c`, toujours visible) utilisé pour valider `RpcAsk_AdminEndWarmup` lui-même. `PerformAction()` appelle ce même RPC, déjà testé de bout en bout.

**Câblage** : même procédure que pour le bouton de test (`ActionsManagerComponent` + `Action Context` sur une entité physique — voir récap `warm_up` §3.6). **Limite de test connue** : ce bouton ne s'affichera jamais en Workbench solo, la liste d'admins n'y étant jamais chargée — seule la compilation est vérifiable avant un test serveur réel.

---

## 5. Interface — éléments techniques à noter

- Construit avec des contrôles `RscButton`/`ctrlEdit`/`ctrllistbox` créés dynamiquement par script (`ctrlCreate`), positionnés en coordonnées relatives à la zone sûre (`safeZoneW/H/X/Y`).
- Rafraîchissement de liste en boucle avec `sleep 0.2`, tant que l'affichage existe (`!isNull findDisplay 49`) — un modèle de polling, pas d'événements.
- Sur Reforger, l'équivalent serait un système de Widgets (`.layout`, `ScriptedWidgetComponent`) déjà évoqué dans le récap `score_board` §4 — mais piloté par de vrais événements plutôt que par une boucle de rafraîchissement, si possible. **Le bouton End Warm-up (§4.5) contourne ce sujet pour l'instant** : c'est une action d'interaction physique (`ScriptedUserAction`), pas un widget de panneau — un vrai panneau à la `pauseScreenMenu.sqf` reste à construire pour les boutons restants.

---

## 6. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du module OMTK (Arma 3) | `github.com/ofcrav2/omtk/blob/master/omtk/ui/pauseScreenMenu.sqf` |
| Game Master (mode édition en jeu natif) | `community.bistudio.com/wiki/Arma_Reforger:Game_Master` |
| UI / HUD (concepts, Widgets) | Modding Boot Camp #4 — voir récap `score_board` §7 |
| `SCR_PlayerListedAdminManagerComponent` (doc officielle) | voir récap `warm_up` §5 |
| Câblage d'une `ScriptedUserAction` (`ActionsManagerComponent`) | voir récap `warm_up` §3.6 |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` |

---

## 7. Modules du dépôt OMTK jamais encore audités

Repérés en listant le dépôt à cette occasion — **aucun de ces modules n'a de récapitulatif Reforger à ce jour** :

`respawn_mode`, `view_distance`, `zeus_admins`, `rambo_warn`, `ia_manager`, `radio_lock`, `uniform_lock`, `tactical_paradrop`, `vehicles_thermalimaging`, `difficulty_check`, `map_exploration`, `3rd-parties`.

Plusieurs recoupent déjà ce document : `ia_manager` (référencé par `omtk_delete_playableAiunits`/`omtk_disable_aibehaviour`, §3), `view_distance` (référencé par les boutons de distance d'affichage, §1 et §3), `zeus_admins` (probablement lié au système de permissions, désormais résolu au §4.3). **À auditer avant de considérer la migration complète.**

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- ~~Déterminer précisément ce que Game Master couvre déjà nativement~~ — **fait pour l'essentiel (§4.1)** : téléportation et soin natifs, le reste (bascules collectives, exports, compteurs) reste à construire.
- ~~Trouver le vrai composant de gestion des rôles/permissions admin sur Reforger~~ — **fait (§4.3)**.
- Tester la visibilité réelle du bouton End Warm-up (§4.5) sur un serveur avec une vraie liste d'admins.
- Auditer `ia_manager`, `view_distance` et `zeus_admins` (§7) — probablement des prérequis à ce module plutôt que des modules indépendants.
- Vérifier si `omtk_sb_compute_scoreboard`/`omtk_sb_start_mission_end` doivent rester deux étapes séparées dans notre implémentation de `score_board`.
- Construire un vrai panneau de type `pauseScreenMenu.sqf` (Widgets) pour les boutons restants (téléportation, soin, bascules collectives, export AAR) — le bouton End Warm-up n'est qu'une première brique isolée.

---

*Document mis à jour après la session de test `warm_up` — §3 (End Warm-up), §4.3 (permissions) et §4.5 confirmés/résolus. Le reste du panneau (§2, §3 hors End Warm-up) n'est toujours pas construit.*
