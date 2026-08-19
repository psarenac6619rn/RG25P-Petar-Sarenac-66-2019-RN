# Dokumentacija — Sci-Fi Outpost

**Student:** Petar Šarenac | **Indeks:** 66/2019 | **Predmet:** Računarska grafika (RG25P)

---

## 1. Ideja aplikacije

Za projekat sam napravio malu interaktivnu 3D scenu koja prikazuje noćnu svemirsku stanicu. Ideja mi je bila da igrač može slobodno da se kreće kroz scenu FPS kamerom i da istražuje okruženje. Centralni animirani objekat je dron sa četiri rotora koji kruži iznad scene, a ispod njega sam postavio emiter čestica koji vizuelno dočarava propulziju.

Scena sadrži više elemenata: proceduralno stablo energije, ribnjak sa patkom koja kruži, sto sa stolicom i novinama, cveće po terenu i nekoliko post-processing efekata koji dramatično menjaju izgled.

---

## 2. Generalna 3D grafika

Koristim perspektivnu kameru, dubinski test, više matričnih transformacija i više dinamičkih izvora svetlosti. Teren sam proceduralno generisao kao mrežu trouglova, pri čemu se visina svake tačke računa kombinacijom sinusnih funkcija:

```
y = 0.18*sin(px*0.65)*cos(pz*0.52) + 0.05*sin((px+pz)*1.4)
```

Na ovaj način geometrija terena nije učitana iz eksternog OBJ fajla, već nastaje matematički pri pokretanju programa.

Za materijale sam koristio proceduralno generisanu normal mapu. Ona nastaje u CPU kodu pri inicijalizaciji i šalje se kao OpenGL tekstura shaderu. Ovim sam ispunio uslov da projekat koristi teksturu koja nije klasična albedo/color tekstura.

---

## 3. Hijerarhija i animacija

Dron ima roditeljsku transformaciju `droneRoot` koja se računa svaki frejm na osnovu trigonometrijskih funkcija. Telo drona, nosači i rotori računaju svoje transformacije **relativno u odnosu na `droneRoot`**, pa pomeranje roditeljske transformacije automatski pomera celu hijerarhiju. Svaki rotor dodatno ima sopstvenu brzu rotaciju oko Y ose (`rotateY(now*14)`).

Energetsko stablo je takođe hijerarhijska struktura — deblo, grane, listovi i "jezgro" se crtaju redom, a svaki nivo grana ima ugao koji zavisi od globalnog vremena.

---

## 4. Tehnika 1 — Deferred Shading

Implementirao sam **Deferred Rendering** u dva prolaza.

**Geometry pass** renderuje celu scenu u G-buffer koji se sastoji od tri teksture:
- `gPosition` — pozicija fragmenta u view prostoru (RGB16F)
- `gNormal` — normala fragmenta u view prostoru (RGB16F)
- `gAlbedo` — boja objekta + speculat faktor u alpha kanalu (RGBA16F)

**Lighting pass** je fullscreen prolaz koji čita G-buffer i računa Blinn-Phong osvetljenje za četiri tačkasta izvora svetlosti. Prednost ovakvog pristupa je da se geometrija prolazi samo jednom, a osvetljenje se računa u jednom fullscreen prolasku bez obzira na broj svetala.

Relevantni fajlovi: `GBuffer.cpp`, `GBuffer.h`, `gbuffer.vert`, `gbuffer.frag`, `lighting.frag`.

---

## 5. Tehnika 2 — SSAO (Screen-Space Ambient Occlusion)

SSAO simulira senčenje u udubinama i uglovima — mjestima gde svetlost teže dopire.

Implementirao sam ga sa **32 nasumična uzorka** u hemisferi i **4×4 noise teksturom**. Za svaki fragment:
1. Formiram TBN bazu iz normale fragmenta i nasumičnog noise vektora
2. Transformišem kernel uzorke u view prostor
3. Projicujem svaki uzorak u screen-space i čitam dubinu iz G-buffer-a
4. Ako je uzorak "ispred" površine — to je okluzija
5. Prosečujem 32 rezultata u vrednost [0, 1]

Noise tekstura rotira kernel po ekranu i na taj način smanjuje vidljive šablone koji bi nastali od malog broja uzoraka.

