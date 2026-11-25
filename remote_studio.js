(function(root) {
  'use strict';
  const SVG_NS = 'http://www.w3.org/2000/svg';

  const ACT_TO_SVG = {
    on: ['on', 'zone_on'], off: ['off', 'zone_off'],
    set_white: ['set_white', 'on'], night_mode: ['night_mode', 'off'],
    brightness: ['brightness'], level: ['brightness'],
    kelvin: ['kelvin'], color_temp: ['kelvin'], temperature: ['kelvin'],
    saturation: ['saturation'], hue: ['hue'],
    mode: ['mode'], next_mode: ['mode', 'mode_speed_up'], previous_mode: ['mode', 'mode_speed_down'],
    mode_speed_up: ['mode_speed_up'], mode_speed_down: ['mode_speed_down']
  };

  function polarPt(cx, cy, r, deg) {
    const rad = ((deg - 90) * Math.PI) / 180;
    return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) };
  }

  function ringWedge(cx, cy, rIn, rOut, sDeg, eDeg) {
    const p1 = polarPt(cx, cy, rOut, eDeg), p2 = polarPt(cx, cy, rOut, sDeg);
    const p3 = polarPt(cx, cy, rIn, sDeg), p4 = polarPt(cx, cy, rIn, eDeg);
    const lg = eDeg - sDeg <= 180 ? '0' : '1';
    return `M ${p1.x.toFixed(1)} ${p1.y.toFixed(1)} A ${rOut.toFixed(1)} ${rOut.toFixed(1)} 0 ${lg} 0 ${p2.x.toFixed(1)} ${p2.y.toFixed(1)} L ${p3.x.toFixed(1)} ${p3.y.toFixed(1)} A ${rIn.toFixed(1)} ${rIn.toFixed(1)} 0 ${lg} 1 ${p4.x.toFixed(1)} ${p4.y.toFixed(1)} Z`;
  }

  function normalizeTouchZones(tz) {
    const def = (segs) => ({ enabled: false, segments: segs, offset_deg: 0, zone_mask: 65535, labels: Array(segs).fill('') });
    const src = tz || {};
    const normOne = (s, dSegs) => {
      if (!s) return def(dSegs);
      const n = Number(s.segments || dSegs);
      const arr = Array.isArray(s.labels) ? s.labels.slice(0, 12) : [];
      while (arr.length < n) arr.push('');
      return {
        enabled: Boolean(s.enabled),
        segments: n,
        offset_deg: Number(s.offset_deg ?? s.offsetDeg ?? 0),
        zone_mask: Number(s.zone_mask ?? s.zoneMask ?? 65535),
        labels: arr
      };
    };
    return {
      hue_ring: normOne(src.hue_ring, 6),
      cct_bar: normOne(src.cct_bar, 4),
      sat_bar: normOne(src.sat_bar, 3),
      bri_bar: normOne(src.bri_bar, 4)
    };
  }

  function defaultCalibration() {
    return { hue_offset: 0, hue_invert: false, bri_min: 0, bri_max: 100, bri_invert: false, cct_min: 0, cct_max: 100, cct_invert: false, sat_min: 0, sat_max: 100, sat_invert: false };
  }

  function isHostDark() {
    if (document.documentElement.classList.contains('dark') || document.body.classList.contains('dark')) return true;
    const parseLum = (str) => {
      if (!str || str === 'transparent') return null;
      const m = String(str).match(/[\d.]+/g);
      if (!m || m.length < 3) return null;
      if (m.length >= 4 && Number(m[3]) < 0.15) return null; // Ignore transparent rgba(0,0,0,0)!
      return (0.299 * Number(m[0]) + 0.587 * Number(m[1]) + 0.114 * Number(m[2])) / 255;
    };
    // Check Home Assistant CSS variables first if hosted inside HA
    const haEl = document.querySelector('home-assistant') || document.documentElement;
    const haBg = getComputedStyle(haEl).getPropertyValue('--primary-background-color').trim() ||
                 getComputedStyle(haEl).getPropertyValue('--card-background-color').trim();
    if (haBg && haBg.startsWith('#')) {
      const hex = haBg.slice(1);
      const full = hex.length === 3 ? hex.split('').map(c => c + c).join('') : hex;
      if (full.length === 6) {
        const r = parseInt(full.slice(0, 2), 16), g = parseInt(full.slice(2, 4), 16), b = parseInt(full.slice(4, 6), 16);
        return ((0.299 * r + 0.587 * g + 0.114 * b) / 255) < 0.45;
      }
    }
    const candidates = [haBg, getComputedStyle(document.body).backgroundColor, getComputedStyle(document.documentElement).backgroundColor];
    for (const c of candidates) {
      const lum = parseLum(c);
      if (lum !== null) return lum < 0.45;
    }
    return false; // Default to clean Light Mode stylesheet
  }

  function injectStudioStyles() {
    if (document.getElementById('mlrs-shared-styles')) return;
    const st = document.createElement('style');
    st.id = 'mlrs-shared-styles';
    st.textContent = `
      .mbx-hit, .mbx-seg-hit { transition: filter 0.12s ease, stroke-width 0.12s ease; touch-action: none; }
      .mbx-hit.zone-active { stroke: #ea580c !important; stroke-width: 2.2px !important; }
      .mbx-hit.fire-rf, .mbx-seg-hit.fire-rf { stroke: #16a34a !important; stroke-width: 2.8px !important; filter: drop-shadow(0 0 6px rgba(22,163,74,0.75)); }
      .mbx-hit.fire-ui, .mbx-seg-hit.fire-ui { stroke: #ea580c !important; stroke-width: 2.8px !important; filter: drop-shadow(0 0 6px rgba(234,88,12,0.75)); }

      .mlrs-modal-backdrop {
        --mlrs-bg: #ffffff; --mlrs-fg: #0f172a; --mlrs-muted: #f8fafc; --mlrs-muted-fg: #64748b;
        --mlrs-border: #e2e8f0; --mlrs-input-bg: #ffffff; --mlrs-primary: #0f172a; --mlrs-primary-fg: #ffffff;
        --mlrs-hover: #f1f5f9; --mlrs-ring: #0284c7;
        position: fixed; inset: 0; z-index: 9999; background: rgba(15, 23, 42, 0.48); backdrop-filter: blur(2px);
        display: flex; align-items: center; justify-content: center; padding: 14px;
        font-family: Inter, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; color: var(--mlrs-fg);
      }
      .mlrs-modal-backdrop.mlrs-dark {
        --mlrs-bg: #09090b; --mlrs-fg: #f8fafc; --mlrs-muted: #18181b; --mlrs-muted-fg: #a1a1aa;
        --mlrs-border: #27272a; --mlrs-input-bg: #09090b; --mlrs-primary: #f8fafc; --mlrs-primary-fg: #09090b;
        --mlrs-hover: #27272a; --mlrs-ring: #38bdf8; background: rgba(0, 0, 0, 0.72);
      }
      .mlrs-modal {
        background: var(--mlrs-bg); color: var(--mlrs-fg); border: 1px solid var(--mlrs-border); border-radius: 12px;
        width: 96vw; max-width: 1840px; height: 94vh; display: flex; flex-direction: column; overflow: hidden;
        box-shadow: 0 20px 45px -10px rgba(15, 23, 42, 0.25);
      }
      .mlrs-topbar { display: flex; flex-wrap: wrap; align-items: center; justify-content: space-between; gap: 12px; padding: 14px 20px; background: var(--mlrs-bg); border-bottom: 1px solid var(--mlrs-border); }
      .mlrs-preset-bar { display: flex; flex-wrap: wrap; align-items: center; gap: 8px; padding: 9px 20px; background: var(--mlrs-muted); border-bottom: 1px solid var(--mlrs-border); font-size: 12px; }
      .mlrs-body { display: grid; grid-template-columns: minmax(300px, 360px) 1fr minmax(320px, 400px); flex: 1; min-height: 0; overflow: hidden; }
      @media (max-width: 1040px) { .mlrs-body { grid-template-columns: 1fr; overflow-y: auto; } }
      .mlrs-col { padding: 14px 16px; overflow-y: auto; border-right: 1px solid var(--mlrs-border); background: var(--mlrs-bg); }
      .mlrs-col:last-child { border-right: none; border-left: 1px solid var(--mlrs-border); }
      .mlrs-stage-wrap { display: flex; flex-direction: column; align-items: center; justify-content: center; background: var(--mlrs-muted); padding: 16px; overflow: auto; }
      .mlrs-stage-card { background: var(--mlrs-bg); border: 1px solid var(--mlrs-border); border-radius: 12px; padding: 20px 28px; display: flex; flex-direction: column; align-items: center; box-shadow: 0 1px 3px rgba(0,0,0,0.05); }
      .mlrs-footer { display: flex; flex-wrap: wrap; align-items: center; justify-content: space-between; gap: 10px; padding: 11px 20px; background: var(--mlrs-bg); border-top: 1px solid var(--mlrs-border); }
      .mlrs-btn { background: var(--mlrs-bg); color: var(--mlrs-fg); border: 1px solid var(--mlrs-border); border-radius: 6px; padding: 6px 12px; font-size: 12.5px; font-weight: 500; cursor: pointer; display: inline-flex; align-items: center; gap: 6px; }
      .mlrs-btn:hover { background: var(--mlrs-hover); }
      .mlrs-btn.primary, .mlrs-btn.active-tab { background: var(--mlrs-primary); border-color: var(--mlrs-primary); color: var(--mlrs-primary-fg); font-weight: 600; }
      .mlrs-btn.danger-outline { color: #dc2626; border-color: #fecaca; }
      .mlrs-sec-title { font-size: 12.5px; font-weight: 700; color: var(--mlrs-fg); margin-bottom: 10px; }
      .mlrs-field { margin-bottom: 10px; }
      .mlrs-field label { display: block; font-size: 11.5px; font-weight: 600; color: var(--mlrs-fg); margin-bottom: 4px; }
      .mlrs-input, .mlrs-select { width: 100%; background: var(--mlrs-input-bg); border: 1px solid var(--mlrs-border); color: var(--mlrs-fg); border-radius: 6px; padding: 6px 9px; font-size: 12.5px; }
      .mlrs-swatch { height: 32px; padding: 2px 4px; cursor: pointer; }
      .mlrs-icon-grid { display: grid; grid-template-columns: repeat(6, 1fr); gap: 5px; }
      .mlrs-icon-cell { aspect-ratio: 1; border: 1px solid var(--mlrs-border); border-radius: 6px; background: var(--mlrs-input-bg); color: var(--mlrs-fg); display: flex; flex-direction: column; align-items: center; justify-content: center; cursor: pointer; padding: 3px; }
      .mlrs-icon-cell:hover { background: var(--mlrs-hover); }
      .mlrs-icon-cell.sel { border: 2px solid var(--mlrs-ring); background: var(--mlrs-hover); }
      .mlrs-pill-group { display: inline-flex; background: var(--mlrs-muted); padding: 3px; border-radius: 8px; border: 1px solid var(--mlrs-border); gap: 3px; }
      .mlrs-pill-group .mlrs-btn { border: none; padding: 5px 11px; font-size: 12px; border-radius: 6px; background: transparent; }
      .mlrs-pill-group .mlrs-btn.active-tab { background: var(--mlrs-primary); color: var(--mlrs-primary-fg); }

      
      .mlrs-modal-backdrop.mlrs-backdrop-fs, .mlrs-modal-backdrop:fullscreen { padding: 0 !important; width: 100vw !important; height: 100vh !important; }
      .mlrs-modal-backdrop:fullscreen .mlrs-modal { width: 100vw !important; max-width: 100vw !important; height: 100vh !important; border-radius: 0 !important; border: none !important; }
      .mlrs-modal.mlrs-fullscreen .mlrs-body, .mlrs-modal-backdrop:fullscreen .mlrs-body { grid-template-columns: 340px 1fr 390px !important; }

      .mlrs-modal.mlrs-fullscreen { width: 100vw !important; max-width: 100vw !important; height: 100vh !important; border-radius: 0 !important; border: none !important; }
      .mlrs-peel-wrap { display: flex; flex-direction: column; align-items: center; justify-content: center; width: 100%; height: 100%; flex: 1; min-height: 0; user-select: none; }
      .mlrs-peel-bar { display: flex; align-items: center; justify-content: center; gap: 8px; flex-wrap: wrap; margin-bottom: 8px; padding: 6px 10px; border-radius: 9px; border: 1px solid rgba(2,132,199,0.32); background: rgba(2,132,199,0.07); font-size: 11.5px; width: 100%; box-sizing: border-box; }
      .mlrs-peel-btn { padding: 4px 10px; border-radius: 6px; border: 1px solid #0284c7; background: #0284c7; color: #fff; font-weight: 700; font-size: 11.5px; cursor: pointer; }
      .mlrs-peel-btn.peeled { background: #0f172a; border-color: #334155; }
      .mlrs-peel-stack { position: relative; display: inline-grid; place-items: center; padding: 6px; flex: 1; min-height: 0; }
      .mlrs-peel-under, .mlrs-peel-over { grid-area: 1 / 1; display: flex; align-items: center; justify-content: center; }
      .mlrs-peel-over { position: relative; z-index: 2; transition: clip-path 0.24s cubic-bezier(.22,.9,.3,1), filter 0.2s ease; filter: drop-shadow(-3px 5px 8px rgba(15,23,42,0.28)); }
      .mlrs-peel-stack.scrubbing .mlrs-peel-over, .mlrs-peel-stack.scrubbing .mlrs-peel-flap { transition: none !important; }
      .mlrs-peel-corner-hit { position: absolute; top: 2px; right: 2px; width: 42px; height: 42px; z-index: 5; cursor: pointer; border-radius: 0 10px 0 28px; background: linear-gradient(225deg, rgba(148,163,184,0.38) 0%, rgba(255,255,255,0.88) 48%, rgba(15,23,42,0.16) 52%, transparent 56%); box-shadow: -2px 2px 5px rgba(15,23,42,0.18); }
      .mlrs-peel-flap { position: absolute; inset: 6px; z-index: 4; pointer-events: none; background: linear-gradient(225deg, rgba(241,245,249,0.97) 0%, rgba(203,213,225,0.94) 55%, rgba(148,163,184,0.88) 100%); filter: drop-shadow(-6px 7px 10px rgba(15,23,42,0.34)); transition: clip-path 0.24s cubic-bezier(.22,.9,.3,1), opacity 0.2s ease; }


      .mlrs-tour-bar {
        display: flex !important; align-items: center !important; justify-content: space-between !important; gap: 14px !important;
        flex-wrap: wrap !important; padding: 10px 14px !important; border-radius: 11px !important;
        border: 1px solid rgba(2,132,199,0.35) !important;
        background: linear-gradient(135deg, rgba(2,132,199,0.09) 0%, rgba(15,23,42,0.03) 100%) !important;
        box-shadow: inset 0 1px 0 rgba(255,255,255,0.35), 0 2px 6px rgba(15,23,42,0.05) !important;
        margin: 0 0 10px 0 !important; font-size: 12.5px !important; line-height: 1.4 !important; box-sizing: border-box !important;
      }
      .mlrs-tour-left { display: flex !important; align-items: center !important; gap: 11px !important; flex: 1 !important; min-width: 240px !important; }
      .mlrs-tour-icon { width: 34px !important; height: 34px !important; border-radius: 9px !important; background: #0284c7 !important; color: #fff !important; display: flex !important; align-items: center !important; justify-content: center !important; font-size: 16px !important; flex-shrink: 0 !important; }
      .mlrs-tour-step-badge { display: inline-flex !important; align-items: center !important; padding: 2px 8px !important; border-radius: 999px !important; background: rgba(2,132,199,0.16) !important; color: #0284c7 !important; font-weight: 800 !important; font-size: 10.5px !important; text-transform: uppercase !important; letter-spacing: 0.04em !important; margin-right: 7px !important; }
      .mlrs-tour-dots { display: inline-flex !important; gap: 4px !important; align-items: center !important; margin-left: 8px !important; vertical-align: middle !important; }
      .mlrs-tour-dot { width: 6px !important; height: 6px !important; border-radius: 999px !important; background: rgba(100,116,139,0.35) !important; }
      .mlrs-tour-dot.on { width: 16px !important; background: #0284c7 !important; }
      .mlrs-tour-nav { display: flex !important; align-items: center !important; gap: 6px !important; flex-wrap: wrap !important; }
      button.mlrs-tour-btn { padding: 5px 11px !important; border-radius: 7px !important; border: 1px solid rgba(100,116,139,0.38) !important; background: #ffffff !important; color: #0f172a !important; font-weight: 700 !important; font-size: 11.5px !important; cursor: pointer !important; }
      button.mlrs-tour-btn.demo { background: #0284c7 !important; border-color: #0284c7 !important; color: #ffffff !important; }
      button.mlrs-tour-reopen { display: inline-flex !important; align-items: center !important; gap: 6px !important; padding: 5px 12px !important; border-radius: 999px !important; border: 1px solid rgba(2,132,199,0.45) !important; background: rgba(2,132,199,0.10) !important; color: #0284c7 !important; font-weight: 700 !important; font-size: 11.5px !important; cursor: pointer !important; }

      .mlrs-box-sub { padding: 9px 10px; border-radius: 8px; background: var(--mlrs-muted); border: 1px solid var(--mlrs-border); margin-bottom: 10px; }
    `;
    document.head.appendChild(st);
  }

  // ============================================================================
  // 1. INTERACTIVE STAGE CONTROLLER (Finger Pucks, Segmented Pads & Step/Hold)
  // ============================================================================
  function mountInteractiveStage(stageEl, opts) {
    injectStudioStyles(stageEl);
    const Cat = root.MiBoxerCatalog;
    if (!stageEl || !Cat) return null;

    const model = typeof opts.model === 'string' ? (Cat.MODELS.find(m => m.id === opts.model) || Cat.MODELS[0]) : (opts.model || Cat.MODELS[0]);
    const cal = Object.assign(defaultCalibration(), opts.calibration || {});
    const tz = normalizeTouchZones(opts.touchZones);
    let activeZone = opts.activeZone !== undefined ? Number(opts.activeZone) : 1;
    const editMode = Boolean(opts.editMode);
    const facia = opts.facia || null;
    const stepCounters = opts.stepState || {};

    stageEl.innerHTML = Cat.renderModelSvg(model, {
      size: opts.size || 'large',
      uid: opts.uid || 'studio_stage',
      facia: facia,
      touchZones: tz,
      activeZone: activeZone,
      editorMode: editMode,
      selectedPadKey: opts.selectedPadKey || null
    });

    const svgEl = stageEl.querySelector('svg');
    if (!svgEl) return null;
    const vb = (svgEl.getAttribute('viewBox') || '0 0 220 310').trim().split(/[\s,]+/).map(Number);
    const vbRatio = (vb[2] > 0 && vb[3] > 0) ? (vb[2] / vb[3]) : 0.71;
    svgEl.removeAttribute('width');
    svgEl.removeAttribute('height');
    svgEl.style.height = opts.maxHeight || 'clamp(370px, 58vh, 640px)';
    svgEl.style.maxHeight = '100%';
    svgEl.style.width = 'auto';
    svgEl.style.maxWidth = '100%';
    svgEl.style.aspectRatio = `${vb[2] || 220} / ${vb[3] || 310}`;
    if (vbRatio > 0.88) {
      svgEl.style.height = opts.squareHeight || 'clamp(330px, 48vh, 520px)';
    }
    svgEl.style.display = 'block';
    svgEl.style.userSelect = 'none';

    const updateZoneHighlight = () => {
      svgEl.querySelectorAll('.mbx-hit[data-act="zone"], .mbx-hit[data-act="zone_on"]').forEach(el => {
        const pKey = el.getAttribute('data-pad-key');
        const target = (pKey && svgEl.querySelector(`g[data-pad-key="${pKey}"] > rect`)) || el;
        target.classList.toggle('zone-active', Number(el.getAttribute('data-zone')) === activeZone);
      });
    };
    updateZoneHighlight();

    // Coordinated Finger-Press Reticle Layer
    const puckLayer = document.createElementNS(SVG_NS, 'g');
    puckLayer.setAttribute('pointer-events', 'none');
    svgEl.appendChild(puckLayer);
    const puckTimers = {};

    function showFingerPuck(svgX, svgY, labelText, isRf, keyName) {
      const id = keyName || (isRf ? 'rf' : 'ui');
      const old = puckLayer.querySelector(`[data-puck-id="${id}"]`);
      if (old) old.remove();
      if (puckTimers[id]) clearTimeout(puckTimers[id]);

      const col = isRf ? '#16a34a' : '#ea580c';
      const g = document.createElementNS(SVG_NS, 'g');
      g.setAttribute('data-puck-id', id);

      const halo = document.createElementNS(SVG_NS, 'circle');
      halo.setAttribute('cx', svgX.toFixed(1)); halo.setAttribute('cy', svgY.toFixed(1));
      halo.setAttribute('r', '9.5'); halo.setAttribute('fill', isRf ? 'rgba(22,163,74,0.24)' : 'rgba(234,88,12,0.26)');
      halo.setAttribute('stroke', col); halo.setAttribute('stroke-width', '1.5');
      g.appendChild(halo);

      const dot = document.createElementNS(SVG_NS, 'circle');
      dot.setAttribute('cx', svgX.toFixed(1)); dot.setAttribute('cy', svgY.toFixed(1));
      dot.setAttribute('r', '4'); dot.setAttribute('fill', col);
      dot.setAttribute('stroke', '#ffffff'); dot.setAttribute('stroke-width', '1.2');
      g.appendChild(dot);

      if (labelText) {
        const pillW = Math.max(26, labelText.length * 4.5 + 9);
        const py = Math.max(12, svgY - 15);
        const rect = document.createElementNS(SVG_NS, 'rect');
        rect.setAttribute('x', (svgX - pillW / 2).toFixed(1)); rect.setAttribute('y', (py - 8).toFixed(1));
        rect.setAttribute('width', pillW.toFixed(1)); rect.setAttribute('height', '11');
        rect.setAttribute('rx', '4'); rect.setAttribute('fill', '#0f172a');
        rect.setAttribute('stroke', col); rect.setAttribute('stroke-width', '0.9');
        g.appendChild(rect);

        const txt = document.createElementNS(SVG_NS, 'text');
        txt.setAttribute('x', svgX.toFixed(1)); txt.setAttribute('y', py.toFixed(1));
        txt.setAttribute('text-anchor', 'middle'); txt.setAttribute('font-size', '6.2');
        txt.setAttribute('font-weight', '700'); txt.setAttribute('fill', '#ffffff');
        txt.textContent = labelText;
        g.appendChild(txt);
      }

      puckLayer.appendChild(g);
      puckTimers[id] = setTimeout(() => { g.remove(); }, 1500);
    }

    function clientToSvgPoint(clientX, clientY) {
      const pt = svgEl.createSVGPoint();
      pt.x = clientX; pt.y = clientY;
      const ctm = svgEl.getScreenCTM();
      return ctm ? pt.matrixTransform(ctm.inverse()) : { x: 0, y: 0 };
    }

    function unionBBox(els) {
      let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
      els.forEach(el => {
        if (!el.getBBox) return;
        try {
          const b = el.getBBox();
          if (b.width <= 0 && b.height <= 0) return;
          if (b.x < minX) minX = b.x;
          if (b.y < minY) minY = b.y;
          if (b.x + b.width > maxX) maxX = b.x + b.width;
          if (b.y + b.height > maxY) maxY = b.y + b.height;
        } catch (_) {}
      });
      if (!Number.isFinite(minX)) return null;
      return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
    }

    function analogPointForValue(act, rawVal, isRf) {
      const aLow = String(act || '').toLowerCase();
      const normAct = (aLow === 'color_temp' || aLow === 'temperature') ? 'kelvin' : (aLow === 'level' ? 'brightness' : aLow);
      let candidates = Array.from(svgEl.querySelectorAll(`.mbx-hit[data-act="${normAct}"]`));
      // For hue, exclude discrete R/G/B rectangular buttons (which have width < 40 and rx/rect tag) when a wheel or strip exists
      if (normAct === 'hue' && candidates.length > 1) {
        const nonRectBtn = candidates.filter(el => el.tagName.toLowerCase() === 'circle' || el.tagName.toLowerCase() === 'path' || !el.hasAttribute('data-val') || (el.getBBox && el.getBBox().width < 20));
        if (nonRectBtn.length) candidates = nonRectBtn;
      }
      if (!candidates.length && normAct === 'kelvin') {
        candidates = Array.from(svgEl.querySelectorAll('.mbx-hit[data-act="saturation"]'));
      }
      if (!candidates.length) return null;
      const bb = unionBBox(candidates);
      if (!bb || !bb.width || !bb.height) return null;

      const numVal = Number(rawVal);
      if (Number.isNaN(numVal)) return null;

      // Wheel vs Bar check: if width & height are roughly square (>35x35), it's a Color Ring!
      if (normAct === 'hue' && bb.height > 32 && bb.width > 32 && Math.abs(bb.width - bb.height) < 20) {
        const cx = bb.x + bb.width / 2, cy = bb.y + bb.height / 2;
        const r = Math.max(bb.width, bb.height) * 0.42;
        const deg = isRf ? physicalHueToWheelDeg(numVal, cal) : (((numVal % 360) + 360) % 360);
        const pt = polarPt(cx, cy, r, deg);
        return { x: pt.x, y: pt.y, text: `${Math.round(numVal)}°` };
      }

      const vert = bb.height > bb.width * 1.2;
      if (normAct === 'saturation' && vert) {
        // Physical vertical saturation slider has 0% at TOP and 100% at BOTTOM!
        const rawPct = Math.max(0, Math.min(100, numVal > 100 ? (numVal / 255) * 100 : numVal));
        const fracFromTop = rawPct / 100;
        return {
          x: bb.x + bb.width / 2,
          y: bb.y + 4 + fracFromTop * Math.max(4, bb.height - 8),
          text: `${Math.round(rawPct)}%`
        };
      }

      const pct = isRf ? normalizeSliderPct(numVal, normAct) : Math.max(0, Math.min(100, numVal > 100 ? (numVal / 255) * 100 : numVal));
      const frac = Math.max(0, Math.min(1, pct / 100));
      return {
        x: vert ? (bb.x + bb.width / 2) : (bb.x + 4 + frac * Math.max(4, bb.width - 8)),
        y: vert ? (bb.y + 4 + (1 - frac) * Math.max(4, bb.height - 8)) : (bb.y + bb.height / 2),
        text: `${Math.round(pct)}%`
      };
    }

    function pulseAction(action, zone, isRf, value) {
      if (zone !== undefined && zone !== null && !isNaN(Number(zone))) {
        activeZone = Number(zone);
        updateZoneHighlight();
      }
      const cls = isRf ? 'fire-rf' : 'fire-ui';

      // 1. Segmented Ring / Slider Pads
      const segEls = svgEl.querySelectorAll(`.mbx-seg-hit[data-seg-act="${action}"]`);
      if (segEls.length > 0) {
        segEls.forEach(el => {
          el.classList.add(cls);
          if (el.getBBox) {
            const bb = el.getBBox();
            showFingerPuck(bb.x + bb.width / 2, bb.y + bb.height / 2, action.replace(/_seg_/, ' #'), isRf, isRf ? 'rf' : 'ui');
          }
          setTimeout(() => el.classList.remove(cls), 750);
        });
        return;
      }

      // 2. Zone-only selection pulse
      if (action === 'zone' || action === 'zone_on' || action === 'zone_off') {
        const zTarget = svgEl.querySelector(`.mbx-hit[data-zone="${zone}"][data-act="${action}"]`) || svgEl.querySelector(`.mbx-hit[data-zone="${zone}"]`);
        if (zTarget && zTarget.getBBox) {
          zTarget.classList.add(cls);
          const bb = zTarget.getBBox();
          showFingerPuck(bb.x + bb.width / 2, bb.y + bb.height / 2, `Z${zone}`, isRf, isRf ? 'rf_btn' : 'ui_btn');
          setTimeout(() => zTarget.classList.remove(cls), 720);
          return;
        }
      }

      // 3. Analog Wheel & Sliders (Factory AND Custom Panels): Show exact finger position puck WITHOUT flashing whole track!
      const isAnalogAct = /^(hue|brightness|level|kelvin|color_temp|temperature|saturation)$/i.test(String(action || ''));
      if (isAnalogAct && value !== undefined && value !== null && value !== '' && !Number.isNaN(Number(value)) && Number(value) >= 0) {
        const pt = analogPointForValue(action, value, isRf);
        if (pt) {
          showFingerPuck(pt.x, pt.y, `${isRf ? 'RF' : 'UI'} ${pt.text}`, isRf, isRf ? `rf_${action}` : `ui_${action}`);
          return;
        }
      }

      // 4. Standard Discrete Buttons (ON, OFF, W, M, S+, S-, 60S)
      const acts = ACT_TO_SVG[action] || [action];
      let shownBtnPuck = false;
      acts.forEach(a => {
        svgEl.querySelectorAll(`.mbx-hit[data-act="${a}"]`).forEach(el => {
          if (el.hasAttribute('data-zone') && Number(el.getAttribute('data-zone')) !== Number(zone)) return;
          el.classList.add(cls);
          if (!shownBtnPuck && el.getBBox) {
            const bb = el.getBBox();
            showFingerPuck(bb.x + bb.width / 2, bb.y + bb.height / 2, String(action).toUpperCase(), isRf, isRf ? 'rf_btn' : 'ui_btn');
            shownBtnPuck = true;
          }
          setTimeout(() => el.classList.remove(cls), 720);
        });
      });
    }

    // DESIGN MODE: Click any stencil slot / segment / rocker to customize it!
    if (editMode) {
      svgEl.querySelectorAll('.mbx-seg-hit, .mbx-hit').forEach(el => {
        el.style.cursor = 'pointer';
        el.onclick = (ev) => {
          ev.stopPropagation();
          const pKey = el.getAttribute('data-pad-key') || el.getAttribute('data-seg-act') || el.getAttribute('data-act') || 'btn_on';
          const act = el.getAttribute('data-seg-act') || el.getAttribute('data-act') || 'on';
          const z = el.getAttribute('data-zone');
          if (typeof opts.onSelectPad === 'function') opts.onSelectPad(pKey, act, z);
        };
      });
      return { pulseAction, setActiveZone: (z) => { activeZone = Number(z); updateZoneHighlight(); } };
    }

    // Helper for Single-Button Multi-Function pads (Short Tap = +1 Step Cycle, Hold = ON/OFF Toggle)
    function attachPadInteraction(el, pKey, baseAct, fixedZone, fixedVal) {
      const padCfg = (facia && facia.pads && facia.pads[pKey]) || {};
      const mf = padCfg.multifunc && padCfg.multifunc.enabled ? padCfg.multifunc : null;
      let holdTimer = null, heldFired = false;

      el.addEventListener('pointerdown', () => {
        if (el.getAttribute('data-blanked') === '1') return;
        heldFired = false;
        holdTimer = setTimeout(() => {
          heldFired = true;
          const zNext = fixedZone !== null ? fixedZone : activeZone;
          const bb = el.getBBox ? el.getBBox() : null;
          if (mf) {
            const st = stepCounters[pKey] || { step: 1, on: true };
            st.on = !st.on;
            stepCounters[pKey] = st;
            const msg = `${padCfg.label || pKey}: ${st.on ? 'ON' : 'OFF'} (Hold Toggle)`;
            if (bb) showFingerPuck(bb.x + bb.width / 2, bb.y + bb.height / 2, st.on ? 'HOLD: ON' : 'HOLD: OFF', false, 'ui_btn');
            if (typeof opts.onAction === 'function') opts.onAction(baseAct, st.on ? 100 : 0, zNext, msg);
          } else if (baseAct === 'on' || baseAct === 'off' || baseAct === 'zone_on' || baseAct === 'zone_off' || baseAct === 'set_white') {
            const longAct = (baseAct === 'off' || baseAct === 'zone_off') ? 'night_mode' : 'set_white';
            if (bb) showFingerPuck(bb.x + bb.width / 2, bb.y + bb.height / 2, `HOLD ${longAct.toUpperCase()}`, false, 'ui_btn');
            if (typeof opts.onAction === 'function') opts.onAction(longAct, 0, zNext);
          }
        }, 580);
      });
      const clr = () => { if (holdTimer) { clearTimeout(holdTimer); holdTimer = null; } };
      el.addEventListener('pointerup', clr);
      el.addEventListener('pointerleave', clr);

      el.addEventListener('click', (ev) => {
        ev.stopPropagation();
        if (el.getAttribute('data-blanked') === '1') return;
        if (heldFired) { heldFired = false; return; }
        let zNext = activeZone;
        if (fixedZone !== null) {
          zNext = fixedZone;
          activeZone = zNext;
          updateZoneHighlight();
          if (typeof opts.onZoneChange === 'function') opts.onZoneChange(zNext);
        }
        const sp = clientToSvgPoint(ev.clientX, ev.clientY);
        const bb = el.getBBox ? el.getBBox() : null;
        const px = (sp && !Number.isNaN(sp.x) && sp.x !== 0) ? sp.x : (bb ? bb.x + bb.width / 2 : 0);
        const py = (sp && !Number.isNaN(sp.y) && sp.y !== 0) ? sp.y : (bb ? bb.y + bb.height / 2 : 0);
        if (padCfg.segmentMode === 'slider') {
          const frac = Math.max(0, Math.min(1, (px - (bb ? bb.x : 0)) / Math.max(1, bb ? bb.width : 1)));
          const sMin = Number(padCfg.sliderMin ?? 0);
          const sMax = Number(padCfg.sliderMax ?? 100);
          const mappedVal = Math.round(sMin + frac * (sMax - sMin));
          const badge = `${padCfg.label || pKey}: ${mappedVal}% (${sMin}%–${sMax}%)`;
          showFingerPuck(px, py, `${mappedVal}%`, false, 'ui_btn');
          if (typeof opts.onAction === 'function') opts.onAction(baseAct, mappedVal, zNext, badge);
          return;
        }
        if (mf) {
          const maxS = Math.max(2, Number(mf.steps || 4));
          const st = stepCounters[pKey] || { step: 0, on: true };
          st.on = true;
          st.step = (st.step % maxS) + 1;
          stepCounters[pKey] = st;
          const badge = `${padCfg.label || pKey}: ${mf.unit || 'Step'} ${st.step}/${maxS}`;
          showFingerPuck(px, py, `${st.step}/${maxS}`, false, 'ui_btn');
          if (typeof opts.onAction === 'function') opts.onAction(baseAct, Math.round((st.step / maxS) * 100), zNext, badge);
          return;
        }
        showFingerPuck(px, py, padCfg.label || (baseAct === 'zone' ? `Z${zNext}` : baseAct.toUpperCase()), false, 'ui_btn');
        if (baseAct === 'zone') return;
        if (typeof opts.onAction === 'function') opts.onAction(baseAct, fixedVal !== null ? fixedVal : 100, zNext, padCfg.label || undefined);
      });
    }

    // Bind Segmented Ring/Bar Cells
    svgEl.querySelectorAll('.mbx-seg-hit').forEach(el => {
      const segAct = el.getAttribute('data-seg-act') || '';
      const pKey = el.getAttribute('data-pad-key') || segAct;
      attachPadInteraction(el, pKey, segAct, null, 1);
    });

    // Bind Continuous Sliders/Wheels + Discrete Buttons
    svgEl.querySelectorAll('.mbx-hit').forEach(el => {
      const act = el.getAttribute('data-act') || 'on';
      const pKey = el.getAttribute('data-pad-key') || act;
      const zAttr = el.getAttribute('data-zone');
      const fixedZ = (zAttr !== null && zAttr !== '') ? Number(zAttr) : null;
      const fixedVal = el.hasAttribute('data-val') ? Number(el.getAttribute('data-val')) : null;
      const tag = el.tagName.toLowerCase();
      const isDiscreteRgbBtn = (act === 'hue' && (fixedVal === 0 || fixedVal === 120 || fixedVal === 240) && tag === 'rect');
      const isContinuous = (act === 'hue' || act === 'brightness' || act === 'kelvin' || act === 'saturation') && !isDiscreteRgbBtn;

      if (isContinuous) {
        let dragging = false, lastTxMs = 0;
        const scrub = (ev, forceTx) => {
          if (el.getAttribute('data-blanked') === '1') return;
          const bb = el.getBBox();
          const sp = clientToSvgPoint(ev.clientX, ev.clientY);
          let outVal = 0, px = sp.x, py = sp.y, lbl = '';
          if (act === 'hue' && bb.height > bb.width * 0.75) {
            const cx = bb.x + bb.width / 2, cy = bb.y + bb.height / 2, r = Math.max(bb.width, bb.height) * 0.39;
            let deg = ((Math.atan2(sp.y - cy, sp.x - cx) * 180) / Math.PI + 90 + 360) % 360;
            const snap = polarPt(cx, cy, r, deg);
            px = snap.x; py = snap.y;
            let calDeg = (deg - Number(cal.hue_offset || 0) + 360) % 360;
            if (cal.hue_invert) calDeg = (360 - calDeg) % 360;
            outVal = Math.round(calDeg) % 360; lbl = `${outVal}°`;
          } else {
            const vert = bb.height > bb.width * 1.3;
            let frac = Math.max(0, Math.min(1, vert ? (1 - (sp.y - bb.y) / bb.height) : ((sp.x - bb.x) / bb.width)));
            px = vert ? (bb.x + bb.width / 2) : (bb.x + frac * bb.width);
            py = vert ? (bb.y + (1 - frac) * bb.height) : (bb.y + bb.height / 2);
            const inv = act === 'brightness' ? cal.bri_invert : act === 'kelvin' ? cal.cct_invert : cal.sat_invert;
            if (inv) frac = 1 - frac;
            outVal = Math.round(frac * 100); lbl = `${outVal}%`;
          }
          showFingerPuck(px, py, lbl, false, 'ui_analog');
          if (typeof opts.onScrubPreview === 'function') opts.onScrubPreview(act, outVal);
          const now = Date.now();
          if (forceTx || now - lastTxMs >= 135) {
            lastTxMs = now;
            if (typeof opts.onAction === 'function') opts.onAction(act, outVal, fixedZ !== null ? fixedZ : activeZone);
          }
        };
        el.addEventListener('pointerdown', (ev) => { ev.preventDefault(); ev.stopPropagation(); dragging = true; try { el.setPointerCapture(ev.pointerId); } catch (_) {} scrub(ev, true); });
        el.addEventListener('pointermove', (ev) => { if (dragging) scrub(ev, false); });
        el.addEventListener('pointerup', (ev) => { if (dragging) { dragging = false; scrub(ev, true); } });
        el.addEventListener('pointercancel', () => { dragging = false; });
        return;
      }

      attachPadInteraction(el, pKey, act, fixedZ, fixedVal);
    });

    return { pulseAction, setActiveZone: (z) => { activeZone = Number(z); updateZoneHighlight(); } };
  }

  // ============================================================================
  // 2. 1:1 PRINT / CUT STENCIL SHEET BUILDER
  // ============================================================================
  function buildOneToOnePrintSvg(model, facia, touchZones, activeZone) {
    const Cat = root.MiBoxerCatalog;
    const dims = Cat.getPhysicalDims(model);
    const rendered = Cat.renderCapacitiveStencilSvg(model, {
      size: 'large', uid: 'print_1to1',
      facia: Object.assign({}, facia || {}, { enabled: true }),
      touchZones, activeZone: activeZone || 1, stencilMode: true
    });
    const vbMatch = rendered.match(/viewBox="([^"]+)"/);
    const vb = vbMatch ? vbMatch[1] : '0 0 210 210';
    const inner = rendered.replace(/^<svg[^>]*>/, '').replace(/<\/svg>$/, '');
    return `<?xml version="1.0" encoding="UTF-8"?>\n<svg xmlns="http://www.w3.org/2000/svg" width="${dims.width_mm}mm" height="${dims.height_mm}mm" viewBox="${vb}" data-model="${model.id}">\n${inner}\n</svg>`;
  }

  function openPrintSheetWindow(remoteName, hexId, model, facia, touchZones, activeZone) {
    const Cat = root.MiBoxerCatalog;
    const dims = Cat.getPhysicalDims(model);
    const svg1to1 = buildOneToOnePrintSvg(model, facia, touchZones, activeZone);
    const html = `<!DOCTYPE html><html><head><meta charset="utf-8"><title>1:1 Capacitive Stencil — ${remoteName} (${model.id})</title>
      <style>
        @page { size: A4 portrait; margin: 12mm; }
        body { font-family: Inter, system-ui, sans-serif; color: #0f172a; margin: 0; padding: 16px; background: #f8fafc; }
        .no-print { background: #0f172a; color: #fff; padding: 12px 18px; border-radius: 10px; margin-bottom: 18px; display: flex; justify-content: space-between; align-items: center; }
        .no-print button { background: #0284c7; color: #fff; border: none; padding: 8px 16px; border-radius: 6px; font-weight: 700; cursor: pointer; }
        .sheet { background: #fff; border: 1px dashed #94a3b8; padding: 16mm; display: flex; flex-direction: column; align-items: center; gap: 10mm; }
        .crop-wrap { padding: 3mm; border: 0.25mm dashed #64748b; display: inline-block; }
        .ruler-row { display: flex; align-items: center; gap: 14px; border-top: 1px solid #cbd5e1; padding-top: 8mm; width: 100%; justify-content: center; font-size: 11px; }
        .box-10mm { width: 10mm; height: 10mm; border: 0.4mm solid #000; display: inline-block; flex-shrink: 0; }
        @media print { body { background: #fff; padding: 0; } .no-print { display: none; } .sheet { border: none; padding: 4mm; } }
      </style></head><body>
      <div class="no-print">
        <div><strong>1:1 Capacitive Glass Overlay Sheet</strong> — Set Printer Scale to <b>100% / Actual Size</b></div>
        <button onclick="window.print()">Print / Save 1:1 PDF</button>
      </div>
      <div class="sheet">
        <div style="text-align:center">
          <div style="font-size:15px;font-weight:800">${remoteName} • ${model.name || model.id} (${hexId})</div>
          <div style="font-size:11px;color:#475569;margin-top:3px">Exact Outer Cut: <b>${dims.width_mm} mm × ${dims.height_mm} mm</b> • Inner Glass: ${dims.glass_w_mm} × ${dims.glass_h_mm} mm</div>
        </div>
        <div class="crop-wrap">${svg1to1}</div>
        <div class="ruler-row">
          <span class="box-10mm"></span>
          <div><b>10 mm × 10 mm Scale Verification Square</b> — Confirm 10.0 mm with a ruler before applying to capacitive panel.</div>
        </div>
      </div></body></html>`;
    const w = window.open('', '_blank');
    if (w) { w.document.open(); w.document.write(html); w.document.close(); }
  }

  // ============================================================================
  // 3. WYSIWYG CAPACITIVE PANEL STENCIL DESIGNER (Presets + Segments + Rooms)
  // ============================================================================
  function slugifyCustomId(baseId, nameStr) {
    const clean = String(nameStr || 'Custom').toUpperCase().replace(/[^A-Z0-9]+/g, '_').replace(/^_+|_+$/g, '').slice(0, 14);
    const sfx = Math.random().toString(36).slice(2, 5).toUpperCase();
    return `CUST_${baseId.replace(/[^A-Z0-9]/gi, '').toUpperCase()}_${clean || 'PANEL'}_${sfx}`;
  }

  function openFaciaDesigner(opts) {
    injectStudioStyles();
    const Cat = root.MiBoxerCatalog;
    const rawModel = typeof opts.model === 'string' ? (Cat.MODELS.find(m => m.id === opts.model) || Cat.MODELS[0]) : opts.model;
    if (!Cat.isCapacitiveFlatPanel(rawModel)) {
      alert(`${rawModel.id} uses mechanical buttons/sliders. Full-surface stencil stickers are currently for flat capacitive glass panels (handheld slider decals coming in v2).`);
      return;
    }

    const isEditingExistingCustom = Boolean(rawModel.isCustom && !opts.forceClone);
    const baseModel = Cat.resolveBaseModel ? Cat.resolveBaseModel(rawModel) : rawModel;
    const bp = Cat.getPanelBlueprint(baseModel);
    const dims = Cat.getPhysicalDims(baseModel);
    const presets = Cat.getSamplePresetsForModel(baseModel);
    const hexId = opts.hexId || '0x0000';
    const remoteName = opts.remoteName || rawModel.name || baseModel.id;
    const hubBase = (opts.hubUrl || '').replace(/\/$/, '');

    let customModelName = isEditingExistingCustom ? (rawModel.name || `${baseModel.id} Custom`) : `${baseModel.id} ${remoteName.replace(baseModel.id, '').trim() || 'Smart Room'}`.trim();
    let bindCurrentRemote = opts.bindRemoteByDefault !== false;

    const seedTz = (isEditingExistingCustom && rawModel.touchZones) ? rawModel.touchZones : opts.touchZones;
    const seedFacia = (isEditingExistingCustom && rawModel.facia) ? rawModel.facia : opts.facia;

    let tz = normalizeTouchZones(seedTz);
    let cal = Object.assign(defaultCalibration(), opts.calibration || {});
    const draft = Object.assign({
      v: 2, enabled: true,
      theme: baseModel.finish === 'black' ? 'matte_black' : 'pure_white',
      monochrome: false, bgPattern: 'clean', sliderStyle: 'factory',
      defaultPadStyle: 'keycap', hideLogo: true, showLedPinhole: true,
      panelTitle: '', material: 'matte_vinyl_peelable', colors: {}, pads: {}
    }, JSON.parse(JSON.stringify(seedFacia || {})), { enabled: true });
    const model = baseModel;

    // Available touch surfaces on this specific panel model
    const panelSurfaces = bp.slots.filter(s => Boolean(s.surf));
    let mode = 'edit';
    let iconTab = model.zones > 1 ? 'room' : 'device';
    let activeZone = 1;
    let selectedPadKey = tz.hue_ring.enabled ? 'hue_seg_1' : (bp.slots.find(s => s.key === 'zone_1') ? 'zone_1' : 'btn_on');
    let lastTestReadout = 'Load a Showcase Preset above or click any stencil slot on the panel to customize.';
    const stepState = {};
    let isFullscreen = false;
    const onFsChange = () => {
      const nativeFs = Boolean(document.fullscreenElement === backdrop);
      if (!nativeFs && isFullscreen && document.fullscreenEnabled) { isFullscreen = false; }
      backdrop.classList.toggle('mlrs-backdrop-fs', isFullscreen || nativeFs);
      const m = backdrop.querySelector('.mlrs-modal');
      if (m) m.classList.toggle('mlrs-fullscreen', isFullscreen || nativeFs);
      const fb = backdrop.querySelector('#mlrsFullBtn');
      if (fb) fb.textContent = (isFullscreen || nativeFs) ? '❐ Exit Fullscreen' : '⤢ Fullscreen';
    };
    document.addEventListener('fullscreenchange', onFsChange);

    const backdrop = document.createElement('div');
    backdrop.className = 'mlrs-modal-backdrop' + (isHostDark() ? ' mlrs-dark' : '');

    function syncLabelToTouchZone(padKey, lbl) {
      const m = padKey.match(/^(hue|cct|sat|bri)_seg_(\d+)$/);
      if (!m) return;
      const map = { hue: 'hue_ring', cct: 'cct_bar', sat: 'sat_bar', bri: 'bri_bar' };
      const sk = map[m[1]], idx = Number(m[2]) - 1;
      if (tz[sk] && idx >= 0 && idx < 12) {
        while (tz[sk].labels.length <= idx) tz[sk].labels.push('');
        tz[sk].labels[idx] = lbl;
      }
    }

    function renderModal() {
      const presetTheme = Cat.FACIA_THEMES[draft.theme] || Cat.FACIA_THEMES.pure_white;
      const effColors = Object.assign({}, presetTheme, draft.colors || {});
      const curPad = draft.pads[selectedPadKey] || {};
      const curMf = curPad.multifunc || { enabled: false, steps: 4, unit: 'Spd' };

      const iconEntries = Object.entries(Cat.FACIA_ICONS).filter(([k, ic]) => k === 'none' || ic.group === iconTab);

      backdrop.innerHTML = `
        <div class="mlrs-modal ${isFullscreen ? 'mlrs-fullscreen' : ''}" onclick="event.stopPropagation()">
          <div class="mlrs-topbar">
            <div>
              <div style="font-size:16px;font-weight:700">Capacitive Panel Facia &amp; Stencil Studio — ${remoteName} (${model.id})</div>
              <div style="font-size:12px;color:var(--mlrs-muted-fg);margin-top:2px">${dims.width_mm} × ${dims.height_mm} mm Glass Stencil • Segment sliders into multi-function device pads &amp; standardized room icons</div>
            </div>
            <div style="display:flex;gap:10px;align-items:center">
              <div class="mlrs-pill-group">
                <button type="button" class="mlrs-btn ${mode === 'edit' ? 'active-tab' : ''}" id="mlrsModeEdit">1. Design Stencil</button>
                <button type="button" class="mlrs-btn ${mode === 'test' ? 'active-tab' : ''}" id="mlrsModeTest">2. Interactive Test (Tap / Hold)</button>
              </div>
              ${opts.onOpenGestures ? `<button type="button" class="mlrs-btn" id="mlrsOpenGestBtn">🎛️ Gestures &amp; Segments</button>` : ''}
              <button type="button" class="mlrs-btn ${isFullscreen ? 'active-tab' : ''}" id="mlrsFullBtn">${isFullscreen ? '❐ Exit Fullscreen' : '⤢ Fullscreen'}</button>
              <button type="button" class="mlrs-btn" id="mlrsCloseBtn">✕ Close</button>
            </div>
          </div>

          <div class="mlrs-preset-bar" style="justify-content:space-between">
            <div style="display:flex;align-items:center;gap:8px;flex-wrap:wrap">
              <span style="padding:3px 8px;border-radius:5px;background:var(--mlrs-bg);border:1px solid var(--mlrs-border);font-weight:700">
                ${isEditingExistingCustom ? `Editing Custom Type: ${rawModel.id}` : `Base Hardware: ${baseModel.id} (Read-Only)`}
              </span>
              <label style="font-weight:600;margin:0">Custom Remote Name:</label>
              <input type="text" class="mlrs-input" id="mlrsCustomModelName" style="width:230px;padding:4px 8px" value="${customModelName.replace(/"/g, '&quot;')}" placeholder="e.g. Master Suite B0"/>
              ${opts.hexId ? `<label style="display:flex;align-items:center;gap:5px;margin-left:6px;cursor:pointer;font-weight:500"><input type="checkbox" id="mlrsBindNowChk" ${bindCurrentRemote ? 'checked' : ''}/> Assign to ${hexId} now</label>` : ''}
            </div>
            <div style="display:flex;align-items:center;gap:6px;flex-wrap:wrap">
              <span style="font-weight:600;color:var(--mlrs-muted-fg)">Starter Templates:</span>
              ${presets.map((pr, idx) => `<button type="button" class="mlrs-btn" data-preset-idx="${idx}" title="${pr.desc}">✨ ${pr.name.split(' (')[0]}</button>`).join('')}
            </div>
          </div>

          <div id="mlrsStudioTourMount" style="padding:8px 20px 0;background:var(--mlrs-bg)"></div>
          <div class="mlrs-body">
            <!-- LEFT: Touch Bar/Ring Segmentation + Surface Theme -->
            <div class="mlrs-col">
              ${panelSurfaces.length ? `
                <div class="mlrs-sec-title">1. Segment Sliders / Ring Into Buttons</div>
                <div class="mlrs-box-sub">
                  ${panelSurfaces.map(ps => {
                    const sc = tz[ps.surf];
                    const maxS = ps.surf === 'hue_ring' ? 8 : 5;
                    const minS = ps.surf === 'hue_ring' ? 3 : 2;
                    const friendly = ps.surf === 'hue_ring' ? (ps.type === 'ring' ? 'Color Ring' : 'Top Color Strip') : ps.surf === 'cct_bar' ? 'CCT Strip' : ps.surf === 'sat_bar' ? 'Left Touch Bar' : 'Dimmer Bar';
                    return `<div style="display:flex;align-items:center;justify-content:space-between;gap:6px;margin-bottom:6px;font-size:12px">
                      <label style="display:flex;align-items:center;gap:6px;margin:0;cursor:pointer;font-weight:600">
                        <input type="checkbox" data-seg-toggle="${ps.surf}" ${sc.enabled ? 'checked' : ''}/> ${friendly}
                      </label>
                      <select class="mlrs-select" style="width:108px;padding:3px 6px;font-size:11.5px" data-seg-count="${ps.surf}" ${!sc.enabled ? 'disabled' : ''}>
                        ${Array.from({ length: maxS - minS + 1 }, (_, k) => minS + k).map(n => `<option value="${n}" ${sc.segments === n ? 'selected' : ''}>${n} Buttons</option>`).join('')}
                      </select>
                    </div>`;
                  }).join('')}
                  <div style="font-size:11px;color:var(--mlrs-muted-fg);margin-top:4px">Enabled bars split into individual icon buttons on both the stencil &amp; ESP32 Hub!</div>
                </div>
              ` : ''}

              <div class="mlrs-sec-title">2. Surface Theme &amp; Print Palette</div>
              <div class="mlrs-field">
                <label>Theme Preset</label>
                <select class="mlrs-select" id="mlrsThemeSel">
                  ${Object.entries(Cat.FACIA_THEMES).map(([k, v]) => `<option value="${k}" ${draft.theme === k ? 'selected' : ''}>${v.name}</option>`).join('')}
                </select>
              </div>
              <div class="mlrs-field" style="display:grid;grid-template-columns:1fr 1fr;gap:7px">
                <div><label>Glass</label><input type="color" class="mlrs-input mlrs-swatch" id="mlrsColGlass" value="${effColors.glass}"/></div>
                <div><label>Keycap Fill</label><input type="color" class="mlrs-input mlrs-swatch" id="mlrsColPad" value="${effColors.padFill}"/></div>
                <div><label>Cut/Border</label><input type="color" class="mlrs-input mlrs-swatch" id="mlrsColStroke" value="${effColors.padStroke}"/></div>
                <div><label>Icon/Text Ink</label><input type="color" class="mlrs-input mlrs-swatch" id="mlrsColInk" value="${effColors.ink}"/></div>
              </div>
              <div class="mlrs-field">
                <label>Default Pad Style</label>
                <select class="mlrs-select" id="mlrsDefPadStyle">
                  <option value="keycap" ${draft.defaultPadStyle === 'keycap' ? 'selected' : ''}>Filled Keycap</option>
                  <option value="outline" ${draft.defaultPadStyle === 'outline' ? 'selected' : ''}>Crisp Stencil Outline</option>
                  <option value="minimal" ${draft.defaultPadStyle === 'minimal' ? 'selected' : ''}>Frameless Icon + Label</option>
                </select>
              </div>
              <div class="mlrs-field" style="display:grid;grid-template-columns:1fr 1fr;gap:8px">
                <div>
                  <label>Background Color</label>
                  <input type="color" class="mlrs-input" id="mlrsBgColor" style="height:32px;padding:2px" value="${draft.bgColor || '#111827'}"/>
                </div>
                <div>
                  <label>Boundary Line Color</label>
                  <input type="color" class="mlrs-input" id="mlrsBoundaryColor" style="height:32px;padding:2px" value="${draft.boundaryColor || '#94a3b8'}"/>
                </div>
              </div>
              <div class="mlrs-field">
                <label>Panel Header Title</label>
                <input type="text" class="mlrs-input" id="mlrsPanelTitle" placeholder="e.g. MASTER SUITE" value="${draft.panelTitle || ''}"/>
              </div>
              <div class="mlrs-box-sub" style="margin-bottom:8px">
                <label style="display:flex;align-items:center;gap:6px;font-size:12px;font-weight:700;cursor:pointer;margin-bottom:5px">
                  <input type="checkbox" id="mlrsLumMaster" ${(draft.luminous && draft.luminous.enabled) ? 'checked' : ''}/> Luminous / Glow-in-the-Dark (POD)
                </label>
                <div style="display:flex;flex-wrap:wrap;gap:10px;font-size:11px">
                  <label style="display:flex;align-items:center;gap:4px;cursor:pointer"><input type="checkbox" id="mlrsLumBorder" ${(draft.luminous && draft.luminous.border) ? 'checked' : ''}/> Outer Boundary</label>
                  <label style="display:flex;align-items:center;gap:4px;cursor:pointer"><input type="checkbox" id="mlrsLumHeader" ${(draft.luminous && draft.luminous.header) ? 'checked' : ''}/> Header / Title</label>
                  <label style="display:flex;align-items:center;gap:4px;cursor:pointer"><input type="checkbox" id="mlrsLumSlots" ${(draft.luminous && draft.luminous.slots) ? 'checked' : ''}/> All Pads / Ring</label>
                </div>
              </div>
              <div class="mlrs-field" style="display:flex;flex-direction:column;gap:5px">
                <label style="display:flex;align-items:center;gap:6px;font-weight:500;cursor:pointer"><input type="checkbox" id="mlrsMonoChk" ${draft.monochrome ? 'checked' : ''}/> Pure Monochrome / B&amp;W Ink</label>
                <label style="display:flex;align-items:center;gap:6px;font-weight:500;cursor:pointer"><input type="checkbox" id="mlrsLedHole" ${draft.showLedPinhole ? 'checked' : ''}/> Show Status LED Pinhole</label>
              </div>
            </div>

            <!-- CENTER: Accurate 1:1 Capacitive Stencil Canvas -->
            <div class="mlrs-stage-wrap">
              <div style="margin-bottom:10px;font-size:12px;color:var(--mlrs-muted-fg);text-align:center">
                ${mode === 'edit' ? '👆 <b>Design Mode:</b> Click any button, room rocker, or segmented wheel/bar slot to assign its icon &amp; function' : '🧪 <b>Interactive Test:</b> Short-tap multi-function pads to step (1→6), or <b>Hold 0.6s</b> to toggle ON/OFF!'}
              </div>
              <div class="mlrs-stage-card">
                <div id="mlrsCanvasStage" style="display:flex;align-items:center;justify-content:center;min-height:415px"></div>
                <div id="mlrsTestBanner" style="margin-top:12px;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:11.5px;color:var(--mlrs-fg);background:var(--mlrs-muted);padding:6px 12px;border-radius:6px;border:1px solid var(--mlrs-border);text-align:center">
                  ${lastTestReadout}
                </div>
              </div>
            </div>

            <!-- RIGHT: Selected Slot Inspector + Standardized Room & Device Icons -->
            <div class="mlrs-col">
              <div class="mlrs-sec-title" style="margin-bottom:6px">3. Selected Slot: <code style="padding:2px 6px;border-radius:4px;background:var(--mlrs-muted);border:1px solid var(--mlrs-border)">${selectedPadKey}</code></div>

              ${/^(hue|bri|cct|sat)_seg_\d+$/.test(selectedPadKey) ? `
              <div class="mlrs-field">
                <label>Segment Behavior (Button vs. Variable Slider)</label>
                <select class="mlrs-select" id="mlrsSegMode">
                  <option value="button" ${(curPad.segmentMode || 'button') === 'button' ? 'selected' : ''}>Button (Tap / Step / Hold)</option>
                  <option value="slider" ${curPad.segmentMode === 'slider' ? 'selected' : ''}>Variable Slider (Custom X% to Y% Range)</option>
                </select>
                ${curPad.segmentMode === 'slider' ? `
                <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:6px">
                  <div>
                    <label style="font-size:11px">Start End (X %)</label>
                    <input type="number" min="0" max="100" class="mlrs-input" id="mlrsSegMin" value="${curPad.sliderMin ?? 0}"/>
                  </div>
                  <div>
                    <label style="font-size:11px">Far / Max End (Y %)</label>
                    <input type="number" min="0" max="100" class="mlrs-input" id="mlrsSegMax" value="${curPad.sliderMax ?? 100}"/>
                  </div>
                </div>` : ''}
              </div>` : ''}

              <div class="mlrs-field">
                <label style="display:flex;align-items:center;gap:6px;font-weight:600;cursor:pointer">
                  <input type="checkbox" id="mlrsPadLuminous" ${curPad.luminous ? 'checked' : ''}/> Luminous / Glow on This Slot (POD)
                </label>
              </div>
              <div class="mlrs-field">
                <label>Slot Visibility / Style</label>
                <select class="mlrs-select" id="mlrsPadStyle">
                  <option value="" ${!curPad.style ? 'selected' : ''}>Use Panel Default (${draft.defaultPadStyle})</option>
                  <option value="keycap" ${curPad.style === 'keycap' ? 'selected' : ''}>Filled Keycap</option>
                  <option value="outline" ${curPad.style === 'outline' ? 'selected' : ''}>Crisp Outline</option>
                  <option value="minimal" ${curPad.style === 'minimal' ? 'selected' : ''}>Icon + Label Only</option>
                  <option value="blank" ${curPad.style === 'blank' ? 'selected' : ''}>Mask / Blank Unused Slot</option>
                </select>
              </div>

              <div class="mlrs-field" style="display:grid;grid-template-columns:1fr 1fr;gap:8px">
                <div>
                  <label>Primary Label</label>
                  <input type="text" class="mlrs-input" id="mlrsPadLabel" placeholder="e.g. Living, Fan" value="${curPad.label !== undefined ? curPad.label : ''}"/>
                </div>
                <div>
                  <label>Sub-Label / Hint</label>
                  <input type="text" class="mlrs-input" id="mlrsPadSub" placeholder="Auto or custom" value="${curPad.sublabel !== undefined ? curPad.sublabel : ''}"/>
                </div>
              </div>

              <!-- Single-Button Multi-Function Showcase Config (Tap = Step 1..N, Hold = Toggle ON/OFF) -->
              <div class="mlrs-box-sub">
                <label style="display:flex;align-items:center;gap:6px;font-size:12px;font-weight:700;cursor:pointer;margin-bottom:5px">
                  <input type="checkbox" id="mlrsMfChk" ${curMf.enabled ? 'checked' : ''}/> Single-Button Multi-Function (Step + Hold)
                </label>
                <div style="font-size:11px;color:var(--mlrs-muted-fg);margin-bottom:6px">Short tap increments step (+1 cycle); long press toggles ON/OFF.</div>
                ${curMf.enabled ? `
                  <div style="display:grid;grid-template-columns:1fr 1fr;gap:7px">
                    <div><label style="font-size:11px">Cycle Steps (2–8)</label><input type="number" min="2" max="8" class="mlrs-input" id="mlrsMfSteps" value="${curMf.steps || 6}"/></div>
                    <div><label style="font-size:11px">Step Unit</label><input type="text" class="mlrs-input" id="mlrsMfUnit" placeholder="Spd / Pos / Temp" value="${curMf.unit || 'Spd'}"/></div>
                  </div>
                ` : ''}
              </div>

              <div class="mlrs-field">
                <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:6px">
                  <label style="margin:0">Standardized Icons</label>
                  <div class="mlrs-pill-group">
                    <button type="button" class="mlrs-btn ${iconTab === 'room' ? 'active-tab' : ''}" style="padding:3px 8px;font-size:11px" id="mlrsTabRoom">Rooms (12)</button>
                    <button type="button" class="mlrs-btn ${iconTab === 'device' ? 'active-tab' : ''}" style="padding:3px 8px;font-size:11px" id="mlrsTabDev">Devices (20)</button>
                  </div>
                </div>
                <div class="mlrs-icon-grid" id="mlrsIconGrid">
                  ${iconEntries.map(([k, ic]) => `
                    <button type="button" class="mlrs-icon-cell ${(curPad.icon || 'none') === k ? 'sel' : ''}" data-ico="${k}" title="${ic.name}">
                      ${ic.svg ? `<svg viewBox="0 0 24 24" width="18" height="18">${ic.svg}</svg>` : '<span style="font-size:10px;color:var(--mlrs-muted-fg)">None</span>'}
                    </button>
                  `).join('')}
                </div>
              </div>
            </div>
          </div>

          <div class="mlrs-footer">
            <div style="display:flex;gap:8px;align-items:center">
              ${isEditingExistingCustom ? `<button type="button" class="mlrs-btn danger-outline" id="mlrsDeleteCustomBtn">Delete Custom Remote (${rawModel.id})</button>` : `<span style="font-size:12px;color:var(--mlrs-muted-fg)">Factory ${baseModel.id} is read-only — saving creates a reusable Custom Remote type.</span>`}
            </div>
            <div style="display:flex;gap:8px;align-items:center;flex-wrap:wrap">
              <button type="button" class="mlrs-btn" id="mlrsPrintBtn">Print 1:1 Stencil Sheet</button>
              <button type="button" class="mlrs-btn" id="mlrsDlSvgBtn">Download 1:1 SVG</button>
              ${isEditingExistingCustom ? `<button type="button" class="mlrs-btn" id="mlrsSaveCopyBtn">Save as New Copy</button>` : ''}
              <button type="button" class="mlrs-btn" id="mlrsCancelBtn">Cancel</button>
              <button type="button" class="mlrs-btn primary" id="mlrsDeployBtn">${isEditingExistingCustom ? 'Update Custom Remote' : '+ Create Custom Remote'}</button>
            </div>
          </div>
        </div>
      `;

      const stageBox = backdrop.querySelector('#mlrsCanvasStage');
      const refreshStageOnly = () => {
        mountInteractiveStage(stageBox, {
          model, uid: 'facia_editor_svg', maxHeight: isFullscreen ? 'min(78vh, 820px)' : 'min(66vh, 590px)',
          squareHeight: isFullscreen ? 'min(70vh, 720px)' : 'min(56vh, 500px)',
          facia: draft, touchZones: tz, calibration: cal, activeZone,
          editMode: mode === 'edit', selectedPadKey: mode === 'edit' ? selectedPadKey : null,
          stepState,
          onSelectPad: (pk) => { selectedPadKey = pk; renderModal(); },
          onZoneChange: (z) => { activeZone = z; },
          onAction: (act, val, z, customMsg) => {
            lastTestReadout = customMsg ? `🟢 ${customMsg}  →  Hub TX (${act}=${val}, Z${z})` : `🟠 TX → Zone ${z} • ${act}(${val})`;
            const b = backdrop.querySelector('#mlrsTestBanner');
            if (b) b.textContent = lastTestReadout;
            if (typeof opts.onTestAction === 'function') opts.onTestAction(act, val, z);
          }
        });
      };
      refreshStageOnly();

      // Mode & Close buttons
      backdrop.querySelector('#mlrsModeEdit').onclick = () => { mode = 'edit'; renderModal(); };
      backdrop.querySelector('#mlrsModeTest').onclick = () => { mode = 'test'; lastTestReadout = 'Tap any multi-function device button to cycle steps (1→6), or Hold 0.6s to toggle ON/OFF!'; renderModal(); };
      backdrop.querySelector('#mlrsCloseBtn').onclick = () => backdrop.remove();
      const cleanClose = () => {
        document.removeEventListener('fullscreenchange', onFsChange);
        if (document.fullscreenElement === backdrop && document.exitFullscreen) {
          document.exitFullscreen().catch(() => {});
        }
        backdrop.remove();
      };
      backdrop.querySelector('#mlrsCloseBtn').onclick = cleanClose;
      backdrop.querySelector('#mlrsCancelBtn').onclick = cleanClose;

      const fb = backdrop.querySelector('#mlrsFullBtn');
      if (fb) {
        fb.onclick = async () => {
          const currentlyFs = isFullscreen || (document.fullscreenElement === backdrop);
          if (!currentlyFs) {
            isFullscreen = true;
            backdrop.classList.add('mlrs-backdrop-fs');
            if (backdrop.requestFullscreen) {
              try { await backdrop.requestFullscreen(); } catch (_) {}
            }
          } else {
            isFullscreen = false;
            backdrop.classList.remove('mlrs-backdrop-fs');
            if (document.fullscreenElement && document.exitFullscreen) {
              try { await document.exitFullscreen(); } catch (_) {}
            }
          }
          renderModal();
        };
      }
      const gb = backdrop.querySelector('#mlrsOpenGestBtn');
      if (gb && typeof opts.onOpenGestures === 'function') {
        gb.onclick = () => { backdrop.remove(); opts.onOpenGestures(); };
      }
      const tourBox = backdrop.querySelector('#mlrsStudioTourMount');
      if (tourBox) {
        mountShowcaseTour(tourBox, {
          tourId: 'stencil_editor',
          steps: [
            { title: 'Full-Viewport Canvas & Starter Templates', body: 'Click Fullscreen above for maximum design space, or load a Starter Template to auto-populate room icons & segmented wheel buttons.', actionLabel: 'Toggle Fullscreen', onAction: () => backdrop.querySelector('#mlrsFullBtn')?.click() },
            { title: 'Split Rings & Sliders Into Discrete Buttons', body: 'Use Left Panel Section 1 to turn continuous Color Rings or Touch Bars into 2 to 8 labeled macro buttons.', actionLabel: 'Select Ring Slot #1', onAction: () => { selectedPadKey = 'hue_seg_1'; mode = 'edit'; renderModal(); } },
            { title: 'Architectural Themes & Blank Pad Masking', body: 'Switch between Pure White, Warm Travertine, or Matte Black vinyl palettes and set unused buttons to Blank Masked.', actionLabel: 'Preview Warm Stone', onAction: () => { draft.theme = 'warm_stone'; draft.colors = null; renderModal(); } },
            { title: 'Single-Button Step-Cycling + Hold Toggle', body: 'Enable Multi-Function on any pad so short taps cycle speeds/positions (1 to 6) while a long hold toggles OFF — test it live in Mode 2!', actionLabel: 'Switch to Interactive Test', onAction: () => { mode = 'test'; renderModal(); } },
            { title: 'Calibrated 1:1 Millimeter Print & SVG Export', body: 'Print Sheet adds a 10 mm verification square, and Download 1:1 SVG uses an HTTP attachment stream compatible with download extensions.', actionLabel: 'Download 1:1 SVG', onAction: () => backdrop.querySelector('#mlrsDlSvgBtn')?.click() }
          ]
        });
      }
      backdrop.querySelector('#mlrsCancelBtn').onclick = () => backdrop.remove();

      // Preset Showcase Loader buttons
      backdrop.querySelectorAll('[data-preset-idx]').forEach(btn => {
        btn.onclick = () => {
          const pr = presets[Number(btn.getAttribute('data-preset-idx'))];
          if (!pr) return;
          tz = normalizeTouchZones(JSON.parse(JSON.stringify(pr.touch_zones)));
          Object.assign(draft, JSON.parse(JSON.stringify(pr.facia)));
          selectedPadKey = Object.keys(draft.pads || {})[0] || selectedPadKey;
          lastTestReadout = `Loaded "${pr.name}" — switch to "2. Interactive Test" to try short-tap step cycling & long-hold toggle!`;
          renderModal();
        };
      });

      // Segmentation toggles & counts
      backdrop.querySelectorAll('[data-seg-toggle]').forEach(chk => {
        chk.onchange = () => {
          const sk = chk.getAttribute('data-seg-toggle');
          tz[sk].enabled = chk.checked;
          if (chk.checked) {
            const pref = sk === 'hue_ring' ? 'hue_seg_1' : sk === 'cct_bar' ? 'cct_seg_1' : sk === 'sat_bar' ? 'sat_seg_1' : 'bri_seg_1';
            selectedPadKey = pref;
          }
          renderModal();
        };
      });
      backdrop.querySelectorAll('[data-seg-count]').forEach(sel => {
        sel.onchange = () => {
          const sk = sel.getAttribute('data-seg-count');
          tz[sk].segments = Number(sel.value);
          while (tz[sk].labels.length < tz[sk].segments) tz[sk].labels.push('');
          renderModal();
        };
      });

      // Surface Theme & Colors
      backdrop.querySelector('#mlrsThemeSel').onchange = (e) => {
        draft.theme = e.target.value; draft.colors = {};
        draft.monochrome = Boolean((Cat.FACIA_THEMES[draft.theme] || {}).mono);
        renderModal();
      };
      [['#mlrsColGlass','glass'],['#mlrsColPad','padFill'],['#mlrsColStroke','padStroke'],['#mlrsColInk','ink']].forEach(([id, k]) => {
        backdrop.querySelector(id).oninput = (e) => { draft.colors = draft.colors || {}; draft.colors[k] = e.target.value; refreshStageOnly(); };
      });
      backdrop.querySelector('#mlrsDefPadStyle').onchange = (e) => { draft.defaultPadStyle = e.target.value; renderModal(); };
      backdrop.querySelector('#mlrsPanelTitle').oninput = (e) => { draft.panelTitle = e.target.value; refreshStageOnly(); };
      const bgColEl = backdrop.querySelector('#mlrsBgColor');
      if (bgColEl) bgColEl.oninput = (e) => { draft.bgColor = e.target.value; refreshStageOnly(); };
      const bdrColEl = backdrop.querySelector('#mlrsBoundaryColor');
      if (bdrColEl) bdrColEl.oninput = (e) => { draft.boundaryColor = e.target.value; refreshStageOnly(); };
      const ensureLum = () => { if (!draft.luminous) draft.luminous = { enabled: false, border: true, header: false, slots: false }; return draft.luminous; };
      const lumMasterEl = backdrop.querySelector('#mlrsLumMaster');
      if (lumMasterEl) lumMasterEl.onchange = (e) => { const l = ensureLum(); l.enabled = e.target.checked; if (e.target.checked && !l.border && !l.header && !l.slots) l.border = true; refreshStageOnly(); };
      const lumBorderEl = backdrop.querySelector('#mlrsLumBorder');
      if (lumBorderEl) lumBorderEl.onchange = (e) => { const l = ensureLum(); l.border = e.target.checked; if (e.target.checked) l.enabled = true; refreshStageOnly(); };
      const lumHeaderEl = backdrop.querySelector('#mlrsLumHeader');
      if (lumHeaderEl) lumHeaderEl.onchange = (e) => { const l = ensureLum(); l.header = e.target.checked; if (e.target.checked) l.enabled = true; refreshStageOnly(); };
      const lumSlotsEl = backdrop.querySelector('#mlrsLumSlots');
      if (lumSlotsEl) lumSlotsEl.onchange = (e) => { const l = ensureLum(); l.slots = e.target.checked; if (e.target.checked) l.enabled = true; refreshStageOnly(); };
      const padLumEl = backdrop.querySelector('#mlrsPadLuminous');
      if (padLumEl) padLumEl.onchange = (e) => { const p = ensurePad(); p.luminous = e.target.checked; refreshStageOnly(); };
      backdrop.querySelector('#mlrsMonoChk').onchange = (e) => { draft.monochrome = e.target.checked; renderModal(); };
      backdrop.querySelector('#mlrsLedHole').onchange = (e) => { draft.showLedPinhole = e.target.checked; refreshStageOnly(); };

      // Right Column Inspector
      const ensurePad = () => { if (!draft.pads[selectedPadKey]) draft.pads[selectedPadKey] = {}; return draft.pads[selectedPadKey]; };
      backdrop.querySelector('#mlrsPadStyle').onchange = (e) => { const p = ensurePad(); if (e.target.value) p.style = e.target.value; else delete p.style; renderModal(); };
      const segModeEl = backdrop.querySelector('#mlrsSegMode');
      if (segModeEl) segModeEl.onchange = (e) => {
        const p = ensurePad();
        p.segmentMode = e.target.value;
        if (p.segmentMode === 'slider' && p.multifunc) p.multifunc.enabled = false;
        renderModal();
      };
      const segMinEl = backdrop.querySelector('#mlrsSegMin');
      if (segMinEl) segMinEl.oninput = (e) => {
        const p = ensurePad();
        p.sliderMin = Math.max(0, Math.min(100, Number(e.target.value || 0)));
        refreshStageOnly();
      };
      const segMaxEl = backdrop.querySelector('#mlrsSegMax');
      if (segMaxEl) segMaxEl.oninput = (e) => {
        const p = ensurePad();
        p.sliderMax = Math.max(0, Math.min(100, Number(e.target.value || 100)));
        refreshStageOnly();
      };
      backdrop.querySelector('#mlrsPadLabel').oninput = (e) => { const p = ensurePad(); p.label = e.target.value; syncLabelToTouchZone(selectedPadKey, e.target.value); refreshStageOnly(); };
      backdrop.querySelector('#mlrsPadSub').oninput = (e) => { const p = ensurePad(); p.sublabel = e.target.value; refreshStageOnly(); };
      backdrop.querySelector('#mlrsMfChk').onchange = (e) => {
        const p = ensurePad();
        p.multifunc = Object.assign({ steps: 6, unit: 'Spd' }, p.multifunc || {}, { enabled: e.target.checked });
        renderModal();
      };
      const mfS = backdrop.querySelector('#mlrsMfSteps');
      if (mfS) mfS.oninput = (e) => { ensurePad().multifunc.steps = Number(e.target.value || 4); refreshStageOnly(); };
      const mfU = backdrop.querySelector('#mlrsMfUnit');
      if (mfU) mfU.oninput = (e) => { ensurePad().multifunc.unit = e.target.value; refreshStageOnly(); };

      backdrop.querySelector('#mlrsTabRoom').onclick = () => { iconTab = 'room'; renderModal(); };
      backdrop.querySelector('#mlrsTabDev').onclick = () => { iconTab = 'device'; renderModal(); };
      backdrop.querySelectorAll('#mlrsIconGrid .mlrs-icon-cell').forEach(btn => {
        btn.onclick = () => {
          const p = ensurePad();
          const icoKey = btn.getAttribute('data-ico');
          p.icon = icoKey;
          // Auto-fill clean room/device name if label wasn't set yet
          if (icoKey !== 'none' && !p.label) {
            const icObj = Cat.FACIA_ICONS[icoKey];
            if (icObj) {
              p.label = icObj.name.split(' (')[0].split(' / ')[0];
              syncLabelToTouchZone(selectedPadKey, p.label);
            }
          }
          renderModal();
        };
      });

      // Print & Export
      backdrop.querySelector('#mlrsPrintBtn').onclick = () => openPrintSheetWindow(remoteName, hexId, model, draft, tz, activeZone);
      backdrop.querySelector('#mlrsDlSvgBtn').onclick = (ev) => {
        if (ev) { ev.preventDefault(); ev.stopPropagation(); }
        const svgStr = buildOneToOnePrintSvg(model, draft, tz, activeZone);
        const fName = `${model.id.toLowerCase()}_${hexId.toLowerCase()}_stencil_1to1.svg`;
        downloadTextFile(fName, svgStr, 'image/svg+xml', { hubBase });
      };

      const nameInp = backdrop.querySelector('#mlrsCustomModelName');
      if (nameInp) nameInp.oninput = (e) => { customModelName = e.target.value; };
      const bindChk = backdrop.querySelector('#mlrsBindNowChk');
      if (bindChk) bindChk.onchange = (e) => { bindCurrentRemote = e.target.checked; };

      const persistCustomModel = async (forceNewCopy) => {
        const cleanName = (customModelName || '').trim() || `${baseModel.id} Custom`;
        const targetId = (isEditingExistingCustom && !forceNewCopy) ? rawModel.id : slugifyCustomId(baseModel.id, cleanName);
        const payload = {
          id: targetId,
          name: forceNewCopy ? `${cleanName} Copy` : cleanName,
          base_model: baseModel.id,
          stencil_mode: 'full_panel',
          facia: draft,
          touch_zones: tz
        };
        const res = await fetch(`${hubBase}/api/remotes/custom_models`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        await Cat.syncCustomModelsFromHub(hubBase);
        if (typeof opts.onSaveCustomModel === 'function') {
          await opts.onSaveCustomModel({ customModel: payload, bindNow: bindCurrentRemote, calibration: cal, touchZones: tz, facia: draft });
        } else if (bindCurrentRemote && typeof opts.onSave === 'function') {
          await opts.onSave(draft, cal, tz, targetId);
        }
        backdrop.remove();
      };

      const delBtn = backdrop.querySelector('#mlrsDeleteCustomBtn');
      if (delBtn) {
        delBtn.onclick = async () => {
          if (!confirm(`Delete custom remote "${rawModel.name}" (${rawModel.id})? Any bound remotes will safely revert to factory ${baseModel.id}.`)) return;
          await fetch(`${hubBase}/api/remotes/custom_models/delete`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: rawModel.id })
          });
          await Cat.syncCustomModelsFromHub(hubBase);
          if (typeof opts.onDeleteCustomModel === 'function') await opts.onDeleteCustomModel(rawModel.id, baseModel.id);
          backdrop.remove();
        };
      }

      const copyBtn = backdrop.querySelector('#mlrsSaveCopyBtn');
      if (copyBtn) copyBtn.onclick = () => persistCustomModel(true);

      backdrop.querySelector('#mlrsDeployBtn').onclick = async () => {
        const btn = backdrop.querySelector('#mlrsDeployBtn');
        btn.textContent = 'Saving Custom Remote...';
        try {
          await persistCustomModel(false);
        } catch (err) {
          btn.textContent = isEditingExistingCustom ? 'Update Custom Remote' : '+ Create Custom Remote';
          alert('Save failed: ' + err);
        }
      };
    }

    document.body.appendChild(backdrop);
    renderModal();
    return backdrop;
  }

  function normalizeSliderPct(rawVal, act) {
    if (rawVal === undefined || rawVal === null || rawVal === '') return 0;
    const n = Number(rawVal);
    if (Number.isNaN(n)) return 0;
    let pct = n > 100 ? (n / 255) * 100 : n;
    const a = String(act || '').toLowerCase();
    if (a === 'brightness' || a === 'level') {
      // Physical MiBoxer brightness sliders report ~26 at the bottom stop and ~98-100 at the top
      if (pct <= 26.5) return 0;
      if (pct >= 98) return 100;
      pct = ((pct - 26.5) / (98 - 26.5)) * 100;
    }
    return Math.max(0, Math.min(100, pct));
  }

  // Physical RF hue -> clockwise degrees from 12 o'clock (0=top, 90=3h, 180=6h, 270=9h)
  function physicalHueToWheelDeg(rawHue, cal) {
    let h = Number(rawHue || 0);
    if (h > 360) h = (h / 255) * 360;
    h = ((h % 360) + 360) % 360;
    let deg = (270 - h + 360) % 360;
    if (cal && cal.hue_invert) deg = (360 - deg) % 360;
    if (cal && cal.hue_offset) deg = (deg + Number(cal.hue_offset) + 360) % 360;
    return deg;
  }

  function resolvePhysicalToCustomAction(action, value, zone, touchZones, calibration) {
    const tz = normalizeTouchZones(touchZones);
    const cal = Object.assign(defaultCalibration(), calibration || {});
    const z = Number(zone || 0);
    const zBit = z < 16 ? (1 << z) : 0x8000;
    const act = String(action || '').toLowerCase();
    const numVal = (value !== undefined && value !== null && value !== '') ? Number(value) : -1;

    if (act === 'hue' && numVal >= 0 && tz.hue_ring.enabled && (tz.hue_ring.zone_mask & zBit)) {
      const n = Math.min(12, Math.max(2, Number(tz.hue_ring.segments || 4)));
      const span = 360 / n;
      const deg = physicalHueToWheelDeg(numVal, cal);
      const rel = ((deg - Number(tz.hue_ring.offset_deg || 0)) % 360 + 360) % 360;
      const idx = Math.min(n - 1, Math.max(0, Math.floor(rel / span)));
      return {
        action: `hue_seg_${idx + 1}`,
        value: idx + 1,
        label: tz.hue_ring.labels[idx] || `Slice ${idx + 1}`,
        converted: true
      };
    }

    let targetBar = null, pref = '';
    if (act === 'saturation' && tz.sat_bar.enabled && (tz.sat_bar.zone_mask & zBit)) {
      targetBar = tz.sat_bar; pref = 'sat_seg_';
    } else if ((act === 'color_temp' || act === 'kelvin') && tz.cct_bar.enabled && (tz.cct_bar.zone_mask & zBit)) {
      targetBar = tz.cct_bar; pref = 'cct_seg_';
    } else if ((act === 'color_temp' || act === 'kelvin') && tz.sat_bar.enabled && !tz.cct_bar.enabled && (tz.sat_bar.zone_mask & zBit)) {
      targetBar = tz.sat_bar; pref = 'sat_seg_';
    } else if ((act === 'brightness' || act === 'level') && tz.bri_bar.enabled && (tz.bri_bar.zone_mask & zBit)) {
      targetBar = tz.bri_bar; pref = 'bri_seg_';
    }

    if (targetBar && numVal >= 0) {
      const n = Math.min(6, Math.max(2, Number(targetBar.segments || 4)));
      const rawPct = numVal > 100 ? (numVal / 255) * 100 : Math.max(0, Math.min(100, numVal));
      // Physical B8 saturation bar outputs 0 at TOP and 100 at BOTTOM:
      // In miboxer_catalog.js, sat_seg_1 is drawn at BOTTOM (idx=0) and sat_seg_N at TOP (idx=n-1).
      // So 0% (top touch) -> sat_seg_N (top button), 50% -> middle, 100% (bottom touch) -> sat_seg_1 (bottom button)!
      if (pref === 'sat_seg_') {
        const fromTopIdx = Math.min(n - 1, Math.max(0, Math.floor((rawPct / 100.0001) * n)));
        const idx = (n - 1) - fromTopIdx;
        return {
          action: `${pref}${idx + 1}`,
          value: idx + 1,
          label: targetBar.labels[idx] || `Slot ${idx + 1}`,
          converted: true
        };
      }
      const effPct = (pref === 'bri_seg_') ? normalizeSliderPct(numVal, 'brightness') : rawPct;
      const idx = Math.min(n - 1, Math.max(0, Math.floor((effPct / 100.0001) * n)));
      return {
        action: `${pref}${idx + 1}`,
        value: idx + 1,
        label: targetBar.labels[idx] || `Slot ${idx + 1}`,
        converted: true
      };
    }

    return { action, value: numVal, label: '', converted: false };
  }


  function resolveCustomToPhysicalAction(action, value, zone, touchZones, cal) {
    const act = String(action || '');
    const m = act.match(/^(hue|sat|cct|bri)_seg_(\d+)$/);
    if (!m) return { action: act, value: value, visDeg: undefined, pct: undefined, converted: false };
    const kind = m[1];
    const segNum = Math.max(1, Number(m[2] || value || 1));
    const tz = normalizeTouchZones(touchZones);
    if (kind === 'hue') {
      const n = Math.min(12, Math.max(2, Number(tz.hue_ring.segments || 6)));
      const span = 360 / n;
      const off = Number(tz.hue_ring.offset_deg || 0);
      const visDeg = (((off + (segNum - 0.5) * span) % 360) + 360) % 360;
      const c = Object.assign(defaultCalibration(), cal || {});
      const topRaw = typeof c.hueTopRaw === 'number' ? c.hueTopRaw : (c.wheel_top_raw ?? 0);
      const dir = (c.hueDir === -1 || c.wheel_dir === -1) ? -1 : 1;
      const rawHue = Math.round((((topRaw + dir * visDeg) % 360) + 360) % 360);
      return { action: 'hue', value: rawHue, visDeg, converted: true };
    }
    const cfg = kind === 'sat' ? tz.sat_bar : (kind === 'cct' ? tz.cct_bar : tz.bri_bar);
    const n = Math.min(6, Math.max(2, Number((cfg && cfg.segments) || 4)));
    const pct = Math.max(0, Math.min(100, Math.round(((segNum - 0.5) / n) * 100)));
    if (kind === 'sat') return { action: 'saturation', value: Math.max(0, Math.min(100, Math.round(100 - pct))), pct, converted: true };
    if (kind === 'cct') return { action: 'color_temp', value: Math.round(370 - (pct / 100) * 217), pct, converted: true };
    return { action: 'brightness', value: pct, pct, converted: true };
  }


  function computePeelClipPolygons(peelPct) {
    const p = Math.max(0, Math.min(100, Number(peelPct) || 0));
    if (p <= 0.5) return { overClip: 'polygon(0% 0%, 100% 0%, 100% 100%, 0% 100%)', flapClip: 'polygon(100% 0%, 100% 0%, 100% 0%)', flapOpacity: 0 };
    if (p >= 99.5) return { overClip: 'polygon(0% 100%, 0% 100%, 0% 100%)', flapClip: 'polygon(0% 0%, 0% 0%, 0% 0%)', flapOpacity: 0 };
    const d = (p / 100) * 200;
    if (d <= 100) {
      const xCut = 100 - d, yCut = d;
      const fx = Math.max(0, xCut - d * 0.28), fy = Math.min(100, yCut + d * 0.28);
      return {
        overClip: `polygon(0% 0%, ${xCut.toFixed(1)}% 0%, 100% ${yCut.toFixed(1)}%, 100% 100%, 0% 100%)`,
        flapClip: `polygon(${xCut.toFixed(1)}% 0%, 100% ${yCut.toFixed(1)}%, ${fx.toFixed(1)}% ${fy.toFixed(1)}%)`,
        flapOpacity: 0.92
      };
    }
    const rem = 200 - d, xBot = rem, yLeft = 100 - rem;
    const fx = Math.max(0, xBot * 0.55), fy = Math.min(100, yLeft + (100 - yLeft) * 0.45);
    return {
      overClip: `polygon(0% ${yLeft.toFixed(1)}%, ${xBot.toFixed(1)}% 100%, 0% 100%)`,
      flapClip: `polygon(0% ${yLeft.toFixed(1)}%, ${xBot.toFixed(1)}% 100%, ${fx.toFixed(1)}% ${fy.toFixed(1)}%)`,
      flapOpacity: Math.max(0.18, rem / 110)
    };
  }

  function mountPeelableStage(containerEl, opts) {
    injectStudioStyles(containerEl);
    const Cat = root.MiBoxerCatalog;
    if (!containerEl || !Cat) return null;
    const overlayModel = typeof opts.overlayModel === 'string' ? (Cat.MODELS.find(m => m.id === opts.overlayModel) || Cat.MODELS[0]) : opts.overlayModel;
    const baseId = (overlayModel && (overlayModel.baseModel || overlayModel.base)) || opts.physicalModelId || 'B8';
    const candPhys = opts.physicalModel
      ? (typeof opts.physicalModel === 'string' ? Cat.MODELS.find(m => m.id === opts.physicalModel) : opts.physicalModel)
      : null;
    const physicalModel = (candPhys && (!Cat.areModelsPhysicallyCompatible || Cat.areModelsPhysicallyCompatible(overlayModel, candPhys)))
      ? candPhys
      : (Cat.MODELS.find(m => m.id === baseId && !m.isCustom) || Cat.MODELS.find(m => (m.base || m.id) === String(baseId).replace(/-B$/, '') && !m.isCustom) || Cat.MODELS[0]);

    let peelPct = Math.max(0, Math.min(100, Number(opts.initialPeelPct || 0)));
    const maxH = opts.maxHeight || 'clamp(370px, 56vh, 620px)';
    const linkedTitle = opts.linkedBadgeText || `Custom Stencil ⇄ Bare ${physicalModel.id}`;

    containerEl.innerHTML = `
      <div class="mlrs-peel-wrap">
        <div class="mlrs-peel-bar">
          <button type="button" class="mlrs-peel-btn" data-role="peel-toggle">🩹 Peel Back Stencil</button>
          <input type="range" min="0" max="100" value="${peelPct}" data-role="peel-range" style="width:105px;cursor:pointer;accent-color:#0284c7"/>
          <span data-role="peel-lbl" style="font-weight:700;min-width:110px;text-align:left">Stencil On (0%)</span>
          <span style="color:#0284c7;font-weight:700">${linkedTitle}</span>
        </div>
        <div class="mlrs-peel-stack" data-role="peel-stack">
          <div class="mlrs-peel-under" data-role="under-stage"></div>
          <div class="mlrs-peel-over" data-role="over-stage"></div>
          <div class="mlrs-peel-flap" data-role="peel-flap"></div>
          <div class="mlrs-peel-corner-hit" data-role="peel-corner" title="Click corner to peel back custom stencil"></div>
        </div>
      </div>
    `;

    const stackEl = containerEl.querySelector('[data-role="peel-stack"]');
    const underEl = containerEl.querySelector('[data-role="under-stage"]');
    const overEl  = containerEl.querySelector('[data-role="over-stage"]');
    const flapEl  = containerEl.querySelector('[data-role="peel-flap"]');
    const cornerEl = containerEl.querySelector('[data-role="peel-corner"]');
    const btnEl   = containerEl.querySelector('[data-role="peel-toggle"]');
    const rangeEl = containerEl.querySelector('[data-role="peel-range"]');
    const lblEl   = containerEl.querySelector('[data-role="peel-lbl"]');

    const underCtrl = mountInteractiveStage(underEl, {
      model: physicalModel, uid: `${opts.uid || 'peel'}_phys`, maxHeight: maxH,
      facia: null, touchZones: null,
      calibration: opts.physicalCalibration || opts.calibration, activeZone: opts.activeZone ?? 1,
      onZoneChange: (z) => { if (opts.onZoneChange) opts.onZoneChange(z); },
      onScrubPreview: opts.onScrubPreview,
      onAction: (act, val, z, lbl) => { if (opts.onPhysicalAction) opts.onPhysicalAction(act, val, z, lbl); else if (opts.onAction) opts.onAction(act, val, z, lbl); }
    });

    const overCtrl = mountInteractiveStage(overEl, {
      model: overlayModel, uid: `${opts.uid || 'peel'}_cust`, maxHeight: maxH,
      facia: opts.facia || (overlayModel && overlayModel.facia),
      touchZones: opts.touchZones || (overlayModel && overlayModel.touchZones),
      calibration: opts.calibration, activeZone: opts.activeZone ?? 1,
      onZoneChange: (z) => { if (opts.onZoneChange) opts.onZoneChange(z); },
      onScrubPreview: opts.onScrubPreview,
      onAction: (act, val, z, lbl) => { if (opts.onCustomAction) opts.onCustomAction(act, val, z, lbl); else if (opts.onAction) opts.onAction(act, val, z, lbl); }
    });

    function applyPeel(nextPct, isScrub) {
      peelPct = Math.max(0, Math.min(100, Number(nextPct) || 0));
      if (stackEl) stackEl.classList.toggle('scrubbing', Boolean(isScrub));
      const clips = computePeelClipPolygons(peelPct);
      if (overEl) { overEl.style.clipPath = clips.overClip; overEl.style.pointerEvents = peelPct >= 85 ? 'none' : 'auto'; }
      if (flapEl) { flapEl.style.clipPath = clips.flapClip; flapEl.style.opacity = String(clips.flapOpacity); }
      if (rangeEl && Number(rangeEl.value) !== Math.round(peelPct)) rangeEl.value = String(Math.round(peelPct));
      if (btnEl) {
        btnEl.classList.toggle('peeled', peelPct >= 50);
        btnEl.textContent = peelPct >= 85 ? '🩹 Re-Apply Stencil' : (peelPct > 10 ? '🩹 Peel Fully (100%)' : '🩹 Peel Back Stencil');
      }
      if (lblEl) lblEl.textContent = peelPct === 0 ? 'Stencil On (0%)' : (peelPct >= 98 ? `Bare ${physicalModel.id}` : `Peeled ${Math.round(peelPct)}%`);
    }
    const cyclePeel = () => applyPeel(peelPct < 40 ? 52 : (peelPct < 92 ? 100 : 0), false);
    if (btnEl) btnEl.onclick = cyclePeel;
    if (cornerEl) cornerEl.onclick = (e) => { e.stopPropagation(); cyclePeel(); };
    if (rangeEl) { rangeEl.oninput = (e) => applyPeel(e.target.value, true); rangeEl.onchange = (e) => applyPeel(e.target.value, false); }
    applyPeel(peelPct, false);

    return {
      underCtrl, overCtrl,
      setPeel: (p) => applyPeel(p, false),
      pulseAction: (action, zone, isRf, val, sourceLayer) => {
        if (sourceLayer === 'physical') {
          if (underCtrl && underCtrl.pulseAction) underCtrl.pulseAction(action, zone, isRf, val);
          const conv = resolvePhysicalToCustomAction(action, val ?? -1, zone, opts.touchZones || (overlayModel && overlayModel.touchZones), opts.calibration);
          if (overCtrl && overCtrl.pulseAction) overCtrl.pulseAction(conv.converted ? conv.action : action, zone, isRf, conv.converted ? conv.value : val);
        } else {
          if (overCtrl && overCtrl.pulseAction) overCtrl.pulseAction(action, zone, isRf, val);
          const rev = resolveCustomToPhysicalAction(action, val, zone, opts.touchZones || (overlayModel && overlayModel.touchZones), opts.physicalCalibration || opts.calibration);
          if (underCtrl && underCtrl.pulseAction) underCtrl.pulseAction(rev.action, zone, isRf, rev.value);
        }
      }
    };
  }


  function submitPostDownloadForm(actionUrl, safeName, text) {
    let ifr = document.getElementById('mlrs_hidden_dl_iframe');
    if (!ifr) {
      ifr = document.createElement('iframe');
      ifr.id = 'mlrs_hidden_dl_iframe';
      ifr.name = 'mlrs_hidden_dl_iframe';
      ifr.style.display = 'none';
      document.body.appendChild(ifr);
    }
    const form = document.createElement('form');
    form.method = 'POST';
    form.action = actionUrl;
    form.target = 'mlrs_hidden_dl_iframe';
    form.enctype = 'application/x-www-form-urlencoded';
    form.style.display = 'none';
    const fnInput = document.createElement('input');
    fnInput.type = 'hidden'; fnInput.name = 'filename'; fnInput.value = safeName;
    const ctInput = document.createElement('input');
    ctInput.type = 'hidden'; ctInput.name = 'content'; ctInput.value = text;
    form.appendChild(fnInput);
    form.appendChild(ctInput);
    document.body.appendChild(form);
    form.submit();
    setTimeout(() => form.remove(), 5000);
  }

  async function downloadTextFile(filename, content, mimeType, opts) {
    const safeName = String(filename || 'stencil_1to1.svg').replace(/[^a-zA-Z0-9._-]/g, '_');
    const mime = mimeType || 'image/svg+xml';
    const text = String(content || '');
    if (root.__MLRS_STAGE_DOWNLOAD) {
      try {
        const dlUrl = await root.__MLRS_STAGE_DOWNLOAD(safeName, text, mime);
        if (dlUrl) {
          const iframe = document.createElement('iframe');
          iframe.style.display = 'none';
          iframe.src = dlUrl;
          document.body.appendChild(iframe);
          setTimeout(() => iframe.remove(), 60000);
          return true;
        }
      } catch (_) {}
    }
    const postEndpoint = (opts && opts.downloadEndpoint)
      || (opts && opts.hubBase ? `${String(opts.hubBase).replace(/\/+$/, '')}/api/remotes/download` : '/api/remotes/download');
    if (postEndpoint && !root.__MLRS_STAGE_DOWNLOAD) {
      submitPostDownloadForm(postEndpoint, safeName, text);
      return true;
    }
    if (typeof window !== 'undefined' && typeof window.showSaveFilePicker === 'function') {
      try {
        const ext = safeName.endsWith('.json') ? '.json' : '.svg';
        const handle = await window.showSaveFilePicker({
          suggestedName: safeName,
          types: [{ description: ext === '.json' ? 'POD JSON' : '1:1 Vector SVG', accept: { [mime]: [ext] } }]
        });
        const writable = await handle.createWritable();
        await writable.write(text);
        await writable.close();
        return true;
      } catch (err) {
        if (err && err.name === 'AbortError') return false;
      }
    }
    try {
      const b64 = btoa(unescape(encodeURIComponent(text)));
      const a = document.createElement('a');
      a.href = `data:application/octet-stream;base64,${b64}`;
      a.setAttribute('download', safeName);
      a.style.display = 'none';
      document.body.appendChild(a);
      a.click();
      setTimeout(() => a.remove(), 4000);
      return true;
    } catch (_) {
      const blob = new Blob([text], { type: 'application/octet-stream' });
      const bUrl = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = bUrl;
      a.setAttribute('download', safeName);
      document.body.appendChild(a);
      a.click();
      setTimeout(() => { a.remove(); URL.revokeObjectURL(bUrl); }, 120000);
      return true;
    }
  }

  function mountShowcaseTour(containerEl, opts) {
    injectStudioStyles(containerEl);
    if (!containerEl || !opts || !Array.isArray(opts.steps) || !opts.steps.length) return null;
    const storageKey = `mlrs_tour_${opts.tourId || 'default'}`;
    let idx = Number(localStorage.getItem(storageKey + '_idx') || 0);
    if (idx < 0 || idx >= opts.steps.length) idx = 0;
    let hidden = localStorage.getItem(storageKey + '_hide') === '1';

    function render() {
      if (hidden) {
        containerEl.innerHTML = `<div style="display:flex;justify-content:flex-end;margin-bottom:8px"><button type="button" class="mlrs-tour-reopen" data-tour="open">💡 Guided Showcase Tour (${opts.steps.length} Interactive Tips)</button></div>`;
        const ob = containerEl.querySelector('[data-tour="open"]');
        if (ob) ob.onclick = () => { hidden = false; localStorage.setItem(storageKey + '_hide', '0'); render(); };
        return;
      }
      const step = opts.steps[idx];
      const dotsHtml = opts.steps.map((_, di) => `<span class="mlrs-tour-dot ${di === idx ? 'on' : ''}"></span>`).join('');
      containerEl.innerHTML = `
        <div class="mlrs-tour-bar">
          <div class="mlrs-tour-left">
            <div class="mlrs-tour-icon">💡</div>
            <div style="min-width:0">
              <div>
                <span class="mlrs-tour-step-badge">Guided Tip ${idx + 1} of ${opts.steps.length}</span>
                <strong style="font-size:13px">${step.title}</strong>
                <span class="mlrs-tour-dots">${dotsHtml}</span>
              </div>
              <div style="margin-top:2px;opacity:0.88;font-size:12px">${step.body}</div>
            </div>
          </div>
          <div class="mlrs-tour-nav">
            <button type="button" class="mlrs-tour-btn" data-tour="prev">◀ Prev</button>
            ${step.actionLabel ? `<button type="button" class="mlrs-tour-btn demo" data-tour="demo">✨ ${step.actionLabel}</button>` : ''}
            <button type="button" class="mlrs-tour-btn" data-tour="next">Next ▶</button>
            <button type="button" class="mlrs-tour-btn" data-tour="hide" title="Minimize Tour Frame">✕ Hide</button>
          </div>
        </div>
      `;
      containerEl.querySelector('[data-tour="prev"]').onclick = () => { idx = (idx - 1 + opts.steps.length) % opts.steps.length; localStorage.setItem(storageKey + '_idx', String(idx)); render(); };
      containerEl.querySelector('[data-tour="next"]').onclick = () => { idx = (idx + 1) % opts.steps.length; localStorage.setItem(storageKey + '_idx', String(idx)); render(); };
      containerEl.querySelector('[data-tour="hide"]').onclick = () => { hidden = true; localStorage.setItem(storageKey + '_hide', '1'); render(); };
      const db = containerEl.querySelector('[data-tour="demo"]');
      if (db && typeof step.onAction === 'function') db.onclick = () => step.onAction();
    }
    render();
    return { show: () => { hidden = false; localStorage.setItem(storageKey + '_hide', '0'); render(); } };
  }

  root.MiLightRemoteStudio = {
    version: '2.5.0',
    normalizeTouchZones, defaultCalibration, resolvePhysicalToCustomAction, resolveCustomToPhysicalAction, mountInteractiveStage, mountPeelableStage, downloadTextFile, mountShowcaseTour,
    buildOneToOnePrintSvg, openPrintSheetWindow, openFaciaDesigner
  };
})(typeof window !== 'undefined' ? window : globalThis);
