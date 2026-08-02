# Récapitulatif — Portage du module `test_mode` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `test_mode` d'OMTK (Arma 3, SQF) facilite les tests d'une mission en développement. Sur Reforger, il n'existe pas de "mode test" natif unique équivalent — la fonction se répartit entre plusieurs outils déjà croisés dans les modules précédents.

---

## 2. Les briques natives qui composent un vrai "mode test"

- **Debug Areas / Core Areas** (Scenario Framework, déjà documentés dans le récap [`dynamic_startup`](OMTK_DynamicStartup_Reforger_Recap.md)) :
  - **Debug Areas** — force certaines zones (et éventuellement leur LayerTask) à apparaître systématiquement, plutôt que de dépendre de la randomisation, pour reproduire un cas de test précis.
  - **Core Areas** — zones essentielles qui doivent toujours apparaître, indépendamment du tirage aléatoire des Debug Areas.
  - Le Scenario Framework permet aussi d'activer des **jeux d'actions de debug prédéfinis**, exécutables à la volée pendant le test.

- **Exécutables "Diag"** (`ArmaReforgerSteamDiag.exe`, `ArmaReforgerServerDiag.exe`) — versions spéciales du client/serveur qui peuvent se connecter au Workbench pour du debug en direct, avec accès à des menus développeur normalement inaccessibles (Diag Menu). Un client/serveur non-Diag ne peut pas se connecter à ces outils, et inversement.

- **Remote Console** — permet d'exécuter du code Enforce Script directement (`cPrint`, `PrintFormat`, etc.) sans charger un monde complet, pour tester une portion de logique isolément avant de l'intégrer dans un composant complet.

---

## 3. Ce que ça implique pour notre portage

`test_mode` ne serait **pas un module en soi** à recréer, mais une combinaison de pratiques et d'outils déjà natifs :
1. Utiliser les **Debug Areas** pour figer un scénario de test reproductible (au lieu de dépendre de la randomisation à chaque lancement).
2. Utiliser les **exécutables Diag** pour déboguer en conditions quasi réelles (client + serveur connectés au Workbench).
3. Utiliser la **Remote Console** pour valider rapidement un bout de logique (ex. une méthode d'un composant `OMTK_*`) avant de le tester en jeu.

Concrètement, ça veut dire que l'équivalent OFCRA de `test_mode` serait probablement un **document de procédure** ("comment tester un scénario OMTK_Reforger") plutôt qu'un composant de script à écrire — un changement de nature par rapport à l'ancien module SQF.

---

## 4. Ressources de référence

| Sujet | Lien |
|---|---|
| Scenario Framework (Debug/Core Areas) | `community.bistudio.com/wiki/Arma_Reforger:Scenario_Framework` |
| Exécutables Diag et debug | `community.bistudio.com/wiki/Arma_Reforger:Development_Executables` |
| Changements scripting 1.1 (mention des exécutables Diag) | `reforger.armaplatform.com/news/modding-update-scripting-1-1` |
| Scripting First Steps (Remote Console) | `community.bistudio.com/wiki/Arma_Reforger:Scripting_First_Steps` |
| Explorateur de code source du jeu | `arexplorer.zeroy.com` — utile pour vérifier un nom de classe/méthode avant de coder |

---

## 5. Ce qui reste à valider en pratique (Workbench requis)

- Tester concrètement la connexion d'un exécutable Diag au Workbench pour un scénario OMTK.
- Construire un premier jeu de Debug Areas reproductible pour un scénario type OFCRA.
- Documenter une procédure de test standard pour l'équipe (remplaçant à terme ce document technique par un guide pratique une fois testé en conditions réelles).

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §5).*