Taster `1` u toku rada uključuje/isključuje SSAO, pa se vidi direktna razlika u kvalitetu senčenja.

Relevantni fajlovi: `SSAO.cpp`, `SSAO.h`, `ssao.frag`.

---

## 6. Tehnika 3 — Animirani sistem čestica

Sistem čestica održava niz `Particle` struktura od kojih svaka ima poziciju, brzinu i vreme života. Svaki frejm:
1. Svakoj čestici se smanjuje `life` za `dt`
2. Ako je `life <= 0`, čestica se respawn-uje oko emitera sa nasumičnom pozicijom i brzinom
3. Primenjujem blagu anti-gravitacionu silu (čestica lagano lebdi nagore)
4. Pozicija se integruje: `pos += velocity * dt`

Čestice se renderuju kao `GL_POINTS` sa aditivnim blendingom — tako se efektno sabiru tamo gde se preklapaju. Emiter čestica ispod drona prati dron pomoću hijerarhijske transformacije.

Koristim tri instance sistema:
- **dron emiter** (220 čestica) — uvek aktivan
- **treeEnergy** (420 čestica) — aktivno samo u BHC modu
- **launchMist** (520 čestica) — aktivno tokom launch sekvence

Relevantni fajlovi: `Particles.cpp`, `Particles.h`, `particle.vert`, `particle.frag`.

---

## 7. Post-processing pipeline

Lighting rezultat renderujem u `PostProcess` framebuffer umesto direktno na ekran. Poslednji fullscreen prolaz čita tu teksturu i primenjuje izabrani efekat. Ovakva arhitektura odvaja renderovanje scene od obrade slike i lako se proširuje novim efektima.

Implementirani režimi (prebacivanje tasterom `2`):
- `0` — bez efekta (sirovi HDR izlaz)
- `1` — HDR exponential tone mapping + gamma korekcija (2.2)
- `2` — Grayscale (luminance formula)
- `3` — Invertovane boje

Relevantni fajlovi: `PostProcess.cpp`, `PostProcess.h`, `post.frag`.

---

## 8. Proceduralno nebo i Brain HyperConnectivity mod

Pozadina scene je u potpunosti proceduralna — renderujem je direktno u `lighting.frag`. Kada G-buffer nema geometriju na nekom fragmentu (normala ≈ 0), shader prikazuje noćni gradijent neba, proceduralne zvezde sa treperenjem, konstelacionu mrežu linija i svetlosni glow. Nisu korišćene nikakve spoljne sky teksture.

Taster **F1** uključuje Brain HyperConnectivity (BHC) mod koji menja vizuelni doživljaj na svim nivoima pipeline-a:
- `gbuffer.vert` — deformiše verteks pozicije sinusnim talasima, objekti "dišu"
- `lighting.frag` — psihedelična paleta boja, žile svetlosti na stablu, cvetni glow, aurora na nebu, fraktalna mandala na zenitu
- `post.frag` — talasanje UV koordinata, blaga radijalna distorzija, chromatic aberration, hue shift, vinjeta

BHC efekat je implementiran isključivo u shaderima, pa se može uključiti u realnom vremenu bez menjanja scene.

---

## 9. Energy Tree i BHC proširenje

U BHC modu aktivira se proceduralno energetsko stablo na koordinatama `{0, 0, -2.80}`. Stablo je hijerarhija transformisanih kocki — deblo u 5 segmenata, zatim tri nivoa grana sa po 8 grana po nivou, i listovi na vrhovima. Sve grane dobijaju vremenski zavisno njihanje.

U BHC modu lighting shader generiše svetleće "žilice" na zelenim/braon materijalima stabla, a poseban particle emiter (420 čestica) prenosi energiju vizuelno od stabla ka nebu. Jezgro stabla pulsiše zelenim svetlom.

Na nebu se pojavljuju:
- Mreža linija između zvezda (proceduralne konstelacije)
- Fraktalna mandala sa prstenovima, zracima i rekurzivnim fraktalima
- Aurora borealis efekat
- Centralna os svetlosti

---

## 10. F2 Launch sekvenca i proceduralno cveće

