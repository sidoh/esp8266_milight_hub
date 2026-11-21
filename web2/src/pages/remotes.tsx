import React, { useEffect, useRef, useState, useCallback } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
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

interface RegisteredRemote {
  device_id: number;
  hex_id: string;
  remote_type: string;
  visual_model: string;
  name: string;
  topic_prefix: string;
  num_zones: number;
  active: boolean;
}

interface RemoteEventItem {
  seq: number;
  age_ms?: number;
  device_id: number;
  hex_id: string;
  group_id: number;
  remote_type: string;
  action: string;
  value: number;
  v2_dec?: string;
  source: string;
}

interface GuideStep {
  key: string;
  title: string;
  prompt: string;
  svgAct: string;
  svgZone?: number;
  match: (ev: RemoteEventItem) => boolean;
}

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

function buildVerificationSteps(model: CatalogModel | undefined, numZones: number): GuideStep[] {
  const steps: GuideStep[] = [];
  const maxZ = Math.min(Math.max(numZones || 4, 1), 8);
  const proto = model?.proto || "fut089";

  steps.push({
    key: "master_on",
    title: maxZ > 1 ? "Master All ON (Zone 0)" : "Main ON Button",
    prompt: maxZ > 1 ? "Press the top Master ON (I) button on your remote" : "Press the ON button on your remote",
    svgAct: "on",
    svgZone: maxZ > 1 ? 0 : 1,
    match: (ev) => ev.action === "on" && (maxZ === 1 || ev.group_id === 0 || ev.group_id === 1),
  });

  steps.push({
    key: "master_off",
    title: maxZ > 1 ? "Master All OFF (Zone 0)" : "Main OFF Button",
    prompt: maxZ > 1 ? "Press the top Master OFF (O) button" : "Press the OFF button",
    svgAct: "off",
    svgZone: maxZ > 1 ? 0 : 1,
    match: (ev) => ev.action === "off" && (maxZ === 1 || ev.group_id === 0 || ev.group_id === 1),
  });

  if (maxZ > 1) {
    for (let z = 1; z <= Math.min(maxZ, 4); z++) {
      steps.push({
        key: `z_${z}_on`,
        title: `Zone ${z} ON`,
        prompt: `Tap Zone ${z} ON on your physical remote`,
        svgAct: "zone",
        svgZone: z,
        match: (ev) => ev.group_id === z && (ev.action === "on" || ev.action === "off"),
      });
    }
  }

  steps.push({
    key: "brightness",
    title: "Brightness Slider / Dimmer",
    prompt: "Slide or rotate the Brightness control",
    svgAct: "brightness",
    match: (ev) => ev.action === "brightness" || ev.action === "level",
  });

  if (proto === "fut089" || proto === "rgb_cct" || proto === "rgbw" || proto === "fut020" || proto === "fut100") {
    steps.push({
      key: "hue",
      title: "Color Ring (Hue)",
      prompt: "Touch anywhere along the RGB Color Ring",
      svgAct: "hue",
      match: (ev) => ev.action === "hue",
    });
  }

  if (proto === "fut089" || proto === "rgb_cct" || proto === "fut091" || proto === "cct" || proto === "fut100") {
    steps.push({
      key: "cct",
      title: "CCT / Saturation Slider",
      prompt: "Slide the Warm/Cool White (CCT) or Saturation bar",
      svgAct: "kelvin",
      match: (ev) => ev.action === "color_temp" || ev.action === "kelvin" || ev.action === "saturation",
    });
  }

  steps.push({
    key: "night_hold",
    title: "Long-Press Night Mode",
    prompt: "Press and HOLD an OFF button for ~1.5 seconds",
    svgAct: "off",
    match: (ev) => ev.action === "night_mode" || ev.action === "set_white",
  });

  return steps;
}

