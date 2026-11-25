// Hardware-Accurate MiBoxer / Mi-Light SVG Catalog (Shared by ESP32 Hub /remotes-ui & Homebridge v2.0)
(function(root){
  const MODELS = [
    // ================= 1. PANEL REMOTES (White & Black Editions) =================
    {id:'B8',    base:'B8',    finish:'white', cat:'panel', proto:'fut089',  zones:8, name:'B8 Smart Panel (White)',   sub:'8-Zone RGB+CCT Glass Panel', layout:'b8'},
    {id:'B8-B',  base:'B8',    finish:'black', cat:'panel', proto:'fut089',  zones:8, name:'B8-B Smart Panel (Black)', sub:'8-Zone RGB+CCT Glass Panel', layout:'b8'},
    {id:'B4',    base:'B4',    finish:'white', cat:'panel', proto:'rgb_cct', zones:4, name:'B4 / T4 Panel (White)',    sub:'4-Zone RGB+CCT Strip Panel', layout:'b_strip', kind:'b4'},
    {id:'B4-B',  base:'B4',    finish:'black', cat:'panel', proto:'rgb_cct', zones:4, name:'B4-B / T4 Panel (Black)',  sub:'4-Zone RGB+CCT Strip Panel', layout:'b_strip', kind:'b4'},
    {id:'B3',    base:'B3',    finish:'white', cat:'panel', proto:'rgbw',    zones:4, name:'B3 / T3 Panel (White)',    sub:'4-Zone RGBW Strip Panel',    layout:'b_strip', kind:'b3'},
    {id:'B3-B',  base:'B3',    finish:'black', cat:'panel', proto:'rgbw',    zones:4, name:'B3-B / T3 Panel (Black)',  sub:'4-Zone RGBW Strip Panel',    layout:'b_strip', kind:'b3'},
    {id:'B2',    base:'B2',    finish:'white', cat:'panel', proto:'fut091',  zones:4, name:'B2 / T2 Panel (White)',    sub:'4-Zone CCT Strip Panel',     layout:'b_strip', kind:'b2'},
    {id:'B2-B',  base:'B2',    finish:'black', cat:'panel', proto:'fut091',  zones:4, name:'B2-B / T2 Panel (Black)',  sub:'4-Zone CCT Strip Panel',     layout:'b_strip', kind:'b2'},
    {id:'B1',    base:'B1',    finish:'white', cat:'panel', proto:'cct',     zones:4, name:'B1 / T1 Panel (White)',    sub:'4-Zone Brightness Panel',    layout:'b_strip', kind:'b1'},
    {id:'B1-B',  base:'B1',    finish:'black', cat:'panel', proto:'cct',     zones:4, name:'B1-B / T1 Panel (Black)',  sub:'4-Zone Brightness Panel',    layout:'b_strip', kind:'b1'},
    {id:'B0',    base:'B0',    finish:'white', cat:'panel', proto:'rgb_cct', zones:1, name:'B0 Panel (White)',         sub:'1-Zone RGB+CCT Wheel Panel', layout:'b0'},
    {id:'B0-B',  base:'B0',    finish:'black', cat:'panel', proto:'rgb_cct', zones:1, name:'B0-B Panel (Black)',       sub:'1-Zone RGB+CCT Wheel Panel', layout:'b0'},
    {id:'B5',    base:'B5',    finish:'white', cat:'panel', proto:'cct',     zones:4, name:'B5 4-Bar Panel',           sub:'4-Zone Brightness + S1..S4', layout:'b5'},
    {id:'U3',    base:'U3',    finish:'white', cat:'panel', proto:'rgb_cct', zones:3, name:'U3 US Panel (RGB+CCT)',    sub:'3-Zone US Portrait Glass',   layout:'u_panel', kind:'u3'},
    {id:'U2',    base:'U2',    finish:'white', cat:'panel', proto:'fut091',  zones:3, name:'U2 US Panel (CCT)',        sub:'3-Zone US Portrait Glass',   layout:'u_panel', kind:'u2'},
    {id:'U1',    base:'U1',    finish:'white', cat:'panel', proto:'cct',     zones:3, name:'U1 US Panel (Dimmer)',     sub:'3-Zone US Portrait Glass',   layout:'u_panel', kind:'u1'},
    {id:'M4',    base:'M4',    finish:'white', cat:'panel', proto:'rgb_cct', zones:4, name:'M4 Italian Panel',         sub:'4-Zone Landscape 7-Column',  layout:'m4'},
    {id:'K4',    base:'K4',    finish:'white', cat:'panel', proto:'cct',     zones:4, name:'K4 4-Gang Dimmer',         sub:'4-Quadrant Tactile Panel',   layout:'k4'},
    {id:'ESR2T', base:'ESR2T', finish:'white', cat:'panel', proto:'cct',     zones:2, name:'ESR2T 2-Gang Switch',      sub:'2-Gang Blue-Ring Panel',     layout:'esr2t'},
    {id:'PUSH3', base:'PUSH3', finish:'white', cat:'panel', proto:'fut020',  zones:1, name:'PUSH3 Rocker Panel',       sub:'1-Gang Push Dimmer Switch',  layout:'push3'},

    // ================= 2. 2.4G RGB+CCT REMOTES =================
    {id:'FUT100',   base:'FUT100',   finish:'white', cat:'rgbcct', proto:'fut100',  zones:100, name:'FUT100 100-Zone LCD',   sub:'LCD + 3 Sliders + Dual-Bank', layout:'fut100'},
    {id:'FUT089',   base:'FUT089',   finish:'white', cat:'rgbcct', proto:'fut089',  zones:8,   name:'FUT089 (White)',        sub:'8-Zone RGB+CCT Handheld',     layout:'fut089', sixZone:false},
    {id:'FUT089-B', base:'FUT089',   finish:'black', cat:'rgbcct', proto:'fut089',  zones:8,   name:'FUT089-B (Black)',      sub:'8-Zone RGB+CCT Handheld',     layout:'fut089', sixZone:false},
    {id:'FUT089S',  base:'FUT089S',  finish:'white', cat:'rgbcct', proto:'fut089',  zones:6,   name:'FUT089S 6-Zone+Scenes', sub:'6-Zone + S1..S4 Handheld',    layout:'fut089', sixZone:true},
    {id:'FUT092',   base:'FUT092',   finish:'white', cat:'rgbcct', proto:'rgb_cct', zones:4,   name:'FUT092 (White/Lime)',   sub:'4-Zone RGB+CCT Pebble',       layout:'fut092'},
    {id:'FUT092-B', base:'FUT092',   finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:4,   name:'FUT092-B (Black)',      sub:'4-Zone RGB+CCT Pebble',       layout:'fut092'},
    {id:'FUT088',   base:'FUT088',   finish:'white', cat:'rgbcct', proto:'rgb_cct', zones:1,   name:'FUT088 Full-Touch',     sub:'1-Zone RGB+CCT 3x3 Yellow',   layout:'fut088'},
    {id:'FUT096',   base:'FUT096',   finish:'white', cat:'rgbcct', proto:'rgbw',    zones:4,   name:'FUT096 (White/Yellow)', sub:'4-Zone Touch RGBW Pebble',    layout:'fut_pebble', kind:'fut096'},
    {id:'FUT096-B', base:'FUT096',   finish:'black', cat:'rgbcct', proto:'rgbw',    zones:4,   name:'FUT096-B (Black)',      sub:'4-Zone Touch RGBW Pebble',    layout:'fut_pebble', kind:'fut096'},
    {id:'FUT095',   base:'FUT095',   finish:'white', cat:'rgbcct', proto:'rgbw',    zones:4,   name:'FUT095 Button RGBW',    sub:'4-Zone Black-Key RGBW',       layout:'fut_pebble', kind:'fut095'},
    {id:'C8',       base:'C8',       finish:'black', cat:'rgbcct', proto:'fut089',  zones:8,   name:'C8 8-Zone Matte Black', sub:'8-Zone RGB+CCT Modern',       layout:'c_wheel', zones:8},
    {id:'C5',       base:'C5',       finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:4,   name:'C5 4-Zone Matte Black', sub:'4-Zone RGB+CCT Modern',       layout:'c_wheel', zones:4},
    {id:'C4',       base:'C4',       finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:1,   name:'C4 Slot RGB+CCT',       sub:'1-Zone Vertical Touch Bar',   layout:'c_slot',  kind:'rgb'},
    {id:'K6',       base:'K6',       finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:1,   name:'K6 RGB+CCT LED Knob',   sub:'0-330° Rainbow Rotary Display',layout:'k_display', kind:'k6'},
    {id:'Y3',       base:'Y3',       finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:3,   name:'Y3 Desk RGB+CCT',       sub:'3-Zone Vertical Stand Remote',layout:'y_desk',  kind:'y3'},
    {id:'S2-W',     base:'S2',       finish:'white', cat:'rgbcct', proto:'rgb_cct', zones:1,   name:'S2-W Rainbow Puck',     sub:'1-Zone RGB+CCT Touch Puck',   layout:'s_puck',  kind:'s2', multi:false},
    {id:'S2-B',     base:'S2',       finish:'black', cat:'rgbcct', proto:'rgb_cct', zones:1,   name:'S2-B Rainbow (Black)',  sub:'1-Zone RGB+CCT Touch Puck',   layout:'s_puck',  kind:'s2', multi:false},
    {id:'S2W+',     base:'S2W+',     finish:'white', cat:'rgbcct', proto:'rgb_cct', zones:4,   name:'S2W+ 4-Zone Puck',      sub:'4-Zone RGB+CCT Rainbow Puck', layout:'s_puck',  kind:'s2', multi:true},

    // ================= 3. CCT & BRIGHTNESS REMOTES =================
    {id:'FUT094',   base:'FUT094',   finish:'white', cat:'cct', proto:'fut091', zones:4, name:'FUT094 4-Zone CCT',       sub:'CCT Arc + S1..S3 Pebble',     layout:'fut_pebble', kind:'fut094'},
    {id:'FUT007',   base:'FUT007',   finish:'white', cat:'cct', proto:'cct',    zones:4, name:'FUT007 4-Dot CCT',        sub:'4-Zone Classic Cross-Ring',   layout:'fut_classic', kind:'fut007'},
    {id:'FUT006',   base:'FUT006',   finish:'white', cat:'cct', proto:'cct',    zones:4, name:'FUT006 Wheel CCT',        sub:'4-Zone Ridged Wheel Remote',  layout:'fut_classic', kind:'fut006'},
    {id:'FUT087',   base:'FUT087',   finish:'white', cat:'cct', proto:'fut020', zones:1, name:'FUT087 Dimmer (White)',   sub:'Touch Dimming Puck',          layout:'fut087'},
    {id:'FUT087-B', base:'FUT087',   finish:'black', cat:'cct', proto:'fut020', zones:1, name:'FUT087-B Dimmer (Black)', sub:'Touch Dimming Puck',          layout:'fut087'},
    {id:'S1-W',     base:'S1',       finish:'white', cat:'cct', proto:'fut091', zones:1, name:'S1-W Sunrise (White)',    sub:'1-Zone CCT Touch Puck',       layout:'s_puck', kind:'s1', multi:false},
    {id:'S1-B',     base:'S1',       finish:'black', cat:'cct', proto:'fut091', zones:1, name:'S1-B Sunrise (Black)',    sub:'1-Zone CCT Touch Puck',       layout:'s_puck', kind:'s1', multi:false},
    {id:'S1W+',     base:'S1W+',     finish:'white', cat:'cct', proto:'fut091', zones:4, name:'S1W+ 4-Zone Sunrise',     sub:'4-Zone CCT Touch Puck',       layout:'s_puck', kind:'s1', multi:true},
    {id:'C1',       base:'C1',       finish:'black', cat:'cct', proto:'fut091', zones:4, name:'C1 4-Zone Diamond CCT',   sub:'5-Diamond Pad + 4 Bars',      layout:'c1'},
    {id:'C12',      base:'C12',      finish:'black', cat:'cct', proto:'fut091', zones:4, name:'C12 Scroll-Wheel LED',    sub:'100% Display + Thumb Wheel',  layout:'c12'},
    {id:'C3',       base:'C3',       finish:'black', cat:'cct', proto:'fut091', zones:1, name:'C3 Slot CCT Remote',      sub:'1-Zone Vertical CCT Slot',    layout:'c_slot', kind:'cct'},
    {id:'C3P',      base:'C3P',      finish:'black', cat:'cct', proto:'fut020', zones:1, name:'C3P Slot Dimmer',         sub:'1-Zone 12-Dot Dimmer Slot',   layout:'c_slot', kind:'dim'},
    {id:'K5',       base:'K5',       finish:'black', cat:'cct', proto:'fut091', zones:1, name:'K5 CCT LED Knob',         sub:'100% Display + S1..S4 Knob',  layout:'k_display', kind:'k5'},
    {id:'K0S',      base:'K0S',      finish:'black', cat:'cct', proto:'fut091', zones:4, name:'K0S 4-Zone Knob Panel',   sub:'4-Corner Zone + Center Knob', layout:'k0', zonesMode:true},
    {id:'K0',       base:'K0',       finish:'black', cat:'cct', proto:'fut091', zones:1, name:'K0 4-Scene Knob Panel',   sub:'4-Corner Scene + Center Knob',layout:'k0', zonesMode:false},
    {id:'K3',       base:'K3',       finish:'black', cat:'cct', proto:'fut020', zones:1, name:'K3 / K3P Wheel Panel',    sub:'Square Ridged Wheel Dimmer',  layout:'k3', square:true},
    {id:'K3S',      base:'K3S',      finish:'black', cat:'cct', proto:'fut020', zones:1, name:'K3S Round Wheel Puck',    sub:'Round Ridged Wheel Dimmer',   layout:'k3', square:false},
    {id:'K2',       base:'K2',       finish:'white', cat:'cct', proto:'fut091', zones:1, name:'K2 Wall Mini (White)',    sub:'Magnetic Wall 5-Way CCT',     layout:'k2', plate:true},
    {id:'K2-B',     base:'K2',       finish:'black', cat:'cct', proto:'fut091', zones:1, name:'K2-B Wall Mini (Black)',  sub:'Magnetic Wall 5-Way CCT',     layout:'k2', plate:true},
    {id:'K2S',      base:'K2S',      finish:'white', cat:'cct', proto:'fut091', zones:1, name:'K2S Mini Puck (White)',   sub:'Round 5-Way CCT Mini Puck',   layout:'k2', plate:false},
    {id:'K2S-B',    base:'K2S',      finish:'black', cat:'cct', proto:'fut091', zones:1, name:'K2S-B Mini Puck (Black)', sub:'Round 5-Way CCT Mini Puck',   layout:'k2', plate:false},
    {id:'K1',       base:'K1',       finish:'white', cat:'cct', proto:'fut091', zones:1, name:'K1 Minimalist Knob',      sub:'Square Push-Rotary Panel',    layout:'k1'},
    {id:'Y2',       base:'Y2',       finish:'black', cat:'cct', proto:'fut091', zones:3, name:'Y2 Desk CCT Remote',      sub:'3-Zone Dual-Bar Stand',       layout:'y_desk', kind:'y2'},
    {id:'Y1',       base:'Y1',       finish:'black', cat:'cct', proto:'cct',    zones:3, name:'Y1 Desk Brightness',      sub:'3-Zone Single-Bar Stand',     layout:'y_desk', kind:'y1'}
  ];

  function pal(finish){
    const b = finish === 'black';
    return {
      b,
      body:   b ? '#1e2229' : '#f7f9fb',
      bezel:  b ? '#384152' : '#d0d7e2',
      glass:  b ? '#161920' : '#f2f5f8',
      key:    b ? '#252b36' : '#eef2f6',
      keyStr: b ? '#525e73' : '#94a3b8',
      txt:    b ? '#e2e8f0' : '#475569',
      mut:    b ? '#94a3b8' : '#7c8ba1',
      slot:   b ? '#12151b' : '#e2e8f0'
    };
  }

  function sharedDefs(u){
    return `<defs>
      <linearGradient id="cct_${u}" x1="0" y1="0" x2="1" y2="0"><stop offset="0%" stop-color="#facc15"/><stop offset="52%" stop-color="#fef9c3"/><stop offset="100%" stop-color="#38bdf8"/></linearGradient>
      <linearGradient id="vcct_${u}" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#38bdf8"/><stop offset="50%" stop-color="#f8fafc"/><stop offset="100%" stop-color="#facc15"/></linearGradient>
      <linearGradient id="vrgb_${u}" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#ef4444"/><stop offset="25%" stop-color="#3b82f6"/><stop offset="55%" stop-color="#22c55e"/><stop offset="80%" stop-color="#eab308"/><stop offset="100%" stop-color="#ef4444"/></linearGradient>
      <linearGradient id="hrgb_${u}" x1="0" y1="0" x2="1" y2="0"><stop offset="0%" stop-color="#ef4444"/><stop offset="20%" stop-color="#eab308"/><stop offset="42%" stop-color="#22c55e"/><stop offset="65%" stop-color="#06b6d4"/><stop offset="84%" stop-color="#2563eb"/><stop offset="100%" stop-color="#dc2626"/></linearGradient>
      <linearGradient id="dual_top_${u}" x1="0" y1="0" x2="1" y2="0"><stop offset="0%" stop-color="#facc15"/><stop offset="55%" stop-color="#f8fafc"/><stop offset="100%" stop-color="#7dd3fc"/></linearGradient>
      <linearGradient id="dual_bot_${u}" x1="0" y1="0" x2="1" y2="0"><stop offset="0%" stop-color="#b91c1c"/><stop offset="50%" stop-color="#ef4444"/><stop offset="100%" stop-color="#fee2e2"/></linearGradient>
      <linearGradient id="hsatfade_${u}" x1="0" y1="0" x2="1" y2="0"><stop offset="0%" stop-color="#ffffff" stop-opacity="0"/><stop offset="55%" stop-color="#ffffff" stop-opacity="0.35"/><stop offset="100%" stop-color="#ffffff" stop-opacity="0.96"/></linearGradient>
      <linearGradient id="vsat_rgb_${u}" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="#eab308"/><stop offset="28%" stop-color="#ef4444"/><stop offset="68%" stop-color="#16a34a"/><stop offset="100%" stop-color="#064e3b"/></linearGradient>
    <radialGradient id="b8led_${u}" cx="50%" cy="50%" r="50%">
        <stop offset="0%" stop-color="#ef4444" stop-opacity="0.72"/>
        <stop offset="65%" stop-color="#ef4444" stop-opacity="0.32"/>
        <stop offset="100%" stop-color="#ef4444" stop-opacity="0"/>
      </radialGradient>
    </defs>`;
  }

  // Segmented horizontal bar (used on B1..B4, T1..T4, U1..U3, M4)
  function segBarH(x, y, w, h, n, mode, act){
    const step = w / n, bw = Math.max(1.8, step * 0.62);
    const rCols = ['#dc2626','#be185d','#9333ea','#6d28d9','#4338ca','#1d4ed8','#0284c7','#0891b2','#0d9488','#16a34a','#65a30d','#ca8a04','#eab308','#f59e0b','#ea580c','#dc2626'];
    let out = `<g class="mbx-hit" data-act="${act}" style="cursor:pointer">`;
    for(let i=0; i<n; i++){
      const t = i / (n - 1);
      let c = '#64748b';
      if(mode === 'rgb') c = rCols[Math.floor(t * (rCols.length - 1))];
      else if(mode === 'cct') c = t < 0.5 ? `rgb(250,${Math.round(204+t*96)},${Math.round(21+t*420)})` : `rgb(${Math.round(250-(t-0.5)*390)},${Math.round(252-(t-0.5)*130)},248)`;
      else if(mode === 'sat') c = `rgb(239,${Math.round(35+t*185)},${Math.round(35+t*190)})`;
      else { const v = Math.round(38 + t * 195); c = `rgb(${v},${v},${v})`; }
      out += `<rect x="${(x + i*step).toFixed(1)}" y="${y}" width="${bw.toFixed(1)}" height="${h}" fill="${c}"/>`;
    }
    return out + `</g>`;
  }

  // Smooth 36-segment color wheel matching MiBoxer photo orientation (purple/blue top, green right, yellow bottom, red left)
  function colorRing(cx, cy, r, sw){
    const cols = ['#ef4444','#dc2626','#be123c','#9d174d','#86198f','#6b21a8','#5b21b6','#4338ca','#1d4ed8','#0284c7','#0891b2','#06b6d4','#0d9488','#10b981','#16a34a','#22c55e','#65a30d','#84cc16','#eab308','#facc15','#f59e0b','#f97316','#ea580c','#ef4444'];
    const n = cols.length, circ = 2 * Math.PI * r, seg = circ / n;
    // Offset so index 0 (red) starts at ~7:30 o'clock going clockwise through purple(top)->blue->green(right)->yellow(bottom)
    const rot = 145;
    return `<g transform="rotate(${rot} ${cx} ${cy})">` + cols.map((c,i) =>
      `<circle class="mbx-hit" data-act="hue" data-val="${Math.round((i/n)*360)}" cx="${cx}" cy="${cy}" r="${r}" fill="none" stroke="${c}" stroke-width="${sw}" stroke-dasharray="${(seg+0.9).toFixed(2)} ${circ.toFixed(2)}" stroke-dashoffset="${(-i*seg).toFixed(2)}" style="cursor:pointer"/>`
    ).join('') + `</g>`;
  }

  // Blue brightness pill with graduated dots + sun icon (B8, B0, FUT089, FUT100, FUT088, FUT092)
  function blueBriPillH(x, y, w, h){
    let dots = '';
    for(let i=0; i<8; i++){
      const r = 0.8 + i*0.22;
      dots += `<circle cx="${(x + 12 + i*((w-30)/7)).toFixed(1)}" cy="${y + h/2}" r="${r.toFixed(2)}" fill="#fff"/>`;
    }
    return `<g class="mbx-hit" data-act="brightness" data-val="85" style="cursor:pointer">
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${h/2}" fill="#0ea5e9"/>
      ${dots}
      <text x="${x + w - 9}" y="${y + h/2 + 3.2}" text-anchor="middle" fill="#fff" font-size="9">☀</text>
    </g>`;
  }

  function vSatPill2D(x, y, w, h, u, strokeCol){
    const cid = `vsat_${u}_${Math.round(x)}_${Math.round(y)}`;
    return `<g class="mbx-hit" data-act="saturation" data-val="50" style="cursor:pointer">
      <clipPath id="${cid}"><rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${w/2}"/></clipPath>
      <g clip-path="url(#${cid})">
        <rect x="${x}" y="${y}" width="${w}" height="${h}" fill="url(#vsat_rgb_${u})"/>
        <rect x="${x}" y="${y}" width="${w}" height="${h}" fill="url(#hsatfade_${u})"/>
      </g>
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${w/2}" fill="none" stroke="${strokeCol}" stroke-width="0.7"/>
    </g>`;
  }

  function crescentKey(cx, cy, w, h, dip, fill, str, act, innerSvg, zoneAttr){
    // Upward-smiling crescent button matching FUT092_EN_V1.0.pdf
    const xL = cx - w/2, xR = cx + w/2;
    const d = `M ${xL} ${cy - h*0.35} Q ${cx} ${cy - h*0.35 + dip} ${xR} ${cy - h*0.35} C ${xR + 1.5} ${cy + h*0.25}, ${cx + w*0.28} ${cy + h*0.68}, ${cx} ${cy + h*0.68} C ${cx - w*0.28} ${cy + h*0.68}, ${xL - 1.5} ${cy + h*0.25}, ${xL} ${cy - h*0.35} Z`;
    return `<g class="mbx-hit" data-act="${act}" ${zoneAttr!==undefined?`data-zone="${zoneAttr}"`:''} style="cursor:pointer">
      <path d="${d}" fill="${fill}" stroke="${str}" stroke-width="0.8" stroke-linejoin="round"/>
      ${innerSvg}
    </g>`;
  }

  function sceneSlotPair(x, y, w, h, n1, n2, p, tc){
    const c = tc || '#0284c7';
    const tray = (tx, num) => `
      <path d="M ${tx-5.5} ${y+h-5.8} L ${tx-5.5} ${y+h-3.2} L ${tx+5.5} ${y+h-3.2} L ${tx+5.5} ${y+h-5.8}" fill="none" stroke="${c}" stroke-width="1"/>
      <text x="${tx}" y="${y+h-5}" text-anchor="middle" fill="${c}" font-size="6.2" font-weight="700">${num}</text>`;
    return `<g class="mbx-hit" data-act="mode" style="cursor:pointer">
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
      ${tray(x + 9.5, n1)}
      ${n2 === 'M'
        ? `<text x="${x + w - 9.5}" y="${y + h/2 + 2.5}" text-anchor="middle" fill="${c}" font-size="7" font-weight="700">M</text>`
        : tray(x + w - 9.5, n2)}
    </g>`;
  }

  function blueBriPillV(x, y, w, h){
    let dots = '';
    for(let i=0; i<6; i++){
      const r = 2.1 - i*0.2;
      dots += `<circle cx="${x + w/2}" cy="${(y + 18 + i*((h-36)/5)).toFixed(1)}" r="${r.toFixed(2)}" fill="#fff"/>`;
    }
    return `<g class="mbx-hit" data-act="brightness" data-val="85" style="cursor:pointer">
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${w/2}" fill="#0ea5e9"/>
      <text x="${x + w/2}" y="${y + 11}" text-anchor="middle" fill="#fff" font-size="9">☀</text>
      ${dots}
      <text x="${x + w/2}" y="${y + h - 5}" text-anchor="middle" fill="#fff" font-size="6.5">☀</text>
    </g>`;
  }

  // Dual-layer CCT + RGB/Sat horizontal pill (FUT089, FUT089S, FUT100, FUT088)
  function dualCctSatPillH(x, y, w, h, u){
    const cid = `clip_${u}_${Math.round(x)}_${Math.round(y)}`;
    return `<g class="mbx-hit" data-act="kelvin" data-val="50" style="cursor:pointer">
      <clipPath id="${cid}"><rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${h/2}"/></clipPath>
      <g clip-path="url(#${cid})">
        <rect x="${x}" y="${y}" width="${w}" height="${h*0.52}" fill="url(#dual_top_${u})"/>
        <rect x="${x}" y="${y + h*0.48}" width="${w}" height="${h*0.52}" fill="url(#hrgb_${u})"/>
      </g>
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${h/2}" fill="none" stroke="#cbd5e1" stroke-width="0.6"/>
    </g>`;
  }

  // Vertical zone rocker [ | / label / O ]
  function vZoneRocker(x, y, w, h, z, label, hi, p, fillOverride, txtOverride){
    const fill = fillOverride || p.key;
    const tc   = txtOverride || p.txt;
    const str  = hi ? '#0ea5e9' : p.keyStr;
    return `<g>
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${Math.min(6, w/2)}" fill="${fill}" stroke="${str}" stroke-width="${hi?1.8:0.9}"/>
      <line class="mbx-hit" data-act="zone_on" data-zone="${z}" x1="${x+w/2}" y1="${y+6}" x2="${x+w/2}" y2="${y+12}" stroke="${tc}" stroke-width="1.6" stroke-linecap="round" style="cursor:pointer"/>
      <text class="mbx-hit" data-act="zone" data-zone="${z}" x="${x+w/2}" y="${y + h/2 + 3}" text-anchor="middle" fill="${hi?'#0ea5e9':tc}" font-size="8.5" font-weight="700" style="cursor:pointer">${label}</text>
      <circle class="mbx-hit" data-act="zone_off" data-zone="${z}" cx="${x+w/2}" cy="${y+h-9}" r="3.4" fill="none" stroke="${tc}" stroke-width="1.2" style="cursor:pointer"/>
      <rect class="mbx-hit" data-act="zone_on" data-zone="${z}" x="${x}" y="${y}" width="${w}" height="${h*0.36}" fill="transparent" style="cursor:pointer"/>
      <rect class="mbx-hit" data-act="zone_off" data-zone="${z}" x="${x}" y="${y+h*0.64}" width="${w}" height="${h*0.36}" fill="transparent" style="cursor:pointer"/>
    </g>`;
  }

  // Horizontal zone rocker [ |   Z   O ] (FUT089, FUT089S, FUT100, C8, C1)
  function hZoneRocker(x, y, w, h, z, label, hi, p){
    return `<g>
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="3.2" fill="${p.key}" stroke="${hi?'#0ea5e9':p.keyStr}" stroke-width="${hi?1.6:0.8}"/>
      <line x1="${x+7}" y1="${y+h/2-3.2}" x2="${x+7}" y2="${y+h/2+3.2}" stroke="${p.txt}" stroke-width="1.5" stroke-linecap="round"/>
      <text x="${x+w/2}" y="${y+h/2+3}" text-anchor="middle" fill="${hi?'#0ea5e9':p.txt}" font-size="8" font-weight="700">${label}</text>
      <circle cx="${x+w-8}" cy="${y+h/2}" r="2.8" fill="none" stroke="${p.txt}" stroke-width="1.2"/>
      <rect class="mbx-hit" data-act="zone_on" data-zone="${z}" x="${x}" y="${y}" width="${w*0.45}" height="${h}" fill="transparent" style="cursor:pointer"/>
      <rect class="mbx-hit" data-act="zone" data-zone="${z}" x="${x+w*0.45}" y="${y}" width="${w*0.15}" height="${h}" fill="transparent" style="cursor:pointer"/>
      <rect class="mbx-hit" data-act="zone_off" data-zone="${z}" x="${x+w*0.6}" y="${y}" width="${w*0.4}" height="${h}" fill="transparent" style="cursor:pointer"/>
    </g>`;
  }

  function keyBox(x, y, w, h, lbl, act, p, extra){
    extra = extra || {};
    const rx = extra.rx || 4;
    const fill = extra.fill || p.key;
    const tc = extra.tc || p.txt;
    const fs = extra.fs || 8.5;
    const sub = extra.sub ? `<text x="${x+w/2}" y="${y+h-3.2}" text-anchor="middle" fill="${p.mut}" font-size="5">${extra.sub}</text>` : '';
    const ty = extra.sub ? (y + h/2 + 0.5) : (y + h/2 + fs*0.35);
    return `<g class="mbx-hit" data-act="${act}" ${extra.zone!==undefined?`data-zone="${extra.zone}"`:''} style="cursor:pointer">
      <rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${rx}" fill="${fill}" stroke="${p.keyStr}" stroke-width="0.85"/>
      <text x="${x+w/2}" y="${ty}" text-anchor="middle" fill="${tc}" font-size="${fs}" font-weight="600">${lbl}</text>
      ${sub}
    </g>`;
  }

  function renderModelSvg(modelOrId, opts){
    opts = opts || {};
    const m = (typeof modelOrId === 'string') ? (MODELS.find(x => x.id === modelOrId) || MODELS[0]) : modelOrId;
    const p = pal(m.finish);
    const az = (typeof opts.activeZone === 'number') ? opts.activeZone : null;
    const u = m.id.replace(/[^a-zA-Z0-9]/g, '') + '_' + (opts.uid || '0');
    const sc = opts.size === 'thumb' ? 0.45 : 1;

    // ---------------- 1. B8 (106x146mm Portrait Glass: Wheel + 2 V-Bars + 5 Rect Keys + 2x5 Circles) ----------------
    if(m.layout === 'b8'){
      let rectRow = '';
      [['ON','on'],['OFF','off'],['S-','mode_speed_down'],['M','mode'],['S+','mode_speed_up']].forEach(([l,a], i) => {
        rectRow += keyBox(21 + i*30.5, 148, 26.5, 18, l, a, p, {rx:3.5, fs:8});
      });
      let circRows = '';
      const r1 = [['All',0],['1',1],['2',2],['3',3],['4',4]];
      const r2 = [['60S',-1],['5',5],['6',6],['7',7],['8',8]];
      [r1, r2].forEach((row, ri) => {
        row.forEach(([lbl, z], ci) => {
          const cx = 34 + ci*30.5, cy = 186 + ri*31;
          const hi = z >= 0 && az === z;
          const act = z === -1 ? 'night_mode' : 'zone';
          circRows += `<g class="mbx-hit" data-act="${act}" ${z>=0?`data-zone="${z}"`:''} style="cursor:pointer">
            <circle cx="${cx}" cy="${cy}" r="12" fill="${p.key}" stroke="${hi?'#ef4444':p.keyStr}" stroke-width="${hi?1.2:0.9}"/>
            ${hi ? `<circle cx="${cx}" cy="${cy}" r="11" fill="url(#b8led_${u})"/>` : ''}
            <text x="${cx}" y="${z===-1?cy+0.5:cy+3.2}" text-anchor="middle" fill="${p.txt}" font-size="${z===-1?7.5:8.5}" font-weight="${hi?'700':'600'}">${lbl}</text>
            ${z===-1?`<text x="${cx}" y="${cy+6.5}" text-anchor="middle" fill="${p.mut}" font-size="3.8">Delay OFF</text>`:''}
          </g>`;
        });
      });
      return `<svg viewBox="0 0 190 250" width="${Math.round(190*sc)}" height="${Math.round(250*sc)}">
        ${sharedDefs(u)}
        <rect x="4" y="4" width="182" height="242" rx="8" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>
        <rect x="9" y="9" width="172" height="232" rx="5" fill="${p.glass}" stroke="${p.bezel}" stroke-width="0.7"/>
        <text x="22" y="25" fill="${p.mut}" font-size="8.5" font-weight="800">MiBOXER®</text>
        <text x="168" y="24" text-anchor="end" fill="${p.txt}" font-size="6.5" font-weight="800">2.4G RF</text>
        <!-- Left V-Bar + Center Wheel (with W) + Right Blue Sun V-Bar -->
        ${vSatPill2D(25, 42, 10.5, 86, u, p.keyStr)}
        ${colorRing(95, 85, 36, 12)}
        <g class="mbx-hit" data-act="set_white" style="cursor:pointer">
          <circle cx="95" cy="85" r="14" fill="${p.glass}" stroke="${p.keyStr}" stroke-width="0.9"/>
          <text x="95" y="89" text-anchor="middle" fill="${p.txt}" font-size="11" font-weight="600">W</text>
        </g>
        ${blueBriPillV(153, 42, 12, 86)}
        ${rectRow}
        ${circRows}
      </svg>`;
    }

    // ---------------- 2. B1/T1, B2/T2, B3/T3, B4/T4 Square 86mm Strip Panels ----------------
    if(m.layout === 'b_strip'){
      let bars = '', midRow = '', rockers = '';
      if(m.kind === 'b1'){
        bars = segBarH(26, 56, 158, 12, 34, 'bri', 'brightness');
      } else if(m.kind === 'b2'){
        bars = segBarH(26, 44, 158, 11, 34, 'cct', 'kelvin') + segBarH(26, 74, 158, 11, 34, 'bri', 'brightness');
      } else if(m.kind === 'b3'){
        bars = segBarH(26, 44, 158, 11, 34, 'rgb', 'hue') + segBarH(26, 74, 158, 11, 34, 'bri', 'brightness');
      } else {
        // B4 / T4: Top rainbow bar; second row split left (CCT + Sat double row) & right (Brightness)
        bars = segBarH(26, 44, 158, 11, 34, 'rgb', 'hue')
             + segBarH(26, 73, 72, 5, 18, 'cct', 'kelvin')
             + segBarH(26, 79.5, 72, 5, 18, 'sat', 'saturation')
             + segBarH(112, 73, 72, 11.5, 18, 'bri', 'brightness');
      }
      if(m.kind === 'b1' || m.kind === 'b2'){
        midRow = keyBox(26, 102, 42, 22, 'ON', 'on', p)
               + keyBox(84, 102, 42, 22, '60S', 'night_mode', p, {sub:'Delay OFF'})
               + keyBox(142, 102, 42, 22, 'OFF', 'off', p);
        for(let z=1; z<=4; z++) rockers += vZoneRocker(26 + (z-1)*44.5, 138, 24, 48, z, String(z), az===z, p);
      } else {
        [['White','set_white'],['Mode','mode'],['Speed-','mode_speed_down'],['Speed+','mode_speed_up']].forEach(([l,a], i) => {
          midRow += keyBox(26 + i*41.5, 102, 33.5, 18, l, a, p, {fs:7.2});
        });
        [['All',0],['1',1],['2',2],['3',3],['4',4]].forEach(([lbl,z], i) => {
          rockers += vZoneRocker(26 + i*34.5, 136, 20, 48, z, lbl, az===z, p);
        });
      }
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">
        <rect x="6" y="6" width="198" height="198" rx="11" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.2"/>
        <rect x="11" y="11" width="188" height="188" rx="8" fill="${p.glass}" stroke="${p.bezel}" stroke-width="0.7"/>
        <text x="26" y="30" fill="${p.mut}" font-size="9.5" font-weight="800">MiBOXER®</text>
        ${bars}${midRow}${rockers}
      </svg>`;
    }

    // ---------------- 3. B0 / B0-B Square 1-Zone RGB+CCT Wheel Panel ----------------
    if(m.layout === 'b0'){
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">
        ${sharedDefs(u)}
        <rect x="6" y="6" width="198" height="198" rx="11" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.2"/>
        <rect x="11" y="11" width="188" height="188" rx="8" fill="${p.glass}" stroke="${p.bezel}" stroke-width="0.7"/>
        <text x="26" y="30" fill="${p.mut}" font-size="9.5" font-weight="800">MiBOXER®</text>
        ${vSatPill2D(32, 45, 10.5, 78, u, p.keyStr)}
        ${colorRing(105, 84, 34, 11.5)}
        <g class="mbx-hit" data-act="set_white" style="cursor:pointer">
          <circle cx="105" cy="84" r="13.5" fill="${p.glass}" stroke="${p.keyStr}" stroke-width="0.9"/>
          <text x="105" y="88" text-anchor="middle" fill="${p.txt}" font-size="10.5" font-weight="600">W</text>
        </g>
        ${blueBriPillV(166, 45, 11.5, 78)}
        ${keyBox(28, 138, 33, 19, 'ON', 'on', p, {fs:8})}
        ${keyBox(70, 138, 33, 19, 'R', 'hue', p, {fs:8})}
        ${keyBox(112, 138, 33, 19, 'G', 'hue', p, {fs:8})}
        ${keyBox(154, 138, 33, 19, 'B', 'hue', p, {fs:8})}
        ${keyBox(28, 165, 33, 19, 'OFF', 'off', p, {fs:8})}
        ${keyBox(70, 165, 33, 19, 'M', 'mode', p, {fs:8})}
        <g class="mbx-hit" data-act="mode_speed_down" style="cursor:pointer">
          <rect x="112" y="165" width="33" height="19" rx="4" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
          <line x1="114" y1="183" x2="143" y2="166" stroke="${p.keyStr}" stroke-width="0.7"/>
          <text x="121" y="174" fill="${p.txt}" font-size="7" font-weight="600">S-</text>
          <text x="134" y="182" text-anchor="middle" fill="${p.mut}" font-size="5.5">60s</text>
        </g>
        <g class="mbx-hit" data-act="mode_speed_up" style="cursor:pointer">
          <rect x="154" y="165" width="33" height="19" rx="4" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
          <line x1="156" y1="183" x2="185" y2="166" stroke="${p.keyStr}" stroke-width="0.7"/>
          <text x="163" y="174" fill="${p.txt}" font-size="7" font-weight="600">S+</text>
          <text x="175" y="182" text-anchor="middle" fill="${p.mut}" font-size="5">10min</text>
        </g>
      </svg>`;
    }

    // ---------------- 4. B5 (4 Blue Vertical 11-Bar Columns + S1..S4 + 60S) ----------------
    if(m.layout === 'b5'){
      let cols = '', scenes = '';
      for(let z=1; z<=4; z++){
        const x = 30 + (z-1)*33;
        for(let b=0; b<11; b++){
          const op = (0.35 + (b/10)*0.65).toFixed(2);
          cols += `<rect class="mbx-hit" data-act="brightness" data-zone="${z}" x="${x}" y="${42 + b*7.5}" width="16" height="4.5" rx="1.5" fill="#0ea5e9" opacity="${op}" style="cursor:pointer"/>`;
        }
        cols += vZoneRocker(x - 3, 134, 22, 50, z, String(z), az===z, p);
      }
      ['S1','S2','S3','S4'].forEach((s, i) => { scenes += keyBox(160, 42 + i*31, 26, 18, s, 'mode', p, {fs:7.5}); });
      scenes += keyBox(160, 166, 26, 18, '60S', 'night_mode', p, {fs:7, sub:'Delay'});
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">
        <rect x="6" y="6" width="198" height="198" rx="11" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.2"/>
        <rect x="11" y="11" width="188" height="188" rx="8" fill="${p.glass}" stroke="${p.bezel}" stroke-width="0.7"/>
        <text x="26" y="30" fill="${p.mut}" font-size="9.5" font-weight="800">MiBOXER®</text>
        ${cols}${scenes}
      </svg>`;
    }

    // ---------------- 5. U1 / U2 / U3 US Portrait 3-Zone Panels ----------------
    if(m.layout === 'u_panel'){
      let bars = '';
      if(m.kind === 'u1') bars = segBarH(26, 62, 118, 13, 28, 'bri', 'brightness');
      else if(m.kind === 'u2') bars = segBarH(26, 44, 118, 12, 28, 'cct', 'kelvin') + segBarH(26, 74, 118, 12, 28, 'bri', 'brightness');
      else bars = segBarH(26, 44, 118, 12, 28, 'rgb', 'hue') + segBarH(26, 74, 118, 12, 28, 'bri', 'brightness');

      const midLbl = m.kind === 'u3' ? ['W','set_white'] : ['60S','night_mode'];
      const r2 = m.kind === 'u3'
        ? [['S-/◐','mode_speed_down'],['M','mode'],['S+/⊕','mode_speed_up']]
        : [['1␣','mode'],['2␣','mode'],['3␣','mode']];
      let zRow = '';
      for(let z=1; z<=3; z++) zRow += vZoneRocker(28 + (z-1)*44, 164, 26, 54, z, String(z), az===z, p);
      return `<svg viewBox="0 0 170 240" width="${Math.round(170*sc)}" height="${Math.round(240*sc)}">
        <rect x="8" y="6" width="154" height="228" rx="10" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>
        <rect x="13" y="11" width="144" height="218" rx="7" fill="${p.glass}" stroke="${p.bezel}" stroke-width="0.7"/>
        <text x="24" y="29" fill="${p.mut}" font-size="8.5" font-weight="800">MiBOXER®</text>
        ${bars}
        ${keyBox(26, 104, 30, 20, 'ON', 'on', p, {fs:7.5})}
        ${keyBox(70, 104, 30, 20, midLbl[0], midLbl[1], p, {fs:7.5})}
        ${keyBox(114, 104, 30, 20, 'OFF', 'off', p, {fs:7.5})}
        ${keyBox(26, 132, 30, 20, r2[0][0], r2[0][1], p, {fs:7})}
        ${keyBox(70, 132, 30, 20, r2[1][0], r2[1][1], p, {fs:7.5})}
        ${keyBox(114, 132, 30, 20, r2[2][0], r2[2][1], p, {fs:7})}
        ${zRow}
      </svg>`;
    }

    // ---------------- 6. M4 Italian Landscape 7-Column Panel ----------------
    if(m.layout === 'm4'){
      let cols = '';
      // Col 0: ON / All / OFF
      cols += keyBox(20, 98, 22, 68, 'All', 'zone', p, {zone:0, fs:7.5})
            + `<text x="31" y="111" text-anchor="middle" fill="${p.txt}" font-size="6.5" font-weight="700" pointer-events="none">ON</text>
               <text x="31" y="159" text-anchor="middle" fill="${p.txt}" font-size="6.5" font-weight="700" pointer-events="none">OFF</text>`;
      for(let z=1; z<=4; z++) cols += vZoneRocker(20 + z*29, 98, 22, 68, z, String(z), az===z, p);
      // Col 5: W / • / M, Col 6: + / S / -
      cols += keyBox(165, 98, 22, 68, '•', 'set_white', p, {fs:9})
            + `<text x="176" y="112" text-anchor="middle" fill="${p.txt}" font-size="7.5" font-weight="700" pointer-events="none">W</text>
               <text x="176" y="159" text-anchor="middle" fill="${p.txt}" font-size="7.5" font-weight="700" pointer-events="none">M</text>`;
      cols += keyBox(194, 98, 22, 68, 'S', 'mode_speed_up', p, {fs:8})
            + `<text x="205" y="112" text-anchor="middle" fill="${p.txt}" font-size="10" font-weight="700" pointer-events="none">+</text>
               <text x="205" y="159" text-anchor="middle" fill="${p.txt}" font-size="10" font-weight="700" pointer-events="none">−</text>`;
      return `<svg viewBox="0 0 236 184" width="${Math.round(236*sc)}" height="${Math.round(184*sc)}">
        <rect x="6" y="6" width="224" height="172" rx="10" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>
        <text x="20" y="26" fill="${p.mut}" font-size="8.5" font-weight="800">MiBOXER®</text>
        <text x="216" y="25" text-anchor="end" fill="${p.txt}" font-size="6.5" font-weight="800">2.4G RF</text>
        ${segBarH(20, 38, 196, 12, 38, 'rgb', 'hue')}
        ${segBarH(20, 64, 88, 5.5, 18, 'cct', 'kelvin')}
        ${segBarH(20, 71.5, 88, 5.5, 18, 'sat', 'saturation')}
        ${segBarH(126, 64, 90, 13, 18, 'bri', 'brightness')}
        ${cols}
      </svg>`;
    }

    // ---------------- 7. K4 / ESR2T / PUSH3 Minimalist Panels ----------------
    if(m.layout === 'k4'){
      const pips = [
        [[56,36]], [[149,36],[159,36]],
        [[53,168],[59,168],[56,162]], [[149,162],[159,162],[149,170],[159,170]]
      ];
      let quads = '';
      for(let q=0; q<4; q++){
        const x = (q%2)*99 + 6, y = Math.floor(q/2)*99 + 6, cx = x+49.5, cy = y+49.5;
        const dots = pips[q].map(([px,py]) => `<circle cx="${px}" cy="${py}" r="1.6" fill="#94a3b8"/>`).join('');
        quads += `<g class="mbx-hit" data-act="zone_on" data-zone="${q+1}" style="cursor:pointer">
          <rect x="${x}" y="${y}" width="99" height="99" rx="6" fill="#f1f5f9" stroke="#cbd5e1" stroke-width="1"/>
          <circle cx="${cx}" cy="${cy}" r="40" fill="#e8edf3" stroke="#dde4ec" stroke-width="0.8"/>
          ${dots}
        </g>`;
      }
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">${quads}</svg>`;
    }

    if(m.layout === 'esr2t'){
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">
        <rect x="8" y="8" width="194" height="194" rx="8" fill="#f1f5f9" stroke="#cbd5e1" stroke-width="2"/>
        <g class="mbx-hit" data-act="zone_on" data-zone="1" style="cursor:pointer">
          <rect x="46" y="50" width="118" height="42" rx="16" fill="#e2e8f0" stroke="#38bdf8" stroke-width="4.5"/>
          <text x="105" y="75" text-anchor="middle" fill="#64748b" font-size="10" font-weight="600">ON / OFF</text>
        </g>
        <g class="mbx-hit" data-act="zone_on" data-zone="2" style="cursor:pointer">
          <rect x="46" y="110" width="118" height="42" rx="16" fill="#e2e8f0" stroke="#38bdf8" stroke-width="4.5"/>
          <text x="105" y="135" text-anchor="middle" fill="#64748b" font-size="10" font-weight="600">ON / OFF</text>
        </g>
        <text x="105" y="182" text-anchor="middle" fill="#94a3b8" font-size="9" font-weight="800">MiBOXER®</text>
      </svg>`;
    }

    if(m.layout === 'push3'){
      return `<svg viewBox="0 0 210 210" width="${Math.round(210*sc)}" height="${Math.round(210*sc)}">
        <g class="mbx-hit" data-act="on" style="cursor:pointer">
          <rect x="10" y="10" width="190" height="190" rx="7" fill="#f8fafc" stroke="#cbd5e1" stroke-width="2"/>
          <line x1="14" y1="24" x2="196" y2="24" stroke="#e2e8f0" stroke-width="1"/>
          <text x="184" y="186" text-anchor="end" fill="#94a3b8" font-size="9.5" font-weight="800">MiBOXER®</text>
        </g>
      </svg>`;
    }

    // ---------------- 8. FUT100 (1:1 CAD Matched to fut100_en_v1.0.pdf & Fut100_45649846513.webp) ----------------
    if(m.layout === 'fut100'){
      const curZ = (az !== null && az >= 0) ? az : 100;
      const zDisp = curZ === 0 ? 'ALL' : (curZ === 100 ? '100' : String(curZ).padStart(2, '0'));
      // 20 pages of 5 zones (Page 0 = zones 1..5, Page 1 = zones 6..10 ... Page 19 = zones 96..100)
      const baseZero = (curZ >= 1 && curZ <= 100) ? Math.floor((curZ - 1) / 5) * 5 : 95;
      const pageUpZ   = ((baseZero + 5) % 100) + 1;
      const pageDownZ = ((baseZero + 95) % 100) + 1;

      const zBtn = (x, y, slotIdx, lbl) => {
        const zNum = baseZero + slotIdx;
        return hZoneRocker(x, y, 40, 15, zNum, lbl, az === zNum, p);
      };

      // Dual-action S- / S+ button with independent left/right hitboxes
      const speedDualBtn = `<g>
        <rect x="65" y="217" width="40" height="15" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
        <text x="74" y="227" text-anchor="middle" fill="${p.txt}" font-size="7.2" font-weight="600">S−</text>
        <text x="96" y="227" text-anchor="middle" fill="${p.txt}" font-size="7.2" font-weight="600">S+</text>
        <rect class="mbx-hit" data-act="mode_speed_down" x="65" y="217" width="20" height="15" fill="transparent" style="cursor:pointer"/>
        <rect class="mbx-hit" data-act="mode_speed_up"   x="85" y="217" width="20" height="15" fill="transparent" style="cursor:pointer"/>
      </g>`;

      // Crisp vector Child-Lock / Copy button [ 🔒 / ⧉ ] (no emoji font glitches!)
      const lockCopyBtn = `<g class="mbx-hit" data-act="mode" style="cursor:pointer">
        <rect x="79" y="159" width="26" height="16" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
        <!-- Padlock icon -->
        <rect x="83.5" y="166.8" width="5.2" height="4.2" rx="0.7" fill="none" stroke="${p.txt}" stroke-width="0.9"/>
        <path d="M 84.6 166.8 V 165.2 A 1.5 1.5 0 0 1 87.6 165.2 V 166.8" fill="none" stroke="${p.txt}" stroke-width="0.9"/>
        <!-- Slash -->
        <line x1="90.5" y1="171" x2="93.2" y2="163.5" stroke="${p.txt}" stroke-width="0.85"/>
        <!-- Overlapping Copy-Link squares -->
        <rect x="94.5" y="164" width="4.2" height="4.2" rx="0.6" fill="none" stroke="${p.txt}" stroke-width="0.85"/>
        <rect x="96.5" y="166.5" width="4.2" height="4.2" rx="0.6" fill="${p.key}" stroke="${p.txt}" stroke-width="0.85"/>
      </g>`;

      // Scene 1/2 & Scene 3/M buttons with independent left/right hitboxes
      const trayGlyph = (cx, yBottom, num) => `
        <path d="M ${cx-5.5} ${yBottom-3.2} V ${yBottom} H ${cx+5.5} V ${yBottom-3.2}" fill="none" stroke="#0284c7" stroke-width="1"/>
        <text x="${cx}" y="${yBottom-1.8}" text-anchor="middle" fill="#0284c7" font-size="6.4" font-weight="700">${num}</text>`;

      const sceneRow = `<g>
        <rect x="21" y="236" width="40" height="15" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
        ${trayGlyph(30.5, 247.5, '1')}
        ${trayGlyph(51.5, 247.5, '2')}
        <rect class="mbx-hit" data-act="scene_1" x="21" y="236" width="20" height="15" fill="transparent" style="cursor:pointer"/>
        <rect class="mbx-hit" data-act="scene_2" x="41" y="236" width="20" height="15" fill="transparent" style="cursor:pointer"/>

        <rect x="65" y="236" width="40" height="15" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
        ${trayGlyph(74.5, 247.5, '3')}
        <text x="95.5" y="246.2" text-anchor="middle" fill="#0284c7" font-size="7.5" font-weight="700">M</text>
        <rect class="mbx-hit" data-act="scene_3" x="65" y="236" width="20" height="15" fill="transparent" style="cursor:pointer"/>
        <rect class="mbx-hit" data-act="mode"    x="85" y="236" width="20" height="15" fill="transparent" style="cursor:pointer"/>
      </g>`;

      return `<svg viewBox="0 0 126 296" width="${Math.round(126*sc)}" height="${Math.round(296*sc)}">
        ${sharedDefs(u)}
        <!-- Outer body: curved crown & base matching Fut100_45649846513.webp -->
        <rect x="9" y="4" width="108" height="288" rx="26" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>
        <!-- Contoured Top Screen Bezel (curved top arch + crisp lower corners) -->
        <path d="M 17 30 C 17 15, 32 11, 63 11 C 94 11, 109 15, 109 30 L 109 73 Q 109 77 105 77 L 21 77 Q 17 77 17 73 Z" fill="#ffffff" stroke="#cbd5e1" stroke-width="0.9"/>
        <text x="63" y="22.5" text-anchor="middle" fill="#1e293b" font-size="7" font-weight="800" letter-spacing="0.3">MiBOXER®</text>

        <!-- Monochrome Segmented LCD Window -->
        <rect x="25" y="26.5" width="76" height="40" rx="1.5" fill="#d9e0d7" stroke="#64748b" stroke-width="0.9"/>
        <!-- LCD Padlock + 3-Bar Battery Icon (top right) -->
        <rect x="82" y="30" width="3.4" height="2.8" fill="none" stroke="#1e293b" stroke-width="0.7"/>
        <path d="M 82.7 30 V 28.9 A 1 1 0 0 1 84.7 28.9 V 30" fill="none" stroke="#1e293b" stroke-width="0.7"/>
        <rect x="88" y="28.7" width="9.5" height="4.2" rx="0.5" fill="none" stroke="#1e293b" stroke-width="0.7"/>
        <rect x="97.5" y="29.8" width="1" height="2" fill="#1e293b"/>
        <rect x="89.2" y="29.6" width="2" height="2.4" fill="#1e293b"/>
        <rect x="91.9" y="29.6" width="2" height="2.4" fill="#1e293b"/>
        <rect x="94.6" y="29.6" width="2" height="2.4" fill="#1e293b"/>

        <!-- LCD Left Column: Giant 7-Seg Zone + Mode 8 + Save 3 -->
        <text x="27.5" y="44" fill="#0f172a" font-family="monospace" font-size="16" font-weight="900" letter-spacing="-0.8">${zDisp}</text>
        <text x="27.5" y="53.5" fill="#1e293b" font-size="5" font-weight="700">Mode <tspan font-family="monospace" font-size="6.8" font-weight="800">8</tspan></text>
        <text x="27.5" y="62.5" fill="#1e293b" font-size="5" font-weight="700">Save <tspan font-family="monospace" font-size="6.8" font-weight="800">3</tspan></text>

        <!-- LCD Right Column: Stacked RGB/CCT 8888K, DIM 100%, SAT 100% -->
        <text x="58" y="39.5" fill="#1e293b" font-size="4.3" font-weight="800">RGB</text>
        <text x="58" y="43.8" fill="#1e293b" font-size="4.3" font-weight="800">CCT</text>
        <text x="98.5" y="43.5" text-anchor="end" fill="#0f172a" font-family="monospace" font-size="8.2" font-weight="800">2700<tspan font-size="5.2">K</tspan></text>
        <text x="58" y="53.5" fill="#1e293b" font-size="5" font-weight="800">DIM</text>
        <text x="98.5" y="53.5" text-anchor="end" fill="#0f172a" font-family="monospace" font-size="8" font-weight="800">100<tspan font-size="5.2">%</tspan></text>
        <text x="58" y="62.5" fill="#1e293b" font-size="5" font-weight="800">SAT</text>
        <text x="98.5" y="62.5" text-anchor="end" fill="#0f172a" font-family="monospace" font-size="8" font-weight="800">100<tspan font-size="5.2">%</tspan></text>

        <!-- Recessed Pill Indicator Light below LCD -->
        <rect x="59.5" y="71.2" width="7" height="2.4" rx="1.2" fill="#f1f5f9" stroke="#64748b" stroke-width="0.7"/>

        <!-- 3 Horizontal Touch Pills: Color Slider, Saturation/CCT Slider, Dimming Slider -->
        <rect class="mbx-hit" data-act="hue" data-val="180" x="21" y="86" width="84" height="9.5" rx="4.75" fill="url(#hrgb_${u})" stroke="#cbd5e1" stroke-width="0.6" style="cursor:pointer"/>
        ${dualCctSatPillH(21, 106, 84, 9.5, u)}
        ${blueBriPillH(21, 126, 84, 9.5)}

        <!-- 2x3 Upper Rectangular Key Matrix: [ON] [△] [OFF] / [W] [▽] [Lock/Copy] -->
        ${keyBox(21, 140, 26, 16, 'ON', 'zone_on', p, {zone:0, fs:7.2, rx:2.5})}
        <g class="mbx-hit" data-act="zone" data-zone="${pageUpZ}" style="cursor:pointer">
          <rect x="50" y="140" width="26" height="16" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
          <polygon points="63,144.8 66.5,151 59.5,151" fill="none" stroke="${p.txt}" stroke-width="1.1" stroke-linejoin="round"/>
        </g>
        ${keyBox(79, 140, 26, 16, 'OFF', 'zone_off', p, {zone:0, fs:7.2, rx:2.5})}

        ${keyBox(21, 159, 26, 16, 'W', 'set_white', p, {fs:7.2, rx:2.5})}
        <g class="mbx-hit" data-act="zone" data-zone="${pageDownZ}" style="cursor:pointer">
          <rect x="50" y="159" width="26" height="16" rx="2.5" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.85"/>
          <polygon points="59.5,164 66.5,164 63,170.2" fill="none" stroke="${p.txt}" stroke-width="1.1" stroke-linejoin="round"/>
        </g>
        ${lockCopyBtn}

        <!-- 4x2 Lower Dual-Bank Zone & Scene Matrix -->
        ${zBtn(21, 179, 1, '1/6')}
        ${zBtn(65, 179, 2, '2/7')}
        ${zBtn(21, 198, 3, '3/8')}
        ${zBtn(65, 198, 4, '4/9')}
        ${zBtn(21, 217, 5, '5/0')}
        ${speedDualBtn}
        ${sceneRow}

        <!-- Authentic Bottom 2.4G RF + Waves + Model No.: FUT100 -->
        <text x="58" y="264" text-anchor="middle" fill="#0f172a" font-size="6.8" font-weight="900" letter-spacing="0.4">2.4G RF</text>
        <circle cx="75.5" cy="262" r="1" fill="#0f172a"/>
        <path d="M 77 259.8 A 3 3 0 0 1 77 264.2" fill="none" stroke="#0f172a" stroke-width="0.9"/>
        <path d="M 78.8 258.2 A 5 5 0 0 1 78.8 265.8" fill="none" stroke="#0f172a" stroke-width="0.9"/>
        <text x="63" y="270.5" text-anchor="middle" fill="#475569" font-size="4.8" font-weight="600">Model No.: FUT100</text>
      </svg>`;
    }

    // ---------------- 9. FUT089 (8-Zone, White & Black) & FUT089S (6-Zone + 4 Scenes) — Matched to FUT089/089S_EN_V1.0.pdf ----------------
    if(m.layout === 'fut089'){
      let zGrid = '';
      const maxPairs = m.sixZone ? 3 : 4;
      for(let r=0; r<maxPairs; r++){
        const zL = r*2 + 1, zR = r*2 + 2;
        zGrid += hZoneRocker(22, 178 + r*17, 38, 14, zL, String(zL), az===zL, p);
        zGrid += hZoneRocker(64, 178 + r*17, 38, 14, zR, String(zR), az===zR, p);
      }
      if(m.sixZone){
        zGrid += sceneSlotPair(22, 178 + 3*17, 38, 14, '1', '2', p);
        zGrid += sceneSlotPair(64, 178 + 3*17, 38, 14, '3', '4', p);
      }
      return `<svg viewBox="0 0 124 288" width="${Math.round(124*sc)}" height="${Math.round(288*sc)}">
        ${sharedDefs(u)}
        <!-- 47x153mm slim rectangular body with authentic corner radius -->
        <rect x="12" y="4" width="100" height="280" rx="15" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>
        ${colorRing(62, 50, 27, 11.5)}
        <circle cx="62" cy="50" r="15.5" fill="${p.body}"/>
        <circle cx="62" cy="50" r="4.2" fill="none" stroke="${p.keyStr}" stroke-width="0.9"/>
        ${dualCctSatPillH(22, 95, 80, 9.5, u)}
        ${blueBriPillH(22, 115, 80, 9.5)}
        ${keyBox(22, 136, 24, 15, 'ON', 'zone_on', p, {zone:0, fs:7, rx:2.5})}
        ${keyBox(50, 136, 24, 15, 'W', 'set_white', p, {fs:7, rx:2.5})}
        ${keyBox(78, 136, 24, 15, 'OFF', 'zone_off', p, {zone:0, fs:7, rx:2.5})}
        ${keyBox(22, 154, 24, 15, 'S-', 'mode_speed_down', p, {fs:7, rx:2.5})}
        ${keyBox(50, 154, 24, 15, 'M', 'mode', p, {fs:7, rx:2.5})}
        ${keyBox(78, 154, 24, 15, 'S+', 'mode_speed_up', p, {fs:7, rx:2.5})}
        ${zGrid}
        <text x="62" y="266" text-anchor="middle" fill="${p.txt}" font-size="8.2" font-family="serif" font-weight="700">Mi•Light®</text>
      </svg>`;
    }

    // ---------------- 10. FUT092 (White w/ Lime Keys & Black w/ Dark Keys) — Matched 1:1 to FUT092_EN_V1.0.pdf & FUT092_Black_92SYT.png ----------------
    if(m.layout === 'fut092'){
      const kFill = p.b ? '#1d222b' : '#dce642';
      const kTxt  = p.b ? '#e2e8f0' : '#1e293b';
      const kStr  = p.b ? '#475569' : '#b5c428';
      const rimCol = p.b ? '#334155' : '#60a5fa';

      // 4 aligned stadium/capsule zone rockers (1, 2, 3, 4)
      let zCols = '';
      for(let z=1; z<=4; z++){
        zCols += vZoneRocker(23.5 + (z-1)*22.2, 194, 14.2, 41, z, String(z), az===z, p, kFill, kTxt);
      }

      // Top crescent Master ON (—) and Master OFF (O)
      const masterOn  = crescentKey(46, 24, 18, 11, 4.5, kFill, kStr, 'zone_on',
        `<line x1="42.5" y1="25.5" x2="49.5" y2="25.5" stroke="${kTxt}" stroke-width="1.6" stroke-linecap="round"/>`, 0);
      const masterOff = crescentKey(82, 24, 18, 11, 4.5, kFill, kStr, 'zone_off',
        `<circle cx="82" cy="25.5" r="2.6" fill="none" stroke="${kTxt}" stroke-width="1.25"/>`, 0);

      // 3 stacked center crescent smile buttons: S+, M, S-
      const cSplus  = crescentKey(64, 141, 25, 13, 5.2, kFill, kStr, 'mode_speed_up',
        `<text x="64" y="145" text-anchor="middle" fill="${kTxt}" font-size="7.5" font-weight="700">S+</text>`);
      const cMode   = crescentKey(64, 159, 25, 13, 5.2, kFill, kStr, 'mode',
        `<text x="64" y="163" text-anchor="middle" fill="${kTxt}" font-size="7.5" font-weight="700">M</text>`);
      const cSminus = crescentKey(64, 177, 25, 13, 5.2, kFill, kStr, 'mode_speed_down',
        `<text x="64" y="181" text-anchor="middle" fill="${kTxt}" font-size="7.5" font-weight="700">S-</text>`);

      return `<svg viewBox="0 0 128 268" width="${Math.round(128*sc)}" height="${Math.round(268*sc)}">
        ${sharedDefs(u)}
        <!-- Subtle outer blue/dark housing rim + smooth barrel-stadium faceplate -->
        <path d="M 64 4 C 99 4, 115 26, 116 134 C 115 242, 99 264, 64 264 C 29 264, 13 242, 12 134 C 13 26, 29 4, 64 4 Z" fill="${p.body}" stroke="${rimCol}" stroke-width="1.8"/>
        <path d="M 64 6 C 97 6, 113 28, 114 134 C 113 240, 97 262, 64 262 C 31 262, 15 240, 14 134 C 15 28, 31 6, 64 6 Z" fill="none" stroke="${p.bezel}" stroke-width="0.9"/>
        ${masterOn}${masterOff}
        <!-- Color Ring + Center Cap with Indicator -->
        ${colorRing(64, 71, 29, 13)}
        <circle cx="64" cy="71" r="16" fill="${p.body}" stroke="${p.bezel}" stroke-width="0.7"/>
        <circle cx="64" cy="71" r="3.8" fill="none" stroke="${p.keyStr}" stroke-width="0.85"/>
        <!-- Concentric Curved Color Temperature Smile Arc -->
        <path class="mbx-hit" data-act="kelvin" data-val="50" d="M 30.5 110.5 A 48 48 0 0 0 97.5 110.5" fill="none" stroke="url(#cct_${u})" stroke-width="11.5" stroke-linecap="round" style="cursor:pointer"/>
        <!-- Left 2D Saturation Capsule + Center Crescent S+/M/S- + Right Blue Sun Dimming Capsule -->
        ${vSatPill2D(25, 135, 13, 50, u, p.keyStr)}
        ${cSplus}${cMode}${cSminus}
        ${blueBriPillV(90, 135, 13, 50)}
        ${zCols}
        <text x="64" y="250" text-anchor="middle" fill="${p.txt}" font-family="serif" font-size="8.2" font-weight="700">Mi•Light®</text>
      </svg>`;
    }

    // ---------------- 11. FUT088 Rectangular 1-Zone Full-Touch (47x134mm, Matched to FUT088_EN_V1.0.pdf) ----------------
    if(m.layout === 'fut088'){
      const yf = '#e8eb46', yt = '#1e293b';
      const diagTimerKey = (x, y, topTxt, botTxt, act) => `<g class="mbx-hit" data-act="${act}" style="cursor:pointer">
        <rect x="${x}" y="${y}" width="21" height="18" rx="3.5" fill="${yf}" stroke="#b5c428" stroke-width="0.85"/>
        <line x1="${x+3}" y1="${y+15}" x2="${x+18}" y2="${y+3}" stroke="${yt}" stroke-width="0.8"/>
        <text x="${x+6.5}" y="${y+8}" text-anchor="middle" fill="${yt}" font-size="6.2" font-weight="700">${topTxt}</text>
        <circle cx="${x+15.5}" cy="${y+7.2}" r="2.4" fill="none" stroke="${yt}" stroke-width="0.7"/>
        <polyline points="${x+15.5},${y+5.7} ${x+15.5},${y+7.3} ${x+16.7},${y+7.3}" fill="none" stroke="${yt}" stroke-width="0.65"/>
        <text x="${x+11.5}" y="${y+15.8}" text-anchor="middle" fill="${yt}" font-size="5.2" font-weight="700">${botTxt}</text>
      </g>`;

      return `<svg viewBox="0 0 120 266" width="${Math.round(120*sc)}" height="${Math.round(266*sc)}">
        ${sharedDefs(u)}
        <rect x="13" y="6" width="94" height="254" rx="6" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1.8"/>
        ${colorRing(60, 54, 27, 11.5)}
        <circle cx="60" cy="54" r="15.2" fill="#f8fafc"/>
        <circle cx="60" cy="54" r="4" fill="none" stroke="#94a3b8" stroke-width="0.9"/>
        ${dualCctSatPillH(23, 98, 74, 9.5, u)}
        ${blueBriPillH(23, 118, 74, 9.5)}
        ${keyBox(23, 142, 21, 18, 'ON', 'on', p, {fill:yf, tc:yt, rx:3.5, fs:7})}
        ${keyBox(49.5, 142, 21, 18, 'W', 'set_white', p, {fill:yf, tc:yt, rx:3.5, fs:7})}
        ${keyBox(76, 142, 21, 18, 'OFF', 'off', p, {fill:yf, tc:yt, rx:3.5, fs:7})}
        ${keyBox(23, 167, 21, 18, 'R', 'hue', p, {fill:yf, tc:yt, rx:3.5, fs:7.5})}
        ${keyBox(49.5, 167, 21, 18, 'G', 'hue', p, {fill:yf, tc:yt, rx:3.5, fs:7.5})}
        ${keyBox(76, 167, 21, 18, 'B', 'hue', p, {fill:yf, tc:yt, rx:3.5, fs:7.5})}
        ${diagTimerKey(23, 192, 'S-', '60s', 'mode_speed_down')}
        ${keyBox(49.5, 192, 21, 18, 'M', 'mode', p, {fill:yf, tc:yt, rx:3.5, fs:7.5})}
        ${diagTimerKey(76, 192, 'S+', '10min', 'mode_speed_up')}
        <text x="60" y="238" text-anchor="middle" font-size="8.5" font-weight="800"><tspan fill="#e11d48">Mi</tspan><tspan fill="#1e293b">BOXER®</tspan></text>
      </svg>`;
    }

    // ---------------- 12. Pebble 4-Zone Family: FUT096, FUT095, FUT094 (CAD-Matched to fut094_en_v1.0.pdf) ----------------
    if(m.layout === 'fut_pebble'){
      const is95 = m.kind === 'fut095', is94 = m.kind === 'fut094';
      const kf = is95 ? '#18181b' : (p.b ? '#252b36' : (is94 ? '#f2e76d' : '#facc15'));
      const kt = (is95 || p.b) ? '#f8fafc' : '#334155';
      let zCols = '';
      for(let z=1; z<=4; z++){
        const dy = (z===1 || z===4) ? 178 : 185;
        zCols += vZoneRocker(23 + (z-1)*22.5, dy, 14.5, 42, z, String(z), az===z, p, kf, kt);
      }
      const topMaster = is95 ? '' : `
        <rect x="42" y="21" width="44" height="14" rx="7" fill="${p.body}" stroke="${p.keyStr}" stroke-width="0.9"/>
        <line class="mbx-hit" data-act="zone_on" data-zone="0" x1="48" y1="28" x2="55" y2="28" stroke="${p.txt}" stroke-width="1.6" stroke-linecap="round" style="cursor:pointer"/>
        <circle class="mbx-hit" data-act="zone_off" data-zone="0" cx="76" cy="28" r="3.3" fill="none" stroke="${p.txt}" stroke-width="1.3" style="cursor:pointer"/>`;

      const wheel = is94
        ? `<circle cx="64" cy="78" r="35" fill="${p.slot}" opacity="0.45"/>
           <path class="mbx-hit" data-act="kelvin" data-val="50" d="M 44.6 99.5 A 29 29 0 1 1 83.4 99.5" fill="none" stroke="url(#cct_${u})" stroke-width="13.5" stroke-linecap="round" style="cursor:pointer"/>
           <circle cx="64" cy="78" r="17" fill="${p.body}" stroke="${p.bezel}" stroke-width="0.7"/>
           <circle cx="64" cy="78" r="5" fill="#cbd5e1" opacity="0.65"/>`
        : colorRing(64, 76, 29, 12.5);

      let crescent = '';
      if(is94){
        let arcDots = '';
        [121,111,101,91,81,71,61].forEach((deg, idx) => {
          const rad = deg * Math.PI / 180;
          const cx = (64 + 51 * Math.cos(rad)).toFixed(1);
          const cy = (75 + 51 * Math.sin(rad)).toFixed(1);
          arcDots += `<circle cx="${cx}" cy="${cy}" r="${(1.0 + idx*0.18).toFixed(2)}" fill="#fff" pointer-events="none"/>`;
        });
        crescent = `<g class="mbx-hit" data-act="brightness" data-val="85" style="cursor:pointer">
          <path d="M 28.5 111.5 A 51 51 0 0 0 99.5 111.5" fill="none" stroke="#0999dc" stroke-width="12" stroke-linecap="round"/>
          <text x="29" y="114" text-anchor="middle" fill="#fff" font-size="6.5">☀</text>
          ${arcDots}
          <text x="98.5" y="115" text-anchor="middle" fill="#fff" font-size="9.5">☀</text>
        </g>`;
      } else {
        crescent = `<g class="mbx-hit" data-act="brightness" data-val="85" style="cursor:pointer">
          <path d="M 31 116 A 48 48 0 0 0 97 116" fill="none" stroke="${p.txt}" stroke-width="4.5" stroke-linecap="round"/>
          <text x="26" y="116" fill="${p.mut}" font-size="6.5">☀</text><text x="99" y="117" fill="${p.txt}" font-size="9">☀</text>
        </g>`;
      }

      let midBtns = '';
      if(is95){
        midBtns = keyBox(30, 134, 30, 15, 'ON', 'zone_on', p, {zone:0, fill:'#18181b', tc:'#fff', fs:7.5})
                + keyBox(68, 134, 30, 15, 'OFF', 'zone_off', p, {zone:0, fill:'#18181b', tc:'#fff', fs:7.5})
                + keyBox(30, 155, 18, 16, 'S-', 'mode_speed_down', p, {fill:'#18181b', tc:'#fff', rx:8, fs:7})
                + keyBox(55, 155, 18, 16, 'M', 'mode', p, {fill:'#18181b', tc:'#fff', rx:8, fs:7})
                + keyBox(80, 155, 18, 16, 'S+', 'mode_speed_up', p, {fill:'#18181b', tc:'#fff', rx:8, fs:7});
      } else {
        const lbls = is94 ? ['S1','S2','S3'] : ['S-','M','S+'];
        const acts = is94 ? ['mode','mode','mode'] : ['mode_speed_down','mode','mode_speed_up'];
        midBtns = `<g transform="rotate(23 36 146)">${keyBox(24, 139, 24, 13.5, lbls[0], acts[0], p, {fill:kf, tc:kt, rx:5, fs:7.5})}</g>`
                + keyBox(51, 146, 26, 13.5, lbls[1], acts[1], p, {fill:kf, tc:kt, rx:5, fs:7.5})
                + `<g transform="rotate(-23 92 146)">${keyBox(80, 139, 24, 13.5, lbls[2], acts[2], p, {fill:kf, tc:kt, rx:5, fs:7.5})}</g>`;
      }
      const brandFooter = is94
        ? `<text x="64" y="243" text-anchor="middle" font-size="8" font-weight="800"><tspan fill="#e11d48">Mi</tspan><tspan fill="${p.txt}">BOXER®</tspan></text>
           <text x="64" y="250" text-anchor="middle" fill="${p.mut}" font-size="4.8">Model No.: FUT094</text>`
        : `<text x="64" y="246" text-anchor="middle" fill="${p.txt}" font-family="serif" font-size="8" font-weight="700">Mi•Light®</text>`;

      return `<svg viewBox="0 0 128 268" width="${Math.round(128*sc)}" height="${Math.round(268*sc)}">
        ${sharedDefs(u)}
        <path d="M 64 5 C 97 5, 114 34, 116 122 C 118 202, 106 263, 64 263 C 22 263, 10 202, 12 122 C 14 34, 31 5, 64 5 Z" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.2"/>
        ${topMaster}${wheel}${crescent}${midBtns}${zCols}${brandFooter}
      </svg>`;
    }

    // ---------------- 13. Classic Pebble: FUT007 (4-Dot Ring) & FUT006 (12-Tick Wheel) ----------------
    if(m.layout === 'fut_classic'){
      const is06 = m.kind === 'fut006';
      let ring = `<circle cx="64" cy="112" r="34" fill="#18181b" stroke="#475569" stroke-width="1.2"/>
                  <circle cx="64" cy="112" r="16" fill="#f8fafc"/>`;
      if(is06){
        for(let i=0; i<12; i++){
          const a = (i*30)*Math.PI/180;
          ring += `<line class="mbx-hit" data-act="brightness" data-val="${(i+1)*8}" x1="${(64+20*Math.cos(a)).toFixed(1)}" y1="${(112+20*Math.sin(a)).toFixed(1)}" x2="${(64+30*Math.cos(a)).toFixed(1)}" y2="${(112+30*Math.sin(a)).toFixed(1)}" stroke="#64748b" stroke-width="1.3" style="cursor:pointer"/>`;
        }
      } else {
        [[64,85,'brightness'],[64,139,'brightness'],[37,112,'kelvin'],[91,112,'kelvin']].forEach(([cx,cy,act]) => {
          ring += `<circle class="mbx-hit" data-act="${act}" cx="${cx}" cy="${cy}" r="2.2" fill="#fff" style="cursor:pointer"/>`;
        });
      }
      let zCols = '';
      for(let z=1; z<=4; z++){
        const dy = (z===1 || z===4) ? 164 : 174;
        zCols += vZoneRocker(22 + (z-1)*23, dy, 16, 42, z, String(z), az===z, p);
      }
      return `<svg viewBox="0 0 128 264" width="${Math.round(128*sc)}" height="${Math.round(264*sc)}">
        <rect x="8" y="4" width="112" height="256" rx="48" fill="#f8fafc" stroke="#cbd5e1" stroke-width="2"/>
        <g class="mbx-hit" data-act="zone_on" data-zone="0" style="cursor:pointer"><circle cx="64" cy="28" r="11" fill="#f1f5f9" stroke="#94a3b8"/><line x1="64" y1="23" x2="64" y2="33" stroke="#1e293b" stroke-width="2.2" stroke-linecap="round"/></g>
        <g class="mbx-hit" data-act="zone_off" data-zone="0" style="cursor:pointer"><circle cx="64" cy="55" r="11" fill="#f1f5f9" stroke="#94a3b8"/><circle cx="64" cy="55" r="4.5" fill="none" stroke="#1e293b" stroke-width="2"/></g>
        ${ring}${zCols}
        <text x="64" y="238" text-anchor="middle" fill="#334155" font-size="7.5" font-weight="800">MiBOXER ${m.id}</text>
      </svg>`;
    }

    // ---------------- 14. Modern Matte-Black C5 (4-Zone) & C8 (8-Zone) ----------------
    if(m.layout === 'c_wheel'){
      const is8 = m.zones === 8;
      let zPart = '';
      if(is8){
        for(let r=0; r<4; r++){
          zPart += hZoneRocker(22, 178 + r*16.5, 38, 13.5, r*2+1, String(r*2+1), az===r*2+1, p)
                 + hZoneRocker(64, 178 + r*16.5, 38, 13.5, r*2+2, String(r*2+2), az===r*2+2, p);
        }
      } else {
        for(let z=1; z<=4; z++) zPart += vZoneRocker(22 + (z-1)*21.5, 176, 15.5, 44, z, String(z), az===z, p);
      }
      return `<svg viewBox="0 0 124 282" width="${Math.round(124*sc)}" height="${Math.round(282*sc)}">
        ${sharedDefs(u)}
        <rect x="10" y="4" width="104" height="274" rx="12" fill="#22262e" stroke="#3b4252" stroke-width="2"/>
        ${colorRing(62, 54, 28, 13)}
        ${is8 ? `<rect x="22" y="96" width="80" height="12" rx="6" fill="#161920" stroke="#334155" stroke-width="0.8"/>
                 <rect class="mbx-hit" data-act="kelvin" data-val="50" x="32" y="100" width="60" height="4" rx="2" fill="url(#cct_${u})" style="cursor:pointer"/>` : ''}
        <rect x="22" y="${is8?115:104}" width="80" height="12" rx="6" fill="#161920" stroke="#334155" stroke-width="0.8"/>
        <rect class="mbx-hit" data-act="brightness" data-val="80" x="32" y="${is8?119:108}" width="58" height="4" rx="2" fill="#cbd5e1" style="cursor:pointer"/>
        ${keyBox(22, 136, 24, 15, 'ON', 'zone_on', p, {zone:0, fs:7})}
        ${keyBox(50, 136, 24, 15, 'W', 'set_white', p, {fs:7})}
        ${keyBox(78, 136, 24, 15, 'OFF', 'zone_off', p, {zone:0, fs:7})}
        ${keyBox(22, 154, 24, 15, is8?'S-':'S-/◐', 'mode_speed_down', p, {fs:6.8})}
        ${keyBox(50, 154, 24, 15, 'M', 'mode', p, {fs:7})}
        ${keyBox(78, 154, 24, 15, is8?'S+':'S+/⊕', 'mode_speed_up', p, {fs:6.8})}
        ${zPart}
        <text x="62" y="262" text-anchor="middle" fill="#cbd5e1" font-size="7" font-weight="700">MiBOXER ${m.id}</text>
      </svg>`;
    }

    // ---------------- 15. Minimalist Vertical Slot Remotes: C4 (RGB), C3 (CCT), C3P (12-Dot Dimmer) ----------------
    if(m.layout === 'c_slot'){
      let track = '';
      if(m.kind === 'rgb') track = `<rect class="mbx-hit" data-act="hue" data-val="180" x="56" y="44" width="4" height="82" rx="2" fill="url(#vrgb_${u})" style="cursor:pointer"/>`;
      else if(m.kind === 'cct') track = `<rect class="mbx-hit" data-act="kelvin" data-val="50" x="56" y="44" width="4" height="82" rx="2" fill="url(#vcct_${u})" style="cursor:pointer"/>`;
      else {
        for(let i=0; i<12; i++) track += `<circle class="mbx-hit" data-act="brightness" data-val="${100-i*8}" cx="58" cy="${46+i*7}" r="1.4" fill="#94a3b8" style="cursor:pointer"/>`;
      }
      return `<svg viewBox="0 0 116 268" width="${Math.round(116*sc)}" height="${Math.round(268*sc)}">
        ${sharedDefs(u)}
        <rect x="14" y="6" width="88" height="256" rx="14" fill="#262a33" stroke="#3f4654" stroke-width="2"/>
        <rect x="46" y="28" width="24" height="114" rx="12" fill="#1b1e25" stroke="#3b4252"/>
        <text x="58" y="39" text-anchor="middle" fill="#e2e8f0" font-size="7.5">☀</text>
        ${track}
        <text x="58" y="137" text-anchor="middle" fill="#94a3b8" font-size="5.5">☀</text>
        <rect x="33" y="156" width="50" height="18" rx="9" fill="#2d333f" stroke="#4b5563" stroke-width="0.9"/>
        <text class="mbx-hit" data-act="on" x="45" y="167.5" text-anchor="middle" fill="#e2e8f0" font-size="7" font-weight="700" style="cursor:pointer">ON</text>
        <text class="mbx-hit" data-act="off" x="71" y="167.5" text-anchor="middle" fill="#e2e8f0" font-size="7" font-weight="700" style="cursor:pointer">OFF</text>
        <text x="58" y="242" text-anchor="middle" fill="#cbd5e1" font-size="7.5" font-weight="700">MiBOXER ${m.id}</text>
      </svg>`;
    }

    // ---------------- 16. C1 (5-Diamond X-Pad + 4 Full-Width Horizontal Zone Bars) & C12 (LED + Scroll Wheel) ----------------
    if(m.layout === 'c1'){
      const dm = (cx,cy,lbl,act) => `<g class="mbx-hit" data-act="${act}" style="cursor:pointer">
        <polygon points="${cx},${cy-11} ${cx+14},${cy} ${cx},${cy+11} ${cx-14},${cy}" fill="${p.key}" stroke="${p.keyStr}" stroke-width="0.9"/>
        <text x="${cx}" y="${cy+2.8}" text-anchor="middle" fill="${p.txt}" font-size="7" font-weight="700">${lbl}</text>
      </g>`;
      let bars = '';
      for(let z=1; z<=4; z++) bars += hZoneRocker(24, 114 + (z-1)*20, 72, 15, z, String(z), az===z, p);
      return `<svg viewBox="0 0 120 272" width="${Math.round(120*sc)}" height="${Math.round(272*sc)}">
        <rect x="14" y="6" width="92" height="260" rx="14" fill="#262a33" stroke="#3f4654" stroke-width="2"/>
        ${keyBox(24, 26, 33, 16, 'ON', 'zone_on', p, {zone:0, fs:7.5})}
        ${keyBox(63, 26, 33, 16, 'OFF', 'zone_off', p, {zone:0, fs:7.5})}
        ${dm(40, 62, '◐-', 'kelvin')}${dm(80, 62, '⊕+', 'kelvin')}
        ${dm(60, 76, 'S1', 'mode')}
        ${dm(40, 90, '☀-', 'brightness')}${dm(80, 90, '☀+', 'brightness')}
        ${bars}
        <text x="60" y="246" text-anchor="middle" fill="#cbd5e1" font-size="7.5" font-weight="700">MiBOXER C1</text>
      </svg>`;
    }

    if(m.layout === 'c12'){
      return `<svg viewBox="0 0 120 272" width="${Math.round(120*sc)}" height="${Math.round(272*sc)}">
        <rect x="14" y="6" width="92" height="260" rx="14" fill="#262a33" stroke="#3f4654" stroke-width="2"/>
        <rect x="38" y="26" width="44" height="18" rx="2.5" fill="#181a20" stroke="#3f4654"/>
        <text x="60" y="39.5" text-anchor="middle" fill="#f97316" font-family="monospace" font-size="12" font-weight="800">100%</text>
        <!-- Recessed Mechanical Scroll Wheel -->
        <ellipse cx="60" cy="76" rx="13" ry="26" fill="#14171d" stroke="#3f4654"/>
        <rect class="mbx-hit" data-act="brightness" data-val="85" x="52" y="58" width="16" height="36" rx="6" fill="#272b34" stroke="#475569" style="cursor:pointer"/>
        ${[64,70,76,82,88].map(y=>`<line x1="54" y1="${y}" x2="66" y2="${y}" stroke="#525e73" stroke-width="1"/>`).join('')}
        <!-- 3 Dual-Button Pills -->
        ${keyBox(34, 116, 52, 16, 'ON        OFF', 'on', p, {rx:8, fs:6.8})}
        ${keyBox(34, 138, 52, 16, '1            2', 'zone', p, {zone:1, rx:8, fs:7})}
        ${keyBox(34, 160, 52, 16, '3            4', 'zone', p, {zone:3, rx:8, fs:7})}
        <text x="60" y="246" text-anchor="middle" fill="#cbd5e1" font-size="7.5" font-weight="700">MiBOXER C12</text>
      </svg>`;
    }

    // ---------------- 17. Y1 / Y2 / Y3 Vertical Desk Stand Remotes ----------------
    if(m.layout === 'y_desk'){
      let vbars = '';
      for(let i=0; i<22; i++){
        const y = 34 + i*4.3;
        if(m.kind === 'y1'){
          vbars += `<rect class="mbx-hit" data-act="brightness" x="54" y="${y}" width="16" height="2.6" fill="#cbd5e1" opacity="${(1-i*0.035).toFixed(2)}" style="cursor:pointer"/>`;
        } else {
          const leftCol = m.kind === 'y3' ? `hsl(${i*16},85%,55%)` : (i<11 ? '#7dd3fc' : '#fde047');
          vbars += `<rect class="mbx-hit" data-act="${m.kind==='y3'?'hue':'kelvin'}" x="36" y="${y}" width="15" height="2.6" fill="${leftCol}" style="cursor:pointer"/>`;
          vbars += `<rect class="mbx-hit" data-act="brightness" x="73" y="${y}" width="15" height="2.6" fill="#cbd5e1" opacity="${(1-i*0.035).toFixed(2)}" style="cursor:pointer"/>`;
        }
      }
      let zCols = '';
      for(let z=1; z<=3; z++) zCols += vZoneRocker(24 + (z-1)*30, 192, 18, 42, z, String(z), az===z, p);
      return `<svg viewBox="0 0 124 268" width="${Math.round(124*sc)}" height="${Math.round(268*sc)}">
        <rect x="12" y="6" width="100" height="256" rx="6" fill="#181b22" stroke="#3b4252" stroke-width="2"/>
        <rect x="57" y="16" width="10" height="3" rx="1.5" fill="#64748b"/>
        ${vbars}
        ${keyBox(24, 142, 22, 15, 'ON', 'zone_on', p, {zone:0, fs:7})}
        ${m.kind==='y3' ? keyBox(51, 142, 22, 15, 'W', 'set_white', p, {fs:7}) : ''}
        ${keyBox(78, 142, 22, 15, 'OFF', 'zone_off', p, {zone:0, fs:7})}
        ${keyBox(24, 164, 22, 15, m.kind==='y3'?'S-/◐':'1␣', 'mode', p, {fs:6.8})}
        ${keyBox(51, 164, 22, 15, m.kind==='y3'?'M':'2␣', 'mode', p, {fs:7})}
        ${keyBox(78, 164, 22, 15, m.kind==='y3'?'S+/⊕':'3␣', 'mode', p, {fs:6.8})}
        ${zCols}
        <text x="62" y="252" text-anchor="middle" fill="#cbd5e1" font-size="7" font-weight="700">MiBOXER ${m.id}</text>
      </svg>`;
    }

    // ---------------- 18. S2 / S2W+ / S1 / S1W+ / FUT087 Touch Pucks (White & Black) ----------------
    if(m.layout === 's_puck' || m.layout === 'fut087'){
      if(m.layout === 'fut087'){
        return `<svg viewBox="0 0 190 190" width="${Math.round(190*sc)}" height="${Math.round(190*sc)}">
          <circle cx="95" cy="95" r="85" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.5"/>
          <path class="mbx-hit" data-act="brightness" data-val="80" d="M 48 148 A 68 68 0 1 1 142 148" fill="none" stroke="${p.b?'#94a3b8':'#52525b'}" stroke-width="7" stroke-linecap="round" style="cursor:pointer"/>
          <text x="58" y="162" fill="${p.txt}" font-size="8">☀</text><text x="126" y="163" fill="${p.txt}" font-size="11">☀</text>
          <circle cx="95" cy="95" r="52" fill="${p.glass}" stroke="${p.bezel}"/>
          ${keyBox(67, 62, 56, 22, 'ON', 'on', p, {rx:11, fs:9})}
          ${keyBox(67, 102, 56, 22, 'OFF', 'off', p, {rx:11, fs:9})}
          <text x="95" y="140" text-anchor="middle" fill="${p.mut}" font-size="7.5" font-weight="800">MiBOXER®</text>
        </svg>`;
      }
      const isS2 = m.kind === 's2';
      const arcStroke = isS2 ? `url(#hrgb_${u})` : `url(#cct_${u})`;
      const upperArc = m.multi
        ? `<path class="mbx-hit" data-act="${isS2?'hue':'kelvin'}" d="M 28 104 A 68 68 0 1 1 162 104" fill="none" stroke="${arcStroke}" stroke-width="9" stroke-linecap="round" style="cursor:pointer"/>
           <path d="M 30 112 A 68 68 0 0 0 160 112" fill="none" stroke="#94a3b8" stroke-width="9" stroke-linecap="round"/>
           ${[['All',0,42,138],['1',1,66,158],['2',2,95,165],['3',3,124,158],['4',4,148,138]].map(([l,z,x,y])=>
             `<text class="mbx-hit" data-act="zone" data-zone="${z}" x="${x}" y="${y}" text-anchor="middle" fill="${az===z?'#0ea5e9':'#fff'}" font-size="8" font-weight="800" style="cursor:pointer">${l}</text>`
           ).join('')}`
        : `<path class="mbx-hit" data-act="${isS2?'hue':'kelvin'}" d="M 46 146 A 68 68 0 1 1 144 146" fill="none" stroke="${arcStroke}" stroke-width="8.5" stroke-linecap="round" style="cursor:pointer"/>
           <text x="55" y="160" fill="${p.mut}" font-size="7.5">☀</text><text x="128" y="161" fill="${p.txt}" font-size="10">☀</text>`;

      const centerBtns = isS2
        ? `<text class="mbx-hit" data-act="on" x="76" y="74" text-anchor="middle" fill="${p.txt}" font-size="9" font-weight="700" style="cursor:pointer">ON</text>
           <text class="mbx-hit" data-act="off" x="114" y="74" text-anchor="middle" fill="${p.txt}" font-size="9" font-weight="700" style="cursor:pointer">OFF</text>
           <text class="mbx-hit" data-act="brightness" x="66" y="103" text-anchor="middle" fill="${p.txt}" font-size="11" style="cursor:pointer">☀</text>
           <text class="mbx-hit" data-act="mode" x="124" y="103" text-anchor="middle" fill="${p.txt}" font-size="10" style="cursor:pointer">🎨</text>
           <text class="mbx-hit" data-act="set_white" x="95" y="128" text-anchor="middle" fill="${p.txt}" font-size="9" font-weight="700" style="cursor:pointer">W</text>`
        : `<text class="mbx-hit" data-act="on" x="95" y="68" text-anchor="middle" fill="${p.txt}" font-size="9" font-weight="700" style="cursor:pointer">ON</text>
           <text class="mbx-hit" data-act="brightness" x="66" y="99" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">DIM</text>
           <text class="mbx-hit" data-act="kelvin" x="124" y="99" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">CCT</text>
           <text class="mbx-hit" data-act="off" x="95" y="128" text-anchor="middle" fill="${p.txt}" font-size="9" font-weight="700" style="cursor:pointer">OFF</text>`;

      return `<svg viewBox="0 0 190 190" width="${Math.round(190*sc)}" height="${Math.round(190*sc)}">
        ${sharedDefs(u)}
        <circle cx="95" cy="95" r="85" fill="${p.body}" stroke="${p.bezel}" stroke-width="2.5"/>
        ${upperArc}
        <circle cx="95" cy="95" r="50" fill="${p.glass}" stroke="${p.keyStr}" stroke-width="1"/>
        ${centerBtns}
      </svg>`;
    }

    // ---------------- 19. K6 / K5 Display Knobs, K0 / K0S, K3 / K3S, K2 / K2S, K1 ----------------
    if(m.layout === 'k_display'){
      const isK6 = m.kind === 'k6';
      return `<svg viewBox="0 0 190 190" width="${Math.round(190*sc)}" height="${Math.round(190*sc)}">
        <circle cx="95" cy="95" r="84" fill="#14171d" stroke="#334155" stroke-width="3.5"/>
        ${isK6 ? colorRing(95, 95, 75, 5) : `<circle cx="95" cy="95" r="75" fill="none" stroke="#334155" stroke-width="1.5"/>`}
        <text x="95" y="68" text-anchor="middle" fill="#f97316" font-family="monospace" font-size="19" font-weight="800">100%</text>
        <text class="mbx-hit" data-act="on" x="95" y="102" text-anchor="middle" fill="#fff" font-size="18" style="cursor:pointer">⏻</text>
        <text class="mbx-hit" data-act="brightness" x="62" y="50" text-anchor="middle" fill="#e2e8f0" font-size="11" style="cursor:pointer">☀</text>
        <text class="mbx-hit" data-act="kelvin" x="128" y="50" text-anchor="middle" fill="#e2e8f0" font-size="7.5" font-weight="700" style="cursor:pointer">CCT</text>
        <text class="mbx-hit" data-act="mode" x="40" y="99" text-anchor="middle" fill="#e2e8f0" font-size="9" style="cursor:pointer">${isK6?'🎨':'1␣'}</text>
        <text class="mbx-hit" data-act="saturation" x="150" y="99" text-anchor="middle" fill="#e2e8f0" font-size="9" style="cursor:pointer">${isK6?'◐':'4␣'}</text>
        <text class="mbx-hit" data-act="mode" x="64" y="146" text-anchor="middle" fill="#e2e8f0" font-size="9" font-weight="700" style="cursor:pointer">${isK6?'M':'2␣'}</text>
        <text class="mbx-hit" data-act="mode" x="126" y="146" text-anchor="middle" fill="#e2e8f0" font-size="9" font-weight="700" style="cursor:pointer">${isK6?'1␣':'3␣'}</text>
        <text x="95" y="126" text-anchor="middle" fill="#cbd5e1" font-size="7" font-weight="800">MiBOXER</text>
      </svg>`;
    }

    if(m.layout === 'k0'){
      const cLbl = m.zonesMode ? ['1 ZONE','2 ZONE','3 ZONE','4 ZONE'] : ['1␣','2␣','3␣','4␣'];
      return `<svg viewBox="0 0 200 200" width="${Math.round(200*sc)}" height="${Math.round(200*sc)}">
        <rect x="10" y="10" width="180" height="180" rx="10" fill="#1c1f26" stroke="#384152" stroke-width="2"/>
        <line x1="10" y1="100" x2="190" y2="100" stroke="#0f1115" stroke-width="2"/>
        <text class="mbx-hit" data-act="zone" data-zone="1" x="36" y="34" text-anchor="middle" fill="#94a3b8" font-size="8" font-weight="700" style="cursor:pointer">${cLbl[0]}</text>
        <text class="mbx-hit" data-act="zone" data-zone="2" x="164" y="34" text-anchor="middle" fill="#94a3b8" font-size="8" font-weight="700" style="cursor:pointer">${cLbl[1]}</text>
        <text class="mbx-hit" data-act="zone" data-zone="3" x="36" y="172" text-anchor="middle" fill="#94a3b8" font-size="8" font-weight="700" style="cursor:pointer">${cLbl[2]}</text>
        <text class="mbx-hit" data-act="zone" data-zone="4" x="164" y="172" text-anchor="middle" fill="#94a3b8" font-size="8" font-weight="700" style="cursor:pointer">${cLbl[3]}</text>
        <circle class="mbx-hit" data-act="on" cx="100" cy="100" r="44" fill="#232730" stroke="#0f1115" stroke-width="2.5" style="cursor:pointer"/>
        <text x="100" y="105" text-anchor="middle" fill="#94a3b8" font-size="14" pointer-events="none">${m.zonesMode?'◐':'⏻'}</text>
        <text x="100" y="174" text-anchor="middle" fill="#64748b" font-size="8" font-weight="800">MiBOXER®</text>
      </svg>`;
    }

    if(m.layout === 'k3'){
      let ticks = '';
      for(let i=0; i<12; i++){
        const a = (i*30)*Math.PI/180;
        ticks += `<line class="mbx-hit" data-act="brightness" x1="${(100+46*Math.cos(a)).toFixed(1)}" y1="${(100+46*Math.sin(a)).toFixed(1)}" x2="${(100+56*Math.cos(a)).toFixed(1)}" y2="${(100+56*Math.sin(a)).toFixed(1)}" stroke="#475569" stroke-width="2" stroke-linecap="round" style="cursor:pointer"/>`;
      }
      return `<svg viewBox="0 0 200 200" width="${Math.round(200*sc)}" height="${Math.round(200*sc)}">
        ${m.square ? `<rect x="10" y="10" width="180" height="180" rx="8" fill="#252830" stroke="#3f4654" stroke-width="2"/>` : ''}
        <circle cx="100" cy="100" r="64" fill="#1e2128" stroke="#111318" stroke-width="2.5"/>
        ${ticks}
        <circle class="mbx-hit" data-act="on" cx="100" cy="100" r="36" fill="#262a33" stroke="#111318" stroke-width="1.8" style="cursor:pointer"/>
        <text x="100" y="106" text-anchor="middle" fill="#94a3b8" font-size="16" pointer-events="none">⏻</text>
        ${m.square ? `<text x="100" y="182" text-anchor="middle" fill="#94a3b8" font-size="8" font-weight="800">MiBOXER®</text>` : ''}
      </svg>`;
    }

    if(m.layout === 'k2'){
      return `<svg viewBox="0 0 200 200" width="${Math.round(200*sc)}" height="${Math.round(200*sc)}">
        ${m.plate ? `<rect x="10" y="10" width="180" height="180" rx="6" fill="${p.body}" stroke="${p.bezel}" stroke-width="2"/>` : ''}
        <circle cx="100" cy="96" r="${m.plate?64:80}" fill="${p.glass}" stroke="${p.bezel}" stroke-width="2"/>
        <text class="mbx-hit" data-act="brightness" data-val="100" x="100" y="${m.plate?52:42}" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">DIM+</text>
        <text class="mbx-hit" data-act="brightness" data-val="15" x="100" y="${m.plate?146:156}" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">DIM-</text>
        <text class="mbx-hit" data-act="kelvin" data-val="10" x="${m.plate?52:42}" y="99" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">CCT-</text>
        <text class="mbx-hit" data-act="kelvin" data-val="90" x="${m.plate?148:158}" y="99" text-anchor="middle" fill="${p.txt}" font-size="8.5" font-weight="700" style="cursor:pointer">CCT+</text>
        <circle class="mbx-hit" data-act="on" cx="100" cy="96" r="26" fill="${p.body}" stroke="${p.keyStr}" style="cursor:pointer"/>
        <text x="100" y="102" text-anchor="middle" fill="${p.txt}" font-size="15" pointer-events="none">⏻</text>
        ${m.plate ? `<text x="100" y="182" text-anchor="middle" fill="${p.mut}" font-size="8" font-weight="800">MiBOXER®</text>` : ''}
      </svg>`;
    }

    // K1 Square Minimalist Knob
    return `<svg viewBox="0 0 200 200" width="${Math.round(200*sc)}" height="${Math.round(200*sc)}">
      <rect x="12" y="12" width="176" height="176" rx="10" fill="#e5e7eb" stroke="#cbd5e1" stroke-width="2"/>
      <circle class="mbx-hit" data-act="on" cx="100" cy="96" r="52" fill="#f3f4f6" stroke="#d1d5db" stroke-width="2.5" style="cursor:pointer"/>
      <text x="100" y="174" text-anchor="middle" fill="#6b7280" font-size="8.5" font-weight="800">MiBOXER®</text>
    </svg>`;
  }


// === MIBOXER_FACIA_EXTENSION_START ===
  const CAPACITIVE_LAYOUTS = new Set([
    'b8', 'b_strip', 'b0', 'b5', 'u_panel', 'm4',
    'k_display', 's_puck', 'fut087',
    'fut089', 'fut092', 'c_wheel', 'c_slot'
  ]);

  const SINGLE_STICK_ON_LAYOUTS = new Set([
    'b8', 'b_strip', 'b0', 'b5', 'u_panel', 'm4',
    's_puck', 'fut087'
  ]);

  function getStencilFitMode(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || {}) : (modelOrId || {});
    if (m.isCustom) return 'single_sheet';
    return SINGLE_STICK_ON_LAYOUTS.has(m.layout) ? 'single_sheet' : 'cut_stickers';
  }

  function isSingleStickOnStencilReady(modelOrId) {
    return getStencilFitMode(modelOrId) === 'single_sheet';
  }

  function isCapacitiveFlatPanel(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || {}) : (modelOrId || {});
    if (m.isCustom) return true;
    return CAPACITIVE_LAYOUTS.has(m.layout);
  }


  function resolveBaseHardwareId(modelOrId) {
    if (!modelOrId) return '';
    const m = typeof modelOrId === 'string'
      ? (MODELS.find(x => x.id === modelOrId) || { id: modelOrId })
      : modelOrId;
    const bId = m.baseModel || m.base_model || m.base || m.id || '';
    const baseObj = MODELS.find(x => x.id === bId && !x.isCustom) || MODELS.find(x => x.id === String(bId).replace(/-B$/, '') && !x.isCustom);
    return baseObj ? (baseObj.base || baseObj.id) : String(bId).replace(/-B$/, '');
  }

  function areModelsPhysicallyCompatible(customModelOrRem, physModelOrRem) {
    if (!customModelOrRem || !physModelOrRem) return false;
    const getMod = (item) => {
      if (typeof item === 'string') return MODELS.find(x => x.id === item) || { id: item };
      const vm = item.visual_model || item.model || item.id || '';
      const found = MODELS.find(x => x.id === vm);
      if (found) return found;
      if (item.base_model || item.baseModel) {
        const bm = MODELS.find(x => x.id === (item.base_model || item.baseModel));
        if (bm) return Object.assign({}, bm, { id: vm, baseModel: bm.id, isCustom: true });
      }
      return item;
    };
    const cm = getMod(customModelOrRem);
    const pm = getMod(physModelOrRem);
    if (pm.isCustom || String(pm.id || '').startsWith('CUST_')) return false;
    const cBase = resolveBaseHardwareId(cm);
    const pBase = resolveBaseHardwareId(pm);
    if (!cBase || !pBase) return false;
    return cBase.toUpperCase() === pBase.toUpperCase();
  }

  function getPhysicalDims(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || { id: modelOrId }) : (modelOrId || {});
    const layout = m.layout || '';
    if (layout === 'b8') return { width_mm: 106, height_mm: 146, glass_w_mm: 100, glass_h_mm: 140, corner_mm: 4.5, led_x_pct: 50, led_y_pct: 7.5, mount: 'EU/AU Portrait Glass Wall Panel', form_factor: 'EU/AU Portrait Glass Wall Panel' };
    if (layout === 'u_panel') return { width_mm: 70, height_mm: 114, glass_w_mm: 65, glass_h_mm: 109, corner_mm: 4.0, led_x_pct: 50, led_y_pct: 8.5, mount: 'US Single-Gang Glass Wall Panel', form_factor: 'US Single-Gang Glass Wall Panel' };
    if (layout === 'm4') return { width_mm: 142, height_mm: 86, glass_w_mm: 136, glass_h_mm: 80, corner_mm: 4.0, led_x_pct: 8.5, led_y_pct: 11, mount: 'Italian 503/504 Landscape Glass Panel', form_factor: 'Italian 503/504 Landscape Glass Panel' };
    if (layout === 'b_strip' || layout === 'b0' || layout === 'b5') return { width_mm: 86, height_mm: 86, glass_w_mm: 81, glass_h_mm: 81, corner_mm: 4.0, led_x_pct: 50, led_y_pct: 8.0, mount: '86x86mm Square Glass Wall Panel', form_factor: '86x86mm Square Glass Wall Panel' };
    if (layout === 'k_display') return { width_mm: 72, height_mm: 72, glass_w_mm: 66, glass_h_mm: 66, corner_mm: 36.0, shape: 'round', led_x_pct: 50, led_y_pct: 27, mount: '72mm Round Rotary Display Puck', form_factor: '72mm Round Rotary Knob / Puck' };
    if (layout === 's_puck') return { width_mm: 70, height_mm: 70, glass_w_mm: 64, glass_h_mm: 64, corner_mm: 35.0, shape: 'round', led_x_pct: 50, led_y_pct: 16, mount: '70mm Round Magnetic Touch Puck', form_factor: '70mm Round Capacitive Puck' };
    if (layout === 'fut087') return { width_mm: 80, height_mm: 80, glass_w_mm: 62, glass_h_mm: 62, corner_mm: 40.0, shape: 'round', led_x_pct: 50, led_y_pct: 18, mount: '80mm Round Dimmer Puck', form_factor: '80mm Round Touch Dimmer' };
    if (layout === 'fut089') return { width_mm: 47, height_mm: 153, glass_w_mm: 43, glass_h_mm: 148, corner_mm: 10.0, led_x_pct: 50, led_y_pct: 5.5, mount: 'Handheld 8-Zone Remote Overlay', form_factor: '47x153mm Handheld 8-Zone Remote' };
    if (layout === 'fut092' || layout === 'c_wheel' || layout === 'c_slot') return { width_mm: 47, height_mm: 138, glass_w_mm: 43, glass_h_mm: 133, corner_mm: 10.0, led_x_pct: 50, led_y_pct: 6.0, mount: 'Handheld 4-Zone Remote Overlay', form_factor: '47x138mm Handheld Capacitive Remote' };
    return { width_mm: 86, height_mm: 86, glass_w_mm: 80, glass_h_mm: 80, corner_mm: 4.0, mount: 'Standard Panel', form_factor: 'Standard Remote / Wall Panel' };
  }

  // Standardized 24x24 Architectural Vector Icons: Rooms (Group A) + Devices & Controls (Group B)
  const FACIA_ICONS = {
    none:       { group: 'basic',  name: 'No Icon (Text Only)', svg: '' },
    // --- Standardized Room Icons (for Multi-Zone Remotes) ---
    home_all:   { group: 'room',   name: 'Whole Home / All', svg: '<path d="M3 10.5L12 3l9 7.5V20a1 1 0 0 1-1 1h-5.5v-6h-5v6H4a1 1 0 0 1-1-1v-9.5z" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linejoin="round"/>' },
    sofa:       { group: 'room',   name: 'Living / Lounge', svg: '<path d="M5 11V8a2 2 0 0 1 2-2h10a2 2 0 0 1 2 2v3m-16 0h18v5H3v-5zm2 5v2.5m14-2.5v2.5" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    kitchen:    { group: 'room',   name: 'Kitchen', svg: '<rect x="4" y="4" width="16" height="16" rx="2" fill="none" stroke="currentColor" stroke-width="1.9"/><line x1="4" y1="10" x2="20" y2="10" stroke="currentColor" stroke-width="1.8"/><circle cx="8" cy="7" r="1" fill="currentColor"/><circle cx="12" cy="7" r="1" fill="currentColor"/><line x1="9" y1="13" x2="15" y2="13" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"/>' },
    dining:     { group: 'room',   name: 'Dining Room', svg: '<path d="M7 3v7a2 2 0 0 0 4 0V3M9 3v18m8-18c-2.2 2-2.2 6 0 8v10" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    bed:        { group: 'room',   name: 'Master Bedroom', svg: '<path d="M3 7v11m0-4h18m0 4v-7a2 2 0 0 0-2-2h-8v5M7 11a2 2 0 1 0 0-4 2 2 0 0 0 0 4z" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    bed_single: { group: 'room',   name: 'Bedroom 2 / Guest', svg: '<path d="M4 9v9m16-7v7M4 14h16M6 14v-3a2 2 0 0 1 2-2h8a2 2 0 0 1 2 2v3" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    bath:       { group: 'room',   name: 'Bathroom / Ensuite', svg: '<path d="M4 12h16v3a4 4 0 0 1-4 4H8a4 4 0 0 1-4-4v-3zm2-6a2 2 0 0 1 4 0v1M6 12V6m2 13v2m8-2v2" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    desk:       { group: 'room',   name: 'Study / Office', svg: '<rect x="4" y="4" width="16" height="10" rx="1.5" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M12 14v4m-4 0h8M3 20h18" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    stairs:     { group: 'room',   name: 'Hallway / Stairs', svg: '<path d="M4 19h4v-4h4v-4h4V7h4" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>' },
    garden:     { group: 'room',   name: 'Patio / Outdoor', svg: '<path d="M12 21v-8m0 0C8 13 6 9.5 6 6c3.5 0 6 2 6 7zm0 0c4 0 6-3.5 6-7-3.5 0-6 2-6 7z" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    tv:         { group: 'room',   name: 'Cinema / Media', svg: '<rect x="3" y="5" width="18" height="12" rx="2" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M8 21h8m-4-4v4" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    garage:     { group: 'room',   name: 'Garage / Entry', svg: '<path d="M3 10L12 4l9 6v10H3V10zm3 3h12m-12 3.5h12" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },

    // --- Multi-Function Room Devices (Tap = +1 Step Cycle, Hold = ON/OFF Toggle) ---
    fan:        { group: 'device', name: 'Ceiling Fan (1-6 / Hold)', svg: '<circle cx="12" cy="12" r="2" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M12 10c-2-3.5 1-6.5 3.5-5 1.8 1 .8 4-1.5 5m2 2c3.5-2 6.5 1 5 3.5-1 1.8-4 .8-5-1.5m-2 2c2 3.5-1 6.5-3.5 5-1.8-1-.8-4 1.5-5m-2-2c-3.5 2-6.5-1-5-3.5 1-1.8 4-.8 5 1.5" fill="none" stroke="currentColor" stroke-width="1.85" stroke-linecap="round"/>' },
    blinds:     { group: 'device', name: 'Roller Blinds (Step / Hold)', svg: '<path d="M4 4h16M5 8h14M5 12h14M5 16h14M12 16v4m-2 0h4" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    curtains:   { group: 'device', name: 'Motor Curtains', svg: '<path d="M3 4h18M5 4c0 6 3 9 3 16M19 4c0 6-3 9-3 16M5 13h3m8 0h3" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    ac:         { group: 'device', name: 'AC / Climate', svg: '<path d="M12 3v18M4.2 7.5l15.6 9M19.8 7.5l-15.6 9M9.5 5L12 7.5 14.5 5M9.5 19L12 16.5 14.5 19" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"/>' },
    heater:     { group: 'device', name: 'Heater / Towel Rail', svg: '<path d="M7 20V6a2 2 0 0 1 4 0v14a2 2 0 0 0 4 0V6a2 2 0 0 1 4 0v14" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    speaker:    { group: 'device', name: 'Sonos / Audio', svg: '<rect x="6" y="3" width="12" height="18" rx="2" fill="none" stroke="currentColor" stroke-width="1.9"/><circle cx="12" cy="14.5" r="3" fill="none" stroke="currentColor" stroke-width="1.8"/><circle cx="12" cy="7.5" r="1" fill="currentColor"/>' },
    music:      { group: 'device', name: 'Music / Playlist', svg: '<path d="M9 18V6l11-2v12" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/><circle cx="6.5" cy="18" r="2.4" fill="none" stroke="currentColor" stroke-width="1.9"/><circle cx="17.5" cy="16" r="2.4" fill="none" stroke="currentColor" stroke-width="1.9"/>' },
    pendant:    { group: 'device', name: 'Pendant / Chandelier', svg: '<path d="M12 3v5m-6 6a6 6 0 0 1 12 0H6zm4 0a2 2 0 0 0 4 0" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    downlight:  { group: 'device', name: 'Recessed Downlights', svg: '<ellipse cx="12" cy="8" rx="7" ry="3" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M8 13l-2 5m6-4v5m4-6l2 5" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    strip:      { group: 'device', name: 'LED Cove / Strip', svg: '<rect x="3" y="9" width="18" height="6" rx="2" fill="none" stroke="currentColor" stroke-width="1.9"/><circle cx="7.5" cy="12" r="1" fill="currentColor"/><circle cx="12" cy="12" r="1" fill="currentColor"/><circle cx="16.5" cy="12" r="1" fill="currentColor"/>' },
    lamp:       { group: 'device', name: 'Floor / Reading Lamp', svg: '<path d="M7 4h10l2.5 7h-15L7 4zm5 7v9m-4 0h8" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    bulb:       { group: 'device', name: 'Smart Bulb', svg: '<path d="M9 18h6m-5 3h4M12 3a6 6 0 0 0-3.8 10.6c.7.6 1.3 1.5 1.5 2.4h4.6c.2-.9.8-1.8 1.5-2.4A6 6 0 0 0 12 3z" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    extractor:  { group: 'device', name: 'Exhaust / Ventilation', svg: '<rect x="4" y="4" width="16" height="16" rx="3" fill="none" stroke="currentColor" stroke-width="1.8"/><circle cx="12" cy="12" r="4.5" fill="none" stroke="currentColor" stroke-width="1.8"/><path d="M12 7.5v9M7.5 12h9" stroke="currentColor" stroke-width="1.6"/>' },
    power:      { group: 'device', name: 'Master Power', svg: '<path d="M12 3v9m6.36-5.36a9 9 0 1 1-12.73 0" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>' },
    sun:        { group: 'device', name: 'Bright / Daylight', svg: '<circle cx="12" cy="12" r="4" fill="none" stroke="currentColor" stroke-width="2"/><path d="M12 2v2.5M12 19.5V22M4.9 4.9l1.8 1.8M17.3 17.3l1.8 1.8M2 12h2.5M19.5 12H22M4.9 19.1l1.8-1.8M17.3 6.7l1.8-1.8" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    moon:       { group: 'device', name: 'Night / Sleep', svg: '<path d="M20 14.5A8.5 8.5 0 0 1 9.5 4a8.5 8.5 0 1 0 10.5 10.5z" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linejoin="round"/>' },
    sparkles:   { group: 'device', name: 'Scene / Mood', svg: '<path d="M12 3l1.8 4.7L18.5 9.5l-4.7 1.8L12 16l-1.8-4.7L5.5 9.5l4.7-1.8L12 3zm6 12l.9 2.1L21 18l-2.1.9L18 21l-.9-2.1L15 18l2.1-.9L18 15z" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"/>' },
    coffee:     { group: 'device', name: 'Morning Scene', svg: '<path d="M17 8h1a3 3 0 0 1 0 6h-1M5 8h12v6a5 5 0 0 1-5 5H10a5 5 0 0 1-5-5V8zm-1 13h14" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/>' },
    timer:      { group: 'device', name: 'Delay Off / Timer', svg: '<circle cx="12" cy="13" r="8" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M12 9v4l2.5 2M9.5 2h5" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    lock:       { group: 'device', name: 'Away / Lock', svg: '<rect x="5" y="11" width="14" height="9" rx="2" fill="none" stroke="currentColor" stroke-width="1.9"/><path d="M8 11V7a4 4 0 0 1 8 0v4" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round"/>' },
    warm_cool:  { group: 'device', name: 'Warm / Cool CCT', svg: '<path d="M12 4a8 8 0 1 0 0 16 8 8 0 0 0 0-16zm0 0v16" fill="none" stroke="currentColor" stroke-width="1.9"/>' }
  };

  const FACIA_THEMES = {
    pure_white:      { name: 'Pure White Architectural',  bg: '#f1f5f9', glass: '#ffffff', bezel: '#cbd5e1', padFill: '#f8fafc', padStroke: '#cbd5e1', ink: '#0f172a', subInk: '#64748b', accent: '#0284c7', mono: false },
    matte_black:     { name: 'Matte Black Glass',         bg: '#0f1115', glass: '#171a21', bezel: '#2d3340', padFill: '#212631', padStroke: '#384152', ink: '#f8fafc', subInk: '#94a3b8', accent: '#38bdf8', mono: false },
    ink_bw:          { name: 'Crisp B&W Stencil (Print)', bg: '#ffffff', glass: '#ffffff', bezel: '#0f172a', padFill: '#ffffff', padStroke: '#0f172a', ink: '#000000', subInk: '#334155', accent: '#000000', mono: true },
    reverse_bw:      { name: 'White Ink on Black Vinyl',  bg: '#000000', glass: '#09090b', bezel: '#ffffff', padFill: '#121215', padStroke: '#d4d4d8', ink: '#ffffff', subInk: '#a1a1aa', accent: '#ffffff', mono: true },
    grayscale_slate: { name: 'Soft Grayscale Slate',      bg: '#e4e4e7', glass: '#f4f4f5', bezel: '#a1a1aa', padFill: '#ffffff', padStroke: '#a1a1aa', ink: '#18181b', subInk: '#52525b', accent: '#27272a', mono: true },
    warm_stone:      { name: 'Warm Travertine / Linen',   bg: '#e7e2d8', glass: '#f7f4ee', bezel: '#c7beae', padFill: '#efeae0', padStroke: '#c7beae', ink: '#292524', subInk: '#57534e', accent: '#b45309', mono: false },
    charcoal_brass:  { name: 'Charcoal & Brushed Brass',  bg: '#161618', glass: '#1e1e22', bezel: '#856a3f', padFill: '#26252b', padStroke: '#a18049', ink: '#fef3c7', subInk: '#d6c7a9', accent: '#eab308', mono: false }
  };

  function polarPt(cx, cy, r, deg) {
    const rad = ((deg - 90) * Math.PI) / 180;
    return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) };
  }

  function ringWedgePath(cx, cy, rIn, rOut, startDeg, endDeg) {
    const p1 = polarPt(cx, cy, rOut, endDeg), p2 = polarPt(cx, cy, rOut, startDeg);
    const p3 = polarPt(cx, cy, rIn, startDeg), p4 = polarPt(cx, cy, rIn, endDeg);
    const lg = endDeg - startDeg <= 180 ? '0' : '1';
    return `M ${p1.x.toFixed(2)} ${p1.y.toFixed(2)} A ${rOut.toFixed(2)} ${rOut.toFixed(2)} 0 ${lg} 0 ${p2.x.toFixed(2)} ${p2.y.toFixed(2)} L ${p3.x.toFixed(2)} ${p3.y.toFixed(2)} A ${rIn.toFixed(2)} ${rIn.toFixed(2)} 0 ${lg} 1 ${p4.x.toFixed(2)} ${p4.y.toFixed(2)} Z`;
  }

  function escXml(s) {
    return String(s == null ? '' : s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  // Individual cut-sticker sheet builder for knobs, LCDs, rockers, physical buttons & handhelds
  function getCutStickerSheet(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || MODELS[0]) : (modelOrId || MODELS[0]);
    const dims = getPhysicalDims(m);
    const bp = getPanelBlueprint(m);
    const stickers = [];

    if (Array.isArray(bp.slots) && bp.slots.length > 0) {
      bp.slots.forEach((sl, idx) => {
        const wMm = sl.w ? Math.max(8, Math.round((sl.w / (bp.vw || 190)) * (dims.width_mm || 72) * 10) / 10) : 12;
        const hMm = sl.h ? Math.max(8, Math.round((sl.h / (bp.vh || 190)) * (dims.height_mm || 72) * 10) / 10) : wMm;
        stickers.push({
          key: sl.key || `stk_${idx + 1}`,
          label: sl.label || `Key ${idx + 1}`,
          shape: sl.type === 'circle' ? 'circle' : 'rect',
          width_mm: wMm,
          height_mm: hMm,
          action: sl.act || 'button_press',
          zone: sl.zone != null ? sl.zone : null
        });
      });
    } else {
      const zCount = Math.min(8, Math.max(1, Number(m.zones || 4)));
      stickers.push({ key: 'stk_on', label: 'ON', shape: 'rect', width_mm: 14, height_mm: 10, action: 'on', zone: null });
      stickers.push({ key: 'stk_off', label: 'OFF', shape: 'rect', width_mm: 14, height_mm: 10, action: 'off', zone: null });
      for (let z = 1; z <= zCount; z++) {
        stickers.push({ key: `stk_z${z}`, label: `Z${z}`, shape: 'rect', width_mm: 12, height_mm: 10, action: 'zone', zone: z });
      }
    }

    return {
      model_id: m.id,
      model_name: m.name || m.id,
      layout: m.layout || 'standard',
      fit_mode: getStencilFitMode(m),
      base_dims_mm: { width_mm: dims.width_mm, height_mm: dims.height_mm },
      sticker_count: stickers.length,
      stickers
    };
  }

  function renderCutStickerSheetSvg(modelOrId, opts) {
    const o = opts || {};
    const sheet = getCutStickerSheet(modelOrId);
    const cols = 4;
    const cellW = 54;
    const cellH = 44;
    const padX = 14;
    const padY = 34;
    const rows = Math.max(1, Math.ceil(sheet.stickers.length / cols));
    const vw = padX * 2 + cols * cellW;
    const vh = padY + rows * cellH + 28;

    let itemsSvg = '';
    sheet.stickers.forEach((stk, i) => {
      const r = Math.floor(i / cols);
      const c = i % cols;
      const cx = padX + c * cellW + cellW / 2;
      const cy = padY + r * cellH + cellH / 2 - 4;
      const sw = Math.min(40, Math.max(18, stk.width_mm * 1.4));
      const sh = Math.min(24, Math.max(14, stk.height_mm * 1.4));
      if (stk.shape === 'circle') {
        const rad = Math.min(sw, sh) / 2;
        itemsSvg += `<circle cx="${cx.toFixed(1)}" cy="${cy.toFixed(1)}" r="${rad.toFixed(1)}" fill="#ffffff" stroke="#0f172a" stroke-width="1.2" stroke-dasharray="3,1.5"/>`;
      } else {
        itemsSvg += `<rect x="${(cx - sw / 2).toFixed(1)}" y="${(cy - sh / 2).toFixed(1)}" width="${sw.toFixed(1)}" height="${sh.toFixed(1)}" rx="3" fill="#ffffff" stroke="#0f172a" stroke-width="1.2" stroke-dasharray="3,1.5"/>`;
      }
      itemsSvg += `<text x="${cx.toFixed(1)}" y="${(cy + 2.5).toFixed(1)}" text-anchor="middle" font-family="Inter,system-ui,sans-serif" font-size="7.5" font-weight="700" fill="#0f172a">${escXml(stk.label)}</text>`;
      itemsSvg += `<text x="${cx.toFixed(1)}" y="${(cy + sh / 2 + 9).toFixed(1)}" text-anchor="middle" font-family="monospace" font-size="5.5" fill="#475569">${stk.width_mm}x${stk.height_mm}mm</text>`;
    });

    const calY = vh - 20;
    return `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${vw} ${vh}" width="${o.width || vw * 2}" height="${o.height || vh * 2}">
      <rect x="1" y="1" width="${vw - 2}" height="${vh - 2}" rx="6" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1"/>
      <text x="${padX}" y="17" font-family="Inter,system-ui,sans-serif" font-size="8.5" font-weight="800" fill="#0f172a">${escXml(sheet.model_name)} — Cut Stickers (${sheet.sticker_count})</text>
      ${itemsSvg}
      <rect x="${padX}" y="${calY}" width="12" height="12" fill="none" stroke="#0f172a" stroke-width="1"/>
      <text x="${padX + 16}" y="${calY + 8.5}" font-family="monospace" font-size="6" fill="#334155">10x10mm Scale Check (Print 100%)</text>
    </svg>`;
  }

  // Exact hardware stencil geometry for every capacitive flat panel layout!
  function getPanelBlueprint(model) {
    const m = typeof model === 'string' ? (MODELS.find(x => x.id === model) || MODELS[0]) : (model || MODELS[0]);
    if (m.layout === 'b8') {
      const slots = [
        { key: 'sat_bar', type: 'vbar', surf: 'sat_bar', act: 'saturation', x: 23, y: 42, w: 15, h: 86, label: 'SAT' },
        { key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 95, cy: 85, rIn: 21, rOut: 43, label: 'COLOR' },
        { key: 'center_w', type: 'circle', act: 'set_white', cx: 95, cy: 85, r: 14.5, label: 'W' },
        { key: 'bri_bar', type: 'vbar', surf: 'bri_bar', act: 'brightness', x: 152, y: 42, w: 15, h: 86, label: 'DIM' }
      ];
      [['btn_on','ON','on'],['btn_off','OFF','off'],['btn_sdn','S-','mode_speed_down'],['btn_m','M','mode'],['btn_sup','S+','mode_speed_up']].forEach(([k,l,a], i) => {
        slots.push({ key: k, type: 'rect', act: a, x: 21 + i * 30.5, y: 146, w: 26.5, h: 19, rx: 3.5, label: l });
      });
      const r1 = [['zone_0','All',0,'zone'],['zone_1','1',1,'zone'],['zone_2','2',2,'zone'],['zone_3','3',3,'zone'],['zone_4','4',4,'zone']];
      const r2 = [['btn_60s','60S',null,'night_mode','Delay'],['zone_5','5',5,'zone'],['zone_6','6',6,'zone'],['zone_7','7',7,'zone'],['zone_8','8',8,'zone']];
      [r1, r2].forEach((row, ri) => {
        row.forEach(([k, lbl, z, act, sub], ci) => {
          slots.push({ key: k, type: 'circle', act, zone: z, cx: 34 + ci * 30.5, cy: 185 + ri * 31.5, r: 12.6, label: lbl, sublabel: sub || '' });
        });
      });
      return { vw: 190, vh: 250, outer: { x: 4, y: 4, w: 182, h: 242, rx: 8 }, glass: { x: 9, y: 9, w: 172, h: 232, rx: 5 }, titleY: 24, slots };
    }

    if (m.layout === 'b0') {
      const slots = [
        { key: 'sat_bar', type: 'vbar', surf: 'sat_bar', act: 'saturation', x: 28, y: 43, w: 15, h: 80, label: 'CCT' },
        { key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 105, cy: 83, rIn: 19.5, rOut: 41, label: 'COLOR' },
        { key: 'center_w', type: 'circle', act: 'set_white', cx: 105, cy: 83, r: 14, label: 'W' },
        { key: 'bri_bar', type: 'vbar', surf: 'bri_bar', act: 'brightness', x: 167, y: 43, w: 15, h: 80, label: 'DIM' },
        { key: 'btn_on',  type: 'rect', act: 'on',  x: 26,  y: 137, w: 35, h: 21, rx: 4, label: 'ON' },
        { key: 'btn_r',   type: 'rect', act: 'hue', val: 0,   x: 68,  y: 137, w: 35, h: 21, rx: 4, label: 'R' },
        { key: 'btn_g',   type: 'rect', act: 'hue', val: 120, x: 110, y: 137, w: 35, h: 21, rx: 4, label: 'G' },
        { key: 'btn_b',   type: 'rect', act: 'hue', val: 240, x: 152, y: 137, w: 35, h: 21, rx: 4, label: 'B' },
        { key: 'btn_off', type: 'rect', act: 'off', x: 26,  y: 164, w: 35, h: 21, rx: 4, label: 'OFF' },
        { key: 'btn_m',   type: 'rect', act: 'mode', x: 68, y: 164, w: 35, h: 21, rx: 4, label: 'M' },
        { key: 'btn_sdn', type: 'rect', act: 'mode_speed_down', x: 110, y: 164, w: 35, h: 21, rx: 4, label: 'S-', sublabel: '60s' },
        { key: 'btn_sup', type: 'rect', act: 'mode_speed_up',   x: 152, y: 164, w: 35, h: 21, rx: 4, label: 'S+', sublabel: '10m' }
      ];
      return { vw: 210, vh: 210, outer: { x: 6, y: 6, w: 198, h: 198, rx: 11 }, glass: { x: 11, y: 11, w: 188, h: 188, rx: 8 }, titleY: 27, slots };
    }

    if (m.layout === 'b_strip') {
      const slots = [];
      if (m.kind === 'b1') {
        slots.push({ key: 'bri_bar', type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 26, y: 52, w: 158, h: 20, label: 'DIMMER' });
      } else if (m.kind === 'b2') {
        slots.push({ key: 'cct_bar', type: 'hbar', surf: 'cct_bar', act: 'kelvin', x: 26, y: 40, w: 158, h: 18, label: 'CCT' });
        slots.push({ key: 'bri_bar', type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 26, y: 68, w: 158, h: 18, label: 'DIM' });
      } else if (m.kind === 'b3') {
        slots.push({ key: 'hue_ring', type: 'hbar', surf: 'hue_ring', act: 'hue', x: 26, y: 40, w: 158, h: 18, label: 'COLOR' });
        slots.push({ key: 'bri_bar',  type: 'hbar', surf: 'bri_bar',  act: 'brightness', x: 26, y: 68, w: 158, h: 18, label: 'DIM' });
      } else {
        // B4 / T4
        slots.push({ key: 'hue_ring', type: 'hbar', surf: 'hue_ring', act: 'hue', x: 26, y: 39, w: 158, h: 18, label: 'COLOR' });
        slots.push({ key: 'cct_bar',  type: 'hbar', surf: 'cct_bar',  act: 'kelvin', x: 26, y: 66, w: 75, h: 18, label: 'CCT' });
        slots.push({ key: 'bri_bar',  type: 'hbar', surf: 'bri_bar',  act: 'brightness', x: 109, y: 66, w: 75, h: 18, label: 'DIM' });
      }
      if (m.kind === 'b1' || m.kind === 'b2') {
        slots.push({ key: 'btn_on',  type: 'rect', act: 'on', x: 26, y: 98, w: 46, h: 24, rx: 4, label: 'ON' });
        slots.push({ key: 'btn_60s', type: 'rect', act: 'night_mode', x: 82, y: 98, w: 46, h: 24, rx: 4, label: '60S', sublabel: 'Delay' });
        slots.push({ key: 'btn_off', type: 'rect', act: 'off', x: 138, y: 98, w: 46, h: 24, rx: 4, label: 'OFF' });
        for (let z = 1; z <= 4; z++) {
          slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x: 26 + (z - 1) * 43, y: 134, w: 29, h: 52, rx: 6, label: `${z}` });
        }
      } else {
        [['btn_w','White','set_white'],['btn_m','Mode','mode'],['btn_sdn','Spd-','mode_speed_down'],['btn_sup','Spd+','mode_speed_up']].forEach(([k,l,a], i) => {
          slots.push({ key: k, type: 'rect', act: a, x: 26 + i * 41, y: 98, w: 35, h: 21, rx: 4, label: l });
        });
        [['zone_0','All',0],['zone_1','1',1],['zone_2','2',2],['zone_3','3',3],['zone_4','4',4]].forEach(([k,l,z], i) => {
          slots.push({ key: k, type: 'rocker', act: 'zone', zone: z, x: 26 + i * 33.5, y: 132, w: 24, h: 54, rx: 5, label: l });
        });
      }
      return { vw: 210, vh: 210, outer: { x: 6, y: 6, w: 198, h: 198, rx: 11 }, glass: { x: 11, y: 11, w: 188, h: 188, rx: 8 }, titleY: 27, slots };
    }

    if (m.layout === 'u_panel') {
      const slots = [];
      if (m.kind === 'u1') {
        slots.push({ key: 'bri_bar', type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 24, y: 56, w: 122, h: 20, label: 'DIM' });
      } else if (m.kind === 'u2') {
        slots.push({ key: 'cct_bar', type: 'hbar', surf: 'cct_bar', act: 'kelvin', x: 24, y: 40, w: 122, h: 18, label: 'CCT' });
        slots.push({ key: 'bri_bar', type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 24, y: 68, w: 122, h: 18, label: 'DIM' });
      } else {
        slots.push({ key: 'hue_ring', type: 'hbar', surf: 'hue_ring', act: 'hue', x: 24, y: 40, w: 122, h: 18, label: 'COLOR' });
        slots.push({ key: 'bri_bar',  type: 'hbar', surf: 'bri_bar',  act: 'brightness', x: 24, y: 68, w: 122, h: 18, label: 'DIM' });
      }
      const mid = m.kind === 'u3' ? ['btn_w','W','set_white'] : ['btn_60s','60S','night_mode'];
      const r2 = m.kind === 'u3' ? [['btn_sdn','S-','mode_speed_down'],['btn_m','M','mode'],['btn_sup','S+','mode_speed_up']] : [['btn_s1','S1','mode'],['btn_s2','S2','mode'],['btn_s3','S3','mode']];
      slots.push({ key: 'btn_on', type: 'rect', act: 'on', x: 24, y: 100, w: 34, h: 22, rx: 4, label: 'ON' });
      slots.push({ key: mid[0],   type: 'rect', act: mid[2], x: 68, y: 100, w: 34, h: 22, rx: 4, label: mid[1] });
      slots.push({ key: 'btn_off',type: 'rect', act: 'off', x: 112, y: 100, w: 34, h: 22, rx: 4, label: 'OFF' });
      r2.forEach(([k,l,a], i) => slots.push({ key: k, type: 'rect', act: a, x: 24 + i * 44, y: 130, w: 34, h: 22, rx: 4, label: l }));
      for (let z = 1; z <= 3; z++) slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x: 24 + (z - 1) * 44, y: 162, w: 34, h: 54, rx: 6, label: `${z}` });
      return { vw: 170, vh: 240, outer: { x: 8, y: 6, w: 154, h: 228, rx: 10 }, glass: { x: 13, y: 11, w: 144, h: 218, rx: 7 }, titleY: 26, slots };
    }

    if (m.layout === 'm4') {
      const slots = [
        { key: 'hue_ring', type: 'hbar', surf: 'hue_ring', act: 'hue', x: 20, y: 36, w: 196, h: 17, label: 'COLOR' },
        { key: 'cct_bar',  type: 'hbar', surf: 'cct_bar',  act: 'kelvin', x: 20, y: 61, w: 92, h: 18, label: 'CCT' },
        { key: 'bri_bar',  type: 'hbar', surf: 'bri_bar',  act: 'brightness', x: 124, y: 61, w: 92, h: 18, label: 'DIM' },
        { key: 'zone_0',   type: 'rocker', act: 'zone', zone: 0, x: 20, y: 94, w: 24, h: 72, rx: 4, label: 'All' }
      ];
      for (let z = 1; z <= 4; z++) slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x: 20 + z * 29, y: 94, w: 24, h: 72, rx: 4, label: `${z}` });
      slots.push({ key: 'btn_w',   type: 'rect', act: 'set_white', x: 165, y: 94, w: 23, h: 34, rx: 4, label: 'W' });
      slots.push({ key: 'btn_m',   type: 'rect', act: 'mode',      x: 165, y: 132, w: 23, h: 34, rx: 4, label: 'M' });
      slots.push({ key: 'btn_sup', type: 'rect', act: 'mode_speed_up',   x: 193, y: 94, w: 23, h: 34, rx: 4, label: 'S+' });
      slots.push({ key: 'btn_sdn', type: 'rect', act: 'mode_speed_down', x: 193, y: 132, w: 23, h: 34, rx: 4, label: 'S-' });
      return { vw: 236, vh: 184, outer: { x: 6, y: 6, w: 224, h: 172, rx: 10 }, glass: { x: 11, y: 11, w: 214, h: 162, rx: 7 }, titleY: 25, slots };
    }

    if (m.layout === 'k_display') {
      const isK6 = m.kind === 'k6';
      const slots = [];
      if (isK6) {
        slots.push({ key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 95, cy: 95, rIn: 65, rOut: 81, label: 'COLOR' });
      }
      // Inner buttons positioned cleanly inside rIn=65 and clear of center LED cutout (y=56..74)
      slots.push(
        { key: 'btn_on',  type: 'circle', act: 'on',          cx: 95,  cy: 103, r: 12.2, label: 'PWR', sublabel: 'Hold Off' },
        { key: 'btn_bri', type: 'circle', act: 'brightness',  cx: 62,  cy: 56,  r: 11.0, label: 'BRI' },
        { key: 'btn_cct', type: 'circle', act: 'kelvin',      cx: 128, cy: 56,  r: 11.0, label: 'CCT' },
        { key: 'btn_pal', type: 'circle', act: 'set_white',   cx: 49,  cy: 95,  r: 11.0, label: isK6 ? 'RGB/W' : 'Z1' },
        { key: 'btn_sat', type: 'circle', act: 'saturation',  cx: 141, cy: 95,  r: 11.0, label: isK6 ? 'SAT' : 'Z4' },
        { key: 'btn_m',   type: 'circle', act: 'mode',        cx: 65,  cy: 133, r: 11.0, label: isK6 ? 'MODE' : 'Z2' },
        { key: 'btn_dly', type: 'circle', act: 'night_mode',  cx: 125, cy: 133, r: 11.0, label: isK6 ? 'DELAY' : 'Z3' }
      );
      return {
        vw: 190, vh: 190,
        outer: { x: 10, y: 10, w: 170, h: 170, rx: 85 },
        glass: { x: 14, y: 14, w: 162, h: 162, rx: 81 },
        cutout: { x: 75, y: 57, w: 40, h: 18, rx: 4.5, text: '100%' },
        titleY: 169,
        slots
      };
    }

    if (m.layout === 's_puck') {
      const isRgb = m.kind === 's2';
      const slots = [
        isRgb
          ? { key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 95, cy: 95, rIn: 54, rOut: 78, label: 'COLOR' }
          : { key: 'cct_bar',  type: 'ring', surf: 'cct_bar',  act: 'kelvin', cx: 95, cy: 95, rIn: 54, rOut: 78, label: 'CCT' },
        { key: 'btn_on',  type: 'circle', act: 'on',         cx: 95, cy: 74,  r: 15, label: 'ON' },
        { key: 'btn_off', type: 'circle', act: 'off',        cx: 95, cy: 116, r: 15, label: 'OFF' },
        { key: 'btn_sdn', type: 'circle', act: 'brightness', cx: 63, cy: 95,  r: 12, label: 'DIM-' },
        { key: 'btn_sup', type: 'circle', act: 'mode',       cx: 127, cy: 95, r: 12, label: isRgb ? 'MODE' : 'DIM+' }
      ];
      return {
        vw: 190, vh: 190,
        outer: { x: 11, y: 11, w: 168, h: 168, rx: 84 },
        glass: { x: 16, y: 16, w: 158, h: 158, rx: 79 },
        titleY: 42,
        slots
      };
    }

    if (m.layout === 'fut087') {
      const slots = [
        { key: 'bri_bar', type: 'ring', surf: 'bri_bar', act: 'brightness', cx: 95, cy: 95, rIn: 48, rOut: 76, label: 'DIMMER' },
        { key: 'btn_on',  type: 'circle', act: 'on', cx: 95, cy: 95, r: 28, label: 'ON / OFF', sublabel: 'Hold Night' }
      ];
      return {
        vw: 190, vh: 190,
        outer: { x: 11, y: 11, w: 168, h: 168, rx: 84 },
        glass: { x: 17, y: 17, w: 156, h: 156, rx: 78 },
        titleY: 36,
        slots
      };
    }

    if (m.layout === 'fut089') {
      const slots = [
        { key: 'btn_on',   type: 'rect', act: 'on',  x: 24, y: 24, w: 46, h: 20, rx: 10, label: 'ALL ON' },
        { key: 'btn_off',  type: 'rect', act: 'off', x: 90, y: 24, w: 46, h: 20, rx: 10, label: 'ALL OFF' },
        { key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 80, cy: 86, rIn: 19, rOut: 38, label: 'COLOR' },
        { key: 'center_w', type: 'circle', act: 'set_white', cx: 80, cy: 86, r: 13, label: 'W' },
        { key: 'bri_bar',  type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 24, y: 132, w: 112, h: 15, label: 'BRIGHTNESS' },
        { key: 'cct_bar',  type: 'hbar', surf: 'cct_bar', act: 'kelvin',     x: 24, y: 153, w: 112, h: 15, label: 'WARM / COOL' },
        { key: 'btn_sdn',  type: 'rect', act: 'mode_speed_down', x: 24, y: 175, w: 34, h: 18, rx: 4, label: 'S-' },
        { key: 'btn_m',    type: 'rect', act: 'mode',            x: 63, y: 175, w: 34, h: 18, rx: 4, label: 'M' },
        { key: 'btn_sup',  type: 'rect', act: 'mode_speed_up',   x: 102, y: 175, w: 34, h: 18, rx: 4, label: 'S+' }
      ];
      for (let z = 1; z <= 8; z++) {
        const row = z <= 4 ? 0 : 1;
        const colIdx = (z - 1) % 4;
        slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x: 24 + colIdx * 29, y: 200 + row * 44, w: 25, h: 38, rx: 5, label: `${z}` });
      }
      return { vw: 160, vh: 300, outer: { x: 10, y: 6, w: 140, h: 288, rx: 22 }, glass: { x: 14, y: 10, w: 132, h: 280, rx: 19 }, titleY: 19, slots };
    }

    if (m.layout === 'fut092' || m.layout === 'c_wheel' || m.layout === 'c_slot') {
      const slots = [
        { key: 'btn_on',   type: 'rect', act: 'on',  x: 24, y: 26, w: 46, h: 20, rx: 10, label: 'ALL ON' },
        { key: 'btn_off',  type: 'rect', act: 'off', x: 90, y: 26, w: 46, h: 20, rx: 10, label: 'ALL OFF' },
        m.layout === 'c_slot'
          ? { key: 'hue_ring', type: 'hbar', surf: 'hue_ring', act: 'hue', x: 24, y: 64, w: 112, h: 22, label: 'COLOR' }
          : { key: 'hue_ring', type: 'ring', surf: 'hue_ring', act: 'hue', cx: 80, cy: 88, rIn: 19, rOut: 38, label: 'COLOR' },
        { key: 'bri_bar',  type: 'hbar', surf: 'bri_bar', act: 'brightness', x: 24, y: 136, w: 112, h: 16, label: 'BRIGHTNESS' },
        { key: 'cct_bar',  type: 'hbar', surf: 'cct_bar', act: 'kelvin',     x: 24, y: 158, w: 112, h: 16, label: 'CCT / SAT' },
        { key: 'btn_w',    type: 'rect', act: 'set_white',       x: 24, y: 182, w: 34, h: 19, rx: 4, label: 'W' },
        { key: 'btn_m',    type: 'rect', act: 'mode',            x: 63, y: 182, w: 34, h: 19, rx: 4, label: 'M' },
        { key: 'btn_sup',  type: 'rect', act: 'mode_speed_up',   x: 102, y: 182, w: 34, h: 19, rx: 4, label: 'S+' }
      ];
      for (let z = 1; z <= (m.zones || 4); z++) {
        slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x: 24 + (z - 1) * 29, y: 210, w: 25, h: 52, rx: 5, label: `${z}` });
      }
      return { vw: 160, vh: 280, outer: { x: 10, y: 6, w: 140, h: 268, rx: 22 }, glass: { x: 14, y: 10, w: 132, h: 260, rx: 19 }, titleY: 19, slots };
    }

    // B5 4-bar panel
    const slots = [];
    for (let z = 1; z <= 4; z++) {
      const x = 26 + (z - 1) * 33;
      slots.push({ key: `bri_z${z}`, type: 'vbar', act: 'brightness', zone: z, x, y: 42, w: 24, h: 84, label: `Z${z}` });
      slots.push({ key: `zone_${z}`, type: 'rocker', act: 'zone', zone: z, x, y: 134, w: 24, h: 52, rx: 5, label: `${z}` });
    }
    ['S1','S2','S3','S4'].forEach((s, i) => slots.push({ key: `btn_s${i+1}`, type: 'rect', act: 'mode', x: 160, y: 42 + i * 31, w: 28, h: 20, rx: 4, label: s }));
    slots.push({ key: 'btn_60s', type: 'rect', act: 'night_mode', x: 160, y: 166, w: 28, h: 20, rx: 4, label: '60S' });
    return { vw: 210, vh: 210, outer: { x: 6, y: 6, w: 198, h: 198, rx: 11 }, glass: { x: 11, y: 11, w: 188, h: 188, rx: 8 }, titleY: 27, slots };
  }

  // Render exact coordinate-driven Capacitive Stencil & Custom Facia SVG!
  function renderCapacitiveStencilSvg(modelOrId, opts) {
    const o = opts || {};
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || MODELS[0]) : (modelOrId || MODELS[0]);
    const bp = getPanelBlueprint(m);
    const sc = o.size === 'thumb' ? 0.45 : 1;
    const facia = o.facia || {};
    const preset = FACIA_THEMES[facia.theme] || (m.finish === 'black' ? FACIA_THEMES.matte_black : FACIA_THEMES.pure_white);
    const col = Object.assign({}, preset, facia.colors || {});
    const isMono = Boolean(facia.monochrome !== undefined ? facia.monochrome : col.mono);
    const pads = facia.pads || {};
    const defStyle = facia.defaultPadStyle || 'keycap';
    const selKey = o.selectedPadKey || null;
    const showStencilSlots = Boolean(o.editorMode);
    const tz = o.touchZones || {};
    const u = m.id.replace(/[^a-zA-Z0-9]/g, '') + '_' + (o.uid || 'st');

    let defs = `<defs>
      <linearGradient id="st_rgb_${u}" x1="0" y1="0" x2="1" y2="0">
        <stop offset="0%" stop-color="#ef4444"/><stop offset="20%" stop-color="#eab308"/><stop offset="40%" stop-color="#22c55e"/>
        <stop offset="60%" stop-color="#06b6d4"/><stop offset="80%" stop-color="#3b82f6"/><stop offset="100%" stop-color="#ec4899"/>
      </linearGradient>
      <linearGradient id="st_cct_${u}" x1="0" y1="0" x2="1" y2="0">
        <stop offset="0%" stop-color="#f59e0b"/><stop offset="50%" stop-color="#fef9c3"/><stop offset="100%" stop-color="#38bdf8"/>
      </linearGradient>
      <linearGradient id="st_cct_v_${u}" x1="0" y1="1" x2="0" y2="0">
        <stop offset="0%" stop-color="#f59e0b"/><stop offset="50%" stop-color="#fef9c3"/><stop offset="100%" stop-color="#38bdf8"/>
      </linearGradient>
      <linearGradient id="st_bri_v_${u}" x1="0" y1="1" x2="0" y2="0">
        <stop offset="0%" stop-color="#1e293b"/><stop offset="100%" stop-color="#0ea5e9"/>
      </linearGradient>
      <linearGradient id="st_bri_h_${u}" x1="0" y1="0" x2="1" y2="0">
        <stop offset="0%" stop-color="#334155"/><stop offset="100%" stop-color="#0ea5e9"/>
      </linearGradient>`;
    if (facia.bgPattern === 'grid') {
      defs += `<pattern id="pat_${u}" width="10" height="10" patternUnits="userSpaceOnUse"><path d="M 10 0 L 0 0 0 10" fill="none" stroke="${col.padStroke}" stroke-width="0.3" opacity="0.35"/></pattern>`;
    } else if (facia.bgPattern === 'brushed') {
      defs += `<pattern id="pat_${u}" width="8" height="3" patternUnits="userSpaceOnUse"><line x1="0" y1="1" x2="8" y2="1" stroke="${col.padStroke}" stroke-width="0.35" opacity="0.3"/></pattern>`;
    }
    const lumCfg = facia.luminous || {};
    const anyLum = Boolean(lumCfg.enabled || lumCfg.border || lumCfg.header || lumCfg.slots || Object.values(pads).some(p => p && p.luminous));
    if (anyLum) {
      defs += `<filter id="glow_${u}" x="-35%" y="-35%" width="170%" height="170%"><feGaussianBlur stdDeviation="2.2" result="blur"/><feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge></filter>`;
    }
    defs += `</defs>`;

    let body = `${defs}
      const bgCol = facia.bgColor || col.bg;
      const glassCol = facia.bgColor || col.glass;
      const bdrCol = facia.boundaryColor || col.padStroke || col.bezel;
      const lumCfg = facia.luminous || {};
      const borderGlow = (lumCfg.enabled && lumCfg.border) ? ` filter="url(#glow_${u})"` : '';
      body += `
      <rect x="${bp.outer.x}" y="${bp.outer.y}" width="${bp.outer.w}" height="${bp.outer.h}" rx="${bp.outer.rx}" fill="${bgCol}" stroke="${bdrCol}" stroke-width="2"${borderGlow}/>
      <rect x="${bp.glass.x}" y="${bp.glass.y}" width="${bp.glass.w}" height="${bp.glass.h}" rx="${bp.glass.rx}" fill="${glassCol}" stroke="${bdrCol}" stroke-width="0.8"${borderGlow}/>`;

    if (facia.bgPattern === 'grid' || facia.bgPattern === 'brushed') {
      body += `<rect x="${bp.glass.x}" y="${bp.glass.y}" width="${bp.glass.w}" height="${bp.glass.h}" rx="${bp.glass.rx}" fill="url(#pat_${u})" pointer-events="none"/>`;
    }

    if (facia.panelTitle) {
      const titleTxt = String(facia.panelTitle).trim().toUpperCase();
      if (bp.outer && bp.outer.rx >= bp.outer.w * 0.45 && bp.outer.w === bp.outer.h) {
        const tcx = bp.outer.x + bp.outer.w / 2;
        const tcy = bp.outer.y + bp.outer.h / 2;
        const tr = bp.outer.w / 2 + 5.5;
        const tArcId = `ptArc_${u}`;
        // Bottom-half clock arc from 135 deg to 225 deg (left-to-right upright)
        const pA = polarPt(tcx, tcy, tr, 225);
        const pB = polarPt(tcx, tcy, tr, 135);
        body += `<defs><path id="${tArcId}" d="M ${pA.x.toFixed(2)} ${pA.y.toFixed(2)} A ${tr.toFixed(2)} ${tr.toFixed(2)} 0 0 0 ${pB.x.toFixed(2)} ${pB.y.toFixed(2)}"/></defs>`;
        body += `<text font-family="Inter,system-ui,sans-serif" font-size="6.4" font-weight="800" letter-spacing="0.9" fill="${col.subInk}" pointer-events="none"><textPath href="#${tArcId}" startOffset="50%" text-anchor="middle">${escXml(titleTxt)}</textPath></text>`;
      } else {
        body += `<text x="${(bp.vw / 2).toFixed(1)}" y="${bp.titleY}" text-anchor="middle" font-family="Inter,system-ui,sans-serif" font-size="7.2" font-weight="800" letter-spacing="0.7" fill="${col.subInk}" pointer-events="none">${escXml(titleTxt)}</text>`;
      }
    } else if (facia.hideLogo === false) {
      body += `<text x="${bp.glass.x + 12}" y="${bp.titleY}" font-family="Inter,system-ui,sans-serif" font-size="8" font-weight="800" fill="${col.subInk}" pointer-events="none">MiBOXER®</text>`;
    }

    if (bp.cutout) {
      body += `<rect x="${bp.cutout.x}" y="${bp.cutout.y}" width="${bp.cutout.w}" height="${bp.cutout.h}" rx="${bp.cutout.rx}" fill="${o.stencilMode ? '#ffffff' : '#090b10'}" stroke="${col.padStroke}" stroke-width="0.9" stroke-dasharray="${o.stencilMode ? '2,1.5' : 'none'}" pointer-events="none"/>`;
      if (!o.stencilMode && bp.cutout.text) {
        body += `<text x="${(bp.cutout.x + bp.cutout.w / 2).toFixed(1)}" y="${(bp.cutout.y + bp.cutout.h * 0.74).toFixed(1)}" text-anchor="middle" fill="#f97316" font-family="monospace" font-size="11" font-weight="800" pointer-events="none">${escXml(bp.cutout.text)}</text>`;
      }
    }

    if (facia.showLedPinhole) {
      const dims = getPhysicalDims(m);
      const lx = (bp.vw * (dims.led_x_pct || 50)) / 100;
      const ly = (bp.vh * (dims.led_y_pct || 8)) / 100;
      body += `<circle cx="${lx.toFixed(1)}" cy="${ly.toFixed(1)}" r="1.5" fill="${o.stencilMode ? '#ffffff' : col.bg}" stroke="${col.padStroke}" stroke-width="0.6" pointer-events="none"/>`;
    }

    // Automatic box-aware autofit: sizes icon & text to fit safely inside pad width & height without overflow
    function fitText(str, maxW, desiredFs, minFs) {
      const s = String(str || '').trim();
      if (!s) return { text: '', fs: minFs };
      const usableW = Math.max(8, maxW - 3.5);
      const charFactor = 0.56;
      const byWidth = usableW / Math.max(1, s.length * charFactor);
      const fs = Math.max(minFs, Math.min(desiredFs, byWidth));
      const maxChars = Math.max(1, Math.floor(usableW / Math.max(0.1, fs * charFactor)));
      const clipped = s.length > maxChars ? (maxChars > 2 ? s.slice(0, maxChars - 1) + '…' : s.slice(0, maxChars)) : s;
      return { text: clipped, fs };
    }

    function drawGlyphContent(cx, cy, bw, bh, cfg, fallbackLbl, fallbackSub) {
      const icoKey = (cfg && cfg.icon) || 'none';
      const ico = FACIA_ICONS[icoKey];
      const hasIco = Boolean(ico && ico.svg);
      const rawLbl = (cfg && cfg.label !== undefined) ? String(cfg.label) : (fallbackLbl || '');
      let rawSub = (cfg && cfg.sublabel !== undefined) ? String(cfg.sublabel) : (fallbackSub || '');
      if (!rawSub && cfg && cfg.multifunc && cfg.multifunc.enabled) {
        rawSub = `1-${cfg.multifunc.steps || 4} • Hold`;
      }
      const ink = (cfg && cfg.color) || col.ink;

      const padW = Math.max(8, bw - 3);
      const padH = Math.max(8, bh - 3);
      const minD = Math.min(padW, padH);

      const lblFit = rawLbl ? fitText(rawLbl, padW, Math.min(7.4, minD * (hasIco ? 0.24 : 0.36)), 3.6) : { text: '', fs: 0 };
      const subFit = rawSub ? fitText(rawSub, padW, Math.min(4.8, (lblFit.fs || minD * 0.22) * 0.72), 3.0) : { text: '', fs: 0 };

      const textBlockH = (lblFit.text ? lblFit.fs * 1.15 : 0) + (subFit.text ? subFit.fs * 1.2 : 0);
      const availForIco = hasIco ? (lblFit.text || subFit.text ? Math.max(5.5, padH - textBlockH - 2.2) : Math.min(padW * 0.68, padH * 0.68)) : 0;
      const icoSz = hasIco ? Math.max(5.2, Math.min(14.5, availForIco)) : 0;

      const totalH = icoSz + (hasIco && textBlockH > 0 ? 1.8 : 0) + textBlockH;
      let yCursor = cy - totalH / 2;
      let out = '';

      if (hasIco) {
        const ix = cx - icoSz / 2;
        const iy = yCursor;
        const inner = ico.svg.replace(/currentColor/g, ink);
        out += `<g transform="translate(${ix.toFixed(2)},${iy.toFixed(2)}) scale(${(icoSz / 24).toFixed(4)})" pointer-events="none">${inner}</g>`;
        yCursor += icoSz + 1.8;
      }

      if (lblFit.text) {
        const baselineY = yCursor + lblFit.fs * 0.82;
        out += `<text x="${cx.toFixed(2)}" y="${baselineY.toFixed(2)}" text-anchor="middle" font-family="Inter,system-ui,sans-serif" font-size="${lblFit.fs.toFixed(2)}" font-weight="700" fill="${ink}" pointer-events="none">${escXml(lblFit.text)}</text>`;
        yCursor += lblFit.fs * 1.15;
      }

      if (subFit.text) {
        const subBaselineY = yCursor + subFit.fs * 0.82;
        out += `<text x="${cx.toFixed(2)}" y="${subBaselineY.toFixed(2)}" text-anchor="middle" font-family="Inter,system-ui,sans-serif" font-size="${subFit.fs.toFixed(2)}" font-weight="600" fill="${col.subInk}" pointer-events="none">${escXml(subFit.text)}</text>`;
      }

      return out;
    }

    function padColors(cfg) {
      const st = (cfg && cfg.style) || defStyle;
      if (st === 'blank') return { st, fill: col.glass, stroke: showStencilSlots ? col.padStroke : 'none', dash: showStencilSlots ? '2 2' : '', op: showStencilSlots ? '0.45' : '1' };
      if (st === 'minimal') return { st, fill: col.glass, stroke: showStencilSlots ? col.padStroke : 'none', dash: showStencilSlots ? '1.5 1.5' : '', op: '1' };
      if (st === 'outline') return { st, fill: col.glass, stroke: col.padStroke, dash: '', op: '1' };
      return { st: 'keycap', fill: (cfg && cfg.fill) || col.padFill, stroke: col.padStroke, dash: '', op: '1' };
    }

    function selRingRect(x, y, w, h, rx) {
      return `<rect x="${(x - 1.6).toFixed(1)}" y="${(y - 1.6).toFixed(1)}" width="${(w + 3.2).toFixed(1)}" height="${(h + 3.2).toFixed(1)}" rx="${rx + 1.5}" fill="none" stroke="${col.accent}" stroke-width="1.8" stroke-dasharray="3 1.5" pointer-events="none"/>`;
    }

    bp.slots.forEach(s => {
      // Check if this slot is a touch surface that has been segmented!
      const surfCfg = s.surf ? tz[s.surf] : null;
      const isSeg = Boolean(surfCfg && surfCfg.enabled && Number(surfCfg.segments) >= 2);

      if (s.type === 'ring') {
        if (isSeg) {
          const n = Math.min(12, Math.max(2, Number(surfCfg.segments || 6)));
          const span = 360 / n;
          const off = Number(surfCfg.offset_deg || 0);
          const ringThick = Math.max(8, s.rOut - s.rIn);
          for (let i = 0; i < n; i++) {
            const sKey = `hue_seg_${i + 1}`;
            const cfg = pads[sKey] || {};
            const pc = padColors(cfg);
            const sDeg = off + i * span + 1.5, eDeg = off + (i + 1) * span - 1.5;
            const midDeg = ((off + (i + 0.5) * span) % 360 + 360) % 360;
            const d = ringWedgePath(s.cx, s.cy, s.rIn, s.rOut, sDeg, eDeg);
            const isSelected = selKey === sKey;
            body += `<path class="mbx-seg-hit" data-seg-act="${sKey}" data-pad-key="${sKey}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} d="${d}" fill="${pc.fill}" stroke="${isSelected ? col.accent : pc.stroke}" stroke-width="${isSelected ? 2 : 1.05}" ${pc.dash ? `stroke-dasharray="${pc.dash}"` : ''} style="cursor:pointer"/>`;
            const defL = (surfCfg.labels && surfCfg.labels[i]) || `${i + 1}`;
            const lblStr = (cfg && cfg.label !== undefined) ? String(cfg.label).trim() : defL;
            const icoKey = (cfg && cfg.icon) || 'none';
            const ico = FACIA_ICONS[icoKey];
            const hasIco = Boolean(ico && ico.svg);
            const ink = (cfg && cfg.color) || col.ink;
            if (pc.st !== 'blank') {
              const arcSpan = Math.max(10, eDeg - sDeg);
              const isLowerArc = midDeg > 95 && midDeg < 265;
              // Wide segments (>= 62 deg, e.g. 1/4 clock quadrants) flow text cleanly along a non-overlapping arc
              if (arcSpan >= 62 && lblStr.length > 0) {
                const textR = hasIco ? (s.rIn + ringThick * 0.30) : (s.rIn + ringThick * 0.54);
                const padAng = Math.min(8, arcSpan * 0.12);
                const a1 = isLowerArc ? (eDeg - padAng) : (sDeg + padAng);
                const a2 = isLowerArc ? (sDeg + padAng) : (eDeg - padAng);
                const pStart = polarPt(s.cx, s.cy, textR, a1);
                const pEnd = polarPt(s.cx, s.cy, textR, a2);
                const sweep = isLowerArc ? '0' : '1';
                const lgFlag = Math.abs(eDeg - sDeg - padAng * 2) > 180 ? '1' : '0';
                const arcId = `arc_${u}_${i}`;
                const arcPath = `M ${pStart.x.toFixed(2)} ${pStart.y.toFixed(2)} A ${textR.toFixed(2)} ${textR.toFixed(2)} 0 ${lgFlag} ${sweep} ${pEnd.x.toFixed(2)} ${pEnd.y.toFixed(2)}`;
                const arcFs = Math.max(4.4, Math.min(6.0, ringThick * 0.36));
                body += `<defs><path id="${arcId}" d="${arcPath}"/></defs>`;
                body += `<text font-family="Inter,system-ui,sans-serif" font-size="${arcFs.toFixed(2)}" font-weight="800" fill="${ink}" pointer-events="none"><textPath href="#${arcId}" startOffset="50%" text-anchor="middle">${escXml(lblStr)}</textPath></text>`;
                if (hasIco) {
                  const ip = polarPt(s.cx, s.cy, s.rIn + ringThick * 0.74, midDeg);
                  const isz = Math.max(5.2, Math.min(7.6, ringThick * 0.48));
                  const inner = ico.svg.replace(/currentColor/g, ink);
                  body += `<g transform="translate(${(ip.x - isz / 2).toFixed(2)},${(ip.y - isz / 2).toFixed(2)}) scale(${(isz / 24).toFixed(4)})" pointer-events="none">${inner}</g>`;
                }
                if (cfg && cfg.segmentMode === 'slider') {
                  const sMin = Number(cfg.sliderMin ?? 0);
                  const sMax = Number(cfg.sliderMax ?? 100);
                  const trkR = s.rIn + ringThick * 0.88;
                  const p1 = polarPt(s.cx, s.cy, trkR, sDeg + 3);
                  const p2 = polarPt(s.cx, s.cy, trkR, eDeg - 3);
                  const lg = Math.abs(eDeg - sDeg - 6) > 180 ? '1' : '0';
                  body += `<path d="M ${p1.x.toFixed(2)} ${p1.y.toFixed(2)} A ${trkR.toFixed(2)} ${trkR.toFixed(2)} 0 ${lg} 1 ${p2.x.toFixed(2)} ${p2.y.toFixed(2)}" fill="none" stroke="${ink}" stroke-width="1.2" stroke-dasharray="2,2" opacity="0.85" pointer-events="none"/>`;
                  const rngTxt = `${sMin}–${sMax}%`;
                  const rngFs = Math.max(3.8, Math.min(5.0, ringThick * 0.30));
                  const rArcR = s.rIn + ringThick * 0.16;
                  const ra1 = isLowerArc ? (eDeg - 4) : (sDeg + 4);
                  const ra2 = isLowerArc ? (sDeg + 4) : (eDeg - 4);
                  const rpA = polarPt(s.cx, s.cy, rArcR, ra1);
                  const rpB = polarPt(s.cx, s.cy, rArcR, ra2);
                  const rSwp = isLowerArc ? '0' : '1';
                  const rLg = Math.abs(eDeg - sDeg - 8) > 180 ? '1' : '0';
                  const rArcId = `rng_${u}_${i}`;
                  body += `<defs><path id="${rArcId}" d="M ${rpA.x.toFixed(2)} ${rpA.y.toFixed(2)} A ${rArcR.toFixed(2)} ${rArcR.toFixed(2)} 0 ${rLg} ${rSwp} ${rpB.x.toFixed(2)} ${rpB.y.toFixed(2)}"/></defs>`;
                  body += `<text font-family="monospace" font-size="${rngFs.toFixed(2)}" font-weight="700" fill="${col.subInk}" pointer-events="none"><textPath href="#${rArcId}" startOffset="50%" text-anchor="middle">${escXml(rngTxt)}</textPath></text>`;
                }
              } else {
                // Narrower clock segments (6, 8, 12 slots): show crisp icon OR clean clock numeral/short label without ellipsis clutter
                const mp = polarPt(s.cx, s.cy, (s.rIn + s.rOut) / 2, midDeg);
                if (hasIco) {
                  const isz = Math.max(6.2, Math.min(9.5, ringThick * 0.62));
                  const inner = ico.svg.replace(/currentColor/g, ink);
                  body += `<g transform="translate(${(mp.x - isz / 2).toFixed(2)},${(mp.y - isz / 2).toFixed(2)}) scale(${(isz / 24).toFixed(4)})" pointer-events="none">${inner}</g>`;
                } else if (lblStr) {
                  const clockLbl = lblStr.length > 4 ? String(i + 1) : lblStr;
                  const cFs = Math.max(5.0, Math.min(7.2, ringThick * 0.46));
                  body += `<text x="${mp.x.toFixed(2)}" y="${(mp.y + cFs * 0.35).toFixed(2)}" text-anchor="middle" font-family="Inter,system-ui,sans-serif" font-size="${cFs.toFixed(2)}" font-weight="800" fill="${ink}" pointer-events="none">${escXml(clockLbl)}</text>`;
                }
              }
            }
          }
          return;
        }

        // Continuous Ring (not segmented)
        const cfg = pads[s.key] || {};
        const pc = padColors(cfg);
        const rMid = (s.rIn + s.rOut) / 2, rw = s.rOut - s.rIn;
        if (pc.st === 'blank') {
          if (showStencilSlots) body += `<circle class="mbx-hit" data-act="hue" data-pad-key="${s.key}" data-blanked="1" cx="${s.cx}" cy="${s.cy}" r="${rMid}" fill="none" stroke="${col.padStroke}" stroke-width="${rw}" stroke-dasharray="2 3" opacity="0.3" style="cursor:pointer"/>`;
          return;
        }
        if (isMono || facia.sliderStyle === 'mono_ticks' || facia.sliderStyle === 'minimal') {
          body += `<circle cx="${s.cx}" cy="${s.cy}" r="${s.rOut}" fill="none" stroke="${col.padStroke}" stroke-width="0.9"/>`;
          body += `<circle cx="${s.cx}" cy="${s.cy}" r="${s.rIn}" fill="none" stroke="${col.padStroke}" stroke-width="0.9"/>`;
          body += `<circle class="mbx-hit" data-act="hue" data-pad-key="${s.key}" cx="${s.cx}" cy="${s.cy}" r="${rMid}" fill="none" stroke="${col.ink}" stroke-width="${facia.sliderStyle === 'minimal' ? 1.5 : rw * 0.55}" ${facia.sliderStyle === 'mono_ticks' ? 'stroke-dasharray="2.2 3.2"' : ''} opacity="${facia.sliderStyle === 'minimal' ? 0.5 : 0.75}" style="cursor:pointer"/>`;
          // Invisible wide hit ring for smooth dragging
          body += `<circle class="mbx-hit" data-act="hue" data-pad-key="${s.key}" cx="${s.cx}" cy="${s.cy}" r="${rMid}" fill="none" stroke="transparent" stroke-width="${rw}" style="cursor:pointer"/>`;
        } else {
          // Crisp 12-hue color ring
          const cols12 = ['#ef4444','#f97316','#eab308','#84cc16','#22c55e','#14b8a6','#06b6d4','#3b82f6','#6366f1','#a855f7','#ec4899','#f43f5e'];
          for (let i = 0; i < 12; i++) {
            body += `<path d="${ringWedgePath(s.cx, s.cy, s.rIn, s.rOut, i * 30, (i + 1) * 30 + 0.6)}" fill="${cols12[i]}" pointer-events="none"/>`;
          }
          body += `<circle class="mbx-hit" data-act="hue" data-pad-key="${s.key}" cx="${s.cx}" cy="${s.cy}" r="${rMid}" fill="none" stroke="transparent" stroke-width="${rw}" style="cursor:pointer"/>`;
        }
        if (selKey === s.key) body += `<circle cx="${s.cx}" cy="${s.cy}" r="${s.rOut + 2}" fill="none" stroke="${col.accent}" stroke-width="1.8" stroke-dasharray="3 1.5" pointer-events="none"/>`;
        return;
      }

      if (s.type === 'vbar' || s.type === 'hbar') {
        const vert = s.type === 'vbar';
        if (isSeg) {
          const pref = s.surf === 'hue_ring' ? 'hue_seg_' : s.surf === 'cct_bar' ? 'cct_seg_' : s.surf === 'sat_bar' ? 'sat_seg_' : 'bri_seg_';
          const maxN = s.surf === 'hue_ring' ? 8 : 6;
          const n = Math.min(maxN, Math.max(2, Number(surfCfg.segments || 4)));
          const gap = 2.0;
          for (let i = 0; i < n; i++) {
            const sKey = `${pref}${i + 1}`;
            const cfg = pads[sKey] || {};
            const pc = padColors(cfg);
            const idx = vert ? (n - 1 - i) : i;
            const rx = vert ? s.x : s.x + idx * ((s.w + gap) / n);
            const ry = vert ? s.y + idx * ((s.h + gap) / n) : s.y;
            const rw = vert ? s.w : (s.w - gap * (n - 1)) / n;
            const rh = vert ? (s.h - gap * (n - 1)) / n : s.h;
            const isSelected = selKey === sKey;
            body += `<rect class="mbx-seg-hit" data-seg-act="${sKey}" data-pad-key="${sKey}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} x="${rx.toFixed(2)}" y="${ry.toFixed(2)}" width="${rw.toFixed(2)}" height="${rh.toFixed(2)}" rx="3" fill="${pc.fill}" stroke="${isSelected ? col.accent : pc.stroke}" stroke-width="${isSelected ? 1.9 : 1.0}" ${pc.dash ? `stroke-dasharray="${pc.dash}"` : ''} style="cursor:pointer"/>`;
            const defL = (surfCfg.labels && surfCfg.labels[i]) || `${i + 1}`;
            if (pc.st !== 'blank') {
              body += drawGlyphContent(rx + rw / 2, ry + rh / 2, rw, rh, cfg, defL, '');
              if (cfg && cfg.segmentMode === 'slider') {
                const sMin = Number(cfg.sliderMin ?? 0);
                const sMax = Number(cfg.sliderMax ?? 100);
                if (vert) {
                  body += `<line x1="${(rx + 3).toFixed(1)}" y1="${(ry + rh - 3.5).toFixed(1)}" x2="${(rx + rw - 3).toFixed(1)}" y2="${(ry + rh - 3.5).toFixed(1)}" stroke="${ink}" stroke-width="1.3" stroke-dasharray="2,1.8" opacity="0.85" pointer-events="none"/>`;
                } else {
                  body += `<line x1="${(rx + 3.5).toFixed(1)}" y1="${(ry + rh - 3).toFixed(1)}" x2="${(rx + rw - 3.5).toFixed(1)}" y2="${(ry + rh - 3).toFixed(1)}" stroke="${ink}" stroke-width="1.3" stroke-dasharray="2,1.8" opacity="0.85" pointer-events="none"/>`;
                }
                body += `<text x="${(rx + rw / 2).toFixed(1)}" y="${(ry + rh - 4.8).toFixed(1)}" text-anchor="middle" font-family="monospace" font-size="4.4" font-weight="700" fill="${col.subInk}" pointer-events="none">${escXml(`${sMin}–${sMax}%`)}</text>`;
              }
            }
          }
          return;
        }

        // Continuous Bar (not segmented)
        const cfg = pads[s.key] || {};
        const pc = padColors(cfg);
        if (pc.st === 'blank') {
          if (showStencilSlots) body += `<rect class="mbx-hit" data-act="${s.act}" data-pad-key="${s.key}" data-blanked="1" x="${s.x}" y="${s.y}" width="${s.w}" height="${s.h}" rx="${Math.min(s.w, s.h) / 2}" fill="none" stroke="${col.padStroke}" stroke-dasharray="2 2" opacity="0.35" style="cursor:pointer"/>`;
          return;
        }
        let gradFill = pc.fill;
        if (!isMono && facia.sliderStyle !== 'mono_ticks' && facia.sliderStyle !== 'minimal') {
          if (s.act === 'hue') gradFill = `url(#st_rgb_${u})`;
          else if (s.act === 'kelvin' || s.act === 'saturation') gradFill = vert ? `url(#st_cct_v_${u})` : `url(#st_cct_${u})`;
          else gradFill = vert ? `url(#st_bri_v_${u})` : `url(#st_bri_h_${u})`;
        }
        const rad = Math.min(6, Math.min(s.w, s.h) / 2);
        body += `<rect class="mbx-hit" data-act="${s.act}" ${s.zone !== undefined ? `data-zone="${s.zone}"` : ''} data-pad-key="${s.key}" x="${s.x}" y="${s.y}" width="${s.w}" height="${s.h}" rx="${rad}" fill="${gradFill}" stroke="${col.padStroke}" stroke-width="0.95" style="cursor:pointer"/>`;
        // Graduated ticks for monochrome/minimal clarity
        const steps = 6;
        for (let t = 1; t < steps; t++) {
          if (vert) {
            const ty = s.y + (t * s.h) / steps;
            body += `<line x1="${s.x + 3}" y1="${ty.toFixed(1)}" x2="${s.x + s.w - 3}" y2="${ty.toFixed(1)}" stroke="${isMono ? col.ink : '#ffffff'}" stroke-opacity="0.55" stroke-width="0.8" pointer-events="none"/>`;
          } else {
            const tx = s.x + (t * s.w) / steps;
            body += `<line x1="${tx.toFixed(1)}" y1="${s.y + 3}" x2="${tx.toFixed(1)}" y2="${s.y + s.h - 3}" stroke="${isMono ? col.ink : '#ffffff'}" stroke-opacity="0.55" stroke-width="0.8" pointer-events="none"/>`;
          }
        }
        if (cfg.icon || cfg.label) body += drawGlyphContent(s.x + s.w / 2, s.y + s.h / 2, s.w, s.h, cfg, '', '');
        if (selKey === s.key) body += selRingRect(s.x, s.y, s.w, s.h, rad);
        return;
      }

      if (s.type === 'circle') {
        const cfg = pads[s.key] || {};
        const pc = padColors(cfg);
        const zAttr = (s.zone !== undefined && s.zone !== null) ? `data-zone="${s.zone}"` : '';
        const isLumC = Boolean((facia.luminous && facia.luminous.enabled && facia.luminous.slots) || (cfg && cfg.luminous));
        const glowAttrC = isLumC ? ` filter="url(#glow_${u})"` : '';
        body += `<circle class="mbx-hit" data-act="${s.act}" ${zAttr} data-pad-key="${s.key}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} cx="${s.cx}" cy="${s.cy}" r="${s.r}" fill="${pc.fill}" stroke="${selKey === s.key ? col.accent : pc.stroke}" stroke-width="${selKey === s.key ? 1.9 : 1.0}" ${pc.dash ? `stroke-dasharray="${pc.dash}"` : ''} opacity="${pc.op}"${glowAttrC} style="cursor:pointer"/>`;
        if (pc.st !== 'blank') {
          body += drawGlyphContent(s.cx, s.cy, s.r * 1.85, s.r * 1.85, cfg, s.label, s.sublabel || '');
        } else if (showStencilSlots && selKey === s.key) {
          body += drawGlyphContent(s.cx, s.cy, s.r * 1.8, s.r * 1.8, { label: 'MASK' }, 'MASK', '');
        }
        return;
      }

      if (s.type === 'rocker') {
        // Vertical Zone Rocker: Top = zone_on, Center = room icon + label, Bottom = zone_off
        const cfg = pads[s.key] || {};
        const pc = padColors(cfg);
        const z = s.zone ?? 1;
        body += `<g data-pad-key="${s.key}">
          <rect x="${s.x}" y="${s.y}" width="${s.w}" height="${s.h}" rx="${s.rx || 5}" fill="${pc.fill}" stroke="${selKey === s.key ? col.accent : pc.stroke}" stroke-width="${selKey === s.key ? 1.9 : 1.0}" ${pc.dash ? `stroke-dasharray="${pc.dash}"` : ''} opacity="${pc.op}"/>`;
        if (pc.st !== 'blank') {
          // Subtle top '|' and bottom 'o' rocker pips + room icon/name in center
          body += `<line x1="${(s.x + s.w / 2).toFixed(1)}" y1="${(s.y + 4.5).toFixed(1)}" x2="${(s.x + s.w / 2).toFixed(1)}" y2="${(s.y + 9).toFixed(1)}" stroke="${col.subInk}" stroke-width="1.2" stroke-linecap="round" pointer-events="none"/>`;
          body += `<circle cx="${(s.x + s.w / 2).toFixed(1)}" cy="${(s.y + s.h - 6.8).toFixed(1)}" r="2.3" fill="none" stroke="${col.subInk}" stroke-width="1.0" pointer-events="none"/>`;
          body += drawGlyphContent(s.x + s.w / 2, s.y + s.h / 2, s.w * 0.92, s.h * 0.56, cfg, s.label, s.sublabel || '');
        }
        body += `<rect class="mbx-hit" data-act="zone_on" data-zone="${z}" data-pad-key="${s.key}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} x="${s.x}" y="${s.y}" width="${s.w}" height="${(s.h * 0.5).toFixed(1)}" fill="transparent" style="cursor:pointer"/>`;
        body += `<rect class="mbx-hit" data-act="zone_off" data-zone="${z}" data-pad-key="${s.key}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} x="${s.x}" y="${(s.y + s.h * 0.5).toFixed(1)}" width="${s.w}" height="${(s.h * 0.5).toFixed(1)}" fill="transparent" style="cursor:pointer"/>
        </g>`;
        return;
      }

      // Standard Rectangular Capacitive Button
      const cfg = pads[s.key] || {};
      const pc = padColors(cfg);
      const zAttr = (s.zone !== undefined && s.zone !== null) ? `data-zone="${s.zone}"` : '';
      const vAttr = (s.val !== undefined && s.val !== null) ? `data-val="${s.val}"` : '';
      body += `<rect class="mbx-hit" data-act="${s.act}" ${zAttr} ${vAttr} data-pad-key="${s.key}" ${pc.st === 'blank' ? 'data-blanked="1"' : ''} x="${s.x}" y="${s.y}" width="${s.w}" height="${s.h}" rx="${s.rx || 4}" fill="${pc.fill}" stroke="${selKey === s.key ? col.accent : pc.stroke}" stroke-width="${selKey === s.key ? 1.9 : 1.0}" ${pc.dash ? `stroke-dasharray="${pc.dash}"` : ''} opacity="${pc.op}" style="cursor:pointer"/>`;
      if (pc.st !== 'blank') {
        body += drawGlyphContent(s.x + s.w / 2, s.y + s.h / 2, s.w, s.h, cfg, s.label, s.sublabel || '');
      }
    });

    return `<svg viewBox="0 0 ${bp.vw} ${bp.vh}" width="${Math.round(bp.vw * sc)}" height="${Math.round(bp.vh * sc)}">${body}</svg>`;
  }

  // Ready-to-load Smart Showcase Presets per Panel Type!
  function getSamplePresetsForModel(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || MODELS[0]) : (modelOrId || MODELS[0]);
    const isBlk = m.finish === 'black';
    const defTheme = isBlk ? 'matte_black' : 'pure_white';

    // 1. SINGLE-ZONE B0: Showcase whole-room multi-device control on ONE single-zone glass panel!
    if (m.layout === 'b0') {
      return [
        {
          id: 'b0_smart_suite',
          name: 'Smart Room Multi-Device Hub (6-Slice Ring + Step/Hold)',
          desc: 'Turns 1-zone B0 into a 14-function room controller: Fan (1-6/Hold), Blinds, Curtains, AC, Sonos & Lighting',
          touch_zones: {
            hue_ring: { enabled: true, segments: 6, offset_deg: -30, zone_mask: 65535, labels: ['Fan','Blinds','Curtain','Climate','Audio','Strip'] },
            sat_bar:  { enabled: true, segments: 3, offset_deg: 0,   zone_mask: 65535, labels: ['Bright','Relax','Cinema'] },
            bri_bar:  { enabled: false, segments: 4, offset_deg: 0,  zone_mask: 65535, labels: ['','','',''] },
            cct_bar:  { enabled: false, segments: 4, offset_deg: 0,  zone_mask: 65535, labels: ['','','',''] }
          },
          facia: {
            enabled: true, theme: defTheme, panelTitle: 'MASTER SUITE', defaultPadStyle: 'keycap', showLedPinhole: true, hideLogo: true,
            pads: {
              hue_seg_1: { icon: 'fan',       label: 'Fan',     multifunc: { enabled: true, steps: 6, unit: 'Spd' } },
              hue_seg_2: { icon: 'blinds',    label: 'Blinds',  multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
              hue_seg_3: { icon: 'curtains',  label: 'Drapes',  multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
              hue_seg_4: { icon: 'ac',        label: 'AC',      multifunc: { enabled: true, steps: 5, unit: 'Temp' } },
              hue_seg_5: { icon: 'speaker',   label: 'Sonos',   multifunc: { enabled: true, steps: 5, unit: 'Vol' } },
              hue_seg_6: { icon: 'strip',     label: 'Cove',    multifunc: { enabled: true, steps: 4, unit: 'Hue' } },
              sat_seg_1: { icon: 'sun',       label: 'Day' },
              sat_seg_2: { icon: 'coffee',    label: 'Relax' },
              sat_seg_3: { icon: 'tv',        label: 'Movie' },
              center_w:  { icon: 'pendant',   label: 'Main',    sublabel: 'Hold W' },
              btn_on:    { icon: 'power',     label: 'Room On' },
              btn_r:     { icon: 'lamp',      label: 'Bed L',   multifunc: { enabled: true, steps: 3, unit: 'Lvl' } },
              btn_g:     { icon: 'lamp',      label: 'Bed R',   multifunc: { enabled: true, steps: 3, unit: 'Lvl' } },
              btn_b:     { icon: 'desk',      label: 'Desk',    multifunc: { enabled: true, steps: 3, unit: 'Lvl' } },
              btn_off:   { icon: 'moon',      label: 'All Off', sublabel: 'Hold Night' },
              btn_m:     { icon: 'sparkles',  label: 'Scene' },
              btn_sdn:   { icon: 'timer',     label: 'Sleep',   sublabel: '60m' },
              btn_sup:   { icon: 'extractor', label: 'Vent',    multifunc: { enabled: true, steps: 3, unit: 'Spd' } }
            }
          }
        },
        {
          id: 'b0_minimal_bw',
          name: 'Minimalist B&W Accessibility Stencil (4-Device Ring)',
          desc: 'High-contrast black & white stencil with blanked unused RGB keys',
          touch_zones: {
            hue_ring: { enabled: true, segments: 4, offset_deg: -45, zone_mask: 65535, labels: ['Lights','Fan','Blinds','TV'] },
            sat_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
            bri_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
            cct_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] }
          },
          facia: {
            enabled: true, theme: 'ink_bw', monochrome: true, panelTitle: 'LIVING ROOM', defaultPadStyle: 'outline', showLedPinhole: true, hideLogo: true,
            pads: {
              hue_seg_1: { icon: 'bulb',   label: 'Lights', multifunc: { enabled: true, steps: 4, unit: 'Bri' } },
              hue_seg_2: { icon: 'fan',    label: 'Fan',    multifunc: { enabled: true, steps: 6, unit: 'Spd' } },
              hue_seg_3: { icon: 'blinds', label: 'Blinds', multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
              hue_seg_4: { icon: 'tv',     label: 'TV',     multifunc: { enabled: true, steps: 3, unit: 'Src' } },
              sat_bar:   { style: 'blank' },
              btn_r:     { style: 'blank' },
              btn_g:     { style: 'blank' },
              btn_b:     { style: 'blank' },
              btn_sdn:   { style: 'blank' },
              btn_sup:   { style: 'blank' },
              btn_on:    { icon: 'sun',    label: 'ON' },
              btn_off:   { icon: 'moon',   label: 'OFF' },
              btn_m:     { icon: 'lock',   label: 'Away' }
            }
          }
        }
      ];
    }

    // 2. 8-ZONE B8: Standardized 8-Room Icons + Segmented Device Ring / Sliders!
    if (m.layout === 'b8') {
      return [
        {
          id: 'b8_whole_home_devices',
          name: '8-Room Standardized Icons + 6-Device Segmented Wheel',
          desc: 'Zones 1-8 mapped to standardized rooms; Color Ring segmented into Fan (1-6), Blinds, Curtains, AC, Sonos & Strip',
          touch_zones: {
            hue_ring: { enabled: true, segments: 6, offset_deg: -30, zone_mask: 65535, labels: ['Fan','Blinds','Curtains','AC','Audio','Strip'] },
            sat_bar:  { enabled: true, segments: 3, offset_deg: 0,   zone_mask: 65535, labels: ['Day','Relax','Cinema'] },
            bri_bar:  { enabled: false, segments: 4, offset_deg: 0,  zone_mask: 65535, labels: ['','','',''] },
            cct_bar:  { enabled: false, segments: 4, offset_deg: 0,  zone_mask: 65535, labels: ['','','',''] }
          },
          facia: {
            enabled: true, theme: defTheme, panelTitle: 'SMART HOME CONTROL', defaultPadStyle: 'keycap', showLedPinhole: true, hideLogo: true,
            pads: {
              hue_seg_1: { icon: 'fan',        label: 'Fan',     multifunc: { enabled: true, steps: 6, unit: 'Spd' } },
              hue_seg_2: { icon: 'blinds',     label: 'Blinds',  multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
              hue_seg_3: { icon: 'curtains',   label: 'Drapes',  multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
              hue_seg_4: { icon: 'ac',         label: 'Climate', multifunc: { enabled: true, steps: 5, unit: 'Temp' } },
              hue_seg_5: { icon: 'speaker',    label: 'Sonos',   multifunc: { enabled: true, steps: 5, unit: 'Vol' } },
              hue_seg_6: { icon: 'strip',      label: 'LEDs',    multifunc: { enabled: true, steps: 4, unit: 'Scene' } },
              sat_seg_1: { icon: 'sun',        label: 'Bright' },
              sat_seg_2: { icon: 'coffee',     label: 'Warm' },
              sat_seg_3: { icon: 'tv',         label: 'Movie' },
              center_w:  { icon: 'downlight',  label: 'White' },
              btn_on:    { icon: 'power',      label: 'ON' },
              btn_off:   { icon: 'moon',       label: 'OFF' },
              btn_sdn:   { icon: 'blinds',     label: 'Shut' },
              btn_m:     { icon: 'sparkles',   label: 'Scene' },
              btn_sup:   { icon: 'sun',        label: 'Open' },
              zone_0:    { icon: 'home_all',   label: 'Home' },
              zone_1:    { icon: 'sofa',       label: 'Living' },
              zone_2:    { icon: 'kitchen',    label: 'Kitchen' },
              zone_3:    { icon: 'dining',     label: 'Dining' },
              zone_4:    { icon: 'bed',        label: 'Master' },
              btn_60s:   { icon: 'lock',       label: 'Away' },
              zone_5:    { icon: 'bed_single', label: 'Bed 2' },
              zone_6:    { icon: 'desk',       label: 'Study' },
              zone_7:    { icon: 'bath',       label: 'Bath' },
              zone_8:    { icon: 'garden',     label: 'Patio' }
            }
          }
        },
        {
          id: 'b8_rooms_analog',
          name: '8-Room Standardized Lighting (Continuous Wheel & Sliders)',
          desc: 'Keeps analog Color Wheel & Dimmer sliders while adding crisp Room Icons to Zones 1-8',
          touch_zones: {
            hue_ring: { enabled: false, segments: 6, offset_deg: 0, zone_mask: 65535, labels: ['','','','','',''] },
            sat_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
            bri_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
            cct_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] }
          },
          facia: {
            enabled: true, theme: defTheme, panelTitle: 'RESIDENCE LIGHTING', defaultPadStyle: 'keycap', showLedPinhole: true, hideLogo: true,
            pads: {
              center_w: { icon: 'sun',        label: 'White' },
              zone_0:   { icon: 'home_all',   label: 'All' },
              zone_1:   { icon: 'sofa',       label: 'Living' },
              zone_2:   { icon: 'kitchen',    label: 'Kitchen' },
              zone_3:   { icon: 'dining',     label: 'Dining' },
              zone_4:   { icon: 'bed',        label: 'Master' },
              btn_60s:  { icon: 'timer',      label: '60s Off' },
              zone_5:   { icon: 'bed_single', label: 'Guest' },
              zone_6:   { icon: 'desk',       label: 'Office' },
              zone_7:   { icon: 'bath',       label: 'Bath' },
              zone_8:   { icon: 'garden',     label: 'Outdoor' }
            }
          }
        }
      ];
    }

    // 3. STRIP / MULTI-ZONE PANELS (B1..B4, U1..U3, M4, B5): Standardized Room Rockers + Segmented Multi-Device Bars!
    const topSurf = (m.kind === 'b1' || m.kind === 'u1') ? 'bri_bar' : (m.kind === 'b2' || m.kind === 'u2') ? 'cct_bar' : 'hue_ring';
    const segPref = topSurf === 'bri_bar' ? 'bri_seg_' : topSurf === 'cct_bar' ? 'cct_seg_' : 'hue_seg_';
    return [
      {
        id: 'strip_rooms_plus_devices',
        name: 'Room Rockers + 4-Device Segmented Bar (Fan/Blinds/AC/Audio)',
        desc: 'Top touch strip becomes 4 multi-function device pads (Tap = Step 1-6, Hold = Toggle); bottom rockers use standardized Room Icons',
        touch_zones: {
          hue_ring: { enabled: topSurf === 'hue_ring', segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['Fan','Blinds','AC','Audio'] },
          cct_bar:  { enabled: topSurf === 'cct_bar',  segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['Fan','Blinds','AC','Audio'] },
          bri_bar:  { enabled: topSurf === 'bri_bar',  segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['Fan','Blinds','AC','Audio'] },
          sat_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] }
        },
        facia: {
          enabled: true, theme: defTheme, panelTitle: 'APARTMENT SUITE', defaultPadStyle: 'keycap', showLedPinhole: true, hideLogo: true,
          pads: {
            [segPref + '1']: { icon: 'fan',      label: 'Fan',     multifunc: { enabled: true, steps: 6, unit: 'Spd' } },
            [segPref + '2']: { icon: 'blinds',   label: 'Blinds',  multifunc: { enabled: true, steps: 4, unit: 'Pos' } },
            [segPref + '3']: { icon: 'ac',       label: 'Climate', multifunc: { enabled: true, steps: 5, unit: 'Temp' } },
            [segPref + '4']: { icon: 'speaker',  label: 'Sonos',   multifunc: { enabled: true, steps: 5, unit: 'Vol' } },
            zone_0: { icon: 'home_all', label: 'All' },
            zone_1: { icon: 'sofa',     label: 'Living' },
            zone_2: { icon: 'kitchen',  label: 'Kitchen' },
            zone_3: { icon: 'bed',      label: 'Master' },
            zone_4: { icon: 'garden',   label: 'Patio' }
          }
        }
      },
      {
        id: 'strip_bw_print',
        name: 'High-Contrast B&W Room Stencil',
        desc: 'Clean monochrome room rockers and crisp graduated dimmer bar ready for 1:1 laser/vinyl printing',
        touch_zones: {
          hue_ring: { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
          cct_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
          bri_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] },
          sat_bar:  { enabled: false, segments: 4, offset_deg: 0, zone_mask: 65535, labels: ['','','',''] }
        },
        facia: {
          enabled: true, theme: 'ink_bw', monochrome: true, panelTitle: 'LIGHTING ZONES', defaultPadStyle: 'outline', showLedPinhole: true, hideLogo: true,
          pads: {
            zone_0: { icon: 'home_all', label: 'All' },
            zone_1: { icon: 'sofa',     label: 'Lounge' },
            zone_2: { icon: 'dining',   label: 'Dining' },
            zone_3: { icon: 'bed',      label: 'Bedroom' },
            zone_4: { icon: 'bath',     label: 'Bath' }
          }
        }
      }
    ];
  }

  // Stamp all 61 packaged factory MiBoxer models as read-only!
  const FACTORY_MODELS = MODELS.map(m => {
    m.readonly = true;
    m.isCustom = false;
    return m;
  });

  function resolveBaseModel(modelOrId) {
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || FACTORY_MODELS[0]) : (modelOrId || FACTORY_MODELS[0]);
    if (m.isCustom && m.baseModel) {
      return FACTORY_MODELS.find(f => f.id === m.baseModel) || m;
    }
    return m;
  }

  function registerCustomModels(rawList) {
    // Strip existing custom entries in-place while keeping MODELS array reference stable
    for (let i = MODELS.length - 1; i >= 0; i--) {
      if (MODELS[i].isCustom) MODELS.splice(i, 1);
    }
    if (!Array.isArray(rawList)) return MODELS;
    rawList.forEach(cm => {
      if (!cm || !cm.id) return;
      const bId = cm.base_model || cm.baseModel || 'B8';
      const base = FACTORY_MODELS.find(f => f.id === bId) || FACTORY_MODELS[0];
      MODELS.unshift({
        id: cm.id,
        base: base.base || base.id,
        baseModel: base.id,
        finish: base.finish || 'white',
        cat: 'custom',
        baseCat: base.cat,
        proto: base.proto,
        zones: base.zones,
        layout: base.layout,
        kind: base.kind,
        name: cm.name || `${base.id} Custom`,
        sub: `Custom ${base.id} (${base.zones}Z)`,
        readonly: false,
        isCustom: true,
        stencilMode: cm.stencil_mode || 'full_panel',
        facia: cm.facia || null,
        touchZones: cm.touch_zones || null
      });
    });
    return MODELS;
  }

  async function syncCustomModelsFromHub(hubBaseUrl) {
    const base = (hubBaseUrl || '').replace(/\/$/, '');
    try {
      const res = await fetch(`${base}/api/remotes/custom_models`, { cache: 'no-store' });
      if (!res.ok) return MODELS;
      const list = await res.json();
      return registerCustomModels(list);
    } catch (_) {
      return MODELS;
    }
  }

  const _baseRenderModelSvg = renderModelSvg;
  function renderStudioSvg(modelOrId, opts) {
    const o = opts || {};
    const m = typeof modelOrId === 'string' ? (MODELS.find(x => x.id === modelOrId) || MODELS[0]) : (modelOrId || MODELS[0]);
    const effFacia = o.facia || m.facia || null;
    const effTz = o.touchZones || m.touchZones || null;
    if (isCapacitiveFlatPanel(m) && ((effFacia && effFacia.enabled) || o.editorMode)) {
      return renderCapacitiveStencilSvg(m, Object.assign({}, o, { facia: effFacia, touchZones: effTz }));
    }
    return _baseRenderModelSvg(m, o);
  }

  // Auto-hydrate custom models if loaded inside a browser pointing at Hub
  if (typeof fetch === 'function' && typeof document !== 'undefined') {
    const curScript = document.currentScript && document.currentScript.src ? new URL(document.currentScript.src, location.href).origin : '';
    syncCustomModelsFromHub(curScript);
  }

  root.MiBoxerCatalog = {
    MODELS,
    FACTORY_MODELS,
    FACIA_ICONS,
    FACIA_THEMES,
    isCapacitiveFlatPanel,
    isSingleStickOnStencilReady,
    getStencilFitMode,
    getCutStickerSheet,
    renderCutStickerSheetSvg,
    resolveBaseModel,
    registerCustomModels,
    syncCustomModelsFromHub,
    resolveBaseHardwareId,
    areModelsPhysicallyCompatible,
    getPhysicalDims,
    getPanelBlueprint,
    getSamplePresetsForModel,
    renderCapacitiveStencilSvg,
    baseRenderModelSvg: _baseRenderModelSvg,
    renderModelSvg: renderStudioSvg
  };
})(typeof window !== 'undefined' ? window : globalThis);
