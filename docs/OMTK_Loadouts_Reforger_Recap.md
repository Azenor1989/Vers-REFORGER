# Récapitulatif — Portage du module `infantry_loadouts` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `infantry_loadouts` d'OMTK (Arma 3, SQF) équipe automatiquement les unités selon leur classe/faction, en s'appuyant sur `@RHSmod`. Sur Reforger, RHS existe sous le nom **RHS: Status Quo** (voir §9), mais le système d'équipement fonctionne de façon radicalement différente : moins de script, beaucoup plus de configuration et de prefabs.

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
| RHS: Status Quo — documentation | `docs.rhsmods.org/rhs-status-quo-user-documentation/arma-reforger/rhs-status-quo` |
| RHS: Status Quo — EULA | `docs.rhsmods.org/rhs-status-quo-user-documentation/arma-reforger/rhs-status-quo/eula` |
| RHS: Status Quo — Workshop | `reforger.armaplatform.com/workshop/595F2BF2F44836FB-RHS-StatusQuo` |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Construire un premier personnage de base OFCRA complet et vérifier le rendu des 6 emplacements d'équipement.
- Tester une config de faction custom minimale (non-Conflict d'abord) avant de passer à l'intégration Conflict complète (plus lourde).
- Évaluer la couverture réelle des assets RHS: Status Quo par rapport aux besoins OFCRA (voir §9).

---

## 9. RHS sur Reforger : RHS: Status Quo

RHS existe bien sur Reforger, sous le nom **RHS: Status Quo**, disponible sur le Workshop et activement développé (changelog actif à la mi-2026, version 0.9).

**Différences avec le RHS d'Arma 3 :**
- Cadre **contemporain** (2000 à aujourd'hui, cœur vers 2017) plutôt que Guerre froide.
- **Toutes les factions dans un seul mod**, au lieu des paquets séparés RHSUSAF / RHSAFRF / RHSGREF / RHSSAF.
- Factions principales : **AFRF** (Fédération de Russie) et **USAF** (forces américaines) ; MSV et USMC sont devenues des branches lors de la refonte du système de factions en 0.9.
- Deux canaux de distribution : une version de développement continu et une version stable.
- Maturité : la 0.9 n'est pas complète, et l'équipe annonce viser la qualité plutôt que le volume — le catalogue d'assets ne rattrapera pas celui d'Arma 3 avant longtemps, si jamais.

**Précédent utile :** des mods communautaires définissent déjà des factions BLUFOR/REDFOR/INDFOR par-dessus RHS: Status Quo (par exemple *Freedom Fighters - RHS Faction*). Le schéma que vise l'OFCRA a donc des implémentations qui tournent, à étudier avant de partir de zéro.

### Contraintes de licence (EULA RHS)

RHS: Status Quo est sous **Creative Commons BY-NC-ND 4.0**. Le contenu dérivé englobe tout mod de dépendance qui utilise ou modifie les fichiers RHS — **nos configs de faction en feraient partie dès qu'elles référencent des classes RHS**. Ces mods sont *tolérés*, pas autorisés de droit, sous réserve de règles strictes :

- **Publication publique obligatoire** sur le Workshop ; les mods dérivés non listés ne sont pas admis.
- **Licence non-dérivative imposée** sur le mod dérivé (l'APL-ND est citée en exemple) — ce qui exclut une licence permissive.
- **Lien vers la page EULA de RHS** dans la description du mod.
- **Aucune monétisation** : sont visés notamment les accès prioritaires payants aux serveurs, les avantages en jeu vendus, et les objectifs de dons donnant droit à des perks. À examiner si l'OFCRA a un système de dons lié à des contreparties en jeu.
- **Distribution uniquement via le Workshop officiel** ; interdiction d'embarquer les fichiers RHS dans son propre paquet.
- **Interdiction de représenter les conflits en cours** (Ukraine, Israël-Palestine), marquages Z/O/V compris — contrainte de design pour les scénarios.

RHS maintient également une liste de groupes exclus de tout usage de ses mods. À vérifier avant d'engager le projet.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*