export default function RemotesPage() {
  const { toast } = useToast();
  const [catalogReady, setCatalogReady] = useState<boolean>(Boolean(window.MiBoxerCatalog));
  const [remotes, setRemotes] = useState<RegisteredRemote[]>([]);
  const [selectedId, setSelectedId] = useState<number | null>(null);
  const [activeZone, setActiveZone] = useState<number>(1);
  const [lastRf, setLastRf] = useState<RemoteEventItem | null>(null);
  const [logs, setLogs] = useState<string[]>([]);
  const [svgCaption, setSvgCaption] = useState<string>("Click any button, wheel, or slider on the faceplate — or press your physical remote");

  // Virtual Sliders
  const [hueVal, setHueVal] = useState<number>(180);
  const [briVal, setBriVal] = useState<number>(80);
  const [cctVal, setCctVal] = useState<number>(50);

  // Add / Edit Configurator Modal State
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [modalMode, setModalMode] = useState<"add" | "edit">("add");
  const [wizCat, setWizCat] = useState<string>("all");
  const [wHex, setWHex] = useState<string>("0x125C");
  const [wProto, setWProto] = useState<string>("fut089");
  const [wModel, setWModel] = useState<string>("FUT089");
  const [wName, setWName] = useState<string>("Living Room Remote");
  const [wPrefix, setWPrefix] = useState<string>("milight/actions/rem_125c");
  const [wZones, setWZones] = useState<number>(8);
  const [modalSniffBanner, setModalSniffBanner] = useState<string>("Listening on 2.4GHz... Press ANY button on your physical remote to auto-fill!");

  // Guided Physical Button Verification Wizard State
  const [guideOpen, setGuideOpen] = useState<boolean>(false);
  const [guideIdx, setGuideIdx] = useState<number>(0);
  const [guidePassed, setGuidePassed] = useState<Record<string, boolean>>({});

  const svgStageRef = useRef<HTMLDivElement | null>(null);
  const seenSeqRef = useRef<number>(-1);
  const localUiSuppressRef = useRef<Record<string, number>>({});
  const selectedIdRef = useRef<number | null>(null);
  const activeZoneRef = useRef<number>(1);
  const modalOpenRef = useRef<boolean>(false);
  const modalModeRef = useRef<"add" | "edit">("add");
  const guideOpenRef = useRef<boolean>(false);
  const guideIdxRef = useRef<number>(0);

  selectedIdRef.current = selectedId;
  activeZoneRef.current = activeZone;
  modalOpenRef.current = modalOpen;
  modalModeRef.current = modalMode;
  guideOpenRef.current = guideOpen;
  guideIdxRef.current = guideIdx;

  // Inject vector faceplate highlight rules once
  useEffect(() => {
    if (document.getElementById("mbx-studio-styles")) return;
    const st = document.createElement("style");
    st.id = "mbx-studio-styles";
    st.textContent = `
      .mbx-hit { cursor: pointer; transition: filter 0.14s ease; }
      .mbx-hit:hover { filter: brightness(1.08); }
      .mbx-hit.zone-active > circle, .mbx-hit.zone-active > rect, circle.mbx-hit.zone-active, rect.mbx-hit.zone-active {
        stroke: #ea580c !important; stroke-width: 2.6px !important;
      }
      .mbx-hit.guide-target > circle, .mbx-hit.guide-target > rect, circle.mbx-hit.guide-target, rect.mbx-hit.guide-target {
        stroke: #0284c7 !important; stroke-width: 3.2px !important; stroke-dasharray: 4 2;
      }
      .mbx-hit.fire-rf > rect, .mbx-hit.fire-rf > circle, rect.mbx-hit.fire-rf, circle.mbx-hit.fire-rf {
        fill: #86efac !important; stroke: #16a34a !important; stroke-width: 3.2px !important;
      }
      .mbx-hit.fire-ui > rect, .mbx-hit.fire-ui > circle, rect.mbx-hit.fire-ui, circle.mbx-hit.fire-ui {
        fill: #fde68a !important; stroke: #d97706 !important; stroke-width: 3.2px !important;
      }
    `;
    document.head.appendChild(st);
  }, []);

  // Dynamically load /miboxer_catalog.js from ESP32 PROGMEM
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
    const acts = ACT_TO_SVG[action] || [action];
    acts.forEach((a) => {
      stage.querySelectorAll(`.mbx-hit[data-act="${a}"]`).forEach((el) => {
        el.classList.remove("fire-rf", "fire-ui");
        void (el as HTMLElement).offsetWidth;
        el.classList.add(cls);
        setTimeout(() => el.classList.remove(cls), 720);
      });
    });
    stage.querySelectorAll(`.mbx-hit[data-act="zone"][data-zone="${zone}"]`).forEach((el) => {
      el.classList.remove("fire-rf", "fire-ui");
      void (el as HTMLElement).offsetWidth;
      el.classList.add(cls);
      setTimeout(() => el.classList.remove(cls), 720);
    });
  }, []);

  const sendVirtualTx = useCallback(
    async (action: string, value: number, zoneOverride?: number, remOverride?: RegisteredRemote | null) => {
      const rem = remOverride ?? currentRemote;
      if (!rem) return;
      const z = zoneOverride !== undefined ? zoneOverride : activeZoneRef.current;
      const apiAction = action === "kelvin" ? "color_temp" : action;
      localUiSuppressRef.current[`${rem.device_id}:${z}`] = Date.now();
      pulseFaceplate(apiAction, z, false);
      const stamp = new Date().toLocaleTimeString();
      setSvgCaption(`[Virtual UI] Zone ${z} → ${apiAction}(${value})`);
      setLogs((prev) => [`[${stamp}] 🟠 UI TX  ${rem.hex_id} Z${z} -> ${apiAction} (${value})`, ...prev.slice(0, 79)]);

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

  // Render interactive 1:1 SVG faceplate & wire click/hold handlers
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
      svgEl.style.maxHeight = "350px";
      svgEl.style.width = "auto";
      svgEl.style.display = "block";
    }

    // Highlight active zone & guided target if open
    stage.querySelectorAll(`.mbx-hit[data-act="zone"][data-zone="${activeZone}"]`).forEach((el) => el.classList.add("zone-active"));

    if (guideOpen) {
      const steps = buildVerificationSteps(currentCatalogModel, currentRemote.num_zones);
      const step = steps[guideIdx];
      if (step) {
        const sel =
          step.svgZone !== undefined && step.svgAct === "zone"
            ? `.mbx-hit[data-act="zone"][data-zone="${step.svgZone}"]`
            : `.mbx-hit[data-act="${step.svgAct}"]`;
        stage.querySelectorAll(sel).forEach((el) => el.classList.add("guide-target"));
      }
    }

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
            val = act === "kelvin" ? Math.round(153 + (pct / 100) * 217) : pct;
          } else {
            val = 100;
          }
        }
        sendVirtualTx(act, val, zNext, currentRemote);
      };
    });
  }, [catalogReady, currentCatalogModel, currentRemote, activeZone, guideOpen, guideIdx, sendVirtualTx]);

  // Poll /api/remotes/last_seen @ 350ms for live Radio #2 2.4GHz RF telemetry
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
          const supKey = `${ev.device_id}:${ev.group_id}`;
          if (Date.now() - (localUiSuppressRef.current[supKey] || 0) < 850) continue;

          const stamp = new Date().toLocaleTimeString();
          const tag = isVirtual ? "🟠 UI" : "🟢 RF";
          setLogs((prev) => [
            `[${stamp}] ${tag}  ${ev.hex_id} (${ev.remote_type}) Z${ev.group_id} -> ${ev.action}${ev.value !== -1 ? ` (${ev.value})` : ""}`,
            ...prev.slice(0, 79),
          ]);

          // Auto-fill Add Remote Modal when physical remote is pressed!
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

          // Pulse faceplate if this is the currently selected remote
          if (ev.device_id === selectedIdRef.current) {
            setActiveZone(ev.group_id);
            pulseFaceplate(ev.action, ev.group_id, !isVirtual);
            setSvgCaption(
              `${isVirtual ? "[Virtual UI]" : "[Physical 2.4GHz RF]"} Zone ${ev.group_id} → ${ev.action}${ev.value !== -1 ? `(${ev.value})` : ""}`
            );

            // Check Guided Sniff Verification Wizard step
            if (guideOpenRef.current && !isVirtual) {
              const selRem = remotes.find((r) => r.device_id === selectedIdRef.current);
              const mod = (window.MiBoxerCatalog?.MODELS || []).find((m) => m.id === selRem?.visual_model);
              const steps = buildVerificationSteps(mod, selRem?.num_zones || 8);
              const curStep = steps[guideIdxRef.current];
              if (curStep && curStep.match(ev)) {
                setGuidePassed((prev) => ({ ...prev, [curStep.key]: true }));
                if (guideIdxRef.current + 1 < steps.length) {
                  setGuideIdx((i) => i + 1);
                }
              }
            }
          }
        }
      } catch (_) {}
    }, 350);
    return () => clearInterval(timer);
  }, [pulseFaceplate, remotes]);

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
      setModalSniffBanner(`🟢 Adopted ${adoptEv.hex_id} from Radio #2 Sniffer — pick faceplate & save!`);
    } else {
      setWHex("0x8A10");
      setWProto("fut089");
      setWModel("FUT089");
      setWName("New MiBoxer Remote");
      setWPrefix("milight/actions/rem_8a10");
      setWZones(8);
      setModalSniffBanner("Listening on 2.4GHz... Press ANY button on your physical remote or wall panel!");
    }
    setModalOpen(true);
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

  const quickSwitchFinish = async (targetModelId: string) => {
    if (!currentRemote) return;
    await fetch("/api/remotes", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        device_id: currentRemote.device_id,
        remote_type: currentRemote.remote_type,
        visual_model: targetModelId,
        name: currentRemote.name,
        topic_prefix: currentRemote.topic_prefix,
        num_zones: currentRemote.num_zones,
        active: true,
      }),
    });
    await loadRemotes();
  };

  const pairFinishModel = currentCatalogModel?.pairId
    ? catalogModels.find((m) => m.id === currentCatalogModel.pairId)
    : null;

  const filteredWizardModels = catalogModels.filter((m) => (wizCat === "all" ? true : m.cat === wizCat));
  const guideSteps = currentRemote ? buildVerificationSteps(currentCatalogModel, currentRemote.num_zones) : [];

  return (
    <div className="space-y-6 pb-10">
      {/* Header Toolbar */}
      <div className="flex flex-wrap items-center justify-between gap-4 border-b pb-4">
        <div className="flex flex-wrap items-center gap-3">
          <h1 className="text-2xl font-bold tracking-tight">Remotes Studio</h1>
          <Badge variant="secondary">Dual-Radio 2.4GHz Sniffer + Stateful MQTT</Badge>
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
        {/* LEFT COLUMN: Registered Remotes + Live Radio #2 Sniffer */}
        <div className="lg:col-span-4 space-y-6">
          <Card>
            <CardHeader className="pb-3">
              <CardTitle className="text-base">Configured Remotes &amp; Wall Panels ({remotes.length})</CardTitle>
            </CardHeader>
            <CardContent className="space-y-2">
              {remotes.length === 0 && (
                <p className="text-sm text-muted-foreground py-4">
                  No remotes registered yet. Click <b>+ Detect / Add Remote</b> and press any physical button!
                </p>
              )}
              {remotes.map((r) => {
                const isSel = r.device_id === selectedId;
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
                        {r.visual_model} • {r.hex_id} • {r.remote_type} ({r.num_zones}Z)
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
                    if (existing) {
                      setSelectedId(existing.device_id);
                    } else {
                      openConfigurator("add", lastRf);
                    }
                  }}
                  className="p-3 rounded-lg border bg-muted/40 hover:border-primary cursor-pointer space-y-1 font-mono text-xs"
                  title="Click to jump to or adopt this remote"
                >
                  <div className="flex items-center justify-between">
                    <span className="font-bold text-green-600 dark:text-green-400">
                      {lastRf.source === "virtual" ? "🟠 VIRTUAL TX" : "🟢 2.4GHz RF CAPTURE"}
                    </span>
                    <span>seq #{lastRf.seq}</span>
                  </div>
                  <div>
                    ID: <b>{lastRf.hex_id}</b> • Proto: <b>{lastRf.remote_type}</b> • Zone: <b>{lastRf.group_id}</b>
                  </div>
                  <div>
                    Action: <b>{lastRf.action}</b> {lastRf.value !== -1 ? `(${lastRf.value})` : ""}
                  </div>
                  {!remotes.some((r) => r.device_id === lastRf.device_id) && (
                    <div className="text-primary font-sans font-semibold pt-1">
                      + Unregistered remote — click here to adopt in 1 click!
                    </div>
                  )}
                </div>
              ) : (
                <div className="p-3 rounded-lg border bg-muted/30 text-xs text-muted-foreground font-mono">
                  Waiting for 2.4GHz remote press...
                </div>
              )}
            </CardContent>
          </Card>
        </div>

        {/* RIGHT COLUMN: Interactive SVG Faceplate + Virtual Transmitter + Guided Sniff Wizard */}
        <div className="lg:col-span-8">
          <Card>
            <CardHeader className="flex flex-row flex-wrap items-center justify-between gap-3 border-b pb-4">
              <div>
                <CardTitle className="text-lg">{currentRemote ? currentRemote.name : "Select a Remote"}</CardTitle>
                <p className="text-xs text-muted-foreground font-mono mt-1">
                  {currentRemote
                    ? `${currentRemote.visual_model} • ID ${currentRemote.hex_id} • Protocol ${currentRemote.remote_type} • ${currentRemote.num_zones} Zones`
                    : "Choose or add a MiBoxer remote on the left"}
                </p>
              </div>
              {currentRemote && (
                <div className="flex flex-wrap items-center gap-2">
                  {pairFinishModel && (
                    <Button variant="outline" size="sm" onClick={() => quickSwitchFinish(pairFinishModel.id)}>
                      Finish: Switch to {pairFinishModel.id}
                    </Button>
                  )}
                  <Button variant="outline" size="sm" onClick={() => openConfigurator("edit")}>
                    Edit Config / Faceplate
                  </Button>
                  <Button
                    variant={guideOpen ? "default" : "secondary"}
                    size="sm"
                    onClick={() => {
                      setGuideOpen((v) => !v);
                      setGuideIdx(0);
                    }}
                  >
                    {guideOpen ? "Close Sniff Wizard" : "Guided Sniff Test"}
                  </Button>
                  <Button variant="destructive" size="sm" onClick={deleteSelectedRemote}>
                    Delete
                  </Button>
                </div>
              )}
            </CardHeader>

            <CardContent className="pt-6 space-y-6">
              {/* Optional Guided Physical Button Verification Wizard */}
              {guideOpen && currentRemote && guideSteps.length > 0 && (
                <div className="rounded-lg border-2 border-sky-500/60 bg-sky-500/5 p-4 space-y-3">
                  <div className="flex flex-wrap items-center justify-between gap-2">
                    <div>
                      <span className="text-xs font-bold uppercase tracking-wider text-sky-600 dark:text-sky-400">
                        Guided Physical RF Verification — Step {guideIdx + 1} of {guideSteps.length}
                      </span>
                      <h3 className="text-base font-bold">{guideSteps[guideIdx]?.title}</h3>
                      <p className="text-sm text-muted-foreground">{guideSteps[guideIdx]?.prompt}</p>
                    </div>
                    <div className="flex gap-2">
                      <Button variant="outline" size="sm" onClick={() => setGuideIdx((i) => Math.max(0, i - 1))}>
                        ← Prev
                      </Button>
                      <Button
                        variant="outline"
                        size="sm"
                        onClick={() => setGuideIdx((i) => (i + 1) % guideSteps.length)}
                      >
                        Skip →
                      </Button>
                    </div>
                  </div>
                  <div className="flex flex-wrap gap-1.5">
                    {guideSteps.map((st, idx) => (
                      <Badge
                        key={st.key}
                        variant={guidePassed[st.key] ? "default" : idx === guideIdx ? "secondary" : "outline"}
                        className={`cursor-pointer ${guidePassed[st.key] ? "bg-green-600 text-white" : ""}`}
                        onClick={() => setGuideIdx(idx)}
                      >
                        {guidePassed[st.key] ? "✓ " : ""}
                        {st.title}
                      </Badge>
                    ))}
                  </div>
                </div>
              )}

              {currentRemote && (
                <div className="grid grid-cols-1 md:grid-cols-12 gap-6 items-start">
                  {/* 1:1 Vector Faceplate Stage */}
                  <div className="md:col-span-5 flex flex-col items-center rounded-xl border bg-muted/30 p-5">
                    <div ref={svgStageRef} className="min-h-[340px] flex items-center justify-center w-full" />
                    <div className="mt-3 text-center text-xs font-mono text-muted-foreground">{svgCaption}</div>
                  </div>

                  {/* Virtual Transmitter Controls + MQTT Contract + Sniffer Log */}
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
                        Night
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

                    <div className="space-y-3 pt-1">
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
                          onTouchEnd={() => sendVirtualTx("hue", hueVal)}
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
                          onTouchEnd={() => sendVirtualTx("brightness", briVal)}
                          className="w-full accent-primary cursor-pointer"
                        />
                      </div>
                      <div>
                        <div className="flex justify-between text-xs mb-1">
                          <span>CCT Warm / Cool White</span>
                          <span className="font-mono">{cctVal}%</span>
                        </div>
                        <input
                          type="range"
                          min={0}
                          max={100}
                          value={cctVal}
                          onChange={(e) => setCctVal(Number(e.target.value))}
                          onMouseUp={() => sendVirtualTx("color_temp", Math.round(153 + (cctVal / 100) * 217))}
                          onTouchEnd={() => sendVirtualTx("color_temp", Math.round(153 + (cctVal / 100) * 217))}
                          className="w-full accent-primary cursor-pointer"
                        />
                      </div>
                    </div>

                    <div className="rounded-lg border bg-muted/40 p-3 font-mono text-xs space-y-1">
                      <div className="text-[11px] font-sans font-semibold uppercase text-muted-foreground">
                        Stateful MQTT Action Contract
                      </div>
                      <div>
                        Event: <span className="text-primary">{currentRemote.topic_prefix}/zone/{activeZone}/event</span>
                      </div>
                      <div>
                        State: <span className="text-primary">{currentRemote.topic_prefix}/zone/{activeZone}/state</span>
                      </div>
                    </div>

                    <div>
                      <div className="flex items-center justify-between text-xs font-semibold mb-1.5">
                        <span>Live 2.4GHz Sniffer &amp; Virtual TX Log</span>
                        <div className="space-x-3">
                          <button
                            type="button"
                            className="text-primary hover:underline"
                            onClick={() => navigator.clipboard.writeText(logs.join("\n"))}
                          >
                            Copy
                          </button>
                          <button type="button" className="text-muted-foreground hover:underline" onClick={() => setLogs([])}>
                            Clear
                          </button>
                        </div>
                      </div>
                      <div className="h-36 overflow-y-auto rounded-lg border bg-muted/30 p-2.5 font-mono text-xs space-y-1">
                        {logs.length === 0 ? (
                          <span className="text-muted-foreground">Press any physical remote button or click the SVG faceplate...</span>
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

      {/* 61-Model Configurator & Live 2.4GHz Auto-Detect Modal */}
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

              <div className="grid grid-cols-2 sm:grid-cols-4 md:grid-cols-5 gap-2.5 max-h-64 overflow-y-auto p-2 border rounded-lg bg-muted/20">
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
                        className="h-24 flex items-center justify-center overflow-hidden [&>svg]:max-h-20 [&>svg]:w-auto"
                        dangerouslySetInnerHTML={{ __html: svgThumb }}
                      />
                      <div className="mt-1.5">
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
                <Input
                  value={wHex}
                  onChange={(e) => {
                    setWHex(e.target.value);
                    const clean = e.target.value.replace(/^0x/i, "").toLowerCase();
                    if (clean) setWPrefix(`milight/actions/rem_${clean}`);
                  }}
                />
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
              <Button onClick={saveRemoteConfig}>Save Remote &amp; Publish MQTT Discovery</Button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
