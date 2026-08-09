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
| End Warm-up | `omtk_wu_fn_launch_game` (portée `2`, tous les clients) | **Même fonction que le vote des officiers** dans le récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §1 — pas un mécanisme séparé, juste un déclencheur différent (admin plutôt qu'officier). **À retenir pour le portage : un seul chemin de code suffit pour les deux cas.** |
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

### 4.1 Une bonne partie de ce panneau existe peut-être déjà nativement

Reforger dispose d'un mode **Game Master**, un éditeur-en-jeu déjà natif qui couvre une large part de ces fonctions : téléportation, soin, gestion d'unités IA, contrôle de simulation. **Avant d'écrire le moindre widget custom**, il faut vérifier combien de ces boutons deviennent inutiles simplement en donnant l'accès Game Master aux admins OFCRA, plutôt que de reconstruire un panneau parallèle.

### 4.2 Ce qui reste probablement nécessaire malgré tout

- Le déclenchement admin de fin de warm-up et de calcul de score — spécifique à OMTK, sans équivalent Game Master.
- L'export vers la chaîne AAR (OCAP/statistiques) — action métier propre à l'OFCRA.
- Les bascules collectives (dégâts/sécurité/simulation pour tous) — à vérifier si Game Master les couvre déjà ou non.

### 4.3 Système de permissions à identifier

`admin_uids` (liste blanche) et `serverCommandAvailable "#kick"` (droits serveur) sont deux mécanismes SQF qui n'ont pas d'équivalent confirmé sur Reforger à ce stade. À rechercher : composant de gestion des rôles admin en jeu (`SCR_PlayerAdminManagerComponent` ou similaire — nom à vérifier par recherche de symbole ou sur `arexplorer.zeroy.com`, jamais confirmé par compilation).

### 4.4 Deux outils en libre-service, un statut différent

- **« Fix uniform Bug »** répare un bug de réinitialisation d'objets propre à l'inventaire Arma 3. Le système d'inventaire d'Enfusion étant entièrement différent, ce bug précis n'a probablement pas de raison d'exister sur Reforger — **à ne pas porter tel quel**, mais à garder en tête si un bug équivalent apparaît en pratique.
- **Le raccourci de distance d'affichage** accessible à tout joueur (pas seulement aux admins) est une fonctionnalité de confort à part entière, indépendante du reste du panneau admin.

---

## 5. Interface — éléments techniques à noter

- Construit avec des contrôles `RscButton`/`ctrlEdit`/`ctrllistbox` créés dynamiquement par script (`ctrlCreate`), positionnés en coordonnées relatives à la zone sûre (`safeZoneW/H/X/Y`).
- Rafraîchissement de liste en boucle avec `sleep 0.2`, tant que l'affichage existe (`!isNull findDisplay 49`) — un modèle de polling, pas d'événements.
- Sur Reforger, l'équivalent serait un système de Widgets (`.layout`, `ScriptedWidgetComponent`) déjà évoqué dans le récap `score_board` §4 — mais piloté par de vrais événements plutôt que par une boucle de rafraîchissement, si possible.

---

## 6. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du module OMTK (Arma 3) | `github.com/ofcrav2/omtk/blob/master/omtk/ui/pauseScreenMenu.sqf` |
| Game Master (mode édition en jeu natif) | `community.bistudio.com/wiki/Arma_Reforger:Game_Master` |
| UI / HUD (concepts, Widgets) | Modding Boot Camp #4 — voir récap `score_board` §7 |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` |

---

## 7. Modules du dépôt OMTK jamais encore audités

Repérés en listant le dépôt à cette occasion — **aucun de ces modules n'a de récapitulatif Reforger à ce jour** :

`respawn_mode`, `view_distance`, `zeus_admins`, `rambo_warn`, `ia_manager`, `radio_lock`, `uniform_lock`, `tactical_paradrop`, `vehicles_thermalimaging`, `difficulty_check`, `map_exploration`, `3rd-parties`.

Plusieurs recoupent déjà ce document : `ia_manager` (référencé par `omtk_delete_playableAiunits`/`omtk_disable_aibehaviour`, §3), `view_distance` (référencé par les boutons de distance d'affichage, §1 et §3), `zeus_admins` (probablement lié au système de permissions du §4.3). **À auditer avant de considérer la migration complète.**

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Déterminer précisément ce que Game Master couvre déjà nativement, avant d'écrire un seul widget custom (§4.1).
- Trouver le vrai composant de gestion des rôles/permissions admin sur Reforger (§4.3).
- Auditer `ia_manager`, `view_distance` et `zeus_admins` (§7) — probablement des prérequis à ce module plutôt que des modules indépendants.
- Vérifier si `omtk_sb_compute_scoreboard`/`omtk_sb_start_mission_end` doivent rester deux étapes séparées dans notre implémentation de `score_board`.

---

*Document généré à partir de la lecture du code source réel d'OMTK (`github.com/ofcrav2/omtk`) — aucune partie testée en Workbench à ce stade.*
