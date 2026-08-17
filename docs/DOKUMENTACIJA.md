# Dokumentacija — Sci-Fi Outpost

## 1. Ideja aplikacije
Sci-Fi Outpost je mala interaktivna 3D scena noćne/svemirske stanice. Korisnik se slobodno kreće kroz scenu FPS kamerom. Centralni animirani objekat je dron sa četiri rotora, a ispod drona se nalazi emiter čestica.

## 2. Generalna 3D grafika
Scena koristi perspektivnu kameru, dubinski test, više transformacija i više dinamičkih izvora svetla. Teren se proceduralno generiše kao mreža trouglova čija se visina računa kombinacijom sinusnih funkcija. Time geometrija nije učitana iz OBJ fajla.

Materijal terena i objekata koristi proceduralno generisanu normal mapu. Normal mapa nastaje u CPU kodu pri pokretanju programa i šalje se kao OpenGL tekstura shaderu, pa projekat koristi i teksturu koja nije albedo/colour tekstura.

## 3. Hijerarhija i animacija
Dron ima roditeljsku transformaciju `droneRoot`. Telo, nosači i rotori računaju svoje transformacije u odnosu na roditeljsku transformaciju. Promena položaja/orijentacije drona zato automatski pomera svu decu. Svaki rotor dodatno dobija sopstvenu brzu rotaciju oko Y ose.

## 4. Tehnika 1 — Deferred Shading
Prvi prolaz renderuje geometriju u G-buffer sa tri render mete:
- view-space position
- view-space normal
- albedo + specular faktor

Drugi lighting prolaz je fullscreen pass. On čita G-buffer i računa osvetljenje za četiri point light izvora. Prednost ovog pristupa je da se geometrija ne renderuje ponovo za svaki izvor svetla.

Relevantni fajlovi: `GBuffer.cpp`, `gbuffer.vert`, `gbuffer.frag`, `lighting.frag`.

## 5. Tehnika 2 — SSAO
SSAO koristi 32 nasumična uzorka u hemisferi i 4x4 noise teksturu. Za svaki fragment uzorci se rotiraju TBN bazom formiranom iz normale i noise vektora, projektuju nazad u screen-space i porede sa dubinom iz G-buffera. Dobijeni faktor ambijentalne okluzije utiče na ambient komponentu u deferred lighting prolazu.

Taster `1` demonstrira razliku sa i bez SSAO-a.

Relevantni fajlovi: `SSAO.cpp`, `ssao.frag`.

## 6. Tehnika 3 — Animated Particle System
Particle sistem održava niz čestica sa položajem, brzinom i vremenom života. Kada čestica istekne, ponovo se emituje ispod drona. Čestice se ažuriraju svakog frejma na CPU-u i renderuju kao `GL_POINTS` sa aditivnim blendingom.

Emiter prati dron pomoću hijerarhijske transformacije, tako da je sistem vezan za animirani objekat.

Relevantni fajlovi: `Particles.cpp`, `particle.vert`, `particle.frag`.

## 7. Post-processing pipeline
Lighting rezultat se prvo renderuje u `PostProcess` framebuffer umesto direktno na ekran. Poslednji fullscreen pass zatim čita tu teksturu i primenjuje izabrani efekat. Ovakva struktura odvaja renderovanje scene od screen-space obrade i lako se proširuje dodatnim efektima/među-framebufferima.

Implementirani režimi su:
- bez efekta
- HDR exponential tone mapping + gamma correction
- grayscale
- invert

Taster `2` menja efekat tokom rada.

Relevantni fajlovi: `PostProcess.cpp`, `post.frag`.

## 8. Podela rada
Ako projekat radi jedan student: sve stavke navesti uz svoje ime i indeks.

Ako projekat rade dva studenta, pre predaje zameniti ovaj deo stvarnom podelom, npr. Student A — Deferred Shading + scena; Student B — SSAO + particle sistem; zajednički — post-processing, testiranje i dokumentacija. Commit istorija mora odgovarati stvarnom radu.

