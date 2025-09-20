// ============================================================================
// ATHENE_EntityMarkerHUD (2 Ziele) – Distanz-Scale & Fade
// - Zeichnet Icon + Distanz für bis zu ZWEI Ziel-Entities
// - Ziel 1/2 via Referenz ODER Name
// - Größe über FrameSlot.SetSize() an Icon-Container (FrameWidget) gesteuert
// - Alpha über ImageWidget.SetOpacity()
// - Layout: Pro Item ein Root-Frame (m_wMarkerRootX) mit einem direkten
//           Icon-Container-Frame (m_wIconContainerX), darin das Image (m_wIconX)
//           + ein Distanz-Text (m_wDistanceX).
// ============================================================================

[BaseContainerProps()]
class ATHENE_EntityMarkerHUD : SCR_InfoDisplayExtended
{
	// ----- Layout-IDs (2 Items im Layout) -----------------------------------
	protected const string WID_ROOT1          = "m_wMarkerRoot1";
	protected const string WID_ICONCONT1      = "m_wIconContainer1";
	protected const string WID_ICON1          = "m_wIcon1";
	protected const string WID_DISTANCE1      = "m_wDistance1";

	protected const string WID_ROOT2          = "m_wMarkerRoot2";
	protected const string WID_ICONCONT2      = "m_wIconContainer2";
	protected const string WID_ICON2          = "m_wIcon2";
	protected const string WID_DISTANCE2      = "m_wDistance2";

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

	// ----- Distanz → Scale/Fade ---------------------------------------------
	// Nähe/Ferne
	[Attribute("5.0",  UIWidgets.EditBox, category: "FX", desc: "Distanz [m], ab der maximale Größe/Alpha anliegt")]
	float m_fNear;

	[Attribute("500.0", UIWidgets.EditBox, category: "FX", desc: "Distanz [m], ab der minimale Größe/Alpha anliegt")]
	float m_fFar;

	// Skalen-Bereich
	[Attribute("0.6", UIWidgets.EditBox, category: "FX", desc: "Min. Scale bei Ferne")]
	float m_fMinScale;

	[Attribute("1.4", UIWidgets.EditBox, category: "FX", desc: "Max. Scale bei Nähe")]
	float m_fMaxScale;

	// Alpha-Bereich
	[Attribute("0.35", UIWidgets.EditBox, category: "FX", desc: "Min. Alpha bei Ferne")]
	float m_fMinAlpha;

	[Attribute("1.0", UIWidgets.EditBox, category: "FX", desc: "Max. Alpha bei Nähe")]
	float m_fMaxAlpha;

	// ----- Runtime-Refs ------------------------------------------------------
	protected WorkspaceWidget m_Workspace;
	protected BaseWorld m_World;

	// Item 1
	protected Widget      m_wMarkerRoot1;
	protected Widget      m_wIconContainer1;
	protected ImageWidget m_wIcon1;
	protected TextWidget  m_wDistance1;

	// Item 2
	protected Widget      m_wMarkerRoot2;
	protected Widget      m_wIconContainer2;
	protected ImageWidget m_wIcon2;
	protected TextWidget  m_wDistance2;

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
		m_wMarkerRoot1    = m_wRoot.FindWidget(WID_ROOT1);
		m_wIconContainer1 = m_wRoot.FindAnyWidget(WID_ICONCONT1);
		m_wIcon1          = ImageWidget.Cast(m_wRoot.FindAnyWidget(WID_ICON1));
		m_wDistance1      = TextWidget.Cast(m_wRoot.FindAnyWidget(WID_DISTANCE1));

		// Subwidgets binden (Item 2)
		m_wMarkerRoot2    = m_wRoot.FindWidget(WID_ROOT2);
		m_wIconContainer2 = m_wRoot.FindAnyWidget(WID_ICONCONT2);
		m_wIcon2          = ImageWidget.Cast(m_wRoot.FindAnyWidget(WID_ICON2));
		m_wDistance2      = TextWidget.Cast(m_wRoot.FindAnyWidget(WID_DISTANCE2));

		// Targets initial auflösen
		ResolveTarget1();
		ResolveTarget2();

		// HUD selbst sichtbar lassen, Items steuern ihre Sichtbarkeit
		Show(true);
		SetItemVisible(0, false);
		SetItemVisible(1, false);

