# Récapitulatif — Portage du module `infantry_loadouts` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `infantry_loadouts` d'OMTK (Arma 3, SQF) équipe automatiquement les unités selon leur classe/faction, en s'appuyant sur `@RHSmod`. Sur Reforger, il n'existe pas de mod RHS équivalent, et le système d'équipement fonctionne de façon radicalement différente : moins de script, beaucoup plus de configuration et de prefabs.

---

## 2. Changement de paradigme le plus important

**Il n'y a plus de "camps" figés en dur** (Bluefor/Redfor/Independent comme sous Arma 3). Chaque faction est une simple chaîne de caractères identifiante, avec des alliances configurables librement (`Friendly Factions Ids`). Notre logique BLUEFOR/REDFOR stricte devra être repensée comme deux factions custom déclarées, pas comme une dichotomie native du moteur.

**La majorité du travail se fait par configuration et héritage de prefabs, pas par script.** Le code Enforce Script n'intervient que pour des besoins spécifiques (labels custom dans l'éditeur, logique métier propre à l'OFCRA).

---

## 3. Les composants natifs qui remplacent notre logique SQF

| Composant | Rôle | Équivalent OMTK actuel |
|---|---|---|
| `SCR_FactionManager` | Gère l'ensemble des factions du jeu | `bluefor_classes.sqf` / `redfor_classes.sqf` |
| `SCR_LoadoutManager` | Instance unique, fournit les loadouts disponibles par indices | Sélection de classe dans `infantry_loadouts` |
| `SCR_RespawnSystemComponent` | Attaché au GameMode ; gère faction/loadout/spawn demandés par chaque joueur | `infantry_loadouts` + partie de `dynamic_startup` |
| `SCR_RespawnComponent` | Attaché au contrôleur du joueur (visible uniquement par son propriétaire) | — (nouveau concept, lié à owner/proxy) |
| `FactionAffiliationComponent` | Faction d'appartenance d'une unité (simple chaîne de caractères) | `setFaction` / classes de camp |
| `BaseLoadoutManagerComponent` | Définit la tenue (casque, veste, pantalon, bottes, gilet, sac à dos — 6 emplacements fixes) | Équipement vestimentaire des classes |
| `SCR_InventoryStorageManagerComponent` | Objets préassignés et leur emplacement de stockage | Ajout d'items via script |
| `CharacterWeaponSlotComponent` / `CharacterGrenadeSlotComponent` | Armes (2 primaires + 1 secondaire) et grenades par défaut | Ajout d'armement via script |
| `SCR_CommunicationSoundComponent` | Protocole radio utilisé (ex. variante RU vs EN) | Config radio par camp |

---

## 4. Construire une faction custom (BLUEFOR/REDFOR OFCRA)

Processus type (d'après le tutoriel *Faction Creation*) :

1. **Retexturer/créer l'équipement** si besoin (nouveaux prefabs de vêtements, matériaux dupliqués).
2. **Créer un personnage de base** par faction (`Character_SampleFactionREDFOR_Base.et`), avec la configuration commune : faction, protocole radio, équipement partagé.
3. **Créer les classes par héritage** à partir de ce personnage de base : fusilier, médecin, mitrailleur, chef d'escouade... — chaque classe ne modifie que ce qui la distingue (arme, gilet, sac).
4. **Créer la config de faction** (dupliquer/hériter d'une config existante comme `USSR.conf` ou `OPFOR.conf`) avec : clé unique, couleur, drapeau, si elle est jouable, liste d'Entity Catalogs (contenu d'arsenal), callsigns, rangs.
5. **Créer les groupes** par héritage de `Group_Base.et` (escouade, équipe de feu...).
6. **Intégrer dans l'éditeur** (Game Master) : label custom si besoin, enregistrement des personnages/groupes/véhicules via le plugin *Register Placeable Entities*.
7. **Intégrer dans Conflict** (mode persistant, le plus proche de notre usage OFCRA) : config de faction étendue avec rangs, radio (fréquence/clé par défaut), groupes IA par défaut, véhicules disponibles, structures constructibles.

---

## 5. Contenu d'arsenal (armes, tenues disponibles)

- Le contenu des caisses d'arsenal est défini par des **Entity Catalogs**, rattachés directement à la config de faction — le contenu est donc intrinsèquement lié à la faction, pas une liste globale comme en SQF actuellement.
- Pour ajouter du contenu à une faction existante : **override** de sa config Entity Catalog (mots-clés `modded`/`override`, même logique que pour `score_board`).
- Pour un contenu indépendant des factions vanilla (cas probable pour l'OFCRA) : **Arsenal Override Config** via `SCR_ArsenalComponent`, sans dépendre des catalogues existants.

---

## 6. Points de spawn (lien avec `dynamic_startup`)

- Créés par héritage de `SpawnPoint_Base.et`, liés à une faction via un simple paramètre `Faction`.
- Une variante "éditable" (`E_SpawnPoint_*.et`) est nécessaire pour être utilisable dans Game Master, avec son propre enregistrement (registre de prefabs plaçables).

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Création de faction (tutoriel complet) | `community.bistudio.com/wiki/Arma_Reforger:Faction_Creation` |
| Sample de code réel | `github.com/BohemiaInteractive/Arma-Reforger-Samples` → `SampleMod_NewFaction` |
| Setup général de game mode | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |
| Config d'arme/arsenal | `community.bistudio.com/wiki/Arma_Reforger:Weapon_Creation/Prefab_Configuration` |
| Assets/Tutorials (liste, dont Faction Creation) | `community.bistudio.com/wiki/Category:Arma_Reforger/Modding/Assets/Tutorials` |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Construire un premier personnage de base OFCRA complet et vérifier le rendu des 6 emplacements d'équipement.
- Tester une config de faction custom minimale (non-Conflict d'abord) avant de passer à l'intégration Conflict complète (plus lourde).
- Vérifier si un vrai mod RHS-like existe côté Workshop pour se rapprocher visuellement de l'équipement actuel, plutôt que tout retexturer depuis les assets vanilla.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*