## 9. Moguća pitanja na odbrani
Treba znati objasniti zašto se koristi G-buffer, šta znači view-space, kako se SSAO kernel formira, zašto noise tekstura smanjuje vidljive šablone, šta znači životni vek čestice, kako radi parent-child transformacija, zašto se blending koristi za čestice i zašto se post-processing radi fullscreen quad-om.


## Proceduralno nebo i Brain HyperConnectivity mode

Pozadina scene je proceduralno generisana u `lighting.frag`. Kada G-buffer nema geometriju, shader prikazuje nocni gradijent, proceduralne zvezde, maglicu i svetlosni izvor. Za nebo nisu potrebne spoljne teksture.

Taster `F1` ukljucuje/iskljucuje Brain HyperConnectivity mode. Efekat je podeljen na dva nivoa:
- `gbuffer.vert` blago deformise verteks pozicije sinusnim talasima, zbog cega objekti i teren deluju kao da "disu" i plivaju;
- `post.frag` pomera UV koordinate celog kadra, dodaje talase, blagu radialnu distorziju, chromatic aberration, hue shift i pulsiranje intenziteta boja.

Ovaj rezim je namerno implementiran shaderima tako da se moze ukljuciti u realnom vremenu bez menjanja scene ili ponovnog ucitavanja resursa.


## Brain HyperConnectivity Energy Tree proširenje
F1 režim dodatno demonstrira proceduralnu grafiku kroz mrežu sazvežđa, polarno/kaleidoskopski
generisanu fraktalnu mandalu na zenitu i centralno proceduralno drvo. Drvo je hijerarhija
transformisanih primitiva, bez OBJ modela. Njegove grane dobijaju vremenski zavisno njihanje,
a zaseban particle emitter vizuelno prenosi energiju od drveta ka nebu. Lighting shader
generiše svetleće "žilice" na zelenim/braon materijalima drveta.


## F2 Launch/Tunnel i proceduralno cveće

Launch režim je vremenski kontrolisana sekvenca povezana sa postojećim bhc post-processing pipeline-om.
`launchProgress` se povećava od 0 do 1 i kontroliše fog fazu, pomeranje/orijentaciju kamere,
radijalne tunelske prstenove, speed-line efekat, kaleidoskopske fractal rosette i intenzitet
zenitnog izvora. F2 ponovo prekida sekvencu i vraća sačuvanu poziciju/orijentaciju kamere.

Cveće je proceduralna hijerarhijska geometrija iz primitiva. U bhc režimu aplikacija računa
dot-product između `Camera::front()` i normalizovanog pravca ka svakom cvetu. Kada je cvet
dovoljno blizu centra pogleda, `focusStrength` se glatko povećava. On istovremeno kontroliše
FOV zoom, emissive/fractal odgovor latica i dodatne rosette u završnom post-processing prolazu.


## Ghost / noclip sistem (F3 + F4)

F3 aktivira privremeni 3D „ghost“ režim. Pri ulasku se čuvaju `Camera::pos`, `yaw` i
`pitch`. U ovom režimu se više ne koristi standardno horizontalno `Camera::move()`
kretanje, već se pozicija direktno integriše iz punog `Camera::front()` vektora,
desnog vektora i globalnog Y vektora, pa je moguće prolaziti i vertikalno kroz scenu.

Ponovni F3 vraća zapamćenu poziciju/orijentaciju, čime ghost služi kao bezbedan preview.
F4 umesto toga potvrđuje trenutnu ghost transformaciju i ona postaje nova stvarna
pozicija kamere/igrača.


## Gravitacija / terrain grounding

Normalna kamera koristi `terrainHeight(x,z)` kao visinu kolizione aproksimacije tla i
konstantu `playerEyeHeight` za visinu očiju. Vertikalna brzina se integriše sa gravitacionim
ubrzanjem dok kamera ne stigne do tla. U F3 režimu se ovaj sistem suspenduje i dozvoljava
se puni 3D noclip. F4 izlazi iz ghost režima, nakon čega gravitacija ponovo postaje aktivna.

