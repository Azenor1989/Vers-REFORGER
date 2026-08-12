//------------------------------------------------------------------------------------------------
// OMTK_ObjectiveScoreLink — version qui fonctionnait en jeu, SANS le verrouillage horodaté
//
// À placer dans : Scripts/Game/OMTK/OMTK_ObjectiveScoreLink.c
//
// RETIRÉ — le branchement OnGameStateChanged() qui désactivait OMTK_WarmupInvulnerability.c
// (mécanisme HijackDamageHandling) a été supprimé : ce fichier est retiré du projet, redondant
// et inerte face au mécanisme réellement testé et fiable (EnableDamageHandling dans
// OMTK_WarmupZoneComponent.c, voir son récapitulatif §3.4). Observé en jeu se déclenchant quasi
// immédiatement au lancement de la partie plutôt qu'à la vraie fin du warm-up — jamais synchronisé
// correctement, sans conséquence pratique puisque l'autre mécanisme faisait le vrai travail.
//------------------------------------------------------------------------------------------------

modded class SCR_BaseGameModeComponent
{
	protected static bool s_bOMTK_TaskListenerRegistered = false;

	override void OnGameModeStart()
	{
		super.OnGameModeStart();

		if (!Replication.IsServer())
			return;

		if (s_bOMTK_TaskListenerRegistered)
		{
			Print("[OMTK] Abonnement à s_OnTaskStateChanged déjà actif, ignoré.", LogLevel.NORMAL);
			return;
		}

		SCR_Task.GetOnTaskStateChanged().Insert(OMTK_OnTaskStateChanged);
		s_bOMTK_TaskListenerRegistered = true;

		Print("[OMTK] Abonnement à SCR_Task.GetOnTaskStateChanged() effectué.", LogLevel.NORMAL);
	}

	//! Signature imposée par TaskStateInvokerDelegate — confirmée dans le source :
	//! void TaskStateInvokerDelegate(SCR_Task task, SCR_ETaskState newState);
	protected void OMTK_OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (!task || newState != SCR_ETaskState.COMPLETED)
			return;

		array<string> ownerFactions = task.GetOwnerFactionKeys();
		if (!ownerFactions || ownerFactions.IsEmpty())
			return;

		string taskId = task.GetTaskID();

		// Valeur de points fixe pour ce premier test.
		int points = 1;

		SCR_BaseScoringSystemComponent scoring = SCR_BaseScoringSystemComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_BaseScoringSystemComponent));
		if (!scoring)
			return;

		// Une tâche peut appartenir à plusieurs factions à la fois
		// (ex. "BLUEFOR+REDFOR" côté OMTK) — on crédite chacune.
		foreach (string factionKey : ownerFactions)
		{
			Print("[OMTK] TEST tâche complétée — faction=" + factionKey + " id=" + taskId, LogLevel.NORMAL);
			scoring.AddFactionPoints(factionKey, points, taskId);
		}
	}
}
