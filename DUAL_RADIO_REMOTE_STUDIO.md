# Dual-Radio 2.4GHz Remote Studio & Stateful MQTT Extension (Preview)

This branch previews native **Dual-NRF24L01+ ESP32 physical remote capture**, a **61-model Futlight/MiBoxer Remote Studio UI (`/remotes-ui`)**, **`fut100` 100-zone protocol support**, and **stateful MQTT registry/event/state routing** for `esp8266_milight_hub`.

## 1. Hardware Target (Off-the-Shelf Dual-Radio ESP32)
Designed for Dual-NRF24L01+ ESP32 boards (e.g. AliExpress item `1005012187776793`) while remaining backwards-compatible with single-radio builds:
- **Radio #1 (VSPI):** Dedicated to standard `esp8266_milight_hub` gateway TX & bulb state management (zero TX/RX contention).
- **Radio #2 (HSPI — `HspiNRF24MiLightRadio`):** Dedicated continuous 2.4GHz listener capturing physical handheld remotes and capacitive glass wall panels (`fut089`, `rgb_cct`, `rgbw`, `fut091`, `cct`, `fut020`, `fut100`) without packet drops.

## 2. Native Web UI Integration (`/remotes-ui`)
- Styled around upstream `web2` (shadcn/ui + Tailwind tokens) with a synchronized `MainNav` header (`Dashboard · Sniffer · Remotes`) and shared `localStorage.theme` Light/Dark mode support.
- **61-Model Vector Faceplate Catalog (`miboxer_catalog.js`):** Interactive 1:1 SVGs for White & Black MiBoxer/Futlight handhelds, pucks, and wall panels (`FUT089`, `B8`, `B4`, `T4`, `FUT092`, `FUT096`, `FUT100`, `S1/S2`, `C1–C8`, etc.).
- **1-Click RF Auto-Detect & Guided Button Verification Wizard:** Auto-populates remote hex IDs on physical button press and steps through zone rockers, touch rings, sliders, and long-press `Night`/`White` actions.
- **Dual-Color Visual Telemetry:** Highlights faceplate controls in **Emerald Green** on physical 2.4GHz RF reception and **Amber** on virtual UI clicks/TX.

## 3. Stateful MQTT Contract
Registered remotes publish retained discovery metadata alongside structured per-zone event and state topics:
- **Discovery Registry (Retained):** `milight/remotes_registry/<hex_id>`
- **Per-Zone Instant Event:** `<topic_prefix>/zone/<0..N>/event`
- **Per-Zone Stateful Snapshot (Retained):** `<topic_prefix>/zone/<0..N>/state`
- **Unified Automation Stream:** `milight/ha_events`

## 4. Key Code Touchpoints
- `lib/Radio/HspiNRF24MiLightRadio.*` — Secondary HSPI NRF24L01+ continuous RX driver.
- `lib/MiLight/FUT100PacketFormatter.*` — 100-zone LCD remote packet formatter/parser.
- `lib/RemoteRegistry/` — Persistent remote profile store, MQTT discovery publisher, and gzipped UI headers (`RemoteStudioHtml.gz.h`, `PatchedRootHtml.gz.h`).
- `lib/WebServer/MiLightHttpServer.*` — REST endpoints (`/remotes-ui`, `/api/remotes`, `/api/remotes/last_seen`, `/api/remotes/virtual_send`).

*Note: Currently in preview/RFC stage — `RemoteStudioHtml` and `MainNav` link injection are packaged as PROGMEM headers for rapid testing and can be migrated directly into `web2/src/pages/` TypeScript components for upstream PR submission.*
