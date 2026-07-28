# Récapitulatif — Portage du module `score_board` (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `score_board` d'OMTK (Arma 3, SQF) gère aujourd'hui :
- la définition des objectifs par camp (`OMTK_SB_LIST_OBJECTIFS`),
- le calcul du score en temps réel,
- l'affichage du scoreboard en fin de mission (drapeaux BLUEFOR/REDFOR personnalisables).

Sur Arma Reforger, il n'existe pas de "module" équivalent à copier : la logique doit s'appuyer sur des systèmes natifs du moteur Enfusion (scoring, réplication, UI, écran de fin), **modifiés** plutôt que réécrits de zéro.

---

## 2. Stratégie retenue : modder l'existant, pas réinventer

Plutôt qu'un composant `OMTK_ScoreBoardComponent.c` entièrement custom, la bonne approche est de **modder** le système natif `SCR_ScoringSystemComponent` / `SCR_BaseScoringSystemComponent` via les mots-clés Enforce Script :

- `modded` — modifie une classe existante sans toucher au fichier original.
- `override` — remplace le comportement d'une méthode précise (ex. `CalculateScore`).
- `super` — appelle le code d'origine de la méthode remplacée (pour *ajouter* un comportement plutôt que le remplacer entièrement).

**Convention de nommage** : remplacer le préfixe `SCR_` par un tag propre au mod (ex. `OMTK_ScoringSystemComponent.c`) pour éviter les conflits avec d'autres mods.

**Référence** : `SampleMod_ModdedScript` (dépôt officiel `BohemiaInteractive/Arma-Reforger-Samples`) — exemple concret : changer les multiplicateurs de score pour la mort/suicide, et jouer un son au suicide.

---

## 3. Synchronisation du score en temps réel (réplication)

