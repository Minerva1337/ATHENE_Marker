// ============================================================================
// ATHENE_EntityMarkerHUD (2 Ziele)
// - Zeichnet Icon + Distanz für bis zu ZWEI Ziel-Entities
// - Ziel 1/2 via Referenz ODER Name
// - Kein Ternary, klare if/else-Zweiglogik
// - Erfordert in deinem Layout zwei Item-Blöcke (ROOT1/ROOT2 ...)
// ============================================================================

[BaseContainerProps()]
class ATHENE_EntityMarkerHUD : SCR_InfoDisplayExtended
{
	// ----- Layout-IDs (2 Items im Layout) -----------------------------------
	protected const string WID_ROOT1     = "m_wMarkerRoot1";
	protected const string WID_ICON1     = "m_wIcon1";
	protected const string WID_DISTANCE1 = "m_wDistance1";

	protected const string WID_ROOT2     = "m_wMarkerRoot2";
	protected const string WID_ICON2     = "m_wIcon2";
	protected const string WID_DISTANCE2 = "m_wDistance2";

	// ----- Ziel 1 ------------------------------------------------------------
	[Attribute("", UIWidgets.Object, category: "Target1", desc: "Direkte Referenz auf Ziel-Entity 1")]
	IEntity m_TargetEntity1;

	[Attribute("", UIWidgets.EditBox, category: "Target1", desc: "Name des Ziel-Entities 1 in der World (Editor-Name)")]
	string m_TargetName1;

	// ----- Ziel 2 ------------------------------------------------------------
	[Attribute("", UIWidgets.Object, category: "Target2", desc: "Direkte Referenz auf Ziel-Entity 2")]
	IEntity m_TargetEntity2;

	[Attribute("", UIWidgets.EditBox, category: "Target2", desc: "Name des Ziel-Entities 2 in der World (Editor-Name)")]
	string m_TargetName2;

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

	protected Widget m_wMarkerRoot1;
	protected ImageWidget m_wIcon1;
	protected TextWidget m_wDistance1;

	protected Widget m_wMarkerRoot2;
	protected ImageWidget m_wIcon2;
	protected TextWidget m_wDistance2;

	protected IEntity m_pTarget1;
	protected IEntity m_pTarget2;

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

		// Subwidgets binden (Item 1)
		m_wMarkerRoot1 = m_wRoot.FindWidget(WID_ROOT1);
		m_wIcon1       = ImageWidget.Cast(m_wRoot.FindAnyWidget(WID_ICON1));
		m_wDistance1   = TextWidget.Cast(m_wRoot.FindAnyWidget(WID_DISTANCE1));

		// Subwidgets binden (Item 2)
		m_wMarkerRoot2 = m_wRoot.FindWidget(WID_ROOT2);
		m_wIcon2       = ImageWidget.Cast(m_wRoot.FindAnyWidget(WID_ICON2));
		m_wDistance2   = TextWidget.Cast(m_wRoot.FindAnyWidget(WID_DISTANCE2));

	    ResolveTarget1();
	    ResolveTarget2();
	
