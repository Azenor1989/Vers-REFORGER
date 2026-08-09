# Récapitulatif — Portage du module `kill_logger` et de la chaîne AAR (OMTK) vers Arma Reforger

*Document de synthèse — projet OFCRA, migration OMTK → Arma Reforger*

---

## 1. Contexte

Le module `kill_logger` d'OMTK (Arma 3, SQF) journalise les hits et kills dans le fichier `.RPT` du serveur, via `onPlayerKilled.sqf`.

**Ce n'est pas un module de log, c'est l'amont d'une chaîne complète.** À l'OFCRA il alimente :

- **OCAP**, l'outil de capture qui enregistre les missions et permet de les rejouer ;
- le **centre de statistiques** (`aar.ofcra.org/stats`), alimenté depuis 2017 : plus de 900 missions, plusieurs milliers de joueurs, des dizaines de milliers de kills, avec des vues par mission, joueur, carte et arme.

Migrer `kill_logger` seul ne suffit donc pas : si la chaîne n'est pas portée ou remplacée, l'OFCRA perd son historique vivant et ses analyses d'après-partie — ce qui pèse probablement plus lourd, pour les membres, que n'importe quel module de mission. **À traiter comme un chantier à part entière**, au même titre que le plafond de joueurs.

---

## 2. Les mécanismes natifs de Reforger

### 2.1 Attribution des kills

- **`DamageManagerComponent`** (et `SCR_DamageManagerComponent`) gère les dégâts sur chaque entité. L'événement **`ShouldOverrideInstigator`** décide, à chaque nouvelle source de dégât, si elle devient le nouvel instigateur — permet de personnaliser l'attribution (dernier coup porté vs dégâts cumulés).
- **`OnDamageStateChanged`** — déclenché au changement d'état (passage à « mort »), point d'accroche naturel.
- Le GameMode expose des événements dédiés recevant directement le contexte : entité contrôlable détruite, et joueur tué.

### 2.2 `SCR_InstigatorContextData` — nettement plus riche que le SQF

