# Récapitulatif — Portage des modules `difficulty_check` / `IA_skills` / `ia_manager` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

`difficulty_check` et `IA_skills` gèrent aujourd'hui, en SQF, le niveau de compétence des unités IA. Sur Reforger, ce rôle est couvert par un composant natif déjà bien plus modulaire qu'un simple curseur de skill.

**Correction après lecture du vrai code source** (`github.com/ofcrav2/omtk/tree/master/omtk/ia_manager`, jamais audité jusqu'ici) : il existe un **second usage de l'IA**, distinct de celui documenté au §9, et le module `ia_manager` lui est entièrement dédié. Les deux usages coexistent dans OMTK — voir §9 pour le premier, §9bis pour le second.

**Tension résolue** : seul l'usage §9 (IA d'objectif, non combattante) est retenu pour l'OFCRA. L'usage §9bis (`ia_manager`, IA combattante d'appoint pour équilibrer les effectifs) est **hors périmètre**, confirmé — voir §9bis pour le détail conservé à titre de référence.

---

## 2. Le composant central : `SCR_AIConfigComponent`

Trouvé dans le code source réel du jeu, avec ses attributs exposés directement dans l'éditeur — noms confirmés en jeu (voir §9) : **Enable Movement**, **Enable Danger Events**, **Enable Perception**, **Enable Attack**, **Enable Take Cover**, **Enable Looking**, **Enable Communication**, **Enable Leader Stop**, **Enable Aiming Error**, plus un curseur **Skill** (0 à 1) et une config de gestion d'armes par type (`WeaponTypeSelectionConfig`/`WeaponTypeHandlingConfig`) qui peut assigner un arbre de comportement différent selon le type d'arme utilisée.

**Différence avec SQF** : au lieu d'un seul chiffre de skill qui influence tout en bloc, Reforger permet de désactiver/activer des **capacités précises** indépendamment du niveau de skill général — un réglage modulaire plutôt qu'un simple curseur.

---

## 3. Où s'applique ce composant (lien avec `infantry_loadouts`)

Il se configure au niveau du **prefab de personnage** (`Character_Base.et` ou ses variantes de classe) — directement dans l'arborescence de faction/classes déjà documentée dans le récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md), pas dans un module séparé.

---

## 4. Point de friction communautaire — hors périmètre désormais

Les retours communautaires sur l'équilibrage précision légère/RPG concernaient l'IA **combattante** (`WeaponTypeHandlingConfig`). Sans objet pour l'OFCRA depuis la résolution de la tension §9bis — conservé ici pour mémoire seulement, au cas où l'usage combattant reviendrait un jour sur la table.

---

## 5. Références communautaires

| Mod / ressource | Ce qu'il apporte |
|---|---|
| **AI Difficulty-CYLON** | Mod simple qui surcharge la difficulté IA pour plus de challenge en PvE |
| Tutoriel vidéo "How to make an AI difficulty mod" | Démonstration concrète de la construction de ce type de mod (PvE et Conflict) |
| **CRX Enfusion A.I.** | Réglages fins avancés (ex. "Kill Unconscious Chance" via `SCR_AISettingsComponent`, comportements de waypoint) |

Ces ressources concernent l'IA combattante — sans objet pour l'usage retenu par l'OFCRA (§9), conservées pour mémoire.

---

## 6. Ce que ça implique pour notre portage

`IA_skills`/`difficulty_check` est, pour l'usage retenu, de la **configuration pure sur les prefabs de personnages** (`SCR_AIConfigComponent`) — **aucun script nécessaire**, confirmé en pratique (§9).

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel `SCR_AIConfigComponent` | `arexplorer.zeroy.com` → `SCR_AIConfigComponent.c` |
| Code source réel du module `ia_manager` (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/ia_manager` |

---

## 8. Ce qui reste à valider en pratique

- ~~Construire une première variante de personnage OFCRA avec un `SCR_AIConfigComponent` ajusté et tester le comportement en jeu~~ — **fait, confirmé en jeu (§9)**.
- ~~Clarifier avec l'OFCRA si l'usage §9bis est encore pratiqué~~ — **fait : hors périmètre, seul l'usage §9 est retenu.**
- Étendre l'invulnérabilité/gel de warm-up à ce type de personnage si utilisé en zone — **déjà fait** dans le cadre général du récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §3.8 (toute IA placée en zone, invulnérable, sans distinction de configuration de comportement).
- Vérifier l'écart de précision légère/RPG signalé par la communauté (§4) — **sans objet**, hors périmètre.
- Évaluer un réglage dynamique de la difficulté en cours de partie — **sans objet**, hors périmètre (concernait l'IA combattante).

---

## 9. Contexte OFCRA : usage non combattant de l'IA — CONFIRMÉ EN JEU

L'OFCRA joue en **TvT sans IA combattante**. L'IA sert uniquement à des rôles **non combattants** — civils, otages — sur certaines missions seulement, et jamais en grand nombre (pas de dizaines d'entités simultanées).

**Configuration testée et confirmée en jeu** : sur `SCR_AIConfigComponent`, décocher ces quatre cases précises (noms exacts de l'éditeur) suffit à obtenir une IA non combattante :

- **Enable Danger Events**
- **Enable Perception**
- **Enable Attack**
- **Enable Take Cover**

Le reste (**Enable Movement**, **Enable Looking**, **Enable Communication**, **Enable Leader Stop**, **Enable Aiming Error**, le curseur **Skill**) reste inchangé — sans objet une fois le combat désactivé, sauf `Enable Movement`/`Enable Looking` qui gouvernent le déplacement et les animations normales (à garder actifs pour que l'entité reste vivante et crédible).

**Comportement observé en jeu** : l'entité se déplace normalement si des points de passage lui sont donnés, tourne la tête vers le joueur (cosmétique), mais ne réagit à aucune menace — ni sursaut, ni recherche de couvert, ni riposte, même visée ou attaquée directement.

**Point important, non couvert par cette configuration** : ces quatre cases ne rendent **pas** l'IA invulnérable — c'est un mécanisme de comportement, pas de dégâts. L'invulnérabilité reste un sujet séparé, déjà couvert par le récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §3.8 pour une IA placée en zone de warm-up.

**Conséquence côté serveur, toujours valable :** l'option `disableAI: true` (recommandée en PvP pur pour libérer du CPU) **n'est pas utilisable** puisqu'il faut pouvoir faire apparaître civils et otages. Impact faible vu le volume d'IA en jeu, mais une marge de performance en moins — à garder en tête au vu du plafond de joueurs (voir le README).

---

## 9bis. Ancien contexte — usage combattant d'appoint (`ia_manager`), HORS PÉRIMÈTRE

Conservé pour référence uniquement. Ce module documentait une IA combattante de complément pour équilibrer des effectifs déséquilibrés, avec des valeurs de compétence volontairement dégradées et un mécanisme de gel pendant le warm-up, miroir de celui construit pour les joueurs humains. **Décision : non retenu.** L'OFCRA n'utilise que l'usage non combattant (§9).

---

*Document mis à jour après test en jeu confirmé de la configuration IA d'objectif (§9). Tension avec §9bis résolue.*
