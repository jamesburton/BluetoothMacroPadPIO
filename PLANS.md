# Planning Backlog

## Objectives
- Provide a concrete, prioritized backlog that can be split across a swarm of developers.
- Start with persistence and sharing (import/export + gists) to support on-device editing.
- Define a stylized media view with standard icons and advanced layout control requirements.

## Phase 0: Discovery & Design (short sprint)
- [ ] **Confirm storage strategy**: SPIFFS vs LittleFS, storage partition size impact, migration plan.
- [ ] **Define interchange format** for profiles/macros (JSON) including versioning, metadata, and validation rules.
- [ ] **UX map for import/export**: entry points from settings, confirmation modals, and progress toasts.
- [ ] **Media view iconography**: pick a base set (play, pause, next, prev, volume, mute, record) and define a stylized visual system (stroke weight, corner radius, fill/outline rules).

## Phase 1: Import/Export & Persistence (highest priority)
### 1. Profile Persistence Layer
- [ ] Implement profile serialization/deserialization (JSON) with schema versioning.
- [ ] Add storage API for read/write/erase profile bundles.
- [ ] Add migration handler for older schema versions.
- [ ] Add validation and user feedback on invalid profile files.

### 2. Import/Export UI Flow
- [ ] Add settings entry: “Import / Export”.
- [ ] Export: choose scope (active profile vs all) and write JSON to file storage.
- [ ] Import: browse file, show preview (profile name, grid size, macro count), confirm overwrite.
- [ ] Toast/alert feedback for success/failure.

### 3. Gist Sharing Integration (opt-in)
- [ ] Add Wi-Fi setup flow (temporary AP or credentials entry) for remote sharing.
- [ ] Add “Export to GitHub Gist” (anonymous or token-based) with copyable URL/QR.
- [ ] Add “Import from Gist” using URL input and profile preview.
- [ ] Store tokens securely (NVS) and allow clearing.
- [ ] Rate-limit and handle offline/network errors.

## Phase 2: On-Device Editing Support
- [ ] Add “Edit Mode” UI for reassigning macros.
- [ ] Macro editor screens for: label, type, key combo, sequence, text.
- [ ] Persist edits immediately to storage and allow undo/rollback.

## Phase 3: Media View (Stylized Icons + Advanced Layout)
### 1. Media View Requirements
- [ ] Dedicated media profile layout with fixed icon grid and larger touch targets.
- [ ] Standard icon set: play/pause, next, previous, stop, volume up/down, mute.
- [ ] Stylized design system: consistent stroke widths, rounded corners, filled/outline variants, matching palette with theme.
- [ ] Icon rendering pipeline: support vector-like primitives (lines, circles, polygons) for crisp rendering at 480x480.
- [ ] Layout engine enhancements: per-button icon alignment, padding, and optional secondary text.

### 2. Rendering & Layout Tasks
- [ ] Create icon primitive renderer (triangles, arcs, rounded rects, strokes).
- [ ] Define icon definitions as data (points, radii, stroke widths).
- [ ] Add layout metadata to macros (icon name, icon size, alignment, optional text).
- [ ] Add caching for rendered icons to reduce redraw cost.
- [ ] Implement a “Media View” renderer mode using the icon system.

### 3. Interaction & Feedback
- [ ] Add pressed-state animation for icons (scale or glow).
- [ ] Provide optional hold actions (e.g., long press for mute).
- [ ] Provide visual state toggles (play vs pause).

## Phase 4: Enhancements (stretch)
- [ ] Web-based configuration panel that syncs with on-device storage.
- [ ] Theme editor for colors and fonts.
- [ ] Preset library of profiles with import bundles.
- [ ] Haptic feedback integration.

## Cross-Cutting Tasks
- [ ] Update documentation with import/export and media view usage.
- [ ] Add sample JSON profiles (general + media).
- [ ] Add unit tests for serialization/validation.
- [ ] Add UX review and performance profiling checklist.
