# Récapitulatif — Portage des modules `difficulty_check` / `IA_skills` / `ia_manager` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

`difficulty_check` et `IA_skills` gèrent aujourd'hui, en SQF, le niveau de compétence des unités IA. Sur Reforger, ce rôle est couvert par un composant natif déjà bien plus modulaire qu'un simple curseur de skill.

**Correction après lecture du vrai code source** (`github.com/ofcrav2/omtk/tree/master/omtk/ia_manager`, jamais audité jusqu'ici) : il existe un **second usage de l'IA**, distinct de celui documenté au §9, et le module `ia_manager` lui est entièrement dédié. Les deux usages coexistent dans OMTK — voir §9 pour le premier, §9bis pour le second.

---

## 2. Le composant central : `SCR_AIConfigComponent`

Trouvé dans le code source réel du jeu, avec ses attributs exposés directement dans l'éditeur :

- **`Unit skill`** — un curseur de 0 à 1, l'équivalent direct de notre variable de skill SQF.
- Une série de **cases à cocher** pour activer/désactiver des comportements précis, indépendamment du niveau de skill général :
  - réagir aux événements de danger,
  - réagir aux cibles perçues,
  - tirer et attaquer en général,
  - chercher et prendre une couverture,
  - viser et faire des gestes en général,
  - autoriser le leader à s'arrêter si la formation est déformée,
  - **autoriser une erreur de visée artificielle** — le paramètre qui contrôle la précision réaliste vs "aimbot" de l'IA.
- Une config de gestion d'armes par type (`WeaponTypeSelectionConfig`/`WeaponTypeHandlingConfig`), qui peut assigner un arbre de comportement (behavior tree) différent selon le type d'arme utilisé.

**Différence avec SQF** : au lieu d'un seul chiffre de skill qui influence tout en bloc, Reforger permet de désactiver/activer des **capacités précises** indépendamment du niveau de skill général — un réglage modulaire plutôt qu'un simple curseur.

---

## 3. Où s'applique ce composant (lien avec `infantry_loadouts`)

Il se configure au niveau du **prefab de personnage** (`Character_Base.et` ou ses variantes de classe) — directement dans l'arborescence de faction/classes déjà documentée dans le récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md), pas dans un module séparé.

---

## 4. Point de friction communautaire à connaître avant de régler nos valeurs

Plusieurs retours de joueurs pointent une IA équipée de RPG perçue comme quasi infaillible même sans ligne de vue claire — un déséquilibre entre précision à l'arme légère et précision au lance-roquettes. Si l'OFCRA veut une IA crédible, il faudra probablement affiner `WeaponTypeHandlingConfig` par type d'arme plutôt que de se fier au seul curseur `Unit skill` global.

---

## 5. Références communautaires

| Mod / ressource | Ce qu'il apporte |
|---|---|
| **AI Difficulty-CYLON** | Mod simple qui surcharge la difficulté IA pour plus de challenge en PvE |
| Tutoriel vidéo "How to make an AI difficulty mod" | Démonstration concrète de la construction de ce type de mod (PvE et Conflict) |
| **CRX Enfusion A.I.** | Réglages fins avancés (ex. "Kill Unconscious Chance" via `SCR_AISettingsComponent`, comportements de waypoint) |

---

## 6. Ce que ça implique pour notre portage

`IA_skills`/`difficulty_check` deviendrait principalement de la **configuration sur les prefabs de personnages** (`SCR_AIConfigComponent`), plutôt qu'un module de script autonome. Un composant modded ne serait nécessaire que si l'OFCRA veut un réglage **dynamique en cours de partie** (ex. augmenter la difficulté progressivement) — un comportement qui n'existe probablement pas nativement et nécessiterait un peu de script pour modifier ces attributs à la volée.

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel `SCR_AIConfigComponent` | `arexplorer.zeroy.com` → `SCR_AIConfigComponent.c` |
| Code source réel du module `ia_manager` (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/ia_manager` |
| Mod AI Difficulty-CYLON | `reforger.armaplatform.com/workshop/64C55774CCFAC9F6-AIDifficulty-CYLON` |
| Mod CRX Enfusion A.I. | `reforger.armaplatform.com/workshop/5F268647F8A1A1F4` |
| Tutoriel vidéo (mod de difficulté IA) | YouTube — "How to make an AI difficulty mod in Arma Reforger" |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Construire une première variante de personnage OFCRA avec un `SCR_AIConfigComponent` ajusté et tester le comportement en jeu.
- Vérifier concrètement l'écart de précision entre armes légères et lance-roquettes signalé par la communauté, et ajuster `WeaponTypeHandlingConfig` si besoin.
- Évaluer si un réglage dynamique de la difficulté en cours de partie est nécessaire pour l'usage OFCRA, ou si une config fixe par classe/faction suffit.
- **Clarifier avec l'OFCRA** si l'usage décrit au §9bis (IA combattante d'appoint pour équilibrer les effectifs) est encore pratiqué aujourd'hui, ou s'il a été abandonné au profit du seul usage non combattant (§9).
- Vérifier l'équivalent Reforger de `disableAI "AUTOTARGET"`/`"TARGET"`/`"FSM"`/`"MOVE"` et du hook `HandleDisconnect` utilisés par `ia_manager` pour geler les IA jouables (§9bis).

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*

---

## 9. Contexte OFCRA : usage non combattant de l'IA

L'OFCRA joue en **TvT sans IA combattante**. L'IA sert uniquement à des rôles **non combattants** — civils, otages — sur certaines missions seulement, et jamais en grand nombre (pas de dizaines d'entités simultanées).

**Ce que ça change pour la configuration :**

Le paradigme Reforger colle mieux à ce besoin que celui d'Arma 3. Plutôt que de ramener un `skill` à zéro en espérant que l'unité reste passive, on décoche directement les comportements de combat sur `SCR_AIConfigComponent` :

- réagir aux événements de danger → décoché ;
- réagir aux cibles perçues → décoché ;
- tirer et attaquer en général → décoché ;
- chercher et prendre une couverture → décoché.

L'entité reste vivante et animée, sans logique de combat. Le curseur `Unit skill` et le réglage fin par type d'arme (`WeaponTypeHandlingConfig`, §4) deviennent en revanche sans objet — ils ne concernent que l'IA combattante.

**Conséquence côté serveur :** l'option `disableAI: true`, couramment recommandée en PvP pur pour libérer du CPU et gagner des créneaux joueurs, **n'est pas utilisable** puisqu'il faut pouvoir faire apparaître civils et otages. L'impact reste faible vu le volume d'IA en jeu, mais c'est une marge de performance en moins — à garder en tête au vu du plafond de joueurs (voir le README).

---

## 9bis. Contexte OFCRA : usage combattant d'appoint (module `ia_manager`)

**Découverte en lisant le vrai code source d'OMTK**, distincte de l'usage documenté au §9. Le README d'`ia_manager` est explicite : *« Même si nous essayons de l'éviter, nous devons parfois utiliser des unités IA pour équilibrer les équipes »*. Ce n'est donc pas un civil ni un otage — c'est une **IA combattante de complément**, utilisée quand les effectifs des deux camps sont déséquilibrés.

Objectif du module : rendre ces IA « plus humaines » en abaissant leurs compétences de combat plutôt qu'en les laissant se comporter en tireurs d'élite parfaits. Valeurs observées dans le code (`main.sqf`) :

| Compétence | Valeur | Sur une échelle 0–1 |
|---|---|---|
| Précision de visée | 0,1 | Très faible |
| Tremblement de visée | 0,1 | Très faible (donc très instable) |
| Vitesse de visée | 0,1 | Très lente |
| Endurance | 0,2 | Faible |
| Distance de détection | 0,3 | Faible |
| Temps de détection | 0,4 | Moyen-faible |
| Courage | 0,4 | Moyen-faible |
| Vitesse de rechargement | 0,4 | Moyen-faible |
| Commandement | 0,2 | Faible |
| Général | 0,2 | Faible |

**Un second mécanisme, lié au warm-up** : quand le paramètre `OMTK_MODULE_DISABLE_PLAYABLE_AI` est actif, ces IA combattantes sont **gelées et rendues immortelles** pendant la période de warm-up — `disableAI` sur les comportements de ciblage, mouvement et FSM, comportement `CARELESS`, dégâts désactivés. Un hook `HandleDisconnect` réapplique ce gel si un joueur qui contrôlait une IA se déconnecte. **C'est l'exact miroir de ce que le récapitulatif [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) documente pour les joueurs humains** (invulnérabilité, simulation coupée) — mais notre document actuel ne couvre que le cas des joueurs. Si l'OFCRA utilise encore des IA jouables en warm-up, cette protection doit être étendue.

**Un contournement à noter, pas à reproduire tel quel** : pour empêcher l'IA de riposter automatiquement à un tir ami humain, le module gonfle artificiellement la réputation du joueur (`player addRating 1000000`) plutôt que de désactiver proprement la riposte. Sur Reforger, la relation joueur/IA et la détection de tir ami déjà documentées dans le récap [`kill_logger`](OMTK_KillLogger_Reforger_Recap.md) §2.2 (`SCR_InstigatorContextData`, méthode de détection de tir ami) offrent probablement un point d'accroche plus propre — à explorer plutôt que de reproduire ce contournement.

**Tension à lever avec l'OFCRA** : ce module contredit en partie l'affirmation du §9 (« sans IA combattante »). Les deux ne sont pas nécessairement incompatibles — l'un peut être la règle générale, l'autre une exception d'appoint rarement utilisée — mais ça mérite d'être clarifié avant de décider si ce module a encore sa place dans le portage.
