// BROUILLON — non compilé, non testé. Voir OMTK_WarmUp_Reforger_Recap.md §4.3
// pour l'analyse détaillée des points à corriger avant test en jeu.

modded class SCR_PreGameGameModeStateComponent
{
	[Attribute("120", UIWidgets.Slider, "Warm Up Duration", "0 900 1")]
	int m_iWarmUpDurationSeconds;

	protected float m_fWarmUpEndTime = -1.0;

	void ShortenWarmUp()
	{
		if (m_fWarmUpEndTime < 0)
			return;

		float currentTime = GetGame().GetWorld().GetWorldTime();
		float newEndTime = currentTime + 10000.0; 

		if (newEndTime < m_fWarmUpEndTime)
		{
			m_fWarmUpEndTime = newEndTime;
			Print("[OMTK] Le Warm-Up est raccourci a 10 secondes.", LogLevel.NORMAL);
		}
	}

	override bool CanAdvanceState(SCR_EGameModeState nextState)
	{
		if (super.CanAdvanceState(nextState))
			return true;

		if (m_fWarmUpEndTime < 0)
		{
			m_fWarmUpEndTime = GetGame().GetWorld().GetWorldTime() + (m_iWarmUpDurationSeconds * 1000);
			Print("[OMTK] Warm-Up initie.", LogLevel.NORMAL);
		}

		if (GetGame().GetWorld().GetWorldTime() >= m_fWarmUpEndTime)
		{
			Print("[OMTK] Warm-Up termine.", LogLevel.NORMAL);
			return true;
		}

		return false;
	}
}
