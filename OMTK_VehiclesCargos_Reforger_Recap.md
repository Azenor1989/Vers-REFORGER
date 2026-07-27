<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 780 800" width="780" height="800" role="img" aria-label="Synoptique de l'OMTK actuel sous Arma 3 : dépendances, point d'entrée unique, modules cœur liés entre eux, modules autonomes, outils externes.">
  <defs>
    <marker id="aG" viewBox="0 0 8 8" refX="7" refY="4" markerWidth="5" markerHeight="5" orient="auto">
      <path d="M0 0 L8 4 L0 8 z" fill="#1F5F4E"/>
    </marker>
  </defs>

  <rect width="780" height="800" fill="#F1F3F2"/>

  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="21px" font-weight="700" fill="#16211D" x="40" y="46">Synoptique — OMTK aujourd'hui (Arma 3 / SQF)</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" fill="#5A6560" x="40" y="70">Un point d'entrée unique, et presque tout est du script maintenu par l'OFCRA</text>
  <line x1="40" y1="88" x2="740" y2="88" stroke="#C9D0CC" stroke-width="1"/>

  <!-- 01 DÉPENDANCES -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="116" fill="#5A6265">01</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="116">DÉPENDANCES <tspan font-size="11px" font-weight="400" fill="#5A6560" letter-spacing="0">— chargées avant la mission</tspan></text>

  <rect x="30" y="130" width="336" height="48" rx="6" fill="#E8EAE9" stroke="#8B9490"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="44" y="151" fill="#5A6265">@RHSmod</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="44" y="168">requis — classes d'unités et d'équipement</text>

  <rect x="414" y="130" width="336" height="48" rx="6" fill="#E8EAE9" stroke="#8B9490"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="428" y="151" fill="#5A6265">@ACEmod</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="428" y="168">optionnel — menus d'interaction</text>

  <!-- 02 POINT D'ENTRÉE -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="208" fill="#1F5F4E">02</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="208">POINT D'ENTRÉE UNIQUE</text>

  <rect x="210" y="222" width="360" height="56" rx="6" fill="#1F5F4E"/>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="13.5px" font-weight="700" fill="#FFFFFF" x="390" y="245" text-anchor="middle">Mission Eden — mission.sqm</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="11px" x="390" y="263" text-anchor="middle" fill="#C0DED3">init.sqf · description.ext</text>

  <path d="M390 278 V296" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aG)"/>

  <!-- 03 MODULES CŒUR -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="312" fill="#1F5F4E">03</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="312">MODULES CŒUR <tspan font-size="11px" font-weight="400" fill="#5A6560" letter-spacing="0">— seuls modules réellement liés entre eux</tspan></text>

  <rect x="283" y="326" width="214" height="64" rx="6" fill="#E2EFEA" stroke="#1F5F4E" stroke-width="1.6"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="390" y="349" text-anchor="middle" fill="#1F5F4E">score_board</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="390" y="368" text-anchor="middle">objectifs + score final</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="390" y="382" text-anchor="middle">OMTK_SB_LIST_OBJECTIFS</text>

  <path d="M137 430 L300 394" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aG)"/>
  <path d="M643 430 L480 394" stroke="#1F5F4E" stroke-width="1.2" fill="none" marker-end="url(#aG)"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="10px" font-weight="700" fill="#1F5F4E" x="212" y="410" text-anchor="middle">OMTK_ID</text>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="10px" font-weight="700" fill="#1F5F4E" x="568" y="410" text-anchor="middle">objectifs</text>

  <rect x="30" y="430" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="137" y="454" text-anchor="middle" fill="#1F5F4E">infantry_loadouts</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="137" y="474" text-anchor="middle">équipe selon classe + camp</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="137" y="490" text-anchor="middle">bluefor/redfor_classes.sqf</text>

  <rect x="283" y="430" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="390" y="454" text-anchor="middle" fill="#1F5F4E">kill_logger</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="390" y="474" text-anchor="middle">hits, kills, tirs amis</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="390" y="490" text-anchor="middle">hook onPlayerKilled.sqf</text>

  <rect x="536" y="430" width="214" height="76" rx="6" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="643" y="454" text-anchor="middle" fill="#1F5F4E">dynamic_startup</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#16211D" x="643" y="474" text-anchor="middle">marqueurs + spawns</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="643" y="490" text-anchor="middle">générés au lancement</text>

  <!-- 04 MODULES AUTONOMES -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="538" fill="#1F5F4E">04</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="538">MODULES AUTONOMES <tspan font-size="11px" font-weight="400" fill="#5A6560" letter-spacing="0">— aucun lien entre eux, tous lisent la config centrale</tspan></text>

  <rect x="30" y="552" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="114" y="579" text-anchor="middle" fill="#1F5F4E">radio_lock</text>
  <rect x="214" y="552" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="298" y="579" text-anchor="middle" fill="#1F5F4E">radio_settings</text>
  <rect x="398" y="552" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="482" y="579" text-anchor="middle" fill="#1F5F4E">vehicles_cargos</text>
  <rect x="582" y="552" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="666" y="579" text-anchor="middle" fill="#1F5F4E">difficulty_check</text>

  <rect x="30" y="606" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="114" y="633" text-anchor="middle" fill="#1F5F4E">IA_skills</text>
  <rect x="214" y="606" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="298" y="633" text-anchor="middle" fill="#1F5F4E">warm_up</text>
  <rect x="398" y="606" width="168" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="482" y="633" text-anchor="middle" fill="#1F5F4E">test_mode</text>

  <!-- 05 OUTILS EXTERNES -->
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="12px" font-weight="700" x="40" y="682" fill="#1F5F4E">05</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="12.5px" font-weight="700" fill="#16211D" letter-spacing=".4px" x="68" y="682">OUTILS EXTERNES <tspan font-size="11px" font-weight="400" fill="#5A6560" letter-spacing="0">— dépôts compagnons à maintenir à part</tspan></text>

  <rect x="30" y="696" width="336" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E" stroke-dasharray="4 3"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="44" y="715" fill="#1F5F4E">omtk-groups</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="44" y="730">groupes d'unités pour l'éditeur</text>

  <rect x="414" y="696" width="336" height="44" rx="5" fill="#E2EFEA" stroke="#1F5F4E" stroke-dasharray="4 3"/>
  <text font-family="ui-monospace, 'SF Mono', 'Cascadia Mono', Menlo, Consolas, monospace" font-size="11px" font-weight="700" x="428" y="715" fill="#1F5F4E">OMTK-loadouts</text>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="10.5px" fill="#5A6560" x="428" y="730">cargos véhicules → mission.sqm</text>

  <line x1="40" y1="760" x2="740" y2="760" stroke="#C9D0CC" stroke-width="1"/>
  <text font-family="system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif" font-size="11px" font-style="italic" fill="#5A6560" x="40" y="780">Sortie transversale : HH:MM:SS [OMTK] INFO — chat système + fichier .RPT du serveur.</text>
</svg>
