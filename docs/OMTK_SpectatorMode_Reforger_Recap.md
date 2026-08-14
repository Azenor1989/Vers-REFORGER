# Récapitulatif — Portage du mode spectateur (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

**Module découvert tardivement**, hors de la structure habituelle des dossiers `omtk/<module>/` —
voir §1.

---

## 1. Contexte

Contrairement aux autres modules, le mode spectateur n'a pas son propre dossier dans le dépôt
source OMTK. Il est câblé directement dans les scripts racine de la mission (Arma 3) :

- **`description.ext`** — définit le paramètre de mission `OMTK_MODULE_SPECTATOR`
  (`title = "Spectator"`, valeurs `all`/`team`, défaut `0`/all)
- **`onPlayerKilled.sqf`** — lance le mode spectateur à la mort du joueur, via l'addon tiers
  **EG Spectator Mode** (`BIS_fnc_EGSpectator`)
- **`onPlayerRespawn.sqf`** — le termine si le joueur reprend un rôle (`["Terminate"] call
  BIS_fnc_EGSpectator`)

Directement lié à la règle « aucun respawn » de l'OFCRA (voir README, § Le constat de départ) :
une fois mort, c'est ce système qui permet au joueur de regarder la suite de la partie plutôt que
de fixer un écran noir.

---

## 2. Spec de référence — ce qu'OMTK a réellement aujourd'hui

Confirmée sur deux sources : la précision directe de l'OFCRA, et la documentation officielle
Bohemia du mode utilisé (*Arma 3: End Game Spectator Mode*).

**Point de vocabulaire** : le type de respawn Arma qui déclenche nativement ce mode s'appelle
littéralement **« Bird »** (`respawn = 1`) — l'héritage direct de la **« mouette » d'Operation
Flashpoint**, terme employé par l'OFCRA. Ce n'est **pas** une possession d'IA : c'est une caméra
détachée.

