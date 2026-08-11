# Récapitulatif — Portage du module `warm_up` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `warm_up` d'OMTK (Arma 3, SQF) gère la période d'attente avant le démarrage effectif d'une mission. Confirmé à partir du vrai code source (`github.com/ofcrav2/omtk/tree/master/omtk/warm_up`) : ce n'est **pas** une attente d'un nombre de joueurs, mais un minuteur configuré à l'avance, avec un raccourci possible via un vote des officiers.

Sur Reforger, ce comportement a été **entièrement reconstruit et validé en jeu**, avec des choix de conception qui s'écartent par endroits du SQF d'origine — documentés ci-dessous.

**Fonctionnalités attendues, statut de chacune :**
- décompte / durée de warm-up configurée à l'avance — **validé** (minuteur global, §3)
- joueurs invulnérables pendant cette période — **validé** (§4)
- zone de restriction par camp, avec téléportation en cas de sortie — **validé** (§5)
- vote des officiers pour écourter — non porté (voir §8)
- gel des véhicules, retrait de carburant — non porté (voir §8)

---

## 2. Architecture retenue — un minuteur autonome, pas `CanAdvanceState`

**Décision de conception explicite** : après plusieurs sessions bloquées sur une inconnue non résolue (`super.CanAdvanceState()` renvoie systématiquement `false` en jeu sans qu'on sache pourquoi côté moteur — voir l'historique en §9), le module a été reconstruit sur un mécanisme entièrement différent, qui **contourne** cette inconnue plutôt que de la résoudre :

- Un **minuteur unique**, programmé une seule fois au démarrage de la partie (`GetGame().GetCallqueue().CallLater(OMTK_EndWarmup, durée, false)`), qui bascule l'état de warm-up à sa fin.
- Aucune dépendance à `SCR_EGameModeState`/`CanAdvanceState` : le module gère sa propre notion de "warm-up actif" via un simple booléen (`m_bIsWarmupActive`), indépendante de l'état officiel du GameMode.

**Conséquence à assumer** : la fin du warm-up, telle qu'implémentée, ne correspond pas forcément à la vraie transition `PREGAME → GAME` du moteur — c'est une temporisation propre au module OMTK, pas un branchement sur l'état natif. Si un jour `CanAdvanceState` est débloqué, il faudra décider si on relie les deux mécanismes ou si on garde cette indépendance.

---

## 3. Le composant central : `OMTK_WarmupZoneComponent`

Toute la logique — zones, invulnérabilité, minuteur — vit dans un seul fichier, `Scripts/Game/OMTK/OMTK_WarmupZoneComponent.c`, un composant à part entière (pas un `modded class` sur le GameMode partagé) attaché explicitement au GameMode dans le World Editor.

### 3.1 Zones de confinement par faction — CONFIRMÉ EN JEU

- Chaque faction dispose de sa propre liste de zones (`SCR_VirtualAreaEntity`), réglable dans l'éditeur sous forme de noms séparés par des virgules (`"Zone_A,Zone_B"`) — pas de référence d'entité directe en `[Attribute]`, qui ne s'affiche pas dans l'éditeur pour ce type (essayé avec et sans `uiwidget` explicite, jamais fonctionnel).
- Les zones sont retrouvées par **nom** au démarrage (`GetGame().GetWorld().FindEntityByName(...)`), pas par référence directe.
- Un joueur est "dans sa zone" s'il se trouve dans **au moins une** des zones associées à sa propre faction (`SCR_VirtualAreaEntity.InArea(pos)`).
- **Clés de faction réelles, confirmées en jeu** : `US` et `USSR` — pas `BLUEFOR`/`REDFOR` (convention Arma historique, sans existence dans le moteur) ni `REDFOR` (première hypothèse fausse). Les noms des champs dans l'éditeur (`Blufor Zone Names`, `Redfor Zone Names`) restent des étiquettes de confort ; le code utilise bien les vraies clés.

