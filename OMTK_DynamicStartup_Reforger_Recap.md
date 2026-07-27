# Récapitulatif — Portage du module `vehicles_cargos` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `vehicles_cargos` d'OMTK (Arma 3, SQF) charge cargos et loadouts de véhicules directement dans les fichiers `mission.sqm`, via l'utilitaire externe `OMTK-loadouts`. Sur Reforger, la gestion du chargement des véhicules fait partie intégrante du système d'inventaire natif d'Enfusion.

---

## 2. Le composant central : `SCR_VehicleInventoryStorageManagerComponent`

C'est l'équivalent direct de notre `vehicles_cargos` :
- Attaché au véhicule, il hérite d'un système d'inventaire générique (`ScriptedInventoryStorageManagerComponent`).
- Sa méthode clé, **`FillInitialStorages`**, définit quels espaces de stockage le véhicule possède **au moment de son initialisation** — c'est ici qu'on définirait le chargement de départ d'un véhicule OFCRA (caisses, munitions, équipement).
- Elle s'appuie sur un **`SlotManagerComponent`** pour localiser les emplacements physiques de stockage sur le véhicule (coffre, plateau, remorque...).
- Point technique : la méthode vérifie `GetGame().InPlayMode()` avant de peupler le stockage — un véhicule posé dans l'éditeur n'a pas besoin d'un inventaire rempli tant qu'on n'est pas réellement en partie.

---

## 3. Le système d'inventaire général (au-delà des véhicules)

- **`SCR_InventoryStorageManagerComponent`** — le gestionnaire d'inventaire de base (utilisé aussi pour les personnages via `SCR_CharacterInventoryStorageComponent`) : déplacement/insertion/retrait d'objets, recherche de la prochaine arme disponible, calcul de ce qu'ajoute un réapprovisionnement — exactement la logique de "caisse de ravitaillement" qu'on programmait à la main en SQF.
- **`SCR_UniversalInventoryStorageComponent`** — un stockage à emplacements dynamiques qui accepte n'importe quelle entité, avec gestion automatique de la visibilité des objets insérés/retirés (utile pour un coffre générique sans contrainte de type d'objet).
- **Propriété "Save In Loadout"** (ajoutée en 1.1) sur le stockage de base — détermine si le contenu doit être sauvegardé dans le loadout du véhicule ; pertinent si on veut des chargements persistants entre sessions plutôt que réinitialisés à chaque partie.

---

## 4. Comportement natif utile trouvé dans le changelog 1.1

Déplacer un objet directement vers un véhicule pendant que son coffre est plein et ouvert le fait automatiquement basculer vers le cargo si de la place y est disponible — une répartition intelligente entre plusieurs espaces de stockage déjà gérée par le moteur, à ne pas recoder.

---

## 5. Ce que ça implique pour notre portage

`vehicles_cargos` deviendrait un **composant modded** (même logique que pour `score_board`) plutôt qu'un système entièrement nouveau :
1. Surcharger `FillInitialStorages` sur nos prefabs de véhicules OFCRA pour définir le chargement de départ propre à chaque type de véhicule/faction.
2. S'appuyer sur les storages génériques déjà fournis par le moteur (`SCR_UniversalInventoryStorageComponent`) plutôt que de réinventer un système de cargo depuis zéro.
3. Décider, véhicule par véhicule, si le contenu doit persister via "Save In Loadout" ou être réinitialisé à chaque session.

Contrairement à `OMTK-loadouts` (utilitaire externe qui édite directement `mission.sqm`), le chargement de véhicule sous Reforger se définit **dans le prefab lui-même**, pas dans un outil externe séparé — encore une fois, moins de script autonome, plus de configuration intégrée aux prefabs.

---

## 6. Ressources de référence

| Sujet | Lien |
|---|---|
| `SCR_VehicleInventoryStorageManagerComponent` | `community.bistudio.com/wikidata/.../interfaceSCR__VehicleInventoryStorageManagerComponent.html` |
| Code source réel | `arexplorer.zeroy.com` → `SCR_VehicleInventoryStorageManagerComponent.c` |
| `SCR_InventoryStorageManagerComponent` (base) | `community.bistudio.com/wikidata/.../interfaceSCR__InventoryStorageManagerComponent.html` |
| `SCR_UniversalInventoryStorageComponent` | `community.bistudio.com/wikidata/.../interfaceSCR__UniversalInventoryStorageComponent.html` |
| Changelog 1.1 (Save In Loadout, comportement cargo) | `reforger.armaplatform.com/news/update-march-13-2024` |

---

## 7. Ce qui reste à valider en pratique (Workbench requis)

- Construire un premier véhicule OFCRA avec un `FillInitialStorages` custom et vérifier le chargement au lancement.
- Tester le comportement "Save In Loadout" sur une session longue pour voir si la persistance correspond à l'usage voulu par l'OFCRA.
- Vérifier la compatibilité entre nos futurs prefabs de véhicules custom et le `SlotManagerComponent` natif (emplacements de stockage bien reconnus).

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §7).*
