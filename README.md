# Sci-Fi Outpost — RAF Računarska Grafika projekat

Interaktivna 3D OpenGL 3.3 aplikacija napravljena kao demonstracioni projekat za predmet Računarska Grafika.

## Implementirano
- OpenGL 3.3 / GLSL 330
- proceduralno generisan deformisan teren (nije OBJ)
- proceduralno generisana normal mapa (tekstura pored albedo boje)
- hijerarhijska animacija drona i 4 rotora
- **Deferred Shading** sa G-bufferom (position, normal, albedo/specular)
- **SSAO** sa kernelom od 32 uzorka i 4x4 noise teksturom
- **Animated Particle System** vezan za emiter na dronu
- fleksibilan završni post-processing prolaz sa više efekata
- HDR tone mapping/gamma kao glavni post-processing efekat
- proceduralno nocno nebo sa zvezdama i sci-fi maglicom (bez spoljne skybox teksture)
- **F1 DMT Visual mode**: promena palete, chromatic aberration, UV talasanje, drift ekrana i blaga vertex deformacija geometrije

- **Centralno proceduralno Energy Tree drvo** — hijerarhijski generisan trup, grane i krošnja bez OBJ modela
- u DMT modu zvezde se povezuju animiranom svetlosnom **constellation network** mrežom
- animirana **sacred-geometry / fractal mandala** direktno na zenitu
- vertikalni energetski kanal između drveta i neba + dodatni emitter čestica iz drveta
- fraktalno/svetleće "žilice" na materijalu drveta u DMT režimu

## Kontrole
- `W A S D` — kretanje
- miš — pogled
- `1` — SSAO uključi/isključi
- `2` — promeni post-processing efekat (none / tone map / grayscale / invert)
- `F` — oslobodi/zaključaj miš
- `F1` — uključi/isključi **DMT Visual mode**
- `Esc` — izlaz

## Pokretanje na Windows-u

Najlakse je da dvaput kliknes na `POKRENI.bat`.

Za ovaj launcher **nisu potrebni Git ni vcpkg**. Potrebni su:
- CMake
- Visual Studio 2022 ili Visual Studio Build Tools sa C++ podrskom
- internet veza pri prvom build-u, jer CMake automatski preuzima GLFW i GLEW kao ZIP pakete

`POKRENI.bat` zatim automatski:
1. konfigurise projekat,
2. preuzima potrebne biblioteke pri prvom build-u,
3. pravi Release verziju,
4. pokrece `SciFiOutpost.exe`.

Sledeca pokretanja koriste vec napravljeni executable dok god postoji `build/Release/SciFiOutpost.exe`.

## Build (Linux)
Instalirati OpenGL, GLFW i GLEW development pakete, pa:

```bash
cmake -S . -B build
cmake --build build -j
./build/SciFiOutpost
```

## Napomena za predaju
Pre predaje ukloniti `build/` i sve izvršne fajlove iz ZIP-a.



## F2 Launch / Tunnel i Flower Focus

- **F2** uključuje/isključuje automatsku launch sekvencu.
- Pri aktivaciji se pojavljuje proceduralna ljubičasto-plava magla oko posmatrača.
- Kamera se tokom ~10 sekundi postepeno usmerava i podiže ka zenitnom izvoru.
- Post-processing generiše perspektivne tunelske prstenove, radialne speed streaks i više animiranih fractal/rosette „realm“ obrazaca.
- Ponovni **F2** prekida sekvencu i vraća kameru na poziciju sa koje je launch započet.
- Po terenu je dodato proceduralno cveće (stem + radial petals + core, bez OBJ modela).
- Kada je **F1 DMT mode** aktivan i kamera gleda direktno u cvet, program automatski povećava `focusStrength`: FOV se sužava sa 60 na približno 30 stepeni, cvet svetli jače, a fraktali u kadru se pojačavaju.


## F3 Ghost Mode / F4 Commit

- **F3** prvi put uključuje Ghost Mode i pamti trenutnu realnu poziciju i smer kamere.
- Dok je Ghost Mode aktivan:
  - **W / S**: slobodno napred / nazad u punom 3D pravcu pogleda
  - **A / D**: levo / desno
  - **Space**: gore
  - **Left Ctrl**: dole
  - **Left Shift**: ubrzano ghost kretanje
- Ponovni **F3** otkazuje ghost putovanje i vraća igrača tačno na sačuvanu početnu poziciju i smer.
- **F4** potvrđuje trenutnu ghost poziciju: ona postaje nova realna pozicija igrača i Ghost Mode se gasi.
- Ghost Mode i F2 Launch/Tunnel su međusobno isključivi; F3 tokom tunela prvo bezbedno prekida launch i vraća kameru na njegovu početnu tačku.


### Vidljivo telo u Ghost Mode-u
F3 ostavlja proceduralno telo na sacuvanoj pocetnoj poziciji. Ghost kamera moze da
odleti i vidi telo spolja. Drugi F3 vraca kameru u telo; F4 potvrduje novu ghost
lokaciju kao novu fizicku poziciju i uklanja staro telo.


## Gravitacija i uzemljeno kretanje

- U normalnom režimu igrač više ne može da lebdi: Y pozicija se vezuje za proceduralnu visinu terena.
- Ako se fizička kamera nađe iznad zemlje, primenjuje se ubrzanje gravitacije dok ne dotakne teren.
- **Slobodan let je moguć samo u F3 Ghost Mode-u.**
- Dok je F3 aktivan, gravitacija je isključena i `Space` / `Left Ctrl` daju kretanje gore/dole.
- Ponovni **F3** vraća duha u telo koje ostaje na zemlji.
- **F4** materijalizuje telo na trenutnom X/Z mestu duha; ako je duh bio visoko u vazduhu, fizičko telo zatim pada pod gravitacijom do tla.
- Vidljivo telo koje ostaje iza pri F3 uvek se renderuje na visini proceduralnog terena.


## Interaction-ready layout
Cvece sada prolazi kroz `flowerPlacementAllowed()` filter. Kandidat se odbacuje ako
upada u zonu barice/obodnog kamenja, stola i stolice ili lezaljke. Time cvece ostaje
vizuelno odvojeno i nakon buducih promena rasporeda.

Dodat je centralni `Interactable` sistem:
- `InteractType`
- pozicija
- interaction radius
- opcioni indeks objekta
- naziv

`E` sada koristi jedan centralni dispatcher za Chair, Table, Energy Tree, Pond,
Lounger, Flower i pokretnu Duck interakciju. Stolica zadrzava pravo sedenje, flower
interakcija koristi postojeci focus sistem, a ostali objekti imaju ciste hook tacke
spremne za sledece faze bez dodavanja novih razbacanih input provera.
