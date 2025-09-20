// ============================================================================
// ATHENE_EntityMarkerHUD  (SCR_InfoDisplayExtended)
// - Layout wird im Player-HUD (SCR_BaseHudComponent → Info Displays) gesetzt
// - Zeichnet Icon + Distanz an die Screen-Pos eines Ziel-Entities
// - Ziel via direkte Referenz ODER EntityName
// ============================================================================

[BaseContainerProps()]
class ATHENE_EntityMarkerHUD : SCR_InfoDisplayExtended
{
	// ----- Layout-Widget-IDs (müssen mit deinem .layout übereinstimmen) -----
	protected const string WID_ROOT     = "m_wMarkerRoot";
	protected const string WID_ICON     = "m_wIcon";
	protected const string WID_DISTANCE = "m_wDistance";

	// ----- Ziel-Entity -------------------------------------------------------
	[Attribute("", UIWidgets.Object, category: "Target", desc: "Direkte Referenz auf Ziel-Entity")]
	IEntity m_TargetEntity;  // KEIN 'ref'!

	[Attribute("", UIWidgets.EditBox, category: "Target", desc: "Name des Ziel-Entities in der World (Editor-Name)")]
	string m_TargetName;

	// ----- UI / Performance --------------------------------------------------
	[Attribute("50", UIWidgets.Slider, "0 200 1", category: "UI")]
	int m_iIconHalfWidth;

	[Attribute("50", UIWidgets.Slider, "0 200 1", category: "UI")]
	int m_iIconHalfHeight;

	[Attribute("5", UIWidgets.Slider, "1 30 1", category: "Perf")]
	int m_iUpdateFrequency;

	// ----- Runtime-Refs ------------------------------------------------------
	protected WorkspaceWidget m_Workspace;
	protected BaseWorld m_World;

	protected Widget m_wMarkerRoot;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wDistance;

	protected IEntity m_pTarget;
	protected int m_iFrameCounter = 0;

	// ------------------------------------------------------------------------
	// Lifecycle
	// ------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
		{
			Print("[ATHENE_EntityMarkerHUD] m_wRoot is null (HUD-Eintrag hat kein Layout?)", LogLevel.ERROR);
			return;
		}

		m_Workspace = GetGame().GetWorkspace();
		m_World     = GetGame().GetWorld();

		// Subwidgets binden
		m_wMarkerRoot = m_wRoot.FindWidget(WID_ROOT);
		if (!m_wMarkerRoot)
		{
			Print("[ATHENE_EntityMarkerHUD] Widget m_wMarkerRoot not found – fallback to root", LogLevel.WARNING);
			m_wMarkerRoot = m_wRoot;
		}

		m_wIcon     = ImageWidget.Cast(m_wRoot.FindAnyWidget(WID_ICON));
		if (!m_wIcon)
			Print("[ATHENE_EntityMarkerHUD] Widget m_wIcon not found", LogLevel.WARNING);

		m_wDistance = TextWidget.Cast(m_wRoot.FindAnyWidget(WID_DISTANCE));
		if (!m_wDistance)
			Print("[ATHENE_EntityMarkerHUD] Widget m_wDistance not found", LogLevel.WARNING);

		ResolveTarget();
		SetVisible(false);
	}

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_Workspace || !m_World || !m_wMarkerRoot)
			return;

		// Ziel ggf. (neu) auflösen
		if (!m_pTarget)
		{
			ResolveTarget();
			if (!m_pTarget)
			{
				SetVisible(false);
				return;
			}
		}

		vector worldPos = m_pTarget.GetOrigin();
		vector screen   = m_Workspace.ProjWorldToScreen(worldPos, m_World);

		// Sichtbar nur, wenn vor der Kamera
		bool onScreen = (screen[2] > 0);
		SetVisible(onScreen);
		if (!onScreen) return;

		float x = screen[0] - m_iIconHalfWidth;
		float y = screen[1] - m_iIconHalfHeight;
		FrameSlot.SetPos(m_wMarkerRoot, x, y);

		// Distanz throttlen
		m_iFrameCounter++;
		if (m_iFrameCounter >= m_iUpdateFrequency)
		{
			m_iFrameCounter = 0;
			UpdateDistance(owner.GetOrigin(), worldPos);
		}
	}

	// ------------------------------------------------------------------------
	// Helpers
	// ------------------------------------------------------------------------
	protected void ResolveTarget()
	{
		// 1) direkte Referenz
		if (m_TargetEntity)
		{
			m_pTarget = m_TargetEntity;
			return;
		}

		// 2) Namenssuche
		if (m_TargetName && m_TargetName != "")
		{
			m_pTarget = GetGame().GetWorld().FindEntityByName(m_TargetName);
			if (!m_pTarget)
				Print(string.Format("[ATHENE_EntityMarkerHUD] Entity '%1' nicht gefunden", m_TargetName), LogLevel.WARNING);
		}
		else
		{
			Print("[ATHENE_EntityMarkerHUD] Kein Target gesetzt (weder Referenz noch Name).", LogLevel.WARNING);
		}
	}

	protected void SetVisible(bool show)
	{
		Show(show); // respektiert m_bCanShow der Basisklasse
		if (m_wMarkerRoot)
		{
			m_wMarkerRoot.SetEnabled(show);
			m_wMarkerRoot.SetVisible(show);
		}
	}

	protected void UpdateDistance(vector playerOrigin, vector targetOrigin)
	{
		if (!m_wDistance) return;

		float d = vector.Distance(playerOrigin, targetOrigin);
		if (d < 1000.0)
		{
			m_wDistance.SetText(string.Format("%1 m", Math.Round(d)));
		}
		else
		{
			float km = d / 1000.0;
			m_wDistance.SetText(string.Format("%1 km", Math.Round(km * 10.0) / 10.0));
		}
	}
}