| Donnée | Détail |
|---|---|
| Type de contrôle | Tueur **et** victime : IA, joueur, IA possédée, Game Master/admin |
| Identifiant joueur | Entier stable — **remplace la bidouille `OMTK_ID`** (sous Arma 3, le nom de variable d'une unité humaine est écrasé par le pseudo du joueur) |
| Entité tueur | L'entité elle-même, pas seulement son nom |
| Type d'instigateur | Distingue tir direct, mine, véhicule |
| Relation victime/tueur | Calculée par le moteur en tenant compte des types de contrôle, de la faction et du statut GM ; cas testables (« tué par un allié », « tué par un ennemi ») |
| Tir ami | Méthode dédiée, pas une déduction |
| **Déguisement** | Type de déguisement du tueur **et** de la victime — l'écart entre faction *perçue* et faction *réelle* |
| Crime de guerre | Le moteur attache des sanctions possibles au kill commis sous uniforme adverse |

**Trois de ces données n'ont aucun équivalent SQF et méritent d'être exploitées :**

- Le **déguisement** est directement pertinent en milsim : l'infiltration en tenue ennemie est suivie nativement. Si l'OFCRA veut en faire une règle — interdire ou pénaliser — la donnée existe.
- Le **Game Master comme type distinct** permet de filtrer les interventions d'admin hors des statistiques.
- L'**IA possédée**, distincte de l'IA : un admin prenant le contrôle d'un civil ne polluera pas les chiffres.

### 2.3 Écriture — l'API `FileIO`

Confirmée dans `SCR_AIDebug.c`, un fichier du jeu lui-même :

```
m_FileHandle = FileIO.OpenFile(logFileName, FileMode.WRITE);
if (!m_FileHandle)
    Print("Échec de création du fichier de log", LogLevel.ERROR);
...
m_FileHandle.Close(); // typiquement dans OnGameEnd
```

**Limite à vérifier** : sous Arma 3, une API d'extension permettait de dialoguer avec du code natif externe pour une persistance en base de données (extDB, DayZ, AltisLife). Aucun équivalent confirmé côté Enfusion. `FileIO` couvre l'écriture de fichiers ; une persistance externe reste à explorer.

---

## 3. Principe de conception : événements structurés

### 3.1 Le problème du log en phrases

Le format actuel écrit une ligne lisible par un humain :

```
21:47:03 [OMTK] INFO: Dupont killed Martin
```

Pour en tirer des statistiques, un programme doit **relire cette phrase et la découper**. Trois faiblesses :

- **Ça casse en silence.** Une reformulation du message, ou un pseudo contenant le mot « killed », et le parseur se trompe sans que personne ne le voie.
- **Ça perd de l'information.** La phrase ne contient que ce que son auteur a pensé à écrire. La distance ou l'arme, si elles n'y sont pas, ne seront jamais récupérables pour les missions passées.
- **Ça oblige à trier.** Les lignes sont mêlées aux erreurs du moteur dans le `.RPT`.

### 3.2 La forme cible

Un enregistrement à champs nommés, une ligne par événement :

```json
{"type":"kill","t":2823,"tueur":4417,"victime":2091,"arme":"...","distance":142,"tir_ami":false}
```

Aucun programme n'a besoin de deviner. On ajoute un champ l'an prochain, rien de ce qui existait ne casse.

**Pourquoi maintenant plutôt qu'après** : au démarrage, écrire une ligne structurée ne coûte pas plus cher qu'écrire une phrase. Une fois la chaîne de statistiques bâtie par-dessus des phrases, changer de format oblige à maintenir les deux, réécrire tout l'aval, ou accepter une rupture d'historique.

Rien n'empêche de conserver une ligne lisible **en parallèle**, pour le débogage en direct. Les deux ne s'excluent pas : la version machine est la source, la version humaine le confort.

**Précédent** : le mod communautaire *Logging Enhanced* propose déjà `.json` ou `.txt` au choix, avec une arborescence `ANNÉE/MOIS/JOUR/`.

### 3.3 Deux consommateurs, deux formes

| Consommateur | Besoin | Forme |
|---|---|---|
| **OCAP** (replay) | Continuité — sans échantillons réguliers, pas de rejeu | Série temporelle : une position par entité à intervalle fixe |
| **Statistiques** | Faits datés, typés, agrégeables | Flux d'événements |

Une seule chaîne peut produire les deux, mais **penser aux seuls événements interdit le replay**, et penser aux seuls échantillons donne des statistiques approximatives.

---

## 4. Catalogue d'événements

### 4.1 Ce qui manque aujourd'hui, et qui coûte le moins cher à ajouter

Le journal actuel n'enregistre **que les morts**. L'horodatage est celui du décès ; la dernière ligne est le dernier mort, **pas la fin de la mission**. Conséquence : un journal de morts ne sait pas distinguer « rien ne s'est passé » de « rien n'a été enregistré ». Sept minutes de silence peuvent être un anéantissement, un cessez-le-feu, un repositionnement ou un serveur planté.

Trois événements manquent, prioritaires :

- **Début de mission** — aujourd'hui déduit du premier mort.
- **Fin de mission**, avec sa cause (temps écoulé, anéantissement, arrêt admin) — aujourd'hui inconnue.
- **Changement d'état d'objectif** — aucune trace.

### 4.2 Le socle

| Événement | Contenu |
|---|---|
| `mission_start` | Nom, carte, auteur, liste de mods, factions, effectif par camp, barème d'objectifs |
| `mission_end` | Cause, scores finaux, faction gagnante, état de chaque objectif |
| `player_join` / `player_leave` | Identifiant, camp, escouade, rôle, créneau |
| `damage` | Auteur, cible, arme, distance, zone touchée, tir ami, déguisement |
| `kill` | Contexte complet (§2.2) |
| `vehicle` | Montée, descente, destruction |
| `objective` | Identifiant, ancien état, nouvel état, faction, points |
| `snapshot` | Instantané périodique — positions et effectifs |

**Positions** : un échantillon par seconde suffit pour de l'infanterie. À 128 joueurs sur 90 minutes, environ 700 000 enregistrements par mission — parfaitement gérable.

### 4.3 Ce qui existe déjà et doit être préservé

Le journal actuel enregistre **plus que ce que laissait supposer le format `.RPT`** : chaque ligne de kill porte l'heure, le tueur, la victime, **l'arme nommée** (« Mortar 82 mm », « M240G », « RPG-7V2 »), **la distance en mètres** et un indicateur de tir ami. La fiche de mission porte en outre des **identifiants joueur stables** (entiers), les chefs de camp, les rôles complets et le nombre de tirs par joueur.

Autrement dit, une partie de la richesse visée existe déjà côté Arma 3 : le portage doit au minimum la reproduire, pas la redécouvrir. C'est aussi la liste de champs à retrouver côté Reforger (voir §10).

*Point confirmé : les kills attribués à un joueur avec une arme du camp adverse (un fusilier BLUFOR crédité d'un M240G ou d'un RPG-7V2) ne sont pas une erreur — c'est du ramassage sur cadavre. Le champ enregistre donc **l'arme réellement employée**, pas l'arme de dotation. C'est le bon comportement, et il faudra le reproduire : côté Reforger, l'arme doit être lue sur l'entité au moment du tir, pas déduite du rôle.*

### 4.4 Trois règles

- **Identifiants stables, jamais des noms** — joueur, entité, escouade, objectif. Un pseudo change, un identifiant non ; c'est ce qui permet de comparer neuf ans de missions.
- **Temps de mission monotone en secondes depuis le départ**, pas une horloge murale. Le replay comme la comparaison inter-missions en dépendent. *(Aujourd'hui l'horodatage est à la minute : suffisant pour des tranches de quinze minutes, insuffisant pour analyser un engagement.)*
- **Enregistrer les changements, plus un instantané périodique.** Sans pouls régulier, l'absence d'événement est illisible.

---

## 5. Métriques de mission

Le modèle mental est celui des statistiques de football : le score est la conclusion, pas l'analyse.

### 5.1 La métrique phare — la ligne de front dans le temps

Minute par minute, la frontière entre les deux forces (au plus simple, le milieu entre les barycentres, projeté sur l'axe de progression). **Une seule courbe**, qui se lit en une seconde et se compare d'une mission à l'autre : pente régulière pour une percée, oscillation autour du centre pour une mission équilibrée, chute brutale pour une mission mal calibrée.

### 5.2 Les suivantes

- **Taux d'échange dans le temps** — pertes infligées sur pertes subies, par tranche. C'est ce qui distingue « a bien défendu » de « a été submergé ».
- **Phases** — approche, contact, décision. Le premier contact et le taux de pertes suffisent à les découper automatiquement ; toutes les autres métriques gagnent à être données par phase.
- **Réseau d'escouades** — équivalent du réseau de passes : lesquelles sont restées à portée de soutien mutuel, lesquelles ont opéré isolées.
- **Carte de chaleur par phase** — pas la mission entière : les axes de l'approche ne sont pas ceux du combat. Le retour le plus utile pour les concepteurs.
- **Cohésion d'escouade** — distance moyenne entre membres au fil du temps.
- **Distribution des distances d'engagement** — quel type de combat la mission a réellement produit.
- **Temps de survie** — sans respawn, c'est la vraie monnaie. Le ratio kills/morts n'a presque aucun sens ici : on meurt une fois.
- **Tir ami** — indicateur natif désormais.

### 5.3 Signatures tactiques

Certaines doctrines laissent une trace mesurable :

- Une **embuscade** produit un pic de kills du défenseur à courte portée avec peu de pertes dans la même fenêtre.
- Une **défense statique submergée** produit l'inverse.
- Un **anéantissement à 100 %** est la signature d'une défense conventionnelle : une force irrégulière échange l'espace contre la survie ; être effacée signifie qu'elle n'a pas manœuvré.

Ces signatures permettent de mesurer **l'adéquation à la doctrine** que la mission assigne à chaque camp, plutôt qu'un simple résultat.

---

## 6. Métriques agrégées et strates

### 6.1 Ce qui n'agrège pas

**Le camp.** Les joueurs de l'OFCRA ne sont pas rattachés à une couleur : bleu ce soir, rouge la semaine prochaine. Agréger les joueurs par camp sur neuf ans revient à mesurer un tirage au sort.

En revanche le camp reste porteur d'un **rôle dans la mission** — assaillant ou défenseur, supérieur ou inférieur en nombre. L'agrégat pertinent est « l'assaillant gagne-t-il plus », qui est une propriété des missions, pas des joueurs.

### 6.2 Les strates qui agrègent

| Strate | Pourquoi elle tient | Ce qu'elle révèle |
|---|---|---|
| **Joueur** | Identité stable | Profil de distance, économie de munitions, longévité, tir ami |
| **Escouade membre** | Identités persistantes ([GH], [MOR], [RL], [WT], 23PzD, [FOG], [7BOW], [19th]…) qui réservent des créneaux | Cohésion, survie, distance typique — une escouade qui joue serré n'a pas le profil d'une équipe de tireurs d'élite |
| **Rôle** | Persistant d'une mission à l'autre | Survie par rôle : quels rôles sont mortels, et si c'est voulu |
| **Chef de camp** | **Déjà enregistré** : la fiche de mission porte un champ « Bluefor side leader » et « Redfor side leader », avec identifiant joueur | Performance de commandement — voir précautions §6.3 |
| **Auteur de mission** | Stable, et jamais mesuré | Équilibre des scénarios produits |
| **Carte, arme** | Déjà en place sur le site | — |

### 6.3 Précautions sur le chef de camp

Deux pièges, tous deux corrigeables :

**Les effectifs sont petits.** Une cinquantaine de missions par an, deux chefs par mission : un commandant régulier en cumule dix à quinze. Sur dix commandements, l'écart entre 40 % et 60 % de victoires n'a aucune signification statistique — affiché tel quel, ça fabrique des réputations sur du bruit. Parades : cumul pluriannuel, et **écart de score** plutôt que binaire gagné/perdu.

**Le résultat brut mélange commandement et calibration.** La correction est celle des buts attendus au football : comparer le résultat obtenu au résultat **attendu pour ce camp sur ce type de mission**. Cette moyenne ne peut se calculer qu'avec l'agrégat par rôle du §6.1 — les deux métriques se tiennent.

À noter : les missions asymétriques de l'OFCRA sont **délibérément équilibrées** (dotations différentes, contraintes de terrain compensatoires). L'écart au résultat attendu est donc largement imputable au commandement — à condition de mesurer l'adéquation à la doctrine (§5.3) et pas seulement la victoire.

### 6.4 Deux confusions à modéliser

- **Auteur et chef peuvent être la même personne** sur une mission donnée. Les deux agrégats ne sont alors pas indépendants. À prévoir dans le modèle de données.
- **Certaines missions sont co-écrites** (« Rigga and Rosbif », « Nasa & Rigel »). Le champ auteur est aujourd'hui du texte libre, avec des variantes de casse et d'orthographe pour un même auteur. Un identifiant d'auteur serait nécessaire pour agréger proprement.
- **Une mission peut avoir plusieurs auteurs**, et les deux sites n'enregistrent pas le même. Pour *Siege of Marawi Redux*, la fiche de mission (`game.ofcra.org`) indique « Mission Maker : Nasa » quand le centre de statistiques (`aar.ofcra.org`) attribue la mission à Rosbif — les deux sont exacts, chaque base ne retient qu'un nom. Le champ doit donc être **une liste d'identifiants d'auteur**, pas une chaîne unique, sous peine de fausser toute agrégation sur cette strate.

### 6.5 L'agrégat qui manque le plus — CORRIGÉ après lecture du vrai code source

**Correction d'une conclusion antérieure.** Cette section affirmait que le vainqueur agrégé manquait dès la collecte. C'est faux : la lecture du vrai `library.sqf` de `score_board` (voir récap [`score_board`](OMTK_ScoreBoard_Reforger_Recap.md), §5bis) montre que `omtk_sb_compute_scoreboard` envoie explicitement `[vainqueur, score_ouest, score_est]` à `statslogger_fnc_mission_end` en fin de mission, dès lors que le plugin `STATSLOGGER` est actif. **La donnée part bien de la source.**

**Le vrai problème est donc en aval, pas à la collecte.** Le champ existe sur la fiche de mission individuelle et prend trois valeurs (victoire, défaite, égalité), et le résultat est transmis au moment du calcul final. Mais il **n'apparaît pas agrégé** dans le tableau des missions du centre de statistiques (nom, type, date, carte, auteur, durée, effectifs — pas de colonne vainqueur), ce qui reste à expliquer : perte lors du stockage côté serveur de statistiques, ou simple absence d'affichage d'une donnée pourtant présente en base. À vérifier côté infrastructure du site, pas côté jeu.

**L'étiquette « Equal / not provided » confond toujours deux états distincts** — une égalité réelle et une absence de saisie. Ce point reste valable indépendamment de la correction ci-dessus : impossible de distinguer une mission nulle d'une mission non renseignée, donc impossible de savoir si un corpus contient 3 % ou 30 % de trous. Deux champs séparés, ou une valeur nulle explicite, lèveraient l'ambiguïté.

---

## 7. Cas d'étude — *Siege of Marawi Redux*, 23.07.2026

Mission de 75 minutes, 118 joueurs (61 BLUFOR philippins motorisés avec mortier et IFV, 57 OPFOR en défense avec mines, M2 statiques et RPG), séparés par une rivière à trois ponts. Objectifs communs, verrouillés à 45 min (les trois ponts), 60 min (charnier) et fin de partie (centre-ville).

**Résultat enregistré : « Equal / not provided ».**

Ce que le seul journal de kills permet de reconstituer :

| Minute | Kills BLUFOR | Kills OPFOR | Distance médiane |
|---|---|---|---|
| 0–15 | 3 | 0 | 296 m |
| 15–30 | 9 | 7 | 112 m |
| 30–45 | **19** | 6 | 35 m |
| 45–60 | 18 | 8 | 43 m |
| 60–75 | 5 | 4 | 10 m |

- **La distance médiane d'engagement s'effondre de 296 à 10 mètres** — c'est la ligne de front, mesurée sans aucune donnée de position.
- **Phase d'approche** (0–15) : trois morts à longue portée, dont trois obus de mortier au-delà de 600 m.
- **Défense efficace** (15–30) : 9 contre 7, quasi à l'équilibre.
- **Rupture** (30–45) : 19 contre 6 à 35 m de médiane — assaut sur position, juste avant le verrouillage des ponts à 45 min. La mission se décide là.
- **Attrition finale : 57 morts sur 57 côté OPFOR (100 %), 29 sur 61 côté BLUFOR (48 %).** L'arithmétique ferme exactement (kills + tirs amis + suicides), donc ce n'est pas un artefact.

**Conclusion méthodologique :** ces trois éléments — pente d'engagement, tranche de rupture, asymétrie d'attrition — ont été extraits d'une page web existante en quelques minutes. **Les données les contiennent déjà.** Ce qui manque n'est pas la collecte mais l'agrégation et la restitution — ce qui rend cette partie du chantier faisable **sans attendre Reforger**, sur l'existant.

---

## 8. Ce que dit le corpus (113 missions publiques et officielles, 2024–2026)

- **Concentration des auteurs** : quatre personnes écrivent 74 % des missions (Rosbif 24 %, wombat 19 %, Mrwhite350 17 %, Nasa 15 %), sur 14 auteurs distincts. La strate « auteur » est donc statistiquement exploitable pour les quatre principaux.
- **Équilibre numérique variable** : écart médian de 6 joueurs entre camps, mais seulement 48 % des missions à ±10 %, avec un écart maximal de 35 joueurs. L'asymétrie numérique fait partie du dispositif.
- **Missions à trois factions : 10 %** — avec un GREENFOR médian de 21 joueurs. Ce n'est pas marginal.
- **Effectifs** : médiane 129 joueurs, maximum 212.

### Correction importante sur le plafond de 128

Une estimation antérieure de ce dossier concluait à un impact « minime ». Le corpus complet dit autre chose :

| Seuil | Missions au-dessus |
|---|---|
| 128 joueurs | **50 %** |
| 140 joueurs | 27 % |
| 160 joueurs | 14 % |
| 180 joueurs | 8 % |

Dépassement médian pour les missions concernées : **16 joueurs**. Maximum : **84**.

Pour une publique ordinaire, le plafond est presque indolore. Pour les grandes missions officielles de campagne, c'est une coupe réelle. À reporter au README.

---

## 9. Ressources de référence

| Sujet | Lien |
|---|---|
| Système de dégâts | `community.bistudio.com/wiki/Arma_Reforger:Damage_System` |
| API `SCR_InstigatorContextData` | `community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__InstigatorContextData.html` |
| API `DamageManagerComponent` | `community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__DamageManagerComponent.html` |
| Exemple réel d'usage de `FileIO` | `arexplorer.zeroy.com` → `SCR_AIDebug.c` |
| Mod KillLog | `reforger.armaplatform.com/workshop/60E28D08A9009D01` |
| Mod Logging Enhanced by flabby | `reforger.armaplatform.com/workshop/6316335D6A19E51C-LoggingEnhancedbyflabby` |
| ReforgerJS (logging serveur externe) | dépôt communautaire — kills, dégâts, tirs amis, connexions, captures |
| Chaîne AAR actuelle | `aar.ofcra.org/stats` · `aar.ofcra.org/ocap` · `aar.ofcra.org:5000` (OCAP2) |

---

## 10. Ce qui reste à valider en pratique (Workbench requis)

- **OCAP existe-t-il sur Reforger**, ou est-il en cours de portage ? Si oui, son format d'entrée devient une contrainte à respecter plutôt qu'une décision à prendre.
- **Arme et distance** : ces champs ne semblent pas venir d'`InstigatorContextData`. À chercher du côté de l'événement de dégât ou du projectile.
- Tester `FileIO.OpenFile`/`Close` dans un composant custom, hors du contexte `AI_DEBUG` où l'exemple a été trouvé.
- Confirmer s'il existe une solution de persistance externe, ou si `FileIO` + traitement hors jeu est la seule voie.
- Valider que le contexte d'instigateur est complet **au moment précis de la mort** (tir ami, type de contrôle, déguisement).
- Vérifier le coût en performance du flux de positions à 128 joueurs — la réplication est déjà le facteur limitant du moteur.

---

*Document généré à partir des recherches menées en session et de l'analyse des données AAR réelles de l'OFCRA — à vérifier et corriger contre le comportement réel du Workbench (voir §10).*
