# Récapitulatif — Portage du module `warm_up` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `warm_up` d'OMTK (Arma 3, SQF) gère la période d'attente avant le démarrage effectif d'une mission. Confirmé à partir du vrai code source (`github.com/ofcrav2/omtk/tree/master/omtk/warm_up`) : ce n'est **pas** une attente d'un nombre de joueurs, mais un minuteur configuré à l'avance.

Sur Reforger, ce comportement a été **entièrement reconstruit et validé en jeu**, avec des choix de conception qui s'écartent par endroits du SQF d'origine — documentés ci-dessous.

**Fonctionnalités attendues, statut de chacune :**
- décompte / durée de warm-up configurée à l'avance — **validé** (minuteur global, §3.5)
- joueurs invulnérables pendant cette période — **validé** (§3.4)
- zone de restriction par camp, avec téléportation en cas de sortie — **validé** (§3.1)
- fin de warm-up déclenchée par un admin, en plus du minuteur — **validé** (§3.6)
- véhicules immobilisés pendant le warm-up — **validé** (§3.7)
- IA volontairement placées en zone, invulnérables comme les joueurs — **validé** (§3.8)
- assignation de faction pré-déterminée — **hors périmètre**, ce n'est pas un besoin Reforger (voir §4.3)

---

## 2. Architecture retenue — un minuteur autonome, pas `CanAdvanceState`

**Décision de conception explicite** : après plusieurs sessions bloquées sur une inconnue non résolue (`super.CanAdvanceState()` renvoie systématiquement `false` en jeu sans qu'on sache pourquoi côté moteur — voir l'historique en §9), le module a été reconstruit sur un mécanisme entièrement différent, qui **contourne** cette inconnue plutôt que de la résoudre :

- Un **minuteur unique**, programmé une seule fois au démarrage de la partie (`GetGame().GetCallqueue().CallLater(OMTK_EndWarmup, durée, false)`), qui bascule l'état de warm-up à sa fin.
- Aucune dépendance à `SCR_EGameModeState`/`CanAdvanceState` : le module gère sa propre notion de "warm-up actif" via un simple booléen (`m_bIsWarmupActive`), indépendante de l'état officiel du GameMode.
- Un **second déclencheur**, ajouté cette session : un admin peut appeler la même méthode de fin (`OMTK_EndWarmup`) à la demande, via RPC (§3.6) — le minuteur et le trigger admin convergent tous les deux vers ce point unique.

**Conséquence à assumer** : la fin du warm-up, telle qu'implémentée, ne correspond pas forcément à la vraie transition `PREGAME → GAME` du moteur — c'est une temporisation propre au module OMTK, pas un branchement sur l'état natif. Si un jour `CanAdvanceState` est débloqué, il faudra décider si on relie les deux mécanismes ou si on garde cette indépendance.

---

## 3. Le composant central : `OMTK_WarmupZoneComponent`

Toute la logique — zones, invulnérabilité, minuteur, trigger admin, véhicules, IA — vit dans un seul fichier, `Scripts/Game/OMTK/OMTK_WarmupZoneComponent.c`, un composant à part entière (pas un `modded class` sur le GameMode partagé) attaché explicitement au GameMode dans le World Editor. Un second fichier, `OMTK_VehicleImmobilization.c`, porte le blocage moteur (§3.7).

### 3.1 Zones de confinement par faction — CONFIRMÉ EN JEU

- Chaque faction dispose de sa propre liste de zones (`SCR_VirtualAreaEntity`), réglable dans l'éditeur sous forme de noms séparés par des virgules (`"Zone_A,Zone_B"`) — pas de référence d'entité directe en `[Attribute]`, qui ne s'affiche pas dans l'éditeur pour ce type (essayé avec et sans `uiwidget` explicite, jamais fonctionnel).
- Les zones sont retrouvées par **nom** au démarrage (`GetGame().GetWorld().FindEntityByName(...)`), pas par référence directe.
- Un joueur est "dans sa zone" s'il se trouve dans **au moins une** des zones associées à sa propre faction (`SCR_VirtualAreaEntity.InArea(pos)`).
- **Clés de faction réelles, confirmées en jeu** : `US` et `USSR` — pas `BLUEFOR`/`REDFOR` (convention Arma historique, sans existence dans le moteur).

