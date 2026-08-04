// BROUILLON — non compilé, non testé. Voir OMTK_WarmUp_Reforger_Recap.md §4.3
// pour l'analyse détaillée des points à corriger avant test en jeu.

//------------------------------------------------------------------------------------------------
// OMTK_ReadyAction — Action du menu radial pour déclarer le camp prêt
//------------------------------------------------------------------------------------------------
class OMTK_ReadyAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// 1. Récupérer le GameMode de la mission
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		// 2. Trouver notre composant de Warm-Up sur le GameMode
		SCR_PreGameGameModeStateComponent preGameComp = SCR_PreGameGameModeStateComponent.Cast(gameMode.FindComponent(SCR_PreGameGameModeStateComponent));
		
		// 3. Si on le trouve, on déclenche le raccourcissement du minuteur
		if (preGameComp)
		{
			preGameComp.ShortenWarmUp();
			Print("[OMTK] Un chef de camp a validé le départ via le menu radial.", LogLevel.NORMAL);
		}
	}

	override bool CanBeShownScript(IEntity user)
	{
		// L'action n'apparaît que si le jeu est bien en cours de lancement (Pre-Game)
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			SCR_BaseGameModeStateComponent stateComp = SCR_BaseGameModeStateComponent.Cast(gameMode.FindComponent(SCR_PreGameGameModeStateComponent));
			if (stateComp)
			{
				// Si le pre-game est actif, on affiche l'action dans le menu radial
				return true;
			}
		}
		return false;
	}
}
