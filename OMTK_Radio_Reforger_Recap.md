# Récapitulatif — Portage du module `kill_logger` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `kill_logger` d'OMTK (Arma 3, SQF) journalise les hits/kills dans le fichier `.RPT` du serveur, via `onPlayerKilled.sqf`. Sur Reforger, contrairement à `infantry_loadouts` ou `dynamic_startup` (très orientés config/prefabs), `kill_logger` reste majoritairement un module **scripté**, mais avec une API plus riche que SQF.

---

## 2. L'attribution des kills — `DamageManagerComponent` et `Instigator`

- **`DamageManagerComponent`** (et sa version étendue `SCR_DamageManagerComponent`) gère les dégâts sur chaque entité. L'événement **`ShouldOverrideInstigator`** décide, à chaque nouvelle source de dégât, si elle devient le nouvel **instigateur** (celui qui recevra le crédit du kill) — permet de personnaliser l'attribution (ex. dernier coup porté vs dégâts cumulés).
- **`Instigator`** / **`SCR_InstigatorContextData`** portent les informations utiles pour un log détaillé :
  - le **type de contrôle** du tueur et de la victime (IA, Joueur, IA possédée, Game Master/Admin),
  - la **relation** entre eux (même faction ou ennemi),
  - une méthode dédiée pour détecter le **tir ami** (team-killing).
- **`OnDamageStateChanged`** — l'événement déclenché au changement d'état de dégât (ex. passage à "mort"), le point d'accroche naturel pour écrire une entrée de log.

**Point de vigilance (Damage System)** : éviter d'infliger un dégât exactement égal aux PV max pour tuer une entité — mieux vaut un dégât "réaliste" qui reste théoriquement survivable, pour rester compatible avec d'éventuels mods qui changeraient la résistance d'une entité (ex. mod de blindage renforcé).

---

## 3. Écriture du fichier de log — l'API `FileIO` (confirmée dans du code source officiel)

Trouvé dans `SCR_AIDebug.c`, un fichier du jeu lui-même :

```
m_FileHandle = FileIO.OpenFile(logFileName, FileMode.WRITE);
if (!m_FileHandle)
    Print("Échec de création du fichier de log", LogLevel.ERROR);
...
m_FileHandle.Close(); // fermeture propre, typiquement dans OnGameEnd
```

C'est l'équivalent direct de notre écriture dans le `.RPT` : une classe `FileIO`, un mode d'ouverture (`FileMode.WRITE`), une fermeture propre du handle en fin de partie.

**Limite identifiée à vérifier** : sous Arma 3, une API d'extension permettait de communiquer avec du code natif externe pour une persistance style base de données (utilisée par extDB, DayZ, AltisLife). Aucun équivalent direct confirmé n'a été trouvé côté Enfusion à ce jour — `FileIO` couvre l'écriture de fichiers, mais une vraie persistance externe (BDD) semble plus limitée ou différente. **À vérifier concrètement** si l'OFCRA veut un jour des statistiques persistantes cross-sessions plutôt qu'un simple fichier de log par partie.

---

## 4. Références communautaires (mods existants du même type)

| Mod | Ce qu'il fait | Intérêt pour nous |
|---|---|---|
| **KillLog** | Génère un `KillLog.txt` horodaté à chaque kill joueur-vs-joueur, dans le dossier profil serveur | Quasi identique à notre besoin actuel — bonne base de référence |
| **Logging Enhanced (flabby)** | Logs `.json` ou `.txt` au choix, arborescence `ServerProfile/.../ANNÉE/MOIS/JOUR/`, commandes in-game pour activer/désactiver fichier ou console | Architecture plus aboutie que notre `.RPT` plat — modèle possible pour une meilleure organisation |
| **ReforgerJS** | Logging temps réel côté serveur externe (Node.js) : kills/morts (arme, distance), dégâts (avec détection tir ami), connexions joueurs, captures de base, actions Game Master | Plus large que `kill_logger` seul (chevauche aussi `score_board`/`radio_settings`) ; approche externe plutôt que mod pur |

---

## 5. Ce que ça implique pour notre portage

`kill_logger` resterait le module le plus proche en esprit de son équivalent SQF actuel :
1. Un composant scripté s'accroche à `OnDamageStateChanged` (ou équivalent) sur les personnages.
2. Il lit les données de l'`Instigator` (tueur, team-kill ou non, type de contrôle des deux parties).
3. Il écrit une entrée via `FileIO`, potentiellement organisée en `ANNÉE/MOIS/JOUR` façon flabby plutôt qu'un fichier plat.
4. Optionnellement, une donnée de résumé pourrait être répliquée via `[RplProp]` vers le `score_board` (déjà documenté), pour éviter de dupliquer la logique d'attribution des kills entre les deux modules.

---

## 6. Ressources de référence

| Sujet | Lien |
|---|---|
| Système de dégâts (concepts) | `community.bistudio.com/wiki/Arma_Reforger:Damage_System` |
| API `Instigator` / contexte de kill | `community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__InstigatorContextData.html` |
| API `DamageManagerComponent` | `community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__DamageManagerComponent.html` |
| Exemple réel d'usage de `FileIO` | `arexplorer.zeroy.com` → `SCR_AIDebug.c` (Arma Reforger Explorer, source du jeu) |
| Mod KillLog | `reforger.armaplatform.com/workshop/60E28D08A9009D01` |
| Mod Logging Enhanced by flabby | `reforger.armaplatform.com/workshop/6316335D6A19E51C-LoggingEnhancedbyflabby` |
| Discussion forum sur la persistance de données | forums.bohemia.net — sujet "Enforce Script - data persistence" |

---

## 7. Ce qui reste à valider en pratique (Workbench requis)

- Tester concrètement `FileIO.OpenFile`/`Close` dans un composant custom, en dehors du contexte `AI_DEBUG` où on l'a trouvé.
- Confirmer s'il existe une vraie solution de persistance externe (BDD) pour Enfusion, ou si `FileIO` + traitement externe des fichiers est la seule voie actuelle.
- Valider le format d'`Instigator` disponible côté serveur au moment précis de la mort, pour s'assurer que toutes les infos voulues (team-kill, type de contrôle) sont bien accessibles à cet instant.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §7).*