Le calcul du score se fait uniquement **côté serveur** (l'autorité). Rien n'est synchronisé automatiquement — il faut le déclarer explicitement.

**Mécanisme** :
```
Serveur : AddSuicide() (code modded) → variable [RplProp] → Replication.BumpMe()
   ↓ (diffusion automatique par le moteur, aucun code réseau à écrire)
Client  : callback onRplName déclenché → widget HUD mis à jour
```

**Concepts clés** :
- **Autorité** : la version de référence d'une entité, toujours sur le serveur.
- **Proxy** : copie locale côté client, reçoit les mises à jour mais ne peut rien envoyer.
- **Owner** : un client avec des droits élevés sur une entité précise (peut faire des `RpcAsk_` vers l'autorité).
- `[RplProp(onRplName: "MaMethode", condition: RplCondition.X)]` — décore une variable pour la synchroniser automatiquement.
- `RplRpc` — pour déclencher une **action** ponctuelle (ex. notification "Objectif capturé !"), avec la convention `RpcAsk_` (client → serveur) / `RpcDo_` (serveur → client).
- Conditions utiles : `RplCondition.NoOwner` (tout le monde sauf le propriétaire), `RplCondition.OwnerOnly` (uniquement le propriétaire) — pour un scoreboard **global**, ne pas utiliser `OwnerOnly`.

---

## 4. Affichage en direct (HUD)

- L'UI Reforger est faite de **Widgets** organisés en hiérarchies dans des fichiers `.layout`.
- Un **Script Component** (`ScriptedWidgetComponent`) attaché au widget du scoreboard lit la valeur synchronisée et met à jour l'affichage via `Widget.FindAnyWidget()`.
- **Bonne pratique** : un seul composant "source de vérité" à la racine du prefab HUD — ne jamais modifier un widget depuis l'extérieur de son propre prefab.
- **HUD Slotting** : le système redistribue automatiquement les éléments HUD compatibles vers les menus (carte, inventaire) ou les cache — aucun code supplémentaire à écrire pour ces cas.

---

## 5. Écran de fin de mission (l'équivalent du scoreboard final SQF)

- Le **End Screen** ("Game Over screen") se configure dans l'éditeur ; pour du full custom, hériter de **`SCR_BaseGameOverScreenInfo`** et surcharger **`SCR_GameOverScreenContentUIComponent`**.
- Le layout custom (nos drapeaux BLUEFOR/REDFOR, notre mise en page) se branche via la variable `Game Over Content Layout`.
- Point d'attention : gérer la compatibilité **Game Master**, où plusieurs factions gagnantes sont possibles (contrairement à notre BLUEFOR/REDFOR strict actuel).
- **`SCR_GameModeEndData`** : structure sérialisable **déjà répliquée nativement** à tous les clients (faction gagnante, joueur gagnant...). Pas besoin de réinventer la réplication du résultat final — juste la remplir avec nos données OFCRA.

Les popups intermédiaires (ex. un message "Domination bonus" comme dans notre `OMTK_SB_LIST_OBJECTIFS` actuel) peuvent utiliser le système générique de **Configurable Dialog** (`SCR_ConfigurableDialogUi`, config `SCR_ConfigurableDialogUiPresets`), qui évite d'écrire un layout complet à la main pour chaque petit message.

---

## 6. Chaîne complète

```
Pendant la partie :
  AddSuicide() (modded) → [RplProp] score → BumpMe()
      → onRplName (client) → widget HUD mis à jour en direct

Fin de partie :
  Score final → SCR_GameModeEndData (répliqué nativement)
      → SCR_GameOverScreenContentUIComponent (notre logique custom)
      → notre layout final (drapeaux, mise en page OFCRA)
```

---

## 7. Ressources de référence

| Sujet | Lien |
|---|---|
| Réplication (concepts) | `community.bistudio.com/wiki/Arma_Reforger:Multiplayer_Scripting` |
| Modding par override/super | `community.bistudio.com/wiki/Arma_Reforger:Scripting_Modding` |
| Sample de code réel | `github.com/BohemiaInteractive/Arma-Reforger-Samples` → `SampleMod_ModdedScript` |
| UI / HUD (concepts) | Modding Boot Camp #4 — `reforger.armaplatform.com/news/modding-boot-camp-4-user-interface-and-hud` |
| Dialogues génériques | `community.bistudio.com/wiki/Arma_Reforger:Dialog_Configuration_Tutorial` |
| Écran de fin de mission | `community.bistudio.com/wiki/Arma_Reforger:End_Screen_Creation` |
| Setup général de game mode | `community.bistudio.com/wiki/Arma_Reforger:General_Game_Mode_Setup` |

---

## 8. Ce qui reste à valider en pratique (Workbench requis)

- Compiler et tester réellement un `modded SCR_ScoringSystemComponent` (les noms exacts de méthodes/propriétés peuvent avoir évolué depuis la doc consultée).
- Vérifier le comportement exact de `RplCondition` sur un scoreboard visible par tous les joueurs.
- Construire un premier prototype minimal : score qui s'incrémente et se synchronise, avant d'ajouter l'affichage HUD puis l'écran de fin.

---

*Document généré à partir des recherches menées en session — à vérifier et corriger contre le comportement réel du Workbench (voir §8).*

---

## 9. Contexte OFCRA : structure réelle des objectifs

Le barème d'une mission OFCRA est plus riche que le simple couple objectif/point. Sur une mission type de 90 minutes, on trouve :

**Trois familles d'objectifs**
- **Communs aux deux camps** — typiquement le contrôle de zones (une zone d'éoliennes, une usine, une ville), chacune valant 2 à 3 points.
- **Propres au BLUFOR** — défendre des installations, empêcher une action adverse.
- **Propres au REDFOR** — le miroir : détruire ces mêmes installations, réussir l'action.

**Des points différenciés** : chaque objectif porte sa propre valeur, y compris à l'intérieur d'un lot (« 3 installations à défendre, 1 point chacune »).

**Des verrouillages échelonnés** : les objectifs ne se ferment pas tous à la fin. Sur une mission de 90 minutes, certains se verrouillent à 60 minutes, d'autres à 75, les derniers seulement à l'expiration du temps. Une fois verrouillé, l'état d'un objectif est figé quoi qu'il arrive ensuite sur le terrain.

**La survie comme objectif** : « le chef de camp a survécu » vaut des points, pour chaque camp.

**Ce que ça implique pour l'implémentation :**

- La notion de **faction propriétaire** d'une tâche (`SCR_TaskSystem`, voir récap [`dynamic_startup`](OMTK_DynamicStartup_Reforger_Recap.md), §4) couvre nativement la distinction commun / BLUFOR / REDFOR.
- Le **verrouillage horodaté par objectif** n'a en revanche pas d'équivalent évident : c'est probablement la part la plus spécifique à écrire, sous forme d'un état « figé » atteint à un instant configuré par objectif, et non à la fin de partie.
- Le score final doit distinguer **objectif rempli** et **objectif verrouillé sur un échec**, les deux étant des états terminaux différents en cours de partie.
