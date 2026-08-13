# Récapitulatif — Portage des modules `radio_lock` / `radio_settings` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

`radio_lock` et `radio_settings` gèrent aujourd'hui, en SQF, le contrôle des communications radio par camp (verrouillage entre BLUEFOR/REDFOR, réglages de fréquences). Sur Reforger, c'est le module où la conclusion diffère le plus des précédents : la fonction centrale de `radio_lock` est **déjà native au moteur**, sans code à écrire.

---

## 2. Le composant natif : `SCR_RadioComponent` / `RadioHandlerComponent`

- **`SCR_RadioComponent`** gère la radio elle-même : réglage de fréquence par pas (0,5 MHz), présets de canaux.
- **`RadioHandlerComponent`** est le gestionnaire central par joueur, qui pilote l'ensemble des radios qu'il porte.

---

## 3. Le chiffrement par faction est natif — `radio_lock` n'a presque plus de raison d'être en tant que composant custom

C'est le point le plus important de ce module :
- **Chaque faction est programmée avec une clé de chiffrement différente au spawn.**
- Un joueur ennemi sur la même fréquence numérique n'entend que du bruit statique, jamais la voix — le "verrouillage" par camp qu'on codait à la main est **le comportement par défaut du moteur**.
- **Limite réaliste à connaître** : le chiffrement ne protège pas contre une **radio capturée** physiquement sur un ennemi mort — un joueur qui la ramasse peut potentiellement écouter avec la bonne clé (mécanique de "codebook" mentionnée dans le manuel in-game, encore peu documentée).
- La **voix en proximité** (sans passer par la radio) n'est jamais chiffrée — un ennemi assez proche entend toujours, peu importe la radio utilisée.

---

## 4. Lien direct avec `dynamic_startup` / `infantry_loadouts` : radio comme point de respawn

En mode **Conflict**, une radio manpack (AN/PRC-77, R-107M) portée par un joueur vivant peut servir de **point de respawn** pour toute son équipe, tant qu'elle reste dans la zone de signal — paramétrable dans Game Master. Ça recoupe directement le `SCR_RespawnSystemComponent` et les spawn points par faction déjà documentés dans le récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md).

---

## 5. Ce que `radio_settings` couvrirait encore (config, peu de script)

- **Fréquences par défaut par faction** — déjà présentes dans la config de faction étendue Conflict (voir récap [`infantry_loadouts`](OMTK_Loadouts_Reforger_Recap.md), §4).
- **Portée/type de radio** (manpack longue portée vs radio courte portée) — gérable via le choix des prefabs de radio disponibles à l'arsenal, pas par script.
- **Callsigns randomisés par session** pour les objectifs — déjà natif ; limite la valeur d'une interception radio, indépendamment de notre logique de camp.

---

## 6. Références communautaires (pour aller au-delà du système vanilla)

| Mod | Ce qu'il ajoute | Intérêt pour nous |
|---|---|---|
| **Enhanced ARMA Radio System** | Plusieurs clés de chiffrement par preset, plus de présets, réglage de fréquence au clavier | Si l'OFCRA veut un contrôle plus fin que le système vanilla |
| **Realism Overhaul - Radios** | Retouche des plages de fréquences et portées par radio | Pour se rapprocher des valeurs de notre config actuelle |

---

## 7. Ce que ça implique pour notre portage

Contrairement aux modules précédents, celui-ci est majoritairement une question de **configuration** :
- `radio_lock` : **probablement pas besoin d'un composant custom** — le chiffrement par faction est déjà le comportement natif. À réévaluer seulement si l'OFCRA veut un comportement spécifique non couvert (ex. règles de capture de radio, désactivation volontaire de cette limite pour un scénario d'infiltration).
- `radio_settings` : devient une **configuration de faction** (fréquences par défaut, radios disponibles à l'arsenal) plutôt qu'un script — à documenter dans les fichiers de config de faction OFCRA, pas dans un composant séparé.

---

## 8. Nouveau besoin identifié — effacement/reset de radio capturée

Absent d'OMTK d'origine, identifié séparément avec l'OFCRA. Le chiffrement natif par faction
(§3) protège tant que la radio reste en possession de son camp — mais si elle est **capturée**
physiquement sur un ennemi mort, la mécanique de "codebook" pourrait permettre à l'ennemi de
l'exploiter (voir la limite déjà notée en §3).

**Besoin** : la squad qui perd sa radio doit pouvoir l'effacer avant que l'ennemi ne puisse
l'exploiter.

**Contrainte de conception, précisée par l'OFCRA** : ce n'est **pas** un effacement à distance —
un joueur vivant ne doit pas pouvoir vider une radio à 10 km. Il faut **s'approcher physiquement
de la radio elle-même** pour déclencher l'action. Même patron que le bouton "End Warm-up" déjà
construit et testé (voir récap [`warm_up`](OMTK_WarmUp_Reforger_Recap.md) §3.6) :
`ActionsManagerComponent` + `Action Context` sur l'entité radio, portée d'interaction de
quelques mètres — pas un RPC déclenchable depuis n'importe où.

**Pas prioritaire** — à garder sous le coude, surtout si l'implémentation s'avère lourde.

---

## 9. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du composant radio | `arexplorer.zeroy.com` → `SCR_RadioComponent.c` (Arma Reforger Explorer) |
| API Script `Radio` | `community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/group__Radio.html` |
| Guide radio/communications | `xgamingserver.com/blog/arma-reforger-radio-communications-guide/` |
| Guide communautaire "Radio 101" (spawn sur manpack, codebooks) | Steam Workshop guide, ID 2814452333 |
| Mod Enhanced ARMA Radio System | `reforger.armaplatform.com/workshop/5A7B54748B401497` |
| Mod Realism Overhaul - Radios | `reforger.armaplatform.com/workshop/632137C83DA94DD5-RealismOverhaul-Radios` |

---

## 10. Ce qui reste à valider en pratique (Workbench requis)

- Confirmer en jeu le comportement exact de la capture de radio ennemie (la mécanique de "codebook" reste peu documentée officiellement).
- Vérifier si le respawn sur radio manpack est paramétrable pour matcher exactement les règles de spawn actuelles de l'OFCRA, ou s'il faut un `SCR_RespawnSystemComponent` custom en complément.
- Tester si le système natif suffit tel quel pour l'usage OFCRA, avant d'envisager un des mods communautaires listés en §6.
- Module de reset radio (§8) : vérifier que `ActionsManagerComponent` fonctionne sur une entité radio comme sur un véhicule, et identifier l'appel technique pour réinitialiser/effacer la clé côté `SCR_RadioComponent`.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §10).*
