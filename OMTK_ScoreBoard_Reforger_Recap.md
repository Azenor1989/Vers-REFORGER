# Récapitulatif — Portage des modules `difficulty_check` / `IA_skills` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

`difficulty_check` et `IA_skills` gèrent aujourd'hui, en SQF, le niveau de compétence des unités IA. Sur Reforger, ce rôle est couvert par un composant natif déjà bien plus modulaire qu'un simple curseur de skill.

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
| Mod AI Difficulty-CYLON | `reforger.armaplatform.com/workshop/64C55774CCFAC9F6-AIDifficulty-CYLON` |
| Mod CRX Enfusion A.I. | `reforger.armaplatform.com/workshop/5F268647F8A1A1F4` |
| Tutoriel vidéo (mod de difficulté IA) | YouTube — "How to make an AI difficulty mod in Arma Reforger" |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Construire une première variante de personnage OFCRA avec un `SCR_AIConfigComponent` ajusté et tester le comportement en jeu.
- Vérifier concrètement l'écart de précision entre armes légères et lance-roquettes signalé par la communauté, et ajuster `WeaponTypeHandlingConfig` si besoin.
- Évaluer si un réglage dynamique de la difficulté en cours de partie est nécessaire pour l'usage OFCRA, ou si une config fixe par classe/faction suffit.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*