### 3.2 Piège du moteur découvert en jeu : la forme Ellipse

**Un même code, deux comportements différents entre les deux factions** — la zone USSR fonctionnait, la zone US bouclait indéfiniment (téléportation immédiatement suivie d'un nouveau constat "hors zone"). Cause identifiée : la zone US était restée en forme **Ellipse**, l'autre en **Rectangle**.

Ça recoupe une réserve déjà documentée sur `SCR_VirtualAreaEntity.InArea()` : le calcul d'ellipse non circulaire est marqué `//--- ToDo: Ellipse` dans le vrai code source — **incomplet côté moteur**, renvoie `false` silencieusement plutôt que de lever une erreur. **Recommandation ferme** : utiliser la forme **Rectangle** pour toute zone de warm-up, ou s'assurer d'un cercle parfaitement symétrique (X = Z) si Ellipse est préféré.

### 3.3 Téléportation — deux itérations, la seconde retenue

**Première tentative** : `SetOrigin()` à la hauteur enregistrée sur la zone (`m_TestZone.GetOrigin()`) — **tue le joueur par collision** à chaque fois, de façon reproductible, indépendamment du GameMode utilisé (confirmé sur trois bases différentes : `GameModeSF`, `GameMode_TeamDeathmatch_Selection`, un GameMode neuf). Cause : la zone est souvent positionnée à une hauteur qui ne correspond pas au terrain réel à cet endroit.

**Deuxième tentative** : hauteur recalculée via `world.GetSurfaceY(x, z)` (confirmé exister dans le vrai code du jeu, `SCR_ATLManualCameraComponent.c`) — **corrige la mort par collision**, mais casse la détection de zone : le sol réel étant souvent à plusieurs mètres de la hauteur à laquelle la zone a été positionnée, le joueur atterrit hors du volume vertical de `InArea()` (qui vérifie aussi l'axe Y, avec le même rayon que les axes horizontaux) — boucle de téléportation infinie.

**Version retenue, CONFIRMÉE EN JEU** : revenir à la hauteur de la zone elle-même (`firstZone.GetOrigin()`), **sans** `GetSurfaceY()` — rendu possible par la fiabilisation de l'invulnérabilité (§4), qui couvre désormais le risque de mort par chute que `GetSurfaceY()` cherchait à éviter. Les deux problèmes (mort par collision, boucle de téléportation) sont résolus ensemble par cette combinaison : hauteur de zone + invulnérabilité fiable, plutôt que par la hauteur de terrain seule.

```cpp
vector zoneCenter = firstZone.GetOrigin();
ent.SetOrigin(zoneCenter);
```

### 3.4 Invulnérabilité — deux mécanismes essayés, un seul retenu

**Premier mécanisme, insuffisant** : `HijackDamageHandling()` (event sur `SCR_CharacterDamageManagerComponent`, confirmé exister et bloquer un tir) — **ne bloque pas les dégâts de collision**. Abandonné.

**Second mécanisme, CONFIRMÉ EN JEU** : `DamageManagerComponent.EnableDamageHandling(bool)` — méthode **native** (`proto external`, implémentée dans le moteur, pas en script), qui coupe tout le système de dégâts à la racine plutôt que d'intercepter un chemin précis. Appelée :
- à l'apparition de chaque joueur, via abonnement au `ScriptInvoker` `SCR_BaseGameMode.GetOnPlayerSpawned()` — **pas** un `override OnPlayerSpawned(...)` direct, qui est marqué `[Obsolete]` dans le vrai code source (remplacé officiellement par `OnPlayerSpawnFinalize_S`, non utilisé ici) ;
- réactivée pour tous les joueurs connectés à la fin du minuteur global.

```cpp
gameMode.GetOnPlayerSpawned().Insert(OMTK_OnPlayerSpawned);
// ...
protected void OMTK_OnPlayerSpawned(int playerId, IEntity controlledEntity)
{
    DamageManagerComponent damageMgr = DamageManagerComponent.Cast(controlledEntity.FindComponent(DamageManagerComponent));
    if (damageMgr)
        damageMgr.EnableDamageHandling(false);
}
```

**Testé en conditions réelles** : dizaines de téléportations consécutives sans mort, avant comme après le passage à cette version — la mort par collision qui a bloqué la session pendant plusieurs heures n'apparaît plus.

### 3.5 Minuteur global et fin de warm-up — CONFIRMÉ EN JEU

Un seul minuteur (`m_iGlobalWarmupDuration`, en secondes, réglable dans l'éditeur), programmé une fois au démarrage. À son échéance :
1. la boucle de vérification de zone est arrêtée (`GetGame().GetCallqueue().Remove(OMTK_CheckZone)`) ;
2. les dégâts sont réactivés pour tous les joueurs connectés à cet instant.

Séquence observée en jeu, cycle complet : téléportations répétées sans mort pendant le warm-up, puis `>>> FIN DU WARM-UP ! Dégâts activés, restriction de zone levée.`, suivie d'un comportement normal (mortel) après coup — confirmant que la transition fonctionne dans les deux sens.

---

## 4. Autres obstacles rencontrés cette session, sans lien direct avec `warm_up` mais bloquants

Ces points ont consommé le plus clair du temps de dépannage et méritent d'être connus avant toute nouvelle session de test.

### 4.1 Un seul GameMode par monde — règle stricte, pas une simple bonne pratique

La documentation officielle est explicite : `SCR_BaseGameMode` doit être une **instance unique dans le monde — "mandatory"**. Avoir deux GameModes actifs simultanément dans la même scène (même l'un désactivé via la case "Disabled", qui n'exclut pas réellement l'entité au chargement) casse le démarrage de la partie de façon non déterministe — écran noir, doublons de tous les gestionnaires (`FactionManager`, `LoadoutManager`, `RadioManager`, `SCR_AIWorld`...), comportement de spawn erratique. **Seule la suppression pure et simple de l'ancien GameMode résout le problème** — le désactiver ne suffit pas.

### 4.2 Le choix du GameMode de base a des conséquences profondes

Trois familles de GameMode ont été testées ce soir, avec des résultats très différents :
- **`GameMode_TeamDeathmatch_Selection`** : propose nativement un écran de sélection de faction, mais embarque sa propre logique de score/dégâts, potentiellement en conflit avec `SCR_Task`/`score_board` (piste explorée, jamais formellement confirmée comme cause d'un bug précis — voir §9).
- **`GameModeSF`** (Scenario Framework officiel) : porte nativement `SCR_GroupsManagerComponent` et `SCR_MapMarkerManagerComponent`, essentiels pour éviter une erreur de pointeur nul sur `SCR_MapMarkerEntrySquadLeader` au moment du déploiement. **A aussi `SCR_MenuSpawnLogic` disponible nativement** (le champ `Spawn Logic` de son `SCR_RespawnSystemComponent` en atteste) — contrairement à ce qui a été supposé un temps, il n'est pas nécessaire de passer par `GameMode_Plain` pour obtenir ce comportement.
- **`GameMode_Plain.et`** : la coquille la plus nue ; permet de changer facilement `Spawn Logic` via un bouton `..` visible au niveau `GameMode_Base.et`, mais dépourvue des composants de groupe — cause directe de l'erreur `SCR_MapMarkerEntrySquadLeader`.

**Recommandation** : repartir de `GameModeSF.et` par héritage (`Inherit in 'OFCRA'`), vérifier directement sur ce fichier si `Spawn Logic` est déjà réglé sur `SCR_MenuSpawnLogic` avant de chercher à le changer.

### 4.3 `SCR_AutoSpawnLogic` vs `SCR_MenuSpawnLogic`

`SCR_AutoSpawnLogic` place le joueur automatiquement, sans lui laisser de choix — c'est ce qui causait un spawn direct en USSR sans passage par un écran de sélection. `SCR_MenuSpawnLogic` (*"spawning is done via respawn menu. This object notifies the player when they're ready for spawn and opens respawn menu"* — doc officielle *Respawn Setup*) ouvre un vrai écran de déploiement avec choix de faction et d'escouade. **Utile pour du test rapide en solo uniquement** : pour une vraie session OFCRA, les camps sont décidés en amont (hors jeu), donc ni l'un ni l'autre ne reproduit exactement le vrai usage — un joueur se connecte en sachant déjà où se placer, comme sur le lobby Arma 3 actuel. Aucun mécanisme d'assignation de faction pré-déterminée par identité de joueur n'a été identifié ni testé ce soir.

---

## 5. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du module OMTK (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/warm_up` |
| Code source réel du module `ia_manager` (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/ia_manager` |
| Code source réel du panneau admin (Arma 3) | `github.com/ofcrav2/omtk/blob/master/omtk/ui/pauseScreenMenu.sqf` |
| Setup général de game mode (états PREGAME/GAME/POSTGAME) | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |
| Respawn Setup (Forced Faction, SCR_MenuSpawnLogic) | `community.bistudio.com/wiki/Arma_Reforger:Respawn_Setup` |
| Scenario Framework (officiel) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` — a permis de corriger `CanAdvanceState`, de découvrir `GetSurfaceY`, `EnableDamageHandling`, et l'obsolescence d'`OnPlayerSpawned` |

---

## 6. Ce qui reste à faire

**Confirmé et stable, testé en jeu ce soir** : zones de confinement par faction (forme Rectangle), téléportation sans mort, invulnérabilité fiable, minuteur global avec fin propre.

**Reste à faire, dans un ordre suggéré** :
- Reconstituer proprement le GameMode final sur une base `GameModeSF` héritée, propre, sans les résidus de manipulations de cette session (doublons éventuels, prefabs de test).
- Décider si le vote des officiers (raccourci de warm-up) doit être porté, et sous quelle forme — le mécanisme d'action ciblée sur une classe de joueur précise n'a pas été retrouvé côté Reforger.
- Décider d'un mécanisme réel d'assignation de faction pré-déterminée (§4.3), fidèle à l'usage OFCRA actuel — aucune piste testée à ce jour.
- Étendre l'invulnérabilité et le confinement aux IA jouables si elles sont encore utilisées (voir récap `IA_skills` §9bis).
- Porter le gel des véhicules / retrait de carburant pendant le warm-up (non abordé cette session).
- Reprendre `CanAdvanceState()` séparément si on souhaite un jour relier le minuteur OMTK à la vraie machine à états du GameMode plutôt qu'à un booléen indépendant.

---

## 7. Ancien historique — méthode `CanAdvanceState()` (mise en pause, contournée §2)

Conservé pour référence, au cas où cette piste serait reprise plus tard.

`bool CanAdvanceState(SCR_EGameModeState nextState)` existe sur `SCR_BaseGameModeStateComponent` et ses filles (`SCR_PreGameGameModeStateComponent` etc.), confirmée par compilation. **Testée en jeu** : le hook se déclenche à chaque frame en `PREGAME`, mais renvoie systématiquement `false` sans qu'on sache si c'est le comportement natif ou une surcharge du Scenario Framework — **jamais résolu**, et le module a été reconstruit pour s'en passer entièrement (§2). À reprendre uniquement si un besoin futur impose un vrai branchement sur l'état officiel du GameMode plutôt que sur un minuteur autonome.

---

*Document mis à jour après une session de test approfondie en Workbench (version 1.7.0.54) — §3 et §4 confirmés en pratique de bout en bout. §2 documente un choix de conception assumé (contournement de CanAdvanceState), pas une limitation subie.*