	    // HUD immer aktiv lassen – Items regeln ihre Sichtbarkeit selbst
	    Show(true);
	    SetItemVisible(0, false);
	    SetItemVisible(1, false);
	}

	
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
	    if (!m_Workspace || !m_World) return;
	
	    // --- ITEM 0 ---
	    IEntity t0 = m_pTarget1;
	    if (!t0) { ResolveTarget1(); t0 = m_pTarget1; }
	    if (t0)
	    {
	        vector wp0 = t0.GetOrigin();
	        vector sp0 = m_Workspace.ProjWorldToScreen(wp0, m_World);
	        bool on0 = (sp0[2] > 0);
	        SetItemVisible(0, on0);
	        if (on0)
	        {
	            FrameSlot.SetPos(m_wMarkerRoot1, sp0[0]-m_iIconHalfWidth, sp0[1]-m_iIconHalfHeight);
	            ThrottledUpdateDistance(0, owner.GetOrigin(), wp0);
	        }
	    }
	    else SetItemVisible(0, false);
	
	    // --- ITEM 1 ---
	    IEntity t1 = m_pTarget2;
	    if (!t1) { ResolveTarget2(); t1 = m_pTarget2; }
	    if (t1)
	    {
	        vector wp1 = t1.GetOrigin();
	        vector sp1 = m_Workspace.ProjWorldToScreen(wp1, m_World);
	        bool on1 = (sp1[2] > 0);
	        SetItemVisible(1, on1);
	        if (on1)
	        {
	            FrameSlot.SetPos(m_wMarkerRoot2, sp1[0]-m_iIconHalfWidth, sp1[1]-m_iIconHalfHeight);
	            ThrottledUpdateDistance(1, owner.GetOrigin(), wp1);
	        }
	    }
	    else SetItemVisible(1, false);
	}
	
	// getrennte Distanz-Labels nutzen
	protected void ThrottledUpdateDistance(int idx, vector p, vector t)
	{
	    m_iFrameCounter++;
	    if (m_iFrameCounter < m_iUpdateFrequency) return;
	    m_iFrameCounter = 0;
	
	    TextWidget dist;
		if (idx == 0)
		    dist = m_wDistance1;
		else
		    dist = m_wDistance2;
	    if (!dist) return;
	
	    float d = vector.Distance(p, t);
	    if (d < 1000.0) dist.SetText(string.Format("%1 m", Math.Round(d)));
	    else dist.SetText(string.Format("%1 km", Math.Round((d/1000.0) * 10.0) / 10.0));
	}

	// ------------------------------------------------------------------------
	// Per-Item Update
	// ------------------------------------------------------------------------
	
	// Hilfsmethoden pro Item
	
	
	protected void UpdateItem(IEntity owner, int idx)
	{
		IEntity target = GetTarget(idx);

		// Falls Target noch nicht gesetzt → versuchen aufzulösen
		if (!target)
		{
			ResolveTarget(idx);

			// Nochmal holen (ohne TERNARY!)
			if (idx == 0)
				target = m_pTarget1;
			else
				target = m_pTarget2;

			if (!target)
			{
				SetItemVisible(idx, false);
				return;
			}
		}

		vector worldPos = target.GetOrigin();
		vector screen   = m_Workspace.ProjWorldToScreen(worldPos, m_World);

		bool onScreen = (screen[2] > 0);
		SetItemVisible(idx, onScreen);
		if (!onScreen) return;

		float x = screen[0] - m_iIconHalfWidth;
		float y = screen[1] - m_iIconHalfHeight;
		FrameSlot.SetPos(GetItemRoot(idx), x, y);

		// Distanz throttlen
		m_iFrameCounter++;
		if (m_iFrameCounter >= m_iUpdateFrequency)
		{
			m_iFrameCounter = 0;
			UpdateDistance(idx, owner.GetOrigin(), worldPos);
		}
	}
	
	
	// je ein Resolver pro Ziel (nicht die globale SetVisible() nutzen)
	protected void ResolveTarget1()
	{
	    if (m_TargetEntity1) { m_pTarget1 = m_TargetEntity1; return; }
	    if (!m_TargetName1.IsEmpty())
	    {
	        m_pTarget1 = GetGame().GetWorld().FindEntityByName(m_TargetName1);
	        if (!m_pTarget1) Print(string.Format("[HUD] Target1 '%1' not found", m_TargetName1), LogLevel.WARNING);
	    }
	}
	
	protected void ResolveTarget2()
	{
	    if (m_TargetEntity2) { m_pTarget2 = m_TargetEntity2; return; }
	    if (!m_TargetName2.IsEmpty())
	    {
	        m_pTarget2 = GetGame().GetWorld().FindEntityByName(m_TargetName2);
	        if (!m_pTarget2) Print(string.Format("[HUD] Target2 '%1' not found", m_TargetName2), LogLevel.WARNING);
	    }
	}

	// ------------------------------------------------------------------------
	// Helpers: Targets
	// ------------------------------------------------------------------------
	protected IEntity GetTarget(int idx)
	{
		if (idx == 0) return m_pTarget1;
		return m_pTarget2;
	}

	protected void ResolveTarget(int idx)
	{
		if (idx == 0)
		{
			// 1) direkte Referenz
			if (m_TargetEntity1)
			{
				m_pTarget1 = m_TargetEntity1;
				return;
			}
			// 2) Namenssuche
			if (!m_TargetName1.IsEmpty())
			{
				m_pTarget1 = GetGame().GetWorld().FindEntityByName(m_TargetName1);
				if (!m_pTarget1)
					Print(string.Format("[ATHENE_EntityMarkerHUD] Entity1 '%1' nicht gefunden", m_TargetName1), LogLevel.WARNING);
			}
			else
			{
				// nichts gesetzt
			}
		}
		else
		{
			if (m_TargetEntity2)
			{
				m_pTarget2 = m_TargetEntity2;
				return;
			}

			if (!m_TargetName2.IsEmpty())
			{
				m_pTarget2 = GetGame().GetWorld().FindEntityByName(m_TargetName2);
				if (!m_pTarget2)
					Print(string.Format("[ATHENE_EntityMarkerHUD] Entity2 '%1' nicht gefunden", m_TargetName2), LogLevel.WARNING);
			}
			else
			{
				// nichts gesetzt 
			}
		}
	}

	// ------------------------------------------------------------------------
	// Helpers: UI pro Item
	// ------------------------------------------------------------------------
	protected Widget GetItemRoot(int idx)
	{
		if (idx == 0) return m_wMarkerRoot1;
		return m_wMarkerRoot2;
	}

	protected TextWidget GetItemDistance(int idx)
	{
		if (idx == 0) return m_wDistance1;
		return m_wDistance2;
	}

	protected void SetItemVisible(int idx, bool show)
	{
	    Widget root;
		if (idx == 0)
		    root = m_wMarkerRoot1;
		else
		    root = m_wMarkerRoot2;
		
	    if (!root) return;
	    root.SetEnabled(show);
	    root.SetVisible(show);
	}

	protected void UpdateDistance(int idx, vector playerOrigin, vector targetOrigin)
	{
		TextWidget w = GetItemDistance(idx);
		if (!w) return;

		float d = vector.Distance(playerOrigin, targetOrigin);
		if (d < 1000.0)
		{
			w.SetText(string.Format("%1 m", Math.Round(d)));
		}
		else
		{
			float km = d / 1000.0;
			w.SetText(string.Format("%1 km", Math.Round(km * 10.0) / 10.0));
		}
	}
}
