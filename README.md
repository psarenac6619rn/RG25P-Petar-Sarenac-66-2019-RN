# Sci-Fi Outpost — RAF Računarska Grafika projekat

Interaktivna 3D OpenGL 3.3 aplikacija napravljena kao demonstracioni projekat za predmet Računarska Grafika.

## Implementirano
- OpenGL 3.3 / GLSL 330
- proceduralno generisan deformisan teren
- proceduralno generisana normal mapa
- hijerarhijska animacija drona i rotora
- Deferred Shading sa G-bufferom
- SSAO efekat
- Sistem čestica
- Post-processing (HDR tone mapping, grayscale, invert)
- Proceduralno noćno nebo
- Brain HyperConnectivity mode (promena palete, chromatic aberration, UV talasanje)
- Proceduralno Energy Tree drvo i interaktivni objekti
- Sistem gravitacije i detekcije podloge
- Ghost Mode slobodno kretanje i Launch Tunnel sekvenca

## Kontrole
- `W A S D` — kretanje
- Miš — pogled
- `1` — SSAO uključi/isključi
- `2` — promeni post-processing efekat
- `E` — interakcija sa objektima u okolini
- `F` — oslobodi/zaključaj miš
- `F1` — uključi/isključi Brain HyperConnectivity mode
- `F2` — pokreni/zaustavi Launch Tunnel sekvencu
- `F3` — uključi/isključi Ghost Mode (odvajanje od fizičkog tela)
- `F4` — potvrdi Ghost Mode poziciju (materijalizacija na novoj lokaciji)
- `Space` / `Left Ctrl` — gore/dole (samo u Ghost Mode-u)
- `Left Shift` — ubrzano kretanje (samo u Ghost Mode-u)
- `Esc` — izlaz

## Pokretanje na Windows-u

Najlakše je pokrenuti preko `POKRENI.bat`.

Potrebni su:
- CMake
- Visual Studio 2022 ili Visual Studio Build Tools sa C++ podrškom
- Internet veza pri prvom build-u (za automatsko preuzimanje GLFW i GLEW biblioteka)

Skripta će automatski konfigurisati projekat, preuzeti biblioteke, generisati i pokrenuti aplikaciju.

## Build (Linux)

Instalirati OpenGL, GLFW i GLEW development pakete, pa zatim:

```bash
cmake -S . -B build
cmake --build build -j
./build/SciFiOutpost
```