Launch mod (taster **F2**) je vremenski kontrolisana sekvenca. Varijabla `launchProgress` raste od 0 do 1 tokom 10 sekundi i kontroliše:
- Kretanje i nagib kamere prema gore
- Fog fazu (magla na početku)
- Radijalne tunelske prstenove i speed-line efekat u `post.frag`
- Kaleidoskopske fraktalne rozete
- Intenzitet zenitnog izvora svetlosti

Cveće je proceduralna hijerarhijska geometrija (stablo + latice + centar). U BHC modu računam `dot(Camera::front(), pravac_ka_cvetu)` za svaki cvet. Kad je cvet dovoljno blizu centra pogleda, `focusStrength` se glatko povećava i kontroliše istovremeno:
- FOV zoom (od 60° do 30°)
- Emisivni/fraktalni odgovor latica u lighting shaderu
- Dodatne fraktalne rozete u post-processing prolazu

---

## 11. Ghost / noclip sistem (F3 + F4)

Taster **F3** aktivira "ghost" mod u kome igrač može da leti slobodno kroz scenu. Pri aktivaciji čuvam tekuće `Camera::pos`, `yaw` i `pitch`. U ghost modu koristim puni `Camera::front()` vektor za integrisanje pozicije (umesto horizontalnog kretanja), pa je moguće prolaziti i vertikalno.

Na mestu gde je ghost mod aktiviran ostaje vidljivo 3D telo igrača (`drawGhostBody`) — hijerarhijska figura iz kocki.

Ponovni **F3** vraća zapamćenu poziciju i orijentaciju. **F4** potvrđuje trenutnu ghost poziciju i ona postaje nova stvarna pozicija igrača.

---

## 12. Gravitacija i terrain grounding

Kamera koristi funkciju `terrainHeight(x, z)` kao aproksimaciju kolizije s tlom, zajedno sa konstantom `playerEyeHeight = 1.72f`. Vertikalna brzina se integriše sa gravitacionim ubrzanjem `g = -18 m/s²` dok kamera ne dosegne visinu tla. U ghost modu ovaj sistem se suspenduje.

---

## 13. Sedenje i interaktivni meni iz novina

Na sceni se nalazi stolica pored stola. Pritiskom **E** u blizini stolice kamera se teleportuje na poziciju `{1.45, 1.18, 2.20}` i igrač sedi. Novo **E** vraća igrača na prethodnu poziciju.

Na stolu se nalaze novine — 3D objekat od dve stranice sa blagom animacijom lepršanja. Pritiskom **E** pored novina otvara se interaktivni meni u stilu novina koji prikazuje:
- **[R]** — Reset igre (vraća kameru na početak, gasi sve modove)
- **[ESC]** — Izlaz iz igre

Dok je meni otvoren kursor postaje slobodan (nije zaključan).

---

## 14. Kontrole

| Taster | Akcija |
|---|---|
| W/A/S/D | Kretanje |
| Miš | Okretanje pogleda |
| E | Interakcija sa objektom |
| F | Toggle slobodan/zaključan miš |
| 1 | Toggle SSAO |
| 2 | Ciklus post-processing efekata |
| F1 | Toggle Brain HyperConnectivity mod |
| F2 | Toggle Launch sekvenca |
| F3 | Toggle Ghost (noclip) mod |
| F4 | Materijalizacija na ghost poziciji |
| Space / Ctrl | Let gore / dole (Ghost mod) |
| Shift | Turbo brzina (Ghost mod) |
| R | Reset igre (u meniju novina) |
| ESC | Izlaz |

---

## 15. Pitanja na odbrani

Trebalo bi biti spreman da objasnim:
- Zašto koristim G-buffer i u čemu je prednost deferred u odnosu na forward renderovanje
- Šta znači view-space i zašto SSAO radi u view-space-u
- Kako se formira SSAO kernel i zašto noise tekstura smanjuje vidljive šablone
- Šta znači životni vek čestice i kako funkcioniše respawn mehanizam
- Kako funkcioniše parent-child transformacija kod drona
- Zašto se koristi aditivni blending za čestice
- Zašto se post-processing radi fullscreen quadom
- Kako proceduralna normal mapa utiče na vizuelni rezultat bez dodavanja novih poligona