### 3.2 Piège du moteur découvert en jeu : la forme Ellipse

**Un même code, deux comportements différents entre les deux factions** — la zone USSR fonctionnait, la zone US bouclait indéfiniment (téléportation immédiatement suivie d'un nouveau constat "hors zone"). Cause identifiée : la zone US était restée en forme **Ellipse**, l'autre en **Rectangle**.

Ça recoupe une réserve déjà documentée sur `SCR_VirtualAreaEntity.InArea()` : le calcul d'ellipse non circulaire est marqué `//--- ToDo: Ellipse` dans le vrai code source — **incomplet côté moteur**, renvoie `false` silencieusement plutôt que de lever une erreur. **Recommandation ferme** : utiliser la forme **Rectangle** pour toute zone de warm-up, ou s'assurer d'un cercle parfaitement symétrique (X = Z) si Ellipse est préféré.

### 3.3 Téléportation — deux itérations, la seconde retenue

**Première tentative** : `SetOrigin()` à la hauteur enregistrée sur la zone (`m_TestZone.GetOrigin()`) — **tue le joueur par collision** à chaque fois. Cause : la zone est souvent positionnée à une hauteur qui ne correspond pas au terrain réel à cet endroit.

**Deuxième tentative** : hauteur recalculée via `world.GetSurfaceY(x, z)` — **corrige la mort par collision**, mais casse la détection de zone : le joueur atterrit hors du volume vertical de `InArea()` — boucle de téléportation infinie.

**Version retenue, CONFIRMÉE EN JEU** : revenir à la hauteur de la zone elle-même (`firstZone.GetOrigin()`), **sans** `GetSurfaceY()` — rendu possible par la fiabilisation de l'invulnérabilité (§3.4).

```cpp
vector zoneCenter = firstZone.GetOrigin();
ent.SetOrigin(zoneCenter);
```

### 3.4 Invulnérabilité des joueurs — CONFIRMÉ EN JEU

**Mécanisme retenu** : `DamageManagerComponent.EnableDamageHandling(bool)` — méthode **native**, qui coupe tout le système de dégâts à la racine. Appelée :
- à l'apparition de chaque joueur, via abonnement au `ScriptInvoker` `SCR_BaseGameMode.GetOnPlayerSpawned()` — **pas** un `override OnPlayerSpawned(...)` direct, marqué `[Obsolete]` dans le vrai code source ;
- réactivée pour tous les joueurs connectés à la fin du warm-up.

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

**⚠️ Doublon non résolu** : `OMTK_WarmupInvulnerability.c` implémente un **second** mécanisme d'invulnérabilité, via `HijackDamageHandling()` sur `SCR_CharacterDamageManagerComponent`, déclenché par `OnGameStateChanged` (dans `OMTK_ObjectiveScoreLink.c`) — observé en jeu se désactivant **quasi immédiatement** au lancement de la partie (`Invulnérabilité warm-up réglée sur false` dès le premier `Frame`), pas synchronisé avec la vraie durée du warm-up. Les deux mécanismes coexistent dans le projet actuel ; celui de ce fichier (`EnableDamageHandling`) est celui réellement testé et fiable. **À trancher avant la mise en production : retirer `OMTK_WarmupInvulnerability.c` et son déclenchement dans `OMTK_ObjectiveScoreLink.c`, qui semblent redondants et inertes.**

### 3.5 Minuteur global et fin de warm-up — CONFIRMÉ EN JEU

Un seul minuteur (`m_iGlobalWarmupDuration`, en secondes, réglable dans l'éditeur), programmé une fois au démarrage. À son échéance (`OMTK_EndWarmup`) :
1. la boucle de vérification de zone est arrêtée ;
2. les dégâts sont réactivés pour tous les joueurs connectés ;
3. les véhicules sont libérés (§3.7) ;
4. l'invulnérabilité des IA en zone est retirée (§3.8).

### 3.6 Trigger admin de fin de warm-up — CONFIRMÉ EN JEU (chemin complet, hors validation de la vraie liste d'admins)

Deuxième déclencheur possible pour `OMTK_EndWarmup()`, en plus du minuteur : un admin peut l'appeler à la demande. Repose sur **`SCR_PlayerListedAdminManagerComponent`** (doc officielle Script API, `Game/GameMode/Components/SCR_PlayerListedAdminManagerComponent.c`), qui lit la liste `admins` du `config.json` du serveur — l'équivalent de `admin_uids` en SQF.

```cpp
[RplRpc(RplChannel.Reliable, RplRcver.Server)]
void RpcAsk_AdminEndWarmup(int playerId)
{
    SCR_PlayerListedAdminManagerComponent adminMgr = SCR_PlayerListedAdminManagerComponent.GetInstance();
    if (!adminMgr)
        return;
    if (!adminMgr.IsPlayerOnAdminList(playerId))
        return;
    OMTK_EndWarmup();
}
```

**Point d'implémentation important** : cette méthode doit être **publique** (pas `protected`), puisqu'elle est appelée depuis l'extérieur du composant (un bouton d'UI, ou l'action de test ci-dessous).

**Testé en conditions réelles, deux fois, de bout en bout** : depuis un vrai bouton en jeu (interaction physique sur un véhicule, voir procédure ci-dessous) jusqu'au filtre serveur — `RpcAsk_AdminEndWarmup reçu de playerId=1` puis `playerId=1 n'est pas admin, ignoré`, comportement strictement attendu en session Workbench solo (`config.json` non chargé, aucun admin listé). **La branche "admin confirmé → fin du warm-up" n'a pas encore été testée** : ça nécessite un vrai serveur (dédié ou hébergé) avec l'ID Reforger du testeur dans `game.admins` du `config.json` — hors de portée du Workbench en solo. À faire lors d'une prochaine session avec serveur réel.

**Découverte annexe, utile pour le futur panneau admin (`ui`)** : faire apparaître une `ScriptedUserAction` en jeu ne suffit pas à créer la classe — il faut la câbler explicitement sur une entité physique (procédure officielle *Action Context Setup*) :
1. L'entité cible doit avoir un `ActionsManagerComponent` et un `RplComponent` (les véhicules les ont nativement — pratique pour un test rapide).
2. Ajouter une entrée dans **Action Contexts** (nom + `PointInfo` de position).
3. Ajouter l'action dans **Additional Actions**, avec ce contexte en `Parent Context List`, un `UiInfo` (nom affiché), une portée et une durée.

**Constat important** : `OMTK_ReadyAction.c` (action radiale "chef de camp prêt", pointant vers l'ancien système `CanAdvanceState`/`SCR_PreGameGameModeStateComponent.ShortenWarmUp()` — voir §7) n'a **jamais été câblé** de cette façon dans le projet actuel : la classe existait, mais n'apparaissait nulle part en jeu. Du code mort, pas un mécanisme fonctionnel qu'il aurait fallu juste reconnecter au nouveau système. **Décision : supprimé du projet.**

Une action de test temporaire, `OMTK_TEST_AdminEndWarmupAction.c`, a été ajoutée et câblée sur le véhicule de test pour cette validation — **à retirer avant toute session OFCRA réelle** (visible par tout le monde, pas seulement les admins ; le filtre `IsPlayerOnAdminList` protège le traitement serveur mais pas l'affichage du bouton).

### 3.7 Immobilisation des véhicules — CONFIRMÉ EN JEU (deux itérations, la seconde retenue)

**Première tentative, écartée** : `VehicleControllerComponent.SetCanMove(false)` — trouvée dans le vrai code source (`SCR_VehicleDamageManagerComponent.c`, qui l'utilise nativement pour les dégâts de mobilité). **Compilée et testée, mais inefficace** : le véhicule bougeait quand même en jeu. Cause probable : ce système natif de dégâts recalcule et réécrit périodiquement cet état selon l'intégrité du véhicule — sur un véhicule intact, il repasse à `true` tout seul, écrasant notre appel.

**Version retenue, CONFIRMÉE EN JEU** : bloquer le **démarrage moteur** lui-même, à un niveau que ce système de dégâts ne touche pas. Nouveau fichier `OMTK_VehicleImmobilization.c` :

```cpp
modded class VehicleControllerComponent
{
    protected static bool s_bOMTK_WarmupEngineBlocked = false;

    override bool OnBeforeEngineStart()
    {
        if (s_bOMTK_WarmupEngineBlocked)
            return false;
        return super.OnBeforeEngineStart();
    }

    static void OMTK_SetWarmupEngineBlocked(bool value)
    {
        s_bOMTK_WarmupEngineBlocked = value;
    }
}
```

Au démarrage du warm-up, le drapeau est activé et les véhicules déjà en marche (résidu d'une session précédente) sont coupés via `StopEngine(false)`. À la fin, le drapeau est simplement remis à `false` — les joueurs redémarrent eux-mêmes le moteur normalement.

**Comportement confirmé en jeu** : entrée/sortie du véhicule et gestion de l'inventaire restent disponibles ; seul le démarrage moteur est refusé (`[OMTK] Démarrage moteur bloqué pendant le warm-up.` en boucle tant que le joueur maintient la tentative de démarrage).

### 3.8 Invulnérabilité des IA en zone de warm-up — CONFIRMÉ EN JEU (trois correctifs successifs)

Seules les IA **volontairement placées** dans une zone de warm-up (civils, otages, etc.) sont concernées — une IA-objectif ailleurs sur la carte (cible à capturer) reste inchangée. Le filtre se fait par position réelle (`InArea`), pas par configuration explicite de l'IA.

**Trois problèmes rencontrés et corrigés en une session, dans l'ordre** :

1. **`QueryEntitiesBySphere` sans flags ne remonte pas les personnages.** Le même appel à 3 paramètres qui fonctionne pour les véhicules (§3.7 historique, kill_logger) ne renvoyait jamais l'IA de test. Trouvé dans un tutoriel officiel Bohemia (*Create a Component*, exemple cherchant justement des "human entities") : il faut passer `EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT` en 5e paramètre.
2. **Sans filtre de type, la requête attrape trop d'entités** (portes, objets destructibles avec `DamageManagerComponent`) — une seule IA produisait des dizaines de lignes de log. Corrigé avec `ChimeraCharacter.Cast(ent)` (motif standard confirmé dans de nombreux fichiers du vrai code du jeu) pour ne garder que de vrais personnages.
3. **Une même IA peut déclencher le callback plusieurs fois** (probablement une fois par forme de collision — squelette/membres — hypothèse non confirmée mais le correctif fonctionne). Corrigé par déduplication : un tableau d'entités déjà traitées, vidé avant chaque passage, vérifié dans le callback.

```cpp
protected ref array<IEntity> m_aOMTK_ProcessedEntities = new array<IEntity>();

protected void OMTK_ApplyAIInvulnerability()
{
    m_aOMTK_ProcessedEntities.Clear();
    GetGame().GetWorld().QueryEntitiesBySphere("0 0 0", 100000.0, OMTK_ApplyAIInvulnerabilityCallback, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
}

protected bool OMTK_ApplyAIInvulnerabilityCallback(IEntity ent)
{
    ChimeraCharacter character = ChimeraCharacter.Cast(ent);
    if (!character || m_aOMTK_ProcessedEntities.Contains(character))
        return true;
    m_aOMTK_ProcessedEntities.Insert(character);

    PlayerManager pm = GetGame().GetPlayerManager();
    if (pm && pm.GetPlayerIdFromControlledEntity(character) > 0)
        return true; // joueur réel — déjà géré par OMTK_OnPlayerSpawned

    if (!OMTK_IsInAnyWarmupZone(character))
        return true;

    DamageManagerComponent damageMgr = DamageManagerComponent.Cast(character.FindComponent(DamageManagerComponent));
    if (damageMgr)
        damageMgr.EnableDamageHandling(false);

    return true;
}
```

**Testé en conditions réelles** : un civil placé dans la zone a encaissé plusieurs tirs au visage (impacts visuels confirmés) sans mourir, avec exactement une ligne de log par IA après le troisième correctif — contre 25 avant.

**Note API, jamais utilisée ailleurs dans ce projet avant aujourd'hui** : la syntaxe `foreach (string key, ref array<T> val : uneMap)` pour itérer une `map<string, ref array<T>>` compile et fonctionne — confirmé par ce test, utilisée dans `OMTK_IsInAnyWarmupZone` pour vérifier l'appartenance à n'importe quelle zone toutes factions confondues.

---

## 4. Autres obstacles rencontrés, sans lien direct avec `warm_up` mais bloquants

### 4.1 Un seul GameMode par monde — règle stricte, pas une simple bonne pratique

`SCR_BaseGameMode` doit être une **instance unique dans le monde — "mandatory"**. Avoir deux GameModes actifs simultanément (même l'un désactivé via la case "Disabled") casse le démarrage de façon non déterministe. **Seule la suppression pure et simple de l'ancien GameMode résout le problème.**

### 4.2 `modded class SCR_BaseGameModeComponent` s'applique à TOUS les composants qui en héritent

`OMTK_ObjectiveScoreLink.c` modifie `SCR_BaseGameModeComponent` — la classe de base dont héritent la plupart des composants attachés au GameMode (une trentaine dans le projet actuel, natifs et OMTK confondus). Chacun de ces composants appelle `super.OnGameModeStart()` dans son propre override, ce qui déclenche le code de la modded class **une fois par composant sibling**, pas une fois par composant qu'on aurait pu croire concerné. D'où les `Abonnement à s_OnTaskStateChanged déjà actif, ignoré.` répétés ~20-25 fois dans les logs — un seul enregistrement réel a lieu (le garde-fou `s_bOMTK_TaskListenerRegistered` fonctionne), mais le bruit de log est trompeur.

**Vérifié cette session** : ce mécanisme n'affecte que le corps de la modded class elle-même — il ne provoque **pas** de ré-exécution de `OnGameModeStart()` propre à `OMTK_WarmupZoneComponent` (confirmé par un print d'instance, une seule occurrence observée). Les deux bugs de duplication rencontrés (celui-ci et celui du §3.8) ont des causes différentes malgré des symptômes similaires en apparence (page de log encombrée) — ne pas supposer qu'ils partagent la même origine sans vérifier.

### 4.3 Assignation de faction — confirmé hors périmètre Reforger

Contrairement à ce qui restait ouvert dans une version précédente de ce document, ce n'est **pas** un mécanisme à construire : les camps sont décidés à la création de mission (hors jeu), les joueurs se slotent eux-mêmes à leur place convenue au moment de rejoindre, les admins font la police en cas d'erreur. Aucun code Reforger nécessaire.

### 4.4 Le choix du GameMode de base a des conséquences profondes

Voir version précédente de ce document pour le détail comparatif `GameMode_TeamDeathmatch_Selection` / `GameModeSF` / `GameMode_Plain` — toujours valable, non retesté cette session.

---

## 5. Ressources de référence

| Sujet | Lien |
|---|---|
| Code source réel du module OMTK (Arma 3) | `github.com/ofcrav2/omtk/tree/master/omtk/warm_up` |
| `SCR_PlayerListedAdminManagerComponent` (doc officielle) | `community.bistudio.com/wikidata/.../interfaceSCR__PlayerListedAdminManagerComponent.html` |
| `BaseVehicleControllerComponent` (doc officielle — `SetCanMove`, `OnBeforeEngineStart`, `StopEngine`) | `community.bistudio.com/wikidata/.../interfaceBaseVehicleControllerComponent.html` |
| `SetCanMove` en usage natif réel (dégâts de mobilité) | `arexplorer.zeroy.com` — `SCR_VehicleDamageManagerComponent.c` |
| `QueryEntitiesBySphere` avec flags, exemple officiel ciblant des personnages | `community.bistudio.com/wiki/Arma_Reforger:Create_a_Component` |
| `ChimeraCharacter.Cast()`, motif de filtre standard | `arexplorer.zeroy.com` — nombreux fichiers (`SCR_ChimeraAIAgent.c`, `Crosshair.c`, etc.) |
| Câblage d'une `ScriptedUserAction` sur une entité (`ActionsManagerComponent`) | `community.bistudio.com/wiki/Arma_Reforger:Action_Context_Setup` |
| Admin via `config.json` (`game.admins`, Reforger ID) | doc communautaire hébergeurs (Nodecraft, Loafhosts) — pas de page officielle Bohemia trouvée à ce jour |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` |

---

## 6. Ce qui reste à faire

**Confirmé et stable, testé en jeu cette session** : zones de confinement, téléportation sans mort, invulnérabilité joueurs fiable, minuteur global, trigger admin (chemin complet sauf vraie liste d'admins), véhicules immobilisés, IA en zone invulnérables.

**Reste à faire, dans un ordre suggéré** :
- **Tester la vraie liste d'admins** sur un serveur dédié/hébergé réel (config.json avec un ID Reforger dans `game.admins`) — le chemin "admin confirmé → fin du warm-up" n'a jamais été exercé, seul le rejet "pas admin" l'a été.
- **Retirer le bouton de test** (`OMTK_TEST_AdminEndWarmupAction.c` + son Action Context sur le véhicule) une fois le test ci-dessus fait.
- **Trancher le doublon d'invulnérabilité** (§3.4) : retirer `OMTK_WarmupInvulnerability.c` et son déclenchement dans `OMTK_ObjectiveScoreLink.c`, qui semblent redondants et inertes face au mécanisme réellement testé.
- ~~décider du sort d'`OMTK_ReadyAction.c`~~ — **fait : supprimé** (code mort, jamais câblé, pointait vers l'ancien système `CanAdvanceState`)
- **Nettoyer `docs/drafts/`** : `OMTK_ReadyAction_DRAFT.c` et `OMTK_WarmUpComponent_DRAFT.c` documentent l'approche `CanAdvanceState` abandonnée — à retirer du dépôt, ce récapitulatif couvre déjà l'historique utile (§7).
- Reconstituer proprement le GameMode final sur une base `GameModeSF` héritée, sans les résidus de manipulations de test.
- Reprendre `CanAdvanceState()` séparément si on souhaite un jour relier le minuteur OMTK à la vraie machine à états du GameMode plutôt qu'à un booléen indépendant.

---

## 7. Ancien historique — méthode `CanAdvanceState()` (mise en pause, contournée §2)

Conservé pour référence, au cas où cette piste serait reprise plus tard.

`bool CanAdvanceState(SCR_EGameModeState nextState)` existe sur `SCR_BaseGameModeStateComponent` et ses filles (`SCR_PreGameGameModeStateComponent` etc.), confirmée par compilation. **Testée en jeu** : le hook se déclenche à chaque frame en `PREGAME`, mais renvoie systématiquement `false` sans qu'on sache si c'est le comportement natif ou une surcharge du Scenario Framework — **jamais résolu**, et le module a été reconstruit pour s'en passer entièrement (§2).

---

*Document mis à jour après une session de test approfondie en Workbench (version 1.7.0.54) — §3.1 à §3.8 confirmés en pratique de bout en bout, à l'exception de la vraie liste d'admins (§3.6, nécessite un serveur réel).*