**Fonctionnalités du mode complet (doc officielle Bohemia)** :
- **Trois perspectives** : 1ère personne (sur l'unité suivie), libre, 3ème personne
- **Sélection d'unité au clic gauche** (focus/défocus) ; **barre espace** pour cycler entre les
  modes caméra une fois une unité focalisée
- **Carte custom** dédiée au spectateur, pour naviguer vite et changer d'unité suivie — c'est
  l'équivalent de la « liste de joueurs » attendue
- **Modes de vision** : normal, vision nocturne, thermique (en 1PP, dépend de l'état réel de
  l'unité suivie)
- **Tracé de projectiles** (touche `O`, "Draw projectiles") — visualisation des trajectoires de
  balles, à la demande du spectateur. Fonctionnalité **du mode spectateur lui-même**, pas un
  module séparé ; le déclenchement manuel évite le problème d'échelle qu'aurait un tracé
  permanent à 100+ joueurs.
- **Mode lampe torche** (touche L), **Backspace** pour masquer/afficher l'interface
- **Déplacement libre** WASD + Q/Z, trois paliers de vitesse (Shift, Alt, Shift+Alt)

**OMTK active explicitement toutes les options** — l'appel réel dans `onPlayerKilled.sqf` passe
dix paramètres tous à `true`, là où l'exemple de la doc n'en passe que trois :

```sqf
["Initialize", [player, [], true, true, true, true, true, true, true, true]] call BIS_fnc_EGSpectator;
```

C'est un choix délibéré : la réimplémentation doit viser le mode **complet**, pas une version
minimale.

**Hors périmètre — la communication.** Gérée en dehors du jeu par TeamSpeak + TFAR (canal séparé
pour les spectateurs). Rien à construire côté mod pour ça.

---

## 3. Recherche côté Reforger — rien de natif équivalent

Vérifié contre la doc officielle Script API (branche 1.7.0, celle utilisée par le projet) :
`EnableSpectator()`/`DisableSpectator()` n'apparaissent **pas** sur `SCR_PlayerController` —
pas une fonctionnalité native du jeu de base.

Ce qui existe côté natif :
- **Game Master** a une action "Spectate" (clic droit sur un joueur) — outil admin pour observer
  à la demande, pas un mode que le joueur mort obtient automatiquement à sa mort.
- **`SCR_PlayerController.SetPossessedEntity(IEntity)`** — **testé en jeu, conclusion négative
  définitive.** Premier essai depuis `OMTK_WarmupZoneComponent` (composant serveur, recherche par
  ID via `PlayerManager.GetPlayerController(id)`) : aucun effet visuel. Deuxième essai, seule
  variable changée — `GetGame().GetPlayerController()` **sans argument** (contrôleur local)
  plutôt qu'une recherche serveur par ID : **effet confirmé**, bascule en vue 1ère personne sur
  la cible. Mais le comportement observé disqualifie ce mécanisme pour l'usage visé : c'est une
  **vraie prise de contrôle**, pas juste un changement de vue — le joueur peut déplacer l'entité
  ciblée, et son propre personnage reste planté sur place, sans personne aux commandes. Pour un
  spectateur qui doit observer un **joueur encore vivant** sans lui voler le contrôle, ce
  mécanisme est **inutilisable tel quel**. Piste écartée pour le suivi de joueur ; à chercher
  ailleurs (attachement de caméra à l'entité cible, sans transfert de possession — piste non
  explorée).

Rien d'autre de natif et automatique. Aucun équivalent à EG Spectator livré par Bohemia à ce jour.

### 3ter. Suivi de joueur — piste viable trouvée : `SCR_ManualCamera.AttachTo()`

`SCR_ManualCamera` (la classe du prefab caméra déjà validé, §4.1) expose nativement, confirmé
dans la doc de la branche 1.7.0.54 :

```cpp
void AttachTo(IEntity parent)   // Attach camera to an entity
void Detach()                    // Detach camera from its parent entity
void SetInputEnabled(bool)       // Enable/disable manual input
void Terminate()                 // Destroy the camera proprement (préférable à delete)
```

**Testé en jeu, résultat positif** — remplace avantageusement `SetPossessedEntity` (écarté, §3) :
- l'appel compile et s'exécute (la doc marque ces méthodes `protected`, mais l'appel externe
  passe sans erreur — réserve levée) ;
- la caméra se positionne relativement à la cible (décalage choisi au spawn, ici +3 m en hauteur)
  **sans prise de possession** : le joueur suivi garde entièrement le contrôle de son personnage ;
- les contrôles caméra restent actifs : pivot libre, et déplacement possible ;
- **l'attachement est dynamique** : testé avec une cible déplacée par script (`SetOrigin` répété,
  2 m/s), la caméra suit le mouvement au lieu de rester figée sur la position d'attachement.

Le **décalage au spawn détermine la perspective** : avec +3 m en hauteur, on obtient une vue de
type 3ème personne, propre et exploitable.

**1ère personne — testée, insuffisante, mise en attente.** Attacher à +1,7 m (hauteur des yeux)
fonctionne techniquement mais ne donne pas un rendu correct : la caméra est décalée par rapport
au crâne et on voit l'intérieur du modèle (globes oculaires). Cause de fond : la vraie vue joueur
suit la **tête animée** (respiration, visée, recul, secousses de course), alors qu'un décalage
fixe ne suit que l'origine de l'entité.

Deux pistes écartées ou coûteuses :
- `TrySwitchToControlledEntityCamera()` — **impossible** : la signature ne prend **aucun
  paramètre**, elle ne peut basculer que vers la caméra de sa propre entité contrôlée, jamais
  vers celle d'un autre joueur.
- **Suivi de l'os de la tête** — piste restante et probablement la bonne, mais `AttachTo()` ne
  prend qu'une entité, pas un os : il faudrait recalculer la position de la caméra à chaque frame
  depuis la transformation de l'os via le composant d'animation. Non exploré, fluidité non garantie.

**Décision : 1PP en attente.** La 3ème personne couvre l'essentiel du besoin spectateur (suivre un
joueur, voir ce qu'il fait) et fonctionne proprement. À reprendre seulement si l'OFCRA juge la 1PP
indispensable.

**À vérifier encore** :
- si l'on déplace manuellement la caméra, reste-t-elle attachée, ou le déplacement la détache-t-il
  de fait ?
- `SetInputEnabled(false)` est probablement la clé d'un suivi verrouillé (1PP/3PP) par opposition
  au vol libre — non testé.
- le comportement avec un **vrai joueur** en mouvement (animation de marche réelle) plutôt qu'une
  cible téléportée par script.

### 3bis. Tracé de projectiles — équivalent trouvé, avec une inconnue bloquante

Côté Arma 3, la fonction de référence est `BIS_fnc_traceBullets` (classée "Diagnostic",
s'appuie sur `drawLine3D`, limitée à un tireur et 20 trajectoires par défaut).

Côté Reforger, l'équivalent est l'**API `Shape`** d'Enfusion :

```cpp
Shape.Create(ShapeType.LINE, color, flags, p1, p2);      // ligne simple
Shape.CreateLines(color, flags, points, num);            // trajectoire multi-segments
Shape.CreateArrow(from, to, size, color, flags);         // avec pointe directionnelle
```

Couleur en `0xAARRGGBB`. Deux modes de rendu selon le besoin :
- **`ShapeFlags.ONCE`** — dessiné une frame puis auto-détruit (*"Do not keep pointer to these!!"*),
  patron équivalent à un `drawLine3D` rappelé chaque frame ;
- **persistant** — garder la référence, la trajectoire reste jusqu'à libération. Plus adapté à un
  historique des N derniers tirs, comme le fait `BIS_fnc_traceBullets` avec ses 20 trajectoires.

**⚠️ Inconnue confirmée en jeu — RÉSOLUE** : `Shape` était bien la seule inconnue restée en suspens.
Concernant le prefab caméra (§4.1), **testé en Workbench, confirmé** : spawner
`ManualCameraSpectate.et` via `GetGame().SpawnEntityPrefab(...)` suffit à lui seul — la vue bascule
automatiquement vers cette caméra libre, détachée du personnage, **sans** appel supplémentaire
(pas de `SetPossessedEntity` ni équivalent nécessaire). Ça résout l'inconnue que l'auteur de GRAD
Spectator lui-même n'avait pas éclaircie. Reste à tester si `Shape` (§3bis, tracé de projectiles)
s'affiche bien dans ce même contexte non-Diag — pas encore vérifié séparément.

**Deuxième inconnue** : comment intercepter un tir pour récupérer origine, direction et point
d'impact. Pistes non confirmées : un événement sur `BaseWeaponComponent`, ou le suivi de l'entité
projectile elle-même.

**Voie de repli déjà en place** : `OMTK_KillLoggerComponent` journalise déjà les événements `hit`
(tireur, victime, distance, dégâts). Ajouter les coordonnées d'origine et d'impact à ces lignes
suffirait à alimenter un replay AAR, **sans dépendre de `Shape` du tout** — indépendant du sort de
l'inconnue ci-dessus.

---

## 4. Mods communautaires analysés

### 4.1 GRAD Spectator — code source lu en détail

**`github.com/gruppe-adler/GRAD-Spectator`**, open source (licence APL, pas ND). Analyse
complète du code, fichier par fichier.

**La vraie trouvaille, réutilisable** — `EnableSpectator()` ne construit pas une caméra libre à
partir de rien : il fait apparaître un **prefab de caméra natif de l'éditeur du jeu**, en dehors
de tout contexte d'édition :

```cpp
Resource r = Resource.Load("{E1FF38EC8894C5F3}Prefabs/Editor/Camera/ManualCameraSpectate.et");
IEntity spectator = GetGame().SpawnEntityPrefab(r, GetGame().GetWorld(), params);
```

Trois prefabs de ce type existent nativement (`ManualCameraSpectate`, `ManualCameraStrategy`,
`ManualCameraPhoto`) — l'auteur note lui-même ne pas avoir identifié la différence de
comportement entre les trois hors éditeur. C'est la brique de base de la caméra libre, déjà
fournie par le moteur.

**Mécanisme d'entrées, bon patron mais avec une dette** — le mod ajoute un `ActionContext`
(`SpectatorContext`, priorité 42) et une action `DisableSpectator` (touche END), activés/désactivés
à la demande :

```cpp
inputManager.AddActionListener("DisableSpectator", EActionTrigger.DOWN, DisableSpectator);
inputManager.ActivateContext("SpectatorContext", 3600000);   // durée 0 = désactive
```

Propre en soi, mais ça impose de **remplacer un fichier de config natif du jeu**
(`Configs/System/chimeraInputCommon.conf`). Deux mods qui font ça entrent en conflit — le dernier
chargé écrase l'autre. À documenter comme dette si on reprend l'approche.

**Ce qu'il ne faut PAS reprendre — l'architecture de déclenchement.** À la mort, le mod ne pose
pas simplement une caméra : il **fait respawn un vrai personnage jouable** (faction récupérée
d'avant la mort, loadout « spectateur » dédié, point de spawn nommé "Spectator"), puis pose la
caméra par-dessus 5 secondes plus tard.

```cpp
override void OnPlayerEntityLost_S(int playerId)
{
    //super.OnPlayerEntityLost_S(playerId);
    Spawn(playerId);   // ← fait réapparaître un vrai corps
}
// ...
GetGame().GetCallqueue().CallLater(EnableSpectator, 5000, false, playerId);
```

Problèmes concrets pour l'OFCRA :
1. **Viole la règle « aucun respawn »** — pas au sens gameplay, mais mécaniquement : un corps
   physique réapparaît dans le monde, avec une faction assignée.
2. **Pollue les modules déjà construits** — `OMTK_KillLoggerComponent` s'abonne à
   `OnControllableSpawned` : chaque mort générerait un faux spawn dans les logs, avec une faction
   qui compterait potentiellement dans les logiques de camp (voir `score_board`).
3. **`super.OnPlayerEntityLost_S()` commenté** — le comportement natif de gestion de mort est
   court-circuité plutôt que composé avec. Risque d'interférence avec le reste du GameMode.
4. **`CallLater` de 5 secondes** — contournement assumé (leur propre commentaire : *"could be
   optimised to a test if the spawn is done"*). Fragile sous charge, à 100+ joueurs.

**Régression dans la version recommandée.** Le dépôt contient deux logiques de spawn quasi
identiques. L'ancienne (`GRAD_SpectatorSpawnLogic`) protège contre les déconnexions :

```cpp
protected ref set<int> m_DisconnectingPlayers = new set<int>();
// "Player is disconnecting (and disappearance of controlled entity started this feedback loop).
//  Simply ignore such requests as it would create unwanted entities."
```

Celle que le README dit d'utiliser (`GRAD_MenuSpawnLogic`) **a perdu cette protection** : un
joueur qui se déconnecte déclenche `OnPlayerEntityLost_S` et laisse un corps spectateur fantôme
que personne ne contrôlera jamais. Sur une session OFCRA de plusieurs heures, ça s'accumule.

**Ce qui manque totalement** — le dépôt ne contient que quatre dossiers (`Player`, `Respawn`,
`UserActions`, `Custom`), aucune interface. Le mod couvre uniquement la caméra libre : **rien
pour s'accrocher à un joueur vivant, ni bascule 1PP/3PP, ni carte/liste d'unités.** Toute la
moitié « suivi de joueur » de la spec §2 est absente.

### 4.2 « Spectator Mode » (YouAreBamboozled) — écarté

`reforger.armaplatform.com/workshop/60B52606D9BBF81A`. Fait précisément la partie manquante de
GRAD Spectator (passer d'un joueur à l'autre, observation temps réel), donc **prouve que c'est
faisable sur Reforger**. Mais inexploitable ici :

- **Licence APL-ND** (No Derivatives) — pas de sources, pas de modification, pas de réutilisation
  dans OMTK. Même famille de contrainte que RHS (voir récap `infantry_loadouts` §9).
- **Mauvais déclencheur** — l'entrée se fait via l'action contextuelle "Spectate" depuis Game
  Master : outil admin, pas un mode obtenu automatiquement à la mort.
- **Obsolète** — version de jeu 1.1.0.34, dernière modification mars 2024, contre 1.7.0.54
  aujourd'hui. Deux ans et six versions majeures sans mise à jour.

---

## 5. Analyse — dépendre ou réimplémenter

Le tableau après lecture du code :

| Élément | Verdict |
|---|---|
| Prefab caméra natif `ManualCameraSpectate.et` | **À reprendre** — natif, gratuit, la vraie trouvaille |
| Patron `ActionContext` + touche de sortie | **Bon**, mais impose de remplacer un fichier config natif (conflit possible entre mods) |
| Point d'accroche `OnPlayerEntityLost_S` | **Bon** — c'est le bon hook de mort |
| Respawn d'un corps + `CallLater(5000)` | **À ne pas reprendre** — viole no-respawn, pollue `kill_logger`, fragile sous charge |
| Garde anti-déconnexion | **Seulement dans la version abandonnée** — à réintégrer si on s'inspire du reste |
| Liste de joueurs / bascule 1PP-3PP / carte | **Absent des deux mods exploitables** — entièrement à construire |

**Lecture retenue** : dépendre de GRAD Spectator n'a pas grand intérêt. Il apporte surtout une
information (quel prefab natif utiliser) et un patron d'entrées, mais son architecture centrale
est incompatible avec les contraintes OFCRA, et il lui manque la moitié de la spec. Réécrire
proprement dans OMTK en s'en inspirant semble nettement plus solide — et cohérent avec le reste
du projet.

À confirmer avec l'OFCRA, ce n'est pas une décision purement technique.

---

## 6. Ce qui reste à faire

- ~~Tester en Workbench le prefab `ManualCameraSpectate.et`~~ — **fait, confirmé en jeu** : la
  vue bascule automatiquement vers une caméra libre à l'apparition de l'entité, sans appel
  supplémentaire. Reste à tester les deux autres prefabs (`ManualCameraStrategy`,
  `ManualCameraPhoto`) par comparaison, et le déplacement libre (vitesse, contrôles).
- ~~`SetPossessedEntity()`~~ — **fait, écarté** : transfère le contrôle complet (déplacement inclus),
  pas juste la vue — inutilisable pour suivre un joueur vivant sans lui voler le contrôle.
- **`SCR_ManualCamera.AttachTo()` — piste retenue, largement validée (§3ter)** : attachement sans
  possession **et suivi dynamique d'une cible en mouvement** confirmés en jeu, en 3ème personne.
  Reste à vérifier le comportement en cas de déplacement manuel de la caméra, `SetInputEnabled(false)`
  pour un suivi verrouillé, et le test avec un vrai joueur (pas une cible téléportée).
- **1ère personne — en attente** (§3ter) : décalage fixe insuffisant (on voit l'intérieur du crâne),
  `TrySwitchToControlledEntityCamera()` inutilisable (pas de paramètre de cible). Seule piste
  restante : suivi de l'os de la tête frame par frame. À reprendre uniquement si l'OFCRA le juge
  indispensable.
- Construire le suivi d'unité : sélection, bascule 1PP/3PP, et l'équivalent de la carte custom
  Arma 3 pour changer d'unité suivie (aucun précédent réutilisable trouvé, ni natif ni communautaire).
- Décider du sort des modes de vision (NV/thermique en 1PP) et du mode lampe torche — présents
  dans le mode Arma 3 actuel, à confirmer comme nécessaires ou non côté OFCRA.
- **Tracé de projectiles (touche `O`)** — tester en priorité si l'API `Shape` s'affiche dans un
  client de production (non-Diag), sinon la voie « direct » est morte (voir §3bis). Puis
  identifier le point d'interception des tirs. La voie AAR reste faisable indépendamment via
  `kill_logger`.
- Réintégrer une garde anti-déconnexion (cf. §4.1) dans toute logique branchée sur
  `OnPlayerEntityLost_S`.
- Trancher le déclenchement : caméra seule sans respawn de corps (approche à privilégier,
  fidèle à la règle no-respawn) plutôt que le respawn+caméra de GRAD Spectator.
- Vérifier la cohabitation avec `kill_logger` et `score_board` une fois le mécanisme choisi.

---

## 7. Retombée hors module — à vérifier sur notre propre code

`GRAD_SpectatorUserAction` déclare explicitement :

```cpp
override bool HasLocalEffectOnlyScript() { return true; }
override bool CanBroadcastScript()       { return false; }
```

Nos deux actions (`OMTK_TEST_AdminEndWarmupAction`, `OMTK_AdminEndWarmupAction`) **n'overrident
ni l'un ni l'autre** — elles utilisent donc le comportement par défaut, jamais vérifié. Si le
défaut est « broadcast », `PerformAction()` pourrait s'exécuter sur plusieurs machines et notre
RPC serveur partirait en double. Invisible en test solo (une seule machine), potentiellement
visible en session réelle. **À vérifier.**

---

*Document consolidé après lecture complète du code source de GRAD Spectator et de la doc
officielle Arma 3 EG Spectator. Rien construit ni testé en jeu à ce stade.*
