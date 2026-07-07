# Pure3D Runtime And Rendering Options

Research snapshot: 2026-07-07. This repo contains only `game/code` — the Pure3D,
PDDI, RAD, and Scrooby middleware sources are absent, so Phase 4 (rendering
bring-up) needs either a compatibility layer or an existing implementation.

## Key Finding

No independent open-source project reimplements the Pure3D runtime C++ API
(`tEntity`, `tShader`, `p3d::device`, PDDI). However, the console-port lineage
of the same source restores and ports the full middleware tree:

- **ZenoArrows/The-Simpsons-Hit-and-Run** (Switch/Vita port, active Nov 2025):
  full `libs/{pure3d, radcore, radmath, radsound, radmovie, radmusic,
  radcontent, radscript, choreo, poser, sim, scrooby}` with working PDDI
  backends: `pddi/gl` (desktop OpenGL incl. `display_linux/`), `pddi/gles`
  (GLES 1/2 with GLSL path), `pddi/gxm` (Vita; template for new backends), and
  the original `pddi/dx8` (behavioral reference for every `tShader` variant).
  CMake + SDL2/SDL3 + OpenAL + FFmpeg. Full game playable start to finish.
- **Carlox33/The-Simpsons-Hit-and-Run-Android** (fork, active Jul 2026): best-
  tested GLES variant; story mode fully playable at 60 FPS.
- **3UR/Simpsons-Hit-Run** (Xbox UWP, active Jul 2026): another maintained
  middleware restoration, D3D-oriented.

License caveat: these are derived from the same 2021 source leak as this repo —
no valid open-source license, but no additional legal exposure relative to the
code already being ported.

## License-Clean References (format level)

| Project | License | Use |
| --- | --- | --- |
| Donut Team docs (docs.donutteam.com/docs/pure3d) | MIT | canonical .p3d chunk spec |
| plowteam/donut `dev/codegen/p3d.json` | GPL-3 | machine-readable table of 107/182 chunk types |
| Hampo/LuaP3DLib | MIT | read/write chunk layouts, actively maintained |
| donutteam/pure3d-ts | MIT | read/write, tracks Donut Team docs |
| tigercat2000/p3dtools | MIT-0 | .p3d → glTF, freely reusable |
| aap/scarface-p3d | MIT | only license-clean C++ sketch of PDDI-style architecture |
| plowteam/donut (engine) | GPL-3 | from-scratch SHAR engine; not PDDI-shaped, not graftable |

## Recommendation For This Port

Do not write a PDDI layer from scratch. Base rendering bring-up on the
ZenoArrows middleware tree:

1. Diff its `game` tree against this repo to understand its game-side changes.
2. Adopt `libs/pure3d` with the `pddi/gl` backend for Linux; on macOS prefer
   the `pddi/gles` shader path (via ANGLE), since core-profile GL is a dead end
   for fixed-function code.
3. Reuse its CMake/SDL/OpenAL/FFmpeg platform layer where it fits the existing
   PoC scaffold.
4. Use `pddi/dx8/shaders/` as the spec for shader behavior and `pddi/gxm` as
   the template if a Metal/Vulkan backend is wanted later.

The PoC's asset-request manifest enumerates which .p3d files (and eventually
chunk types) the runtime must support at each milestone.
