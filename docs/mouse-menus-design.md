# Design: mouse-driven front-end menus (`wpr_system.cpp`)

Status: design only, written from source (no runs). Goal from the user: navigate the front end with
the mouse, with a pointer drawn in the game's own art style. Keyboard and pads must keep working
unchanged.

## How the menus work today

- **Screens.** `wpr_system.cpp` runs one screen at a time (`_MenuState.nCurrentScreen`). Each has a
  `Work` function returning a nav code (`FORWARD`, `BACK`, `CONFIRM`, `ALTERNATE`, `NOTHING`), draw
  functions (`DrawOrtho`/`DrawFDraw`), and an `ExitDecisions` function that acts on the nav code
  (`Wpr_DataTypes_ScreenFunctions_t`, `wpr_datatypes.h`).
- **Selection.** Most screens keep the highlighted row in `_MenuState.nCurItemIndex`. Some use their
  own state: the profile-name keyboard (`nPNCurRow`/`nPNCurCol`), memory-unit select (`paMUEntries`),
  sliders in the sound and control settings (left/right changes a value), and level select (meshes).
- **Input** goes through four helpers (~line 2978): `_CheckUpDownAxis`, `_CheckLeftRightAxis`,
  `_CheckAcceptButtons` (A or Start), `_CheckBackButtons` (B or Back2), plus `_CheckYButton`. They
  read `Gamepad_aapSample[_MenuState.nControllerIndex]`. Message boxes (`msgbox.cpp` ~464-494) read
  the pads directly. The in-game pause menu is a separate system (`PauseScreen.cpp`,
  `MenuTypes.cpp` `CMenuMgr`); `CMenuCursor` there is the selection highlight, not a pointer.
- **Layout comes from retail data.** Text items are `Wpr_DataTypes_TextLayout_t` (`fUnitX`, `fUnitY`,
  alignment, scale) drawn by `_DrawText()` (~3277) with `ftext_Printf`. Meshes are
  `Wpr_DataTypes_MeshLayout_t` in bipolar (-1..1) coordinates. Button prompts ("A Select", "B Back")
  are `Wpr_DataTypes_ButtonLayout_t`, drawn by `wpr_drawutils_DrawButtonOverlay()` as an icon
  texture plus text.
- **Coordinates.** ftext's unit space: `fX` 0..1 across the default viewport, `fY` 0..0.75 down it
  (`ftext.cpp` ~1680: `pixelY = vpTop + ResY * fY * 4/3`). Bipolar: `unitX = (bx + 1) / 2`,
  `unitY = (1 - by) / 2 * 0.75` (as `DrawButtonOverlay` does).
- **No text measurement.** `ftext_Printf` queues strings; layout happens in `ftext_Draw()`. Nothing
  exposes a string's on-screen box.

## Design

### 1. Pointer input (port layer: `port/pc_input.cpp`)

`pcinput_WindowMessage()` already receives the game window's messages (it handles `WM_INPUT` for
mouse look), and menus release capture. Add absolute pointer tracking while not captured:

- Handle `WM_MOUSEMOVE`, `WM_LBUTTONDOWN`, `WM_RBUTTONDOWN` and `WM_MOUSEWHEEL` in
  `pcinput_WindowMessage()`; store the client-space position, button presses and wheel steps in the
  same interlocked state `pc_input` uses for the mouse.
- Snapshot once per frame in `pcinput_BeginFrame()` (the menu runs on the game thread).
- New API, in `pc_input.h`'s style (`bool`/`float`):
  - `bool pcinput_MenuPointer(float *unitX, float *unitY)`: false when the pointer is outside the
    client area or hasn't moved since the last pad/keyboard menu input (see rule 5).
  - `bool pcinput_MenuPointerMoved()`: moved this frame.
  - `bool pcinput_TakeMenuClick(u32 button)`: left/right press this frame, consumed on read.
  - `int pcinput_TakeMenuWheel()`: wheel steps this frame (+ up).
- Conversion: client px → back-buffer px (scale by back-buffer/client size) → ftext unit space with
  the default viewport rect: `fX = (px - vpLeft) / ResX`, `fY = (py - vpTop) * 0.75 / ResY` (the
  inverse of `ftext.cpp` ~1680). Do this in one helper so letterboxing or widescreen changes only
  touch it.

### 2. Hit boxes from ftext

Menu items need their real on-screen extents (alignment, scale, `~w` fixed width, the arrow glyphs,
per-language text), so measure where ftext lays them out rather than estimating:

- `ftext_SetNextPrintTag( u32 nTag )`: tags the next `ftext_Printf` (0 = untagged).
- During `ftext_Draw()` layout, record the tagged string's pixel box in a small table (e.g. 64
  entries), converted back to unit space.
- `BOOL ftext_GetTaggedBox( u32 nTag, f32 *pfL, f32 *pfT, f32 *pfR, f32 *pfB )` returns the box from
  the last drawn frame. One frame of latency is invisible for hover and clicks.

### 3. Menu integration (`wpr_system.cpp`)

