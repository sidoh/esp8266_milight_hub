import React, { useEffect, useRef, useState, useCallback } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Switch } from "@/components/ui/switch";
import { useToast } from "@/hooks/use-toast";

declare global {
  interface Window {
    MiBoxerCatalog?: {
      MODELS: CatalogModel[];
      renderModelSvg: (model: CatalogModel, opts?: { size?: string; uid?: string }) => string;
    };
  }
}

interface CatalogModel {
  id: string;
  name?: string;
  title?: string;
  sub?: string;
  cat: string;
  proto: string;
  zones: number;
  finish?: string;
  pairId?: string;
}

interface GestureConfig {
  short: boolean;
  long: boolean;
  double: boolean;
  triple: boolean;
  window_ms: number;
}

interface SurfaceSegmentConfig {
  enabled: boolean;
  segments: number;
  offset_deg?: number;
  zone_mask: number;
  labels: string[];
}

interface TouchZonesConfig {
  hue_ring: SurfaceSegmentConfig;
  cct_bar: SurfaceSegmentConfig;
  sat_bar: SurfaceSegmentConfig;
  bri_bar: SurfaceSegmentConfig;
}

interface RegisteredRemote {
  device_id: number;
  hex_id: string;
  remote_type: string;
  visual_model: string;
  name: string;
  topic_prefix: string;
  num_zones: number;
  active: boolean;
  gestures?: GestureConfig;
  touch_zones?: TouchZonesConfig;
}

interface RemoteEventItem {
  seq: number;
  age_ms?: number;
  device_id: number;
  hex_id: string;
  group_id: number;
  remote_type: string;
  action: string;
  gesture?: string;
  segment?: number;
  segments_total?: number;
  label?: string;
  value: number;
  v2_dec?: string;
  source: string;
}

const DEFAULT_GESTURES: GestureConfig = {
  short: true,
  long: true,
  double: false,
  triple: false,
  window_ms: 360,
};

const makeDefaultSurface = (segs: number, prefix: string): SurfaceSegmentConfig => ({
  enabled: false,
  segments: segs,
  offset_deg: 0,
  zone_mask: 0xffff,
  labels: Array.from({ length: segs }, (_, i) => `${prefix} ${i + 1}`),
});

const normalizeTouchZones = (tz?: Partial<TouchZonesConfig>): TouchZonesConfig => ({
  hue_ring: { ...makeDefaultSurface(6, "Slice"), ...(tz?.hue_ring || {}) },
  cct_bar: { ...makeDefaultSurface(4, "CCT"), ...(tz?.cct_bar || {}) },
  sat_bar: { ...makeDefaultSurface(4, "Sat"), ...(tz?.sat_bar || {}) },
  bri_bar: { ...makeDefaultSurface(4, "Preset"), ...(tz?.bri_bar || {}) },
});

const ACT_TO_SVG: Record<string, string[]> = {
  on: ["on", "zone_on"],
  off: ["off", "zone_off"],
  brightness: ["brightness"],
  saturation: ["saturation", "kelvin"],
  color_temp: ["kelvin", "saturation"],
  kelvin: ["kelvin", "saturation"],
  hue: ["hue"],
  mode_speed_up: ["mode_speed_up", "speed_up"],
  mode_speed_down: ["mode_speed_down", "speed_down"],
  set_white: ["set_white"],
  mode: ["mode", "next_mode"],
  next_mode: ["next_mode", "mode"],
  night_mode: ["off", "night_mode"],
};

const SVG_NS = "http://www.w3.org/2000/svg";

function polarPoint(cx: number, cy: number, r: number, degFromTopClockwise: number) {
  const rad = ((degFromTopClockwise - 90) * Math.PI) / 180;
  return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) };
}

function describeRingWedge(cx: number, cy: number, rInner: number, rOuter: number, startDeg: number, endDeg: number): string {
  const p1 = polarPoint(cx, cy, rOuter, startDeg);
  const p2 = polarPoint(cx, cy, rOuter, endDeg);
  const p3 = polarPoint(cx, cy, rInner, endDeg);
  const p4 = polarPoint(cx, cy, rInner, startDeg);
  const largeArc = endDeg - startDeg > 180 ? 1 : 0;
  return [
    `M ${p1.x.toFixed(2)} ${p1.y.toFixed(2)}`,
    `A ${rOuter.toFixed(2)} ${rOuter.toFixed(2)} 0 ${largeArc} 1 ${p2.x.toFixed(2)} ${p2.y.toFixed(2)}`,
    `L ${p3.x.toFixed(2)} ${p3.y.toFixed(2)}`,
    `A ${rInner.toFixed(2)} ${rInner.toFixed(2)} 0 ${largeArc} 0 ${p4.x.toFixed(2)} ${p4.y.toFixed(2)}`,
    "Z",
  ].join(" ");
}

