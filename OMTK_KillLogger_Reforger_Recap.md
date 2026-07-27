<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 780 800" width="780" height="800" role="img" aria-label="Synoptique révisé du portage OMTK vers Arma Reforger, classé en trois bandes : code à écrire, config et prefabs, déjà natif.">
  <defs>
    <marker id="aC1" viewBox="0 0 8 8" refX="7" refY="4" markerWidth="5" markerHeight="5" orient="auto">
      <path d="M0 0 L8 4 L0 8 z" fill="#1F5F4E"/>
    </marker>
    <marker id="aC2" viewBox="0 0 8 8" refX="7" refY="4" markerWidth="5" markerHeight="5" orient="auto">
      <path d="M0 0 L8 4 L0 8 z" fill="#35547F"/>
    </marker>
  </defs>

  <rect width="780" height="800" fill="#F1F3F2"/>

  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="21px" font-weight="700" fill="#16211D" x="40" y="46">Synoptique révisé — OMTK sur Arma Reforger</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" fill="#5A6560" x="40" y="70">Classé par nature du travail restant, non plus par module</text>
  <line x1="40" y1="88" x2="740" y2="88" stroke="#C9D0CC" stroke-width="1"/>

  <!-- BANDE 01 -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="118" fill="#1F5F4E">01</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="118">CODE À ÉCRIRE — composants moddés</text>

  <rect x="250" y="134" width="280" height="50" rx="6" fill="#1F5F4E"/>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="13px" font-weight="700" fill="#FFFFFF" x="390" y="156" text-anchor="middle">GameMode (prefab racine)</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" x="390" y="173" text-anchor="middle" fill="#C0DED3">PREGAME → GAME → POSTGAME</text>

  <path d="M390 184 V198" stroke="#1F5F4E" stroke-width="1.2" fill="none"/>
  <path d="M137 198 H643" stroke="#1F5F4E" stroke-width="1.2" fill="none"/>
  <path d="M137 198 V208" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aC1)"/>
  <path d="M390 198 V208" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aC1)"/>
  <path d="M643 198 V208" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aC1)"/>

  <rect x="30" y="216" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="137" y="240" text-anchor="middle" fill="#1F5F4E">score_board</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="137" y="260" text-anchor="middle">scoring moddé + RplProp</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="137" y="276" text-anchor="middle">HUD + écran de fin</text>

  <rect x="283" y="216" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="390" y="240" text-anchor="middle" fill="#1F5F4E">warm_up</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="390" y="260" text-anchor="middle">état PREGAME</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="390" y="276" text-anchor="middle">condition de démarrage</text>

  <rect x="536" y="216" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="643" y="240" text-anchor="middle" fill="#1F5F4E">kill_logger</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="643" y="260" text-anchor="middle">Instigator + team-kill</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="643" y="276" text-anchor="middle">écriture via FileIO</text>

  <!-- BANDE 02 -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="334" fill="#35547F">02</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="334">CONFIG &amp; PREFABS — rien à coder</text>

  <rect x="250" y="350" width="280" height="50" rx="6" fill="#35547F"/>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="13px" font-weight="700" fill="#FFFFFF" x="390" y="372" text-anchor="middle">Config de faction</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" x="390" y="389" text-anchor="middle" fill="#C3D3E7">le nouveau hub central</text>

  <path d="M390 400 V414" stroke="#35547F" stroke-width="1.2" fill="none" marker-end="url(#aC2)"/>

  <rect x="30" y="420" width="720" height="132" rx="8" fill="none" stroke="#35547F" stroke-dasharray="3 4" opacity="0.55"/>

  <rect x="44" y="434" width="336" height="54" rx="5" fill="#E3EAF3" stroke="#35547F"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="58" y="456" fill="#35547F">infantry_loadouts</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="58" y="474">factions + classes par héritage de prefabs</text>

  <rect x="400" y="434" width="336" height="54" rx="5" fill="#E3EAF3" stroke="#35547F"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="414" y="456" fill="#35547F">IA_skills / difficulty_check</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="414" y="474">SCR_AIConfigComponent sur les prefabs</text>

  <rect x="44" y="498" width="336" height="54" rx="5" fill="#E3EAF3" stroke="#35547F"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="58" y="520" fill="#35547F">radio_settings</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="58" y="538">fréquences &amp; clés dans la config de faction</text>

  <rect x="400" y="498" width="336" height="54" rx="5" fill="#E3EAF3" stroke="#35547F"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="414" y="520" fill="#35547F">vehicles_cargos</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="414" y="538">FillInitialStorages sur prefabs véhicules</text>

  <rect x="30" y="568" width="720" height="58" rx="6" fill="#E3EAF3" stroke="#35547F"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="44" y="590" fill="#35547F">dynamic_startup</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="44" y="608">Scenario Framework (Area / Layer / Slot) + SCR_TaskSystem · spawn dynamique sur radio manpack</text>

  <!-- BANDE 03 -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="666" fill="#5A6265">03</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="666">DÉJÀ NATIF — rien à produire</text>

  <rect x="30" y="682" width="336" height="58" rx="6" fill="#E8EAE9" stroke="#8B9490"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="44" y="704" fill="#5A6265">radio_lock</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="44" y="722">chiffrement par faction déjà natif</text>

  <rect x="414" y="682" width="336" height="58" rx="6" fill="#E8EAE9" stroke="#8B9490"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="428" y="704" fill="#5A6265">test_mode</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="428" y="722">Debug Areas + Diag + Remote Console</text>

  <line x1="40" y1="762" x2="740" y2="762" stroke="#C9D0CC" stroke-width="1"/>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="11px" font-style="italic" fill="#5A6560" x="40" y="782">Couche transversale nouvelle : réplication (RplProp / BumpMe / RplRpc) — sans équivalent en SQF.</text>
</svg>