- **Tagging items.** `_DrawText()` gets an optional item id (default -1). Draw calls that pass
  `i == _MenuState.nCurItemIndex` as `bSelected` pass `i` too. `_DrawText` tags the print and adds
  `(tag, item, kind = LIST_ITEM)` to a per-frame hit table that is cleared when a screen starts drawing.
- **Button prompts.** In `wpr_drawutils_DrawButtonOverlay()`, add each drawn prompt as a hit region
  (icon rectangle from `fBiPolarUnitX/Y` and `fUnitHeight`, plus the tagged instruction text) of kind
  `BUTTON_A` / `BUTTON_B` / `BUTTON_Y` by index. **This alone makes every screen usable with the
  mouse** (click "A Select" = press A), including complex ones, so do it first.
- **Input helpers.**
  - `_CheckAcceptButtons()`: also TRUE for a left click on the selected list item or on an A prompt.
  - `_CheckBackButtons()`: also TRUE for a right click anywhere, or a left click on a B prompt.
  - `_CheckYButton()`: also TRUE for a left click on a Y prompt.
  - `_CheckUpDownAxis()`: also returns `_UP`/`_DOWN` for wheel steps.
  They only apply when `_MenuState.nControllerIndex` is the keyboard port (`pcinput_KeyboardPort()`).
- **Hover.** A helper `_MouseHover( s32 *pnCurItemIndex )` called at the top of each list screen's
  `Work`: when the pointer moved this frame and is over a list item whose index differs, set it, then
  `ftext_ResetBlinkTimers()` and play `WPR_DATATYPES_SOUNDS_CURSOR_MOVED`, as the pad path does.
  Screens with disabled items must skip them the way their own up/down code does, so give the helper
  an optional "is enabled" callback.
- **Screens with their own state:**
  - Sliders (sound, sensitivity, vibration): click left/right of the value = `_LEFT`/`_RIGHT` from
    `_CheckLeftRightAxis`; dragging can come later.
  - Profile-name keyboard: tag each key cell with its row/col; hover sets `nPNCurRow/Col`. Separately,
    accept typed characters (`WM_CHAR`) that exist on the on-screen keyboard.
  - Memory-unit and profile lists: they already index entries by `nCurItemIndex`; tag entries.
  - Level select (meshes): use the mesh layout's bipolar position and scale for the hit box.
  - Message boxes (`msgbox.cpp`): map clicks on its button prompts, as for screens.
- **Controller ownership.** The "press start" screen picks `_MenuState.nControllerIndex` with
  `wpr_system_FindActiveControllerPort()`; a left click there should pick the keyboard port.

### 4. The pointer

Nothing in the source is a mouse pointer (the consoles had none), so pick art that exists:

1. **The menu font's selection arrows.** `_DrawText` already draws `{`/`}` as arrow glyphs in font 1
   beside selected items. Draw `}` (or a rotated variant) with `ftext_Printf` at the pointer. No new
   assets, and it matches the menu style. **Recommended to start.**
2. **Reticle art.** `CReticle` loads `TFH_cross01`, `TFMPreticl1`, `TFMCretic01` and `TFH2crossD1`
   (`reticle.cpp` ~107). `wpr_drawutils_DrawTextureToScreen()` can draw any `CFTexInst` in menu
   space, as it does for the button icons.
3. **Search the retail textures.** List `.tga` names with `tools/mst_list.py` and look for arrows or
   hands in the `tfm*` (menu) and `tfh*` (HUD) sets; the user can check candidates by eye.

Draw the pointer last in the ortho pass so it's on top. Hide the Windows cursor over the client area
while the game draws its own (`WM_SETCURSOR` → `SetCursor( NULL )`), and show it again when outside or
when mouse look captures the mouse.

### 5. Rules so mouse and keys coexist

- Hover changes the selection only when the pointer **moves**, so a stationary pointer never fights the
  keyboard or pad.
- Any pad/keyboard menu input hides the pointer until the mouse moves again.
- Clicks outside hit regions do nothing (except right click = back).
- In gameplay nothing changes: `pcinput_BeginFrame( allowLook )` already separates menu and gameplay.

## Plan

1. `pc_input` pointer state and API (1); a debug print of the unit position to check the mapping.
2. ftext tags (2).
3. Button-prompt hit regions and the helper changes (3). **Every screen works with clicks after this.**
4. Pointer drawing (4), option 1.
5. Hover for list screens, then sliders, name keyboard, level select, message boxes.
6. Phase 2: the in-game pause menu (`PauseScreen.cpp`/`MenuTypes.cpp`) with the same `pc_input` API.

## Verify (needs a run)

- Pointer tip matches the item under it at 1280x960 and at a widescreen `-res`, windowed and
  fullscreen.
- Keyboard and pads still navigate every screen; a stationary pointer never steals the selection.
- A click on each button prompt does what the button does, including in message boxes.
- Mouse look still captures in gameplay and releases in menus (`F1`, Escape, Alt-Tab).