export default function RemotesPage() {
  const { toast } = useToast();
  const [catalogReady, setCatalogReady] = useState<boolean>(Boolean(window.MiBoxerCatalog));
  const [remotes, setRemotes] = useState<RegisteredRemote[]>([]);
  const [selectedId, setSelectedId] = useState<number | null>(null);
  const [activeZone, setActiveZone] = useState<number>(1);
  const [lastRf, setLastRf] = useState<RemoteEventItem | null>(null);
  const [logs, setLogs] = useState<string[]>([]);
  const [svgCaption, setSvgCaption] = useState<string>("Click any button, wheel slice, or bar slot — or press your physical remote");

  // Virtual Sliders
  const [hueVal, setHueVal] = useState<number>(180);
  const [briVal, setBriVal] = useState<number>(80);
  const [cctVal, setCctVal] = useState<number>(50);

  // Add / Edit Hardware Faceplate Modal
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [modalMode, setModalMode] = useState<"add" | "edit">("add");
  const [wizCat, setWizCat] = useState<string>("all");
  const [wHex, setWHex] = useState<string>("0x125C");
  const [wProto, setWProto] = useState<string>("fut089");
  const [wModel, setWModel] = useState<string>("FUT089");
  const [wName, setWName] = useState<string>("Living Room Remote");
  const [wPrefix, setWPrefix] = useState<string>("milight/actions/rem_125c");
  const [wZones, setWZones] = useState<number>(8);
  const [modalSniffBanner, setModalSniffBanner] = useState<string>("Listening on 2.4GHz... Press ANY button on your physical remote!");

  // Gestures & Graphical Touch-Zone Configurator Modal
  const [macroOpen, setMacroOpen] = useState<boolean>(false);
  const [editGestures, setEditGestures] = useState<GestureConfig>(DEFAULT_GESTURES);
  const [editTouch, setEditTouch] = useState<TouchZonesConfig>(normalizeTouchZones());
  const [activeSurfaceTab, setActiveSurfaceTab] = useState<"hue_ring" | "cct_bar" | "sat_bar" | "bri_bar">("hue_ring");

  const svgStageRef = useRef<HTMLDivElement | null>(null);
  const seenSeqRef = useRef<number>(-1);
  const localUiSuppressRef = useRef<Record<string, number>>({});
  const selectedIdRef = useRef<number | null>(null);
  const activeZoneRef = useRef<number>(1);
  const modalOpenRef = useRef<boolean>(false);
  const modalModeRef = useRef<"add" | "edit">("add");

  selectedIdRef.current = selectedId;
  activeZoneRef.current = activeZone;
  modalOpenRef.current = modalOpen;
  modalModeRef.current = modalMode;

  useEffect(() => {
    if (document.getElementById("mbx-studio-styles")) return;
    const st = document.createElement("style");
    st.id = "mbx-studio-styles";
    st.textContent = `
      .mbx-hit, .mbx-seg-hit { cursor: pointer; transition: filter 0.14s ease, fill 0.15s ease; }
      .mbx-hit:hover, .mbx-seg-hit:hover { filter: brightness(1.12); }
      .mbx-seg-hit:hover { fill: rgba(2, 132, 199, 0.28) !important; }
      .mbx-hit.zone-active > circle, .mbx-hit.zone-active > rect, circle.mbx-hit.zone-active, rect.mbx-hit.zone-active {
        stroke: #ea580c !important; stroke-width: 2.6px !important;
      }
      .mbx-hit.fire-rf > rect, .mbx-hit.fire-rf > circle, rect.mbx-hit.fire-rf, circle.mbx-hit.fire-rf,
      .mbx-seg-hit.fire-rf {
        fill: rgba(34, 197, 94, 0.65) !important; stroke: #15803d !important; stroke-width: 3px !important;
      }
      .mbx-hit.fire-ui > rect, .mbx-hit.fire-ui > circle, rect.mbx-hit.fire-ui, circle.mbx-hit.fire-ui,
      .mbx-seg-hit.fire-ui {
        fill: rgba(245, 158, 11, 0.65) !important; stroke: #d97706 !important; stroke-width: 3px !important;
      }
    `;
    document.head.appendChild(st);
  }, []);

  useEffect(() => {
    if (window.MiBoxerCatalog) {
      setCatalogReady(true);
      return;
    }
    const sc = document.createElement("script");
    sc.src = "/miboxer_catalog.js";
    sc.async = true;
    sc.onload = () => setCatalogReady(true);
    document.head.appendChild(sc);
  }, []);

  const loadRemotes = useCallback(async () => {
    try {
      const res = await fetch("/api/remotes");
      const data: RegisteredRemote[] = await res.json();
      if (Array.isArray(data)) {
        setRemotes(data);
        if (data.length > 0 && (selectedIdRef.current === null || !data.some((r) => r.device_id === selectedIdRef.current))) {
          setSelectedId(data[0].device_id);
        }
      }
    } catch (e) {
      console.error("Failed loading /api/remotes", e);
    }
  }, []);

  useEffect(() => {
    loadRemotes();
  }, [loadRemotes]);

  const currentRemote = remotes.find((r) => r.device_id === selectedId) || null;
  const catalogModels = window.MiBoxerCatalog?.MODELS || [];
  const currentCatalogModel =
    catalogModels.find((m) => m.id === currentRemote?.visual_model) ||
    catalogModels.find((m) => m.proto === currentRemote?.remote_type) ||
    catalogModels[0];

  const pulseFaceplate = useCallback((action: string, zone: number, isRf: boolean) => {
    const stage = svgStageRef.current;
    if (!stage) return;
    const cls = isRf ? "fire-rf" : "fire-ui";

    // Check segmented slice/slot hit first (e.g. hue_seg_3, cct_seg_2)
    const segEls = stage.querySelectorAll(`.mbx-seg-hit[data-seg-act="${action}"]`);
    if (segEls.length > 0) {
      segEls.forEach((el) => {
        el.classList.remove("fire-rf", "fire-ui");
        void (el as HTMLElement).offsetWidth;
        el.classList.add(cls);
        setTimeout(() => el.classList.remove(cls), 800);
      });
    } else {
      const acts = ACT_TO_SVG[action] || [action];
      acts.forEach((a) => {
        stage.querySelectorAll(`.mbx-hit[data-act="${a}"]`).forEach((el) => {
          el.classList.remove("fire-rf", "fire-ui");
          void (el as HTMLElement).offsetWidth;
          el.classList.add(cls);
          setTimeout(() => el.classList.remove(cls), 720);
        });
      });
    }

    stage.querySelectorAll(`.mbx-hit[data-act="zone"][data-zone="${zone}"]`).forEach((el) => {
      el.classList.remove("fire-rf", "fire-ui");
      void (el as HTMLElement).offsetWidth;
      el.classList.add(cls);
      setTimeout(() => el.classList.remove(cls), 720);
    });
  }, []);

  const sendVirtualTx = useCallback(
    async (action: string, value: number, zoneOverride?: number, remOverride?: RegisteredRemote | null, customLabel?: string) => {
      const rem = remOverride ?? currentRemote;
      if (!rem) return;
      const z = zoneOverride !== undefined ? zoneOverride : activeZoneRef.current;
      const apiAction = action === "kelvin" ? "color_temp" : action;
      localUiSuppressRef.current[`${rem.device_id}:${z}:${apiAction}`] = Date.now();
      pulseFaceplate(apiAction, z, false);
      const stamp = new Date().toLocaleTimeString();
      setSvgCaption(`[Virtual UI] Zone ${z} → ${customLabel ? `${apiAction} ("${customLabel}")` : `${apiAction}(${value})`}`);
      setLogs((prev) => [
        `[${stamp}] 🟠 UI TX  ${rem.hex_id} Z${z} -> ${apiAction}${customLabel ? ` [${customLabel}]` : ` (${value})`}`,
        ...prev.slice(0, 79),
      ]);

      try {
        await fetch("/api/remotes/virtual_send", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({
            device_id: rem.device_id,
            remote_type: rem.remote_type,
            group_id: z,
            action: apiAction,
            value: Number(value),
          }),
        });
      } catch (e) {
        console.error("virtual_send error", e);
      }
    },
    [currentRemote, pulseFaceplate]
  );

  // Render interactive 1:1 SVG faceplate + Graphical Segmented Ring & Bar Overlays!
  useEffect(() => {
    const stage = svgStageRef.current;
    if (!stage || !catalogReady || !window.MiBoxerCatalog || !currentCatalogModel || !currentRemote) {
      if (stage && !currentRemote) stage.innerHTML = "";
      return;
    }
    stage.innerHTML = window.MiBoxerCatalog.renderModelSvg(currentCatalogModel, {
      size: "large",
      uid: `hub_${currentRemote.device_id}`,
    });

    const svgEl = stage.querySelector("svg");
    if (svgEl) {
      svgEl.style.maxHeight = "365px";
      svgEl.style.width = "auto";
      svgEl.style.display = "block";
    }

    stage.querySelectorAll(`.mbx-hit[data-act="zone"][data-zone="${activeZone}"]`).forEach((el) => el.classList.add("zone-active"));

    const tz = normalizeTouchZones(currentRemote.touch_zones);
    const zBit = activeZone < 16 ? 1 << activeZone : 0x8000;

    // Helper: Draw Segmented Ring Overlay directly onto the SVG Color Wheel!
    if (svgEl && tz.hue_ring.enabled && (tz.hue_ring.zoneMask & zBit)) {
      const hueEl = svgEl.querySelector('.mbx-hit[data-act="hue"]') as SVGGraphicsElement | null;
      if (hueEl) {
        const bb = hueEl.getBBox();
        const cx = bb.x + bb.width / 2;
        const cy = bb.y + bb.height / 2;
        const rOuter = Math.max(bb.width, bb.height) / 2;
        const rInner = rOuter * 0.52;
        const n = Math.min(12, Math.max(2, tz.hue_ring.segments));
        const span = 360 / n;
        const offset = tz.hue_ring.offset_deg || 0;

        const gOverlay = document.createElementNS(SVG_NS, "g");
        for (let i = 0; i < n; i++) {
          const sDeg = offset + i * span;
          const eDeg = sDeg + span;
          const midDeg = Math.round(((sDeg + span / 2) % 360 + 360) % 360);
          const lbl = tz.hue_ring.labels[i] || `S${i + 1}`;
          const segAct = `hue_seg_${i + 1}`;

          const path = document.createElementNS(SVG_NS, "path");
          path.setAttribute("d", describeRingWedge(cx, cy, rInner, rOuter, sDeg, eDeg));
          path.setAttribute("fill", "rgba(15, 23, 42, 0.14)");
          path.setAttribute("stroke", "#0f172a");
          path.setAttribute("stroke-width", "1.6");
          path.setAttribute("class", "mbx-seg-hit");
          path.setAttribute("data-seg-act", segAct);
          const titleEl = document.createElementNS(SVG_NS, "title");
          titleEl.textContent = `Slice ${i + 1}: ${lbl} (${segAct})`;
          path.appendChild(titleEl);
          path.onclick = (ev) => {
            ev.stopPropagation();
            sendVirtualTx(segAct, midDeg, activeZoneRef.current, currentRemote, lbl);
          };
          gOverlay.appendChild(path);

          const lp = polarPoint(cx, cy, (rInner + rOuter) / 2, sDeg + span / 2);
          const pill = document.createElementNS(SVG_NS, "circle");
          pill.setAttribute("cx", lp.x.toFixed(1));
          pill.setAttribute("cy", lp.y.toFixed(1));
          pill.setAttribute("r", "7.5");
          pill.setAttribute("fill", "#0f172a");
          pill.setAttribute("pointer-events", "none");
          gOverlay.appendChild(pill);

          const txt = document.createElementNS(SVG_NS, "text");
          txt.setAttribute("x", lp.x.toFixed(1));
          txt.setAttribute("y", (lp.y + 3).toFixed(1));
          txt.setAttribute("text-anchor", "middle");
          txt.setAttribute("font-size", "8.5");
          txt.setAttribute("font-weight", "800");
          txt.setAttribute("fill", "#ffffff");
          txt.setAttribute("pointer-events", "none");
          txt.textContent = String(i + 1);
          gOverlay.appendChild(txt);
        }
        hueEl.parentNode?.appendChild(gOverlay);
      }
    }

    // Helper: Draw Segmented Bar Overlays onto CCT / Saturation / Brightness sliders!
    const drawBarSegments = (selector: string, cfg: SurfaceSegmentConfig, prefixAct: string) => {
      if (!svgEl || !cfg.enabled || !(cfg.zoneMask & zBit)) return;
      const barEl = svgEl.querySelector(selector) as SVGGraphicsElement | null;
      if (!barEl) return;
      const bb = barEl.getBBox();
      if (!bb.width || !bb.height) return;
      const n = Math.min(6, Math.max(2, cfg.segments));
      const vertical = bb.height > bb.width * 1.3;
      const gBar = document.createElementNS(SVG_NS, "g");

      for (let i = 0; i < n; i++) {
        // Slot 1 = left (horizontal) or bottom (vertical)
        const idxVis = vertical ? n - 1 - i : i;
        const rx = vertical ? bb.x : bb.x + (idxVis * bb.width) / n;
        const ry = vertical ? bb.y + (idxVis * bb.height) / n : bb.y;
        const rw = vertical ? bb.width : bb.width / n;
        const rh = vertical ? bb.height / n : bb.height;
        const midPct = Math.round(((i + 0.5) / n) * 100);
        const lbl = cfg.labels[i] || `${i + 1}`;
        const segAct = `${prefixAct}${i + 1}`;

        const rect = document.createElementNS(SVG_NS, "rect");
        rect.setAttribute("x", rx.toFixed(1));
        rect.setAttribute("y", ry.toFixed(1));
        rect.setAttribute("width", rw.toFixed(1));
        rect.setAttribute("height", rh.toFixed(1));
        rect.setAttribute("rx", "3");
        rect.setAttribute("fill", "rgba(2, 132, 199, 0.16)");
        rect.setAttribute("stroke", "#0284c7");
        rect.setAttribute("stroke-width", "1.4");
        rect.setAttribute("class", "mbx-seg-hit");
        rect.setAttribute("data-seg-act", segAct);
        const tEl = document.createElementNS(SVG_NS, "title");
        tEl.textContent = `Slot ${i + 1}: ${lbl} (${segAct})`;
        rect.appendChild(tEl);
        rect.onclick = (ev) => {
          ev.stopPropagation();
          sendVirtualTx(segAct, midPct, activeZoneRef.current, currentRemote, lbl);
        };
        gBar.appendChild(rect);

        const txt = document.createElementNS(SVG_NS, "text");
        txt.setAttribute("x", (rx + rw / 2).toFixed(1));
        txt.setAttribute("y", (ry + rh / 2 + 3).toFixed(1));
        txt.setAttribute("text-anchor", "middle");
        txt.setAttribute("font-size", "8");
        txt.setAttribute("font-weight", "800");
        txt.setAttribute("fill", "#0f172a");
        txt.setAttribute("pointer-events", "none");
        txt.textContent = String(i + 1);
        gBar.appendChild(txt);
      }
      barEl.parentNode?.appendChild(gBar);
    };

    drawBarSegments('.mbx-hit[data-act="kelvin"]', tz.cct_bar, "cct_seg_");
    drawBarSegments('.mbx-hit[data-act="saturation"]', tz.sat_bar, "sat_seg_");
    drawBarSegments('.mbx-hit[data-act="brightness"]', tz.bri_bar, "bri_seg_");

    // Standard button & continuous wheel/slider handlers
    stage.querySelectorAll(".mbx-hit").forEach((el) => {
      let holdTimer: ReturnType<typeof setTimeout> | null = null;
      let heldFired = false;

      (el as HTMLElement).onpointerdown = () => {
        const a0 = el.getAttribute("data-act") || "";
        if (a0 !== "on" && a0 !== "off" && a0 !== "zone_on" && a0 !== "zone_off") return;
        heldFired = false;
        holdTimer = setTimeout(() => {
          heldFired = true;
          const zAttr = el.getAttribute("data-zone");
          const zNext = zAttr !== null && zAttr !== "" ? Number(zAttr) : activeZoneRef.current;
          if (zAttr !== null && zAttr !== "") setActiveZone(zNext);
          const longAct = a0 === "off" || a0 === "zone_off" ? "night_mode" : "set_white";
          sendVirtualTx(longAct, 0, zNext, currentRemote);
        }, 650);
      };
      const clr = () => {
        if (holdTimer) {
          clearTimeout(holdTimer);
          holdTimer = null;
        }
      };
      (el as HTMLElement).onpointerup = clr;
      (el as HTMLElement).onpointerleave = clr;

      (el as HTMLElement).onclick = (e: MouseEvent) => {
        e.stopPropagation();
        if (heldFired) {
          heldFired = false;
          return;
        }
        const act = el.getAttribute("data-act") || "on";
        const zAttr = el.getAttribute("data-zone");
        let zNext = activeZoneRef.current;
        if (zAttr !== null && zAttr !== "") {
          zNext = Number(zAttr);
          setActiveZone(zNext);
        }
        if (act === "zone") return;

        let val: number | undefined = el.getAttribute("data-val") !== null ? Number(el.getAttribute("data-val")) : undefined;
        if (val === undefined) {
          const rect = el.getBoundingClientRect();
          if (act === "hue" && rect.width) {
            let deg = Math.round(
              (Math.atan2(e.clientY - (rect.top + rect.height / 2), e.clientX - (rect.left + rect.width / 2)) * 180) / Math.PI + 90
            );
            if (deg < 0) deg += 360;
            val = deg % 360;
            setHueVal(val);
          } else if ((act === "brightness" || act === "saturation" || act === "kelvin") && rect.width) {
            const frac = rect.height > rect.width * 1.3 ? 1 - (e.clientY - rect.top) / rect.height : (e.clientX - rect.left) / rect.width;
            const rawP = Math.max(0, Math.min(100, Math.round(frac * 100)));
            const pct = rawP <= 6 ? 0 : rawP >= 94 ? 100 : rawP;
            if (act === "brightness") setBriVal(pct);
            if (act === "kelvin") setCctVal(pct);
            val = pct;
          } else {
            val = 100;
          }
        }
        sendVirtualTx(act, val, zNext, currentRemote);
      };
    });
  }, [catalogReady, currentCatalogModel, currentRemote, activeZone, sendVirtualTx]);

  // Poll /api/remotes/last_seen @ 320ms
  useEffect(() => {
    const timer = setInterval(async () => {
      try {
        const res = await fetch("/api/remotes/last_seen");
        const data = await res.json();
        if (!data || !data.seq) return;
        if (seenSeqRef.current === -1) {
          seenSeqRef.current = Number(data.seq);
          if (data.device_id) setLastRf(data);
          return;
        }
        if (Number(data.seq) <= seenSeqRef.current) return;

        const recent: RemoteEventItem[] = Array.isArray(data.recent) && data.recent.length ? data.recent : [data];
        const fresh = recent.filter((ev) => Number(ev.seq) > seenSeqRef.current);
        seenSeqRef.current = Number(data.seq);

        for (const ev of fresh) {
          setLastRf(ev);
          const isVirtual = ev.source === "virtual";
          const supKey = `${ev.device_id}:${ev.group_id}:${ev.action}`;
          if (isVirtual && Date.now() - (localUiSuppressRef.current[supKey] || 0) < 750) continue;

          const stamp = new Date().toLocaleTimeString();
          const tag = isVirtual ? "🟠 UI" : "🟢 RF";
          const gBadge =
            ev.gesture === "double"
              ? " [2x DOUBLE]"
              : ev.gesture === "triple"
              ? " [3x TRIPLE]"
              : ev.gesture === "long"
              ? " [LONG HOLD]"
              : "";
          const segText = ev.label ? ` -> "${ev.label}"` : ev.value !== -1 ? ` (${ev.value})` : "";
          setLogs((prev) => [
            `[${stamp}] ${tag}${gBadge} ${ev.hex_id} Z${ev.group_id} -> ${ev.action}${segText}`,
            ...prev.slice(0, 79),
          ]);

          if (modalOpenRef.current && modalModeRef.current === "add" && !isVirtual && ev.device_id) {
            setWHex(ev.hex_id);
            setWProto(ev.remote_type);
            const cleanHex = ev.hex_id.replace(/^0x/i, "").toLowerCase();
            setWPrefix(`milight/actions/rem_${cleanHex}`);
            const matchingMod = (window.MiBoxerCatalog?.MODELS || []).find((m) => m.proto === ev.remote_type);
            if (matchingMod) {
              setWModel(matchingMod.id);
              setWZones(matchingMod.zones || 8);
              setWName(`${matchingMod.id} (${ev.hex_id})`);
            }
            setModalSniffBanner(`🟢 Captured Physical Remote ${ev.hex_id} (${ev.remote_type}, Zone ${ev.group_id} ${ev.action})!`);
          }

          if (ev.device_id === selectedIdRef.current) {
            setActiveZone(ev.group_id);
            pulseFaceplate(ev.action, ev.group_id, !isVirtual);
            setSvgCaption(
              `${isVirtual ? "[Virtual UI]" : "[Physical 2.4GHz RF]"}${gBadge} Z${ev.group_id} → ${ev.action}${
                ev.label ? ` ("${ev.label}")` : ev.value !== -1 ? `(${ev.value})` : ""
              }`
            );
          }
        }
      } catch (_) {}
    }, 320);
    return () => clearInterval(timer);
  }, [pulseFaceplate]);

  const openConfigurator = (mode: "add" | "edit", adoptEv?: RemoteEventItem | null) => {
    setModalMode(mode);
    if (mode === "edit" && currentRemote) {
      setWHex(currentRemote.hex_id);
      setWProto(currentRemote.remote_type);
      setWModel(currentRemote.visual_model);
      setWName(currentRemote.name);
      setWPrefix(currentRemote.topic_prefix);
      setWZones(currentRemote.num_zones);
      setModalSniffBanner(`Editing ${currentRemote.name} (${currentRemote.hex_id})`);
    } else if (adoptEv && adoptEv.device_id) {
      setWHex(adoptEv.hex_id);
      setWProto(adoptEv.remote_type);
      const cleanHex = adoptEv.hex_id.replace(/^0x/i, "").toLowerCase();
      setWPrefix(`milight/actions/rem_${cleanHex}`);
      const matchingMod = catalogModels.find((m) => m.proto === adoptEv.remote_type) || catalogModels[0];
      setWModel(matchingMod?.id || "FUT089");
      setWZones(matchingMod?.zones || 8);
      setWName(`${matchingMod?.id || "Remote"} (${adoptEv.hex_id})`);
      setModalSniffBanner(`🟢 Adopted ${adoptEv.hex_id} from Radio #2 Sniffer!`);
    } else {
      setWHex("0x8A10");
      setWProto("fut089");
      setWModel("FUT089");
      setWName("New MiBoxer Remote");
      setWPrefix("milight/actions/rem_8a10");
      setWZones(8);
      setModalSniffBanner("Listening on 2.4GHz... Press ANY button on your physical remote!");
    }
    setModalOpen(true);
  };

  const openMacroStudio = () => {
    if (!currentRemote) return;
    setEditGestures({ ...DEFAULT_GESTURES, ...(currentRemote.gestures || {}) });
    setEditTouch(normalizeTouchZones(currentRemote.touch_zones));
    const proto = currentRemote.remote_type;
    if (proto === "fut091" || proto === "cct") setActiveSurfaceTab("cct_bar");
    else setActiveSurfaceTab("hue_ring");
    setMacroOpen(true);
  };

  const saveMacroStudio = async () => {
    if (!currentRemote) return;
    await fetch("/api/remotes", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        device_id: currentRemote.device_id,
        remote_type: currentRemote.remote_type,
        visual_model: currentRemote.visual_model,
        name: currentRemote.name,
        topic_prefix: currentRemote.topic_prefix,
        num_zones: currentRemote.num_zones,
        active: true,
        gestures: editGestures,
        touch_zones: editTouch,
      }),
    });
    setMacroOpen(false);
    await loadRemotes();
    toast({
      title: "Gestures & Touch Zones Saved",
      description: "Updated ESP32 Hub firmware & synced to Homebridge via MQTT discovery.",
    });
  };

  const saveRemoteConfig = async () => {
    const parsedId = parseInt(wHex.trim().replace(/^0x/i, ""), 16);
    if (!parsedId || Number.isNaN(parsedId)) {
      toast({ title: "Invalid Hex ID", description: "Enter a valid 16-bit hex ID such as 0x125C" });
      return;
    }
    await fetch("/api/remotes", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        device_id: parsedId,
        remote_type: wProto,
        visual_model: wModel,
        name: wName.trim() || `${wModel} (${wHex})`,
        topic_prefix: wPrefix.trim() || `milight/actions/rem_${parsedId.toString(16).toLowerCase()}`,
        num_zones: Number(wZones) || 8,
        active: true,
      }),
    });
    setModalOpen(false);
    await loadRemotes();
    setSelectedId(parsedId);
    toast({ title: "Remote Saved", description: `${wName} published to MQTT discovery registry.` });
  };

  const deleteSelectedRemote = async () => {
    if (!currentRemote) return;
    if (!window.confirm(`Delete ${currentRemote.name} (${currentRemote.hex_id})?`)) return;
    await fetch("/api/remotes/delete", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ device_id: currentRemote.device_id }),
    });
    setSelectedId(null);
    await loadRemotes();
  };

  const updateSurfaceField = (surf: keyof TouchZonesConfig, patch: Partial<SurfaceSegmentConfig>) => {
    setEditTouch((prev) => {
      const nextSurf = { ...prev[surf], ...patch };
      if (patch.segments !== undefined) {
        const labels = [...nextSurf.labels];
        while (labels.length < patch.segments) {
          labels.push(`${surf === "hue_ring" ? "Slice" : "Slot"} ${labels.length + 1}`);
        }
        nextSurf.labels = labels.slice(0, patch.segments);
      }
      return { ...prev, [surf]: nextSurf };
    });
  };

  const filteredWizardModels = catalogModels.filter((m) => (wizCat === "all" ? true : m.cat === wizCat));
  const curSurf = editTouch[activeSurfaceTab];
  const curTz = currentRemote ? normalizeTouchZones(currentRemote.touch_zones) : null;
  const activeZoneBit = activeZone < 16 ? 1 << activeZone : 0x8000;
  const activeSegmentedBadges = curTz
    ? [
        curTz.hue_ring.enabled && (curTz.hue_ring.zoneMask & activeZoneBit) ? `Wheel: ${curTz.hue_ring.segments} Slices` : null,
        curTz.cct_bar.enabled && (curTz.cct_bar.zoneMask & activeZoneBit) ? `CCT: ${curTz.cct_bar.segments} Slots` : null,
        curTz.sat_bar.enabled && (curTz.sat_bar.zoneMask & activeZoneBit) ? `Sat: ${curTz.sat_bar.segments} Slots` : null,
        curTz.bri_bar.enabled && (curTz.bri_bar.zoneMask & activeZoneBit) ? `Bri: ${curTz.bri_bar.segments} Slots` : null,
      ].filter(Boolean)
    : [];

  return (
    <div className="space-y-6 pb-10">
      <div className="flex flex-wrap items-center justify-between gap-4 border-b pb-4">
        <div className="flex flex-wrap items-center gap-3">
          <h1 className="text-2xl font-bold tracking-tight">Remotes Studio</h1>
          <Badge variant="secondary">Dual-Radio 2.4GHz + Multi-Tap + Touch-Zone Macro Pads</Badge>
          <span className="flex items-center gap-3 text-xs text-muted-foreground ml-2">
            <span className="inline-flex items-center gap-1.5">
              <span className="h-2.5 w-2.5 rounded-full bg-green-500 inline-block" /> Physical RF
            </span>
            <span className="inline-flex items-center gap-1.5">
              <span className="h-2.5 w-2.5 rounded-full bg-amber-500 inline-block" /> Virtual UI Click
            </span>
          </span>
        </div>
        <Button onClick={() => openConfigurator("add")}>+ Detect / Add Remote</Button>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* LEFT COLUMN */}
        <div className="lg:col-span-4 space-y-6">
          <Card>
            <CardHeader className="pb-3">
              <CardTitle className="text-base">Configured Remotes &amp; Wall Panels ({remotes.length})</CardTitle>
            </CardHeader>
            <CardContent className="space-y-2">
              {remotes.map((r) => {
                const isSel = r.device_id === selectedId;
                const g = r.gestures;
                const hasMulti = Boolean(g?.double || g?.triple);
                return (
                  <div
                    key={r.device_id}
                    onClick={() => {
                      setSelectedId(r.device_id);
                      setActiveZone(1);
                    }}
                    className={`p-3 rounded-lg border cursor-pointer transition-colors flex items-center justify-between ${
                      isSel ? "border-primary bg-accent/60" : "hover:bg-accent/30"
                    }`}
                  >
                    <div>
                      <div className="font-semibold text-sm">{r.name}</div>
                      <div className="text-xs text-muted-foreground font-mono">
                        {r.visual_model} • {r.hex_id} • {r.num_zones}Z {hasMulti ? "• 2x/3x" : ""}
                      </div>
                    </div>
                    <Badge variant={isSel ? "default" : "outline"}>{r.visual_model}</Badge>
                  </div>
                );
              })}
            </CardContent>
          </Card>

          <Card>
            <CardHeader className="pb-2">
              <CardTitle className="text-base">Live Radio #2 RF Sniffer</CardTitle>
            </CardHeader>
            <CardContent>
              {lastRf && lastRf.device_id ? (
                <div
                  onClick={() => {
                    const existing = remotes.find((r) => r.device_id === lastRf.device_id);
                    if (existing) setSelectedId(existing.device_id);
                    else openConfigurator("add", lastRf);
                  }}
                  className="p-3 rounded-lg border bg-muted/40 hover:border-primary cursor-pointer space-y-1 font-mono text-xs"
                >
                  <div className="flex items-center justify-between">
                    <span className="font-bold text-green-600 dark:text-green-400">
                      {lastRf.source === "virtual" ? "🟠 VIRTUAL TX" : "🟢 2.4GHz RF CAPTURE"}
                    </span>
                    <span>seq #{lastRf.seq}</span>
                  </div>
                  <div>
                    ID: <b>{lastRf.hex_id}</b> • Zone: <b>{lastRf.group_id}</b> • Gesture: <b>{lastRf.gesture || "single"}</b>
                  </div>
                  <div>
                    Action: <b>{lastRf.action}</b> {lastRf.label ? `("${lastRf.label}")` : lastRf.value !== -1 ? `(${lastRf.value})` : ""}
                  </div>
                </div>
              ) : (
                <div className="p-3 rounded-lg border bg-muted/30 text-xs text-muted-foreground font-mono">
                  Waiting for 2.4GHz remote press...
                </div>
              )}
            </CardContent>
          </Card>
        </div>

        {/* RIGHT COLUMN */}
        <div className="lg:col-span-8">
          <Card>
            <CardHeader className="flex flex-row flex-wrap items-center justify-between gap-3 border-b pb-4">
              <div>
                <CardTitle className="text-lg">{currentRemote ? currentRemote.name : "Select a Remote"}</CardTitle>
                <div className="flex flex-wrap items-center gap-1.5 mt-1">
                  <span className="text-xs text-muted-foreground font-mono">
                    {currentRemote
                      ? `${currentRemote.visual_model} • ${currentRemote.hex_id} • ${currentRemote.remote_type} (${currentRemote.num_zones}Z)`
                      : "Choose a remote"}
                  </span>
                  {currentRemote?.gestures?.double && <Badge variant="secondary">2× Double Tap</Badge>}
                  {currentRemote?.gestures?.triple && <Badge variant="secondary">3× Triple Tap</Badge>}
                  {currentRemote?.gestures?.long !== false && <Badge variant="outline">Hold OFF</Badge>}
                  {activeSegmentedBadges.map((b, idx) => (
                    <Badge key={idx} className="bg-sky-600 text-white">
                      {b}
                    </Badge>
                  ))}
                </div>
              </div>
              {currentRemote && (
                <div className="flex flex-wrap items-center gap-2">
                  <Button size="sm" onClick={openMacroStudio}>
                    Gestures &amp; Touch Zones
                  </Button>
                  <Button variant="outline" size="sm" onClick={() => openConfigurator("edit")}>
                    Edit Faceplate
                  </Button>
                  <Button variant="destructive" size="sm" onClick={deleteSelectedRemote}>
                    Delete
                  </Button>
                </div>
              )}
            </CardHeader>

            <CardContent className="pt-6">
              {currentRemote && (
                <div className="grid grid-cols-1 md:grid-cols-12 gap-6 items-start">
                  {/* 1:1 Vector Faceplate + Live Segmented Ring/Bar Overlays */}
                  <div className="md:col-span-5 flex flex-col items-center rounded-xl border bg-muted/30 p-5">
                    <div ref={svgStageRef} className="min-h-[350px] flex items-center justify-center w-full" />
                    <div className="mt-3 text-center text-xs font-mono text-muted-foreground">{svgCaption}</div>
                  </div>

                  {/* Virtual Transmitter + Segmented Quick Pad + Log */}
                  <div className="md:col-span-7 space-y-4">
                    <div>
                      <Label className="text-xs uppercase tracking-wider text-muted-foreground">Active Zone</Label>
                      <div className="flex flex-wrap gap-1.5 mt-1.5">
                        {[0, ...Array.from({ length: Math.min(currentRemote.num_zones, 8) }, (_, i) => i + 1)].map((z) => (
                          <Button
                            key={z}
                            size="sm"
                            variant={activeZone === z ? "default" : "outline"}
                            className="h-8 px-3 text-xs"
                            onClick={() => setActiveZone(z)}
                          >
                            {z === 0 ? "All (Z0)" : `Z${z}`}
                          </Button>
                        ))}
                      </div>
                    </div>

                    {/* Show clickable labeled Macro-Pad summary whenever active zone has segmented rings/bars! */}
                    {curTz && (curTz.hue_ring.enabled || curTz.cct_bar.enabled || curTz.sat_bar.enabled || curTz.bri_bar.enabled) && (
                      <div className="rounded-lg border border-sky-500/40 bg-sky-500/5 p-3 space-y-2">
                        <div className="text-xs font-bold text-sky-600 dark:text-sky-400">
                          Active Segmented Macro-Pad Buttons (Zone {activeZone})
                        </div>
                        {(["hue_ring", "cct_bar", "sat_bar", "bri_bar"] as const).map((sk) => {
                          const sc = curTz[sk];
                          if (!sc.enabled || !(sc.zoneMask & activeZoneBit)) return null;
                          const pref = sk === "hue_ring" ? "hue_seg_" : sk === "cct_bar" ? "cct_seg_" : sk === "sat_bar" ? "sat_seg_" : "bri_seg_";
                          return (
                            <div key={sk} className="flex flex-wrap gap-1.5">
                              {Array.from({ length: sc.segments }, (_, i) => {
                                const lbl = sc.labels[i] || `#${i + 1}`;
                                return (
                                  <Button
                                    key={i}
                                    size="sm"
                                    variant="outline"
                                    className="h-7 text-xs border-sky-500/50"
                                    onClick={() => sendVirtualTx(`${pref}${i + 1}`, i + 1, activeZone, currentRemote, lbl)}
                                  >
                                    #{i + 1}: {lbl}
                                  </Button>
                                );
                              })}
                            </div>
                          );
                        })}
                      </div>
                    )}

                    <div className="grid grid-cols-4 gap-2">
                      <Button size="sm" className="bg-green-600 hover:bg-green-700 text-white" onClick={() => sendVirtualTx("on", 0)}>
                        I (ON)
                      </Button>
                      <Button size="sm" variant="destructive" onClick={() => sendVirtualTx("off", 0)}>
                        O (OFF)
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("set_white", 0)}>
                        W (White)
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("night_mode", 0)}>
                        Hold Night
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("mode", 1)}>
                        M (Mode)
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("mode_speed_down", 0)}>
                        S-
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("mode_speed_up", 0)}>
                        S+
                      </Button>
                      <Button size="sm" variant="outline" onClick={() => sendVirtualTx("next_mode", 0)}>
                        Next M
                      </Button>
                    </div>

                    <div className="space-y-2.5 pt-1">
                      <div>
                        <div className="flex justify-between text-xs mb-1">
                          <span>Color Ring (Hue)</span>
                          <span className="font-mono">{hueVal}°</span>
                        </div>
                        <input
                          type="range"
                          min={0}
                          max={359}
                          value={hueVal}
                          onChange={(e) => setHueVal(Number(e.target.value))}
                          onMouseUp={() => sendVirtualTx("hue", hueVal)}
                          className="w-full accent-primary cursor-pointer"
                        />
                      </div>
                      <div>
                        <div className="flex justify-between text-xs mb-1">
                          <span>Brightness Slider</span>
                          <span className="font-mono">{briVal}%</span>
                        </div>
                        <input
                          type="range"
                          min={0}
                          max={100}
                          value={briVal}
                          onChange={(e) => setBriVal(Number(e.target.value))}
                          onMouseUp={() => sendVirtualTx("brightness", briVal)}
                          className="w-full accent-primary cursor-pointer"
                        />
                      </div>
                      <div>
                        <div className="flex justify-between text-xs mb-1">
                          <span>CCT Warm / Cool Bar</span>
                          <span className="font-mono">{cctVal}%</span>
                        </div>
                        <input
                          type="range"
                          min={0}
                          max={100}
                          value={cctVal}
                          onChange={(e) => setCctVal(Number(e.target.value))}
                          onMouseUp={() => sendVirtualTx("kelvin", cctVal)}
                          className="w-full accent-primary cursor-pointer"
                        />
                      </div>
                    </div>

                    <div>
                      <div className="flex items-center justify-between text-xs font-semibold mb-1">
                        <span>Live 2.4GHz Gesture &amp; Touch-Zone Log</span>
                        <button type="button" className="text-muted-foreground hover:underline" onClick={() => setLogs([])}>
                          Clear
                        </button>
                      </div>
                      <div className="h-36 overflow-y-auto rounded-lg border bg-muted/30 p-2.5 font-mono text-xs space-y-1">
                        {logs.length === 0 ? (
                          <span className="text-muted-foreground">Tap, double-tap, hold, or touch a wheel slice...</span>
                        ) : (
                          logs.map((line, idx) => <div key={idx}>{line}</div>)
                        )}
                      </div>
                    </div>
                  </div>
                </div>
              )}
            </CardContent>
          </Card>
        </div>
      </div>

      {/* ======================================================================= */}
      {/* MODAL 1: Gestures & Graphical Segmented Ring / Slider Macro-Pad Editor  */}
      {/* ======================================================================= */}
      {macroOpen && currentRemote && (
        <div className="fixed inset-0 z-50 bg-black/60 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-background text-foreground border rounded-xl shadow-2xl max-w-4xl w-full max-h-[92vh] overflow-y-auto p-6 space-y-6">
            <div className="flex items-center justify-between border-b pb-3">
              <div>
                <h2 className="text-lg font-bold">Gestures &amp; Segmented Ring / Slider Configurator</h2>
                <p className="text-xs text-muted-foreground">
                  {currentRemote.name} ({currentRemote.visual_model} • {currentRemote.hex_id}) — synced live to ESP32 Hub &amp; Homebridge
                </p>
              </div>
              <Button variant="ghost" size="sm" onClick={() => setMacroOpen(false)}>
                ✕ Close
              </Button>
            </div>

            {/* Section 1: Button Press Gestures (Short, Long, 2x Double, 3x Triple) */}
            <div className="rounded-xl border p-4 space-y-4 bg-muted/20">
              <div className="flex flex-wrap items-center justify-between gap-2">
                <div>
                  <h3 className="text-sm font-bold">1. Button Multi-Tap &amp; Hold Gestures</h3>
                  <p className="text-xs text-muted-foreground">
                    Tip: Leave 2× and 3× OFF if you want instant 0ms single-tap response without a multi-tap wait window.
                  </p>
                </div>
                <div className="flex items-center gap-2 text-xs">
                  <span>Multi-Tap Window:</span>
                  <Input
                    type="number"
                    min={180}
                    max={750}
                    step={20}
                    value={editGestures.window_ms}
                    onChange={(e) => setEditGestures((g) => ({ ...g, window_ms: Number(e.target.value) || 360 }))}
                    className="w-20 h-8 text-xs font-mono"
                  />
                  <span>ms</span>
                </div>
              </div>

              <div className="grid grid-cols-2 sm:grid-cols-4 gap-3">
                {(
                  [
                    ["short", "Short Press (1×)", "Standard tap (ON, OFF, S+, S-)"],
                    ["long", "Long Press (Hold)", "Hold OFF ~1s -> night_mode"],
                    ["double", "Double Tap (2×)", "Emits on_double, off_double..."],
                    ["triple", "Triple Tap (3×)", "Emits on_triple, s+_triple..."],
                  ] as const
                ).map(([gKey, gTitle, gSub]) => (
                  <div key={gKey} className="rounded-lg border bg-background p-3 flex flex-col justify-between gap-2">
                    <div className="flex items-center justify-between">
                      <span className="text-xs font-bold">{gTitle}</span>
                      <Switch
                        checked={Boolean(editGestures[gKey])}
                        onCheckedChange={(v) => setEditGestures((prev) => ({ ...prev, [gKey]: v }))}
                      />
                    </div>
                    <span className="text-[11px] text-muted-foreground">{gSub}</span>
                  </div>
                ))}
              </div>
            </div>

            {/* Section 2: Graphical Ring & Slider Divider */}
            <div className="rounded-xl border p-4 space-y-4">
              <div>
                <h3 className="text-sm font-bold">2. Divide Color Wheel &amp; Sliders into Action Buttons</h3>
                <p className="text-xs text-muted-foreground">
                  Convert continuous analog touch rings/bars into discrete, debounced button regions (with 140ms RF finger-settle protection).
                </p>
              </div>

              {/* Surface Selector Tabs */}
              <div className="flex flex-wrap gap-2">
                {(
                  [
                    ["hue_ring", "🎨 Color Wheel / Ring"],
                    ["cct_bar", "🌡️ CCT Temp Bar"],
                    ["sat_bar", "💧 Saturation Bar"],
                    ["bri_bar", "☀️ Brightness Slider"],
                  ] as const
                ).map(([sKey, sLabel]) => (
                  <Button
                    key={sKey}
                    size="sm"
                    variant={activeSurfaceTab === sKey ? "default" : "outline"}
                    onClick={() => setActiveSurfaceTab(sKey)}
                  >
                    {sLabel} {editTouch[sKey].enabled ? `(${editTouch[sKey].segments})` : ""}
                  </Button>
                ))}
              </div>

              <div className="grid grid-cols-1 md:grid-cols-12 gap-6 items-center pt-2">
                {/* Interactive Live Graphical SVG Wheel / Bar Preview */}
                <div className="md:col-span-5 flex flex-col items-center justify-center rounded-xl border bg-muted/30 p-4 min-h-[230px]">
                  {activeSurfaceTab === "hue_ring" ? (
                    <svg viewBox="0 0 200 200" className="w-48 h-48">
                      <circle cx="100" cy="100" r="88" fill="none" stroke="currentColor" strokeOpacity={0.15} strokeWidth="36" />
                      {Array.from({ length: curSurf.segments }, (_, i) => {
                        const span = 360 / curSurf.segments;
                        const sDeg = (curSurf.offset_deg || 0) + i * span;
                        const eDeg = sDeg + span;
                        const hueColor = `hsl(${Math.round(((sDeg + span / 2) % 360 + 360) % 360)}, 82%, 52%)`;
                        const lp = polarPoint(100, 100, 70, sDeg + span / 2);
                        return (
                          <g key={i}>
                            <path
                              d={describeRingWedge(100, 100, 50, 90, sDeg, eDeg)}
                              fill={curSurf.enabled ? hueColor : "#94a3b8"}
                              fillOpacity={curSurf.enabled ? 0.85 : 0.25}
                              stroke="#0f172a"
                              strokeWidth="2"
                            />
                            <text
                              x={lp.x}
                              y={lp.y + 4}
                              textAnchor="middle"
                              fontSize="11"
                              fontWeight="800"
                              fill="#ffffff"
                            >
                              {i + 1}
                            </text>
                          </g>
                        );
                      })}
                      <circle cx="100" cy="100" r="46" className="fill-background" />
                      <text x="100" y="97" textAnchor="middle" fontSize="11" fontWeight="700" fill="currentColor">
                        {curSurf.enabled ? `${curSurf.segments} Slices` : "Continuous"}
                      </text>
                      <text x="100" y="112" textAnchor="middle" fontSize="9" fill="#64748b">
                        hue_seg_1..{curSurf.segments}
                      </text>
                    </svg>
                  ) : (
                    <svg viewBox="0 0 240 90" className="w-full max-w-[250px] h-28">
                      {Array.from({ length: curSurf.segments }, (_, i) => {
                        const w = 220 / curSurf.segments;
                        const x = 10 + i * w;
                        return (
                          <g key={i}>
                            <rect
                              x={x + 2}
                              y={22}
                              width={w - 4}
                              height={44}
                              rx={6}
                              fill={curSurf.enabled ? "#0284c7" : "#94a3b8"}
                              fillOpacity={curSurf.enabled ? 0.85 : 0.25}
                              stroke="#0f172a"
                              strokeWidth="1.5"
                            />
                            <text x={x + w / 2} y={48} textAnchor="middle" fontSize="12" fontWeight="800" fill="#ffffff">
                              Slot {i + 1}
                            </text>
                          </g>
                        );
                      })}
                    </svg>
                  )}
                </div>

                {/* Controls: Enable, Segment Count, Rotation, Zone Mask & Custom Slice Labels */}
                <div className="md:col-span-7 space-y-4">
                  <div className="flex items-center justify-between rounded-lg border p-3 bg-muted/20">
                    <div>
                      <div className="text-sm font-bold">Enable Segmented Button Mode</div>
                      <div className="text-xs text-muted-foreground">
                        When OFF, surface behaves as a standard smooth analog control.
                      </div>
                    </div>
                    <Switch
                      checked={curSurf.enabled}
                      onCheckedChange={(v) => updateSurfaceField(activeSurfaceTab, { enabled: v })}
                    />
                  </div>

                  <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                    <div>
                      <Label className="text-xs">
                        Number of Divisions ({activeSurfaceTab === "hue_ring" ? "2–12 Slices" : "2–6 Slots"}):{" "}
                        <b>{curSurf.segments}</b>
                      </Label>
                      <input
                        type="range"
                        min={2}
                        max={activeSurfaceTab === "hue_ring" ? 12 : 6}
                        value={curSurf.segments}
                        onChange={(e) => updateSurfaceField(activeSurfaceTab, { segments: Number(e.target.value) })}
                        className="w-full accent-primary mt-1.5"
                      />
                    </div>

                    {activeSurfaceTab === "hue_ring" && (
                      <div>
                        <Label className="text-xs">Wheel Rotation Offset ({curSurf.offset_deg || 0}°)</Label>
                        <div className="flex gap-1.5 mt-1">
                          <Button
                            size="sm"
                            variant="outline"
                            className="h-8 text-xs"
                            onClick={() => updateSurfaceField("hue_ring", { offset_deg: 0 })}
                          >
                            0° Split
                          </Button>
                          <Button
                            size="sm"
                            variant="outline"
                            className="h-8 text-xs"
                            onClick={() =>
                              updateSurfaceField("hue_ring", { offset_deg: -Math.round(180 / curSurf.segments) })
                            }
                          >
                            Center #1 at Top
                          </Button>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* Per-Zone Scope Selector */}
                  <div>
                    <div className="flex items-center justify-between mb-1">
                      <Label className="text-xs">Active on Zones (Keep lighting zones continuous while turning AC/Shade zones into Macro Pads!)</Label>
                      <button
                        type="button"
                        className="text-xs text-primary hover:underline"
                        onClick={() =>
                          updateSurfaceField(activeSurfaceTab, {
                            zone_mask: curSurf.zone_mask === 0xffff ? 1 << activeZone : 0xffff,
                          })
                        }
                      >
                        {curSurf.zone_mask === 0xffff ? `Only Zone ${activeZone}` : "Select All Zones"}
                      </button>
                    </div>
                    <div className="flex flex-wrap gap-1.5">
                      {[0, ...Array.from({ length: Math.min(currentRemote.num_zones, 8) }, (_, k) => k + 1)].map((z) => {
                        const bit = 1 << z;
                        const on = (curSurf.zone_mask & bit) !== 0;
                        return (
                          <Button
                            key={z}
                            size="sm"
                            variant={on ? "default" : "outline"}
                            className="h-7 px-2.5 text-xs"
                            onClick={() => updateSurfaceField(activeSurfaceTab, { zone_mask: curSurf.zone_mask ^ bit })}
                          >
                            {z === 0 ? "Z0" : `Z${z}`}
                          </Button>
                        );
                      })}
                    </div>
                  </div>

                  {/* Per-Segment Action Button Labels */}
                  <div>
                    <Label className="text-xs">Custom Button Labels (Published in MQTT &amp; Homebridge Mapper)</Label>
                    <div className="grid grid-cols-2 sm:grid-cols-3 gap-2 mt-1.5 max-h-36 overflow-y-auto pr-1">
                      {Array.from({ length: curSurf.segments }, (_, i) => (
                        <div key={i} className="flex items-center gap-1.5">
                          <span className="text-xs font-mono font-bold w-6">#{i + 1}</span>
                          <Input
                            value={curSurf.labels[i] || ""}
                            placeholder={`Button ${i + 1}`}
                            onChange={(e) => {
                              const nextLabels = [...curSurf.labels];
                              nextLabels[i] = e.target.value;
                              updateSurfaceField(activeSurfaceTab, { labels: nextLabels });
                            }}
                            className="h-8 text-xs"
                          />
                        </div>
                      ))}
                    </div>
                  </div>
                </div>
              </div>
            </div>

            <div className="flex justify-end gap-2 border-t pt-4">
              <Button variant="outline" onClick={() => setMacroOpen(false)}>
                Cancel
              </Button>
              <Button onClick={saveMacroStudio}>Save Gestures &amp; Touch Zones</Button>
            </div>
          </div>
        </div>
      )}

      {/* ======================================================================= */}
      {/* MODAL 2: 61-Model Hardware Faceplate & Auto-Detect Configurator         */}
      {/* ======================================================================= */}
      {modalOpen && (
        <div className="fixed inset-0 z-50 bg-black/60 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-background text-foreground border rounded-xl shadow-2xl max-w-4xl w-full max-h-[90vh] overflow-y-auto p-6 space-y-5">
            <div className="flex items-center justify-between border-b pb-3">
              <h2 className="text-lg font-bold">
                {modalMode === "edit" ? "Edit MiBoxer Remote & Faceplate" : "Detect / Add MiBoxer Remote (61 Models)"}
              </h2>
              <Button variant="ghost" size="sm" onClick={() => setModalOpen(false)}>
                ✕ Close
              </Button>
            </div>

            <div className="rounded-lg border border-green-500/40 bg-green-500/10 px-4 py-2.5 text-xs font-medium flex items-center gap-2">
              <span className="h-2.5 w-2.5 rounded-full bg-green-500 animate-pulse inline-block" />
              <span>{modalSniffBanner}</span>
            </div>

            <div className="space-y-3">
              <div className="flex flex-wrap items-center justify-between gap-2">
                <Label className="text-sm font-semibold">1. Choose Hardware Faceplate (61 White &amp; Black Models)</Label>
                <div className="flex flex-wrap gap-1.5">
                  {[
                    ["all", "All (61)"],
                    ["panel", "Wall Panels (20)"],
                    ["rgbcct", "RGB+CCT / RGBW (18)"],
                    ["cct", "CCT & Dimmer (23)"],
                  ].map(([catKey, label]) => (
                    <Button
                      key={catKey}
                      size="sm"
                      variant={wizCat === catKey ? "default" : "outline"}
                      className="h-7 text-xs"
                      onClick={() => setWizCat(catKey)}
                    >
                      {label}
                    </Button>
                  ))}
                </div>
              </div>

              <div className="grid grid-cols-2 sm:grid-cols-4 md:grid-cols-5 gap-2.5 max-h-60 overflow-y-auto p-2 border rounded-lg bg-muted/20">
                {filteredWizardModels.map((m) => {
                  const sel = wModel === m.id;
                  const svgThumb = window.MiBoxerCatalog
                    ? window.MiBoxerCatalog.renderModelSvg(m, { size: "thumb", uid: `wiz_${m.id}` })
                    : "";
                  return (
                    <div
                      key={m.id}
                      onClick={() => {
                        setWModel(m.id);
                        setWProto(m.proto);
                        setWZones(m.zones || 8);
                        if (modalMode === "add") setWName(`${m.id} (${wHex})`);
                      }}
                      className={`p-2 rounded-lg border text-center cursor-pointer flex flex-col items-center justify-between transition-all ${
                        sel ? "border-primary ring-2 ring-primary/30 bg-accent" : "hover:border-primary/50 bg-background"
                      }`}
                    >
                      <div
                        className="h-20 flex items-center justify-center overflow-hidden [&>svg]:max-h-16 [&>svg]:w-auto"
                        dangerouslySetInnerHTML={{ __html: svgThumb }}
                      />
                      <div className="mt-1">
                        <div className="text-xs font-bold leading-tight">{m.id}</div>
                        <div className="text-[10px] text-muted-foreground">
                          {m.proto} • {m.zones}Z
                        </div>
                      </div>
                    </div>
                  );
                })}
              </div>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
              <div className="space-y-1.5">
                <Label>Remote Hex ID</Label>
                <Input value={wHex} onChange={(e) => setWHex(e.target.value)} />
              </div>
              <div className="space-y-1.5">
                <Label>RF Protocol</Label>
                <select
                  value={wProto}
                  onChange={(e) => setWProto(e.target.value)}
                  className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm"
                >
                  <option value="fut089">fut089 (8-Zone RGB+CCT / B8 / FUT089)</option>
                  <option value="rgb_cct">rgb_cct (4-Zone RGB+CCT / FUT092 / B4)</option>
                  <option value="rgbw">rgbw (4-Zone RGBW / FUT096 / B3)</option>
                  <option value="fut091">fut091 (4-Zone CCT / K1 / B2)</option>
                  <option value="cct">cct (Legacy CCT / FUT007)</option>
                  <option value="fut020">fut020 (RGB / FUT020)</option>
                  <option value="fut100">fut100 (100-Zone FUT100)</option>
                </select>
              </div>
              <div className="space-y-1.5">
                <Label>Zones Count</Label>
                <Input type="number" min={1} max={100} value={wZones} onChange={(e) => setWZones(Number(e.target.value))} />
              </div>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div className="space-y-1.5">
                <Label>Friendly Name</Label>
                <Input value={wName} onChange={(e) => setWName(e.target.value)} />
              </div>
              <div className="space-y-1.5">
                <Label>MQTT Topic Prefix</Label>
                <Input value={wPrefix} onChange={(e) => setWPrefix(e.target.value)} />
              </div>
            </div>

            <div className="flex justify-end gap-2 border-t pt-4">
              <Button variant="outline" onClick={() => setModalOpen(false)}>
                Cancel
              </Button>
              <Button onClick={saveRemoteConfig}>Save Remote</Button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
