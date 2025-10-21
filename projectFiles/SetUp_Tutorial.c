// ============================================================================
// Step-by-Step: Using ATHENE_EntityMarkerHUD (2 targets)
// ============================================================================
//
// 1) Prepare the layout (required)
//
// Create—or open—your HUD .layout for the info display. Use two item blocks
// and these exact widget IDs:
//
// - Root (Frame) → used by the info display as m_wRoot
//   - Item 1 – Frame: m_wMarkerRoot1 (slot type: FrameWidgetSlot)
//     - ImageWidget:  m_wIcon1     (slot: FrameWidgetSlot)
//     - TextWidget :  m_wDistance1 (slot: FrameWidgetSlot)
//   - Item 2 – Frame: m_wMarkerRoot2 (slot type: FrameWidgetSlot)
//     - ImageWidget:  m_wIcon2     (slot: FrameWidgetSlot)
//     - TextWidget :  m_wDistance2 (slot: FrameWidgetSlot)
//
// Important: because the script uses FrameSlot.SetPos/SetSize, every widget
// you move/scale (the item frames, icons, and—if you want it to move—each
// distance label) must be in a FrameWidgetSlot. Otherwise you’ll see:
// “invalid slot … expected FrameWidgetSlot”.
//
// -----------------------------------------------------------------------------
// 2) Choose your icons (two options)
//
// A) Set in the layout (recommended, simple)
//   1. Select m_wIcon1 in the layout tree.
//   2. Assign the image (via Image Set + Item, or via Image/Texture path).
//   3. Optional: Keep Aspect, Fit to Size, Tint/color.
//   4. Repeat for m_wIcon2.
//
// B) Override via code/attributes (optional)
//   - Add attributes like m_sIcon1Set/m_sIcon1Item.
//   - Load via ImageWidget.LoadImageFromSet(...) in DisplayStartDraw().
//
// -----------------------------------------------------------------------------
// 3) Register the info display on the player HUD
//
//   - Open PlayerController prefab (e.g. DefaultPlayerControllerMP_ScenarioFramework.et).
//   - In SCR_HUDManagerComponent → Info Displays:
//       Add → Class: ATHENE_EntityMarkerHUD
//       Layout/Resource: your marker layout from step 1
//
// -----------------------------------------------------------------------------
// 4) Create and name the two target entities
//
// By name (robust):
//   - Place both target entities in the world.
//   - Give Editor Names (ATHENE_Marker1 / ATHENE_Marker2).
//   - Set m_TargetName1 = ATHENE_Marker1, m_TargetName2 = ATHENE_Marker2.
//
// By direct reference (drag & drop):
//   - Set m_TargetEntity1 / m_TargetEntity2 directly.
//   - Direct reference wins if both are set.
//
// -----------------------------------------------------------------------------
// 5) Tuning: look & behavior
//
// - Icon positioning:
//   m_iIconHalfWidth / m_iIconHalfHeight = half icon size in px (e.g. 32 for 64px).
//
// - Update frequency:
//   m_iUpdateFrequency = frames between distance updates (5–10 typical).
//
// - Distance-based scale & fade:
//   Min/Max Scale, Full Alpha Distance, Zero Alpha Distance.
//   Option: fade text with icon, move label with icon (FrameWidgetSlot needed).
//
// -----------------------------------------------------------------------------
// 6) Test it
//
// - Start mission, look for two markers.
// - Markers appear only if in front of camera (screen[2] > 0).
// - Icons/labels fade/scale per settings.
//
// -----------------------------------------------------------------------------
// 7) Troubleshooting
//
// - Error “invalid slot … expected FrameWidgetSlot”:
//   Fix slot type or disable m_bMoveDistanceLabel.
//
// - Second marker not visible:
//   Check target names/references, ensure entity exists, ensure not behind camera.
//
// - Icon too big/small:
//   Adjust m_iIconHalfWidth/Height, check Fit to Size in layout.
//
// - Icons fade out too quickly:
//   Increase Zero Alpha Distance.
//
// -----------------------------------------------------------------------------
// 8) Suggested starter values
//
//   m_iIconHalfWidth  = 32
//   m_iIconHalfHeight = 32
//   m_iUpdateFrequency = 5
//   Scale Min 0.7, Max 1.2
//   Alpha: Full @ 10m, Zero @ 800m
//   Text: fade = 1, move = 1, padding Y = 4–8
//
// -----------------------------------------------------------------------------
// 9) Naming example
//
// - World entities:
//     Radio Station North → ATHENE_Marker1
//     Radio Station South → ATHENE_Marker2
//
// - HUD attributes:
//     m_TargetName1 = ATHENE_Marker1
//     m_TargetName2 = ATHENE_Marker2
//
// - Icons in layout:
//     m_wIcon1 = radio symbol
//     m_wIcon2 = radio symbol (or different per target/faction)
//
// ============================================================================