		// Sanity: Defaultwerte, falls Far<=Near
		if (m_fFar <= m_fNear) m_fFar = m_fNear + 0.01;
	}

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_Workspace || !m_World) return;

		UpdateItemFX(owner, 0);
		UpdateItemFX(owner, 1);
	}

	// ------------------------------------------------------------------------
	// Pro-Item Update mit Distanz-abhängigem Scale/Fade
	// ------------------------------------------------------------------------
	protected void UpdateItemFX(IEntity owner, int idx)
	{
		IEntity target = GetTarget(idx);

		// Falls Target noch nicht gesetzt → versuchen aufzulösen
		if (!target)
		{
			ResolveTarget(idx);
			target = GetTarget(idx);
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

		// Distanz → Scale/Alpha berechnen
		float scale, alpha;
		ComputeScaleAndAlphaFromDistance(owner.GetOrigin(), worldPos, scale, alpha);

		// Marker-Root zentriert mit skaliertem Offet positionieren
		float halfW = (float)m_iIconHalfWidth  * scale;
		float halfH = (float)m_iIconHalfHeight * scale;

		Widget root = GetItemRoot(idx);
		if (root)
		{
			float px = screen[0] - halfW;
			float py = screen[1] - halfH;
			FrameSlot.SetPos(root, px, py);
		}

		// Icon-Containergröße + Icon-Opacity anwenden
		ApplyIconFX(idx, scale, alpha);

		// Distanz-Label throttlen
		m_iFrameCounter++;
		if (m_iFrameCounter >= m_iUpdateFrequency)
		{
			m_iFrameCounter = 0;
			UpdateDistance(idx, owner.GetOrigin(), worldPos);
		}
	}

	// ------------------------------------------------------------------------
	// Distanz → Scale/Alpha
	// ------------------------------------------------------------------------
	protected void ComputeScaleAndAlphaFromDistance(vector p, vector t, out float scale, out float alpha)
	{
		float d = vector.Distance(p, t);

		// 0..1, geklemmt
		float f = (d - m_fNear) / (m_fFar - m_fNear);
		if (f < 0.0) f = 0.0;
		if (f > 1.0) f = 1.0;

		// Nähe: f=0 -> Max; Ferne: f=1 -> Min
		scale = Lerp(m_fMaxScale, m_fMinScale, f);
		alpha = Lerp(m_fMaxAlpha, m_fMinAlpha, f);
	}

	protected float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	// ------------------------------------------------------------------------
	// FX anwenden (größe via Container, alpha via Image)
	// ------------------------------------------------------------------------
	protected void ApplyIconFX(int idx, float scale, float alpha)
	{
		ImageWidget icon;
		Widget iconCont;

		if (idx == 0)
		{
			icon     = m_wIcon1;
			iconCont = m_wIconContainer1;
		}
		else
		{
			icon     = m_wIcon2;
			iconCont = m_wIconContainer2;
		}

		// Fade/Alpha am eigentlichen Bild
		if (icon)
			icon.SetOpacity(alpha);

		// Größe über FrameSlot am Container steuern (Parent = FrameWidget!)
		float w = (float)m_iIconHalfWidth  * 2.0 * scale;
		float h = (float)m_iIconHalfHeight * 2.0 * scale;

		if (iconCont)
			FrameSlot.SetSize(iconCont, w, h);
		else
		{
			// Fallback: Root skalieren, falls Container fehlt
			Widget root = GetItemRoot(idx);
			if (root) FrameSlot.SetSize(root, w, h);
		}
	}

	// ------------------------------------------------------------------------
	// Targets auflösen (spezifisch & generisch)
	// ------------------------------------------------------------------------
	protected void ResolveTarget1()
	{
		if (m_TargetEntity1)
		{
			m_pTarget1 = m_TargetEntity1;
			return;
		}
		if (!m_TargetName1.IsEmpty())
		{
			m_pTarget1 = GetGame().GetWorld().FindEntityByName(m_TargetName1);
			if (!m_pTarget1)
				Print(string.Format("[ATHENE_EntityMarkerHUD] Target1 '%1' nicht gefunden", m_TargetName1), LogLevel.WARNING);
		}
	}

	protected void ResolveTarget2()
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
				Print(string.Format("[ATHENE_EntityMarkerHUD] Target2 '%1' nicht gefunden", m_TargetName2), LogLevel.WARNING);
		}
	}

	protected void ResolveTarget(int idx)
	{
		if (idx == 0) { ResolveTarget1(); return; }
		ResolveTarget2();
	}

	protected IEntity GetTarget(int idx)
	{
		if (idx == 0) return m_pTarget1;
		return m_pTarget2;
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
		if (idx == 0) root = m_wMarkerRoot1;
		else          root = m_wMarkerRoot2;

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
