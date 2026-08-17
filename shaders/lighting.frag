#version 330 core
out vec4 FragColor;
in vec2 UV;
uniform sampler2D gPosition, gNormal, gAlbedo, ssaoTex;
uniform vec3 lights[4];
uniform vec3 lightColors[4];
uniform int ssaoEnabled;
uniform float time;
uniform int bhcMode;
uniform float focusStrength;
uniform float launchProgress;
float hash21(vec2 p){
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}
vec2 hash22(vec2 p){
    float n = hash21(p);
    return vec2(n, hash21(p + 37.91));
}
float lineSegment(vec2 p, vec2 a, vec2 b, float width){
    vec2 pa = p-a, ba = b-a;
    float h = clamp(dot(pa,ba)/max(dot(ba,ba),0.0001), 0.0, 1.0);
    return 1.0-smoothstep(width, width*2.3, length(pa-ba*h));
}
float starPoint(vec2 uv, vec2 cell, vec2 grid){
    vec2 rnd = hash22(cell);
    vec2 p = (cell + 0.15 + rnd*0.70) / grid;
    float d = length((uv-p)*vec2(1.0, 1.75));
    return exp(-d*d*180000.0);
}
float constellationNetwork(vec2 uv){
    vec2 grid = vec2(24.0, 13.5);
    vec2 gp = uv * grid;
    vec2 base = floor(gp);
    float net = 0.0;
    for(int y=-1;y<=1;y++){
        for(int x=-1;x<=1;x++){
            vec2 c = base + vec2(x,y);
            vec2 a = (c + 0.15 + hash22(c)*0.70) / grid;
            vec2 cr = c + vec2(1.0,0.0);
            vec2 cu = c + vec2(0.0,1.0);
            vec2 cd = c + vec2(1.0,1.0);
            vec2 b = (cr + 0.15 + hash22(cr)*0.70) / grid;
            vec2 d = (cu + 0.15 + hash22(cu)*0.70) / grid;
            vec2 e = (cd + 0.15 + hash22(cd)*0.70) / grid;
            float gate = smoothstep(0.35,0.75,hash21(c+8.2));
            net += lineSegment(uv,a,b,0.00055)*gate;
            net += lineSegment(uv,a,d,0.00048)*smoothstep(0.50,0.82,hash21(c+19.7));
            net += lineSegment(uv,a,e,0.00036)*smoothstep(0.67,0.88,hash21(c+33.1));
        }
    }
    return clamp(net,0.0,1.0);
}
float zenithMandala(vec2 uv){
    vec2 p = uv - vec2(0.5,0.86);
    p.x *= 1.78;
    float r = length(p);
    float a = atan(p.y,p.x);
    float fade = 1.0-smoothstep(0.08,0.62,r);
    float rings = 0.0;
    for(int i=1;i<=6;i++){
        float rr = float(i)*0.072 + 0.008*sin(time*0.55+float(i));
        rings += 1.0-smoothstep(0.0025,0.008,abs(r-rr));
    }
    float spokes = pow(abs(cos(a*12.0 + 0.15*sin(time*0.35))),28.0);
    spokes *= smoothstep(0.04,0.13,r)*(1.0-smoothstep(0.45,0.56,r));
    vec2 q = p;
    float fractal = 0.0;
    for(int i=0;i<4;i++){
        q = abs(q)/max(dot(q,q),0.16) - vec2(0.72,0.54);
        fractal += exp(-18.0*abs(length(q)-0.52));
    }
    fractal *= 0.18 * fade;
    float flower = pow(abs(sin(a*6.0 + sin(r*26.0-time*0.45))),18.0);
    flower *= (1.0-smoothstep(0.33,0.53,r))*smoothstep(0.05,0.11,r);
    return clamp((rings*0.72 + spokes*0.85 + flower*0.65 + fractal)*fade,0.0,1.0);
}
vec3 proceduralSky(vec2 uv){
    float horizon = smoothstep(0.0,1.0,uv.y);
    vec3 low = vec3(0.018,0.028,0.065);
    vec3 high = vec3(0.002,0.004,0.016);
    vec3 sky = mix(low,high,horizon);
    float band = exp(-pow((uv.y-0.43)*5.0,2.0));
    sky += vec3(0.035,0.025,0.070)*band;
    vec2 fineGrid = floor(uv*vec2(260.0,145.0));
    float rnd = hash21(fineGrid);
    float star = step(0.9865,rnd);
    float twinkle = 0.55+0.45*sin(time*1.8+rnd*45.0);
    sky += vec3(0.52,0.68,1.0)*star*twinkle*0.85;
    vec2 bigGrid = vec2(24.0,13.5);
    vec2 bc = floor(uv*bigGrid);
    float big = 0.0;
    for(int y=-1;y<=1;y++)
        for(int x=-1;x<=1;x++)
            big += starPoint(uv,bc+vec2(x,y),bigGrid);
    sky += vec3(0.65,0.82,1.25)*big*1.5;
    vec2 sunP = uv-vec2(0.77,0.73);
    float glow = exp(-dot(sunP,sunP)*220.0);
    sky += vec3(0.18,0.34,0.65)*glow;
    if(bhcMode==1){
        float aurora = sin(uv.x*15.0 + sin(uv.y*8.0+time)*2.0 + time*0.7);
        aurora = smoothstep(0.25,1.0,aurora)*exp(-pow((uv.y-0.62)*3.6,2.0));
        sky += vec3(0.08,0.30,0.34)*aurora;
        float net = constellationNetwork(uv);
        float pulse = 0.63+0.37*sin(time*1.15 + uv.x*12.0);
        vec3 netColor = mix(vec3(0.05,0.75,1.4),vec3(1.25,0.20,1.15),
                            0.5+0.5*sin(time*0.28+uv.y*8.0));
        sky += netColor*net*(0.18+0.16*pulse);
        float mandala = zenithMandala(uv);
        vec3 mandalaColor = mix(vec3(0.20,1.30,0.82),vec3(1.55,0.35,1.25),
                                0.5+0.5*sin(time*0.32+length(uv-vec2(.5,.86))*18.0));
        sky += mandalaColor*mandala*0.62;
        float axis = exp(-pow((uv.x-0.5)*220.0,2.0));
        axis *= smoothstep(0.34,0.58,uv.y)*(1.0-smoothstep(0.92,0.995,uv.y));
        axis *= 0.55+0.45*sin(time*3.1+uv.y*38.0);
        sky += vec3(0.45,1.10,1.45)*axis*0.75;
        vec2 zp=(uv-vec2(.5,.86))*vec2(1.78,1.0);
        sky += vec3(.22,.08,.32)*exp(-dot(zp,zp)*9.0)*0.30;
        if(launchProgress>0.0){
            float lp=clamp(launchProgress,0.0,1.0);
            float tunnelHalo=exp(-dot(zp,zp)*mix(14.0,45.0,lp));
            sky += mix(vec3(.10,.55,1.2),vec3(1.25,.18,.95),.5+.5*sin(time*.7))*tunnelHalo*(.35+1.45*lp);
            float rings=pow(.5+.5*cos(length(zp)*95.0-time*(2.0+5.0*lp)),18.0);
            sky += vec3(.15,.75,1.1)*rings*(1.0-smoothstep(.05,.68,length(zp)))*lp*.45;
        }
    }
    return sky;
}
void main(){
    vec3 p = texture(gPosition,UV).xyz;
    vec3 nRaw = texture(gNormal,UV).xyz;
    if(length(nRaw)<0.001){
        FragColor=vec4(proceduralSky(UV),1.0);
        return;
    }
    vec3 n=normalize(nRaw);
    vec4 material=texture(gAlbedo,UV);
    vec3 albedo=material.rgb;
    float specK=material.a;
    float ao=ssaoEnabled==1?texture(ssaoTex,UV).r:1.0;
    vec3 c=albedo*(0.07*ao);
    vec3 V=normalize(-p);
    for(int i=0;i<4;i++){
        vec3 Lvec=lights[i]-p;
        float d=length(Lvec);
        vec3 L=Lvec/max(d,.001);
        float diff=max(dot(n,L),0.0);
        vec3 H=normalize(L+V);
        float spec=pow(max(dot(n,H),0.0),32.0)*specK;
        float att=1.0/(1.0+.10*d+.035*d*d);
        c+=(albedo*diff+spec)*lightColors[i]*att;
    }
    if(bhcMode==1){
        float shift=.5+.5*sin(time*.9+p.x*.45+p.z*.32);
        vec3 psychedelic=vec3(
            .65+.35*sin(time*.8+p.x),
            .65+.35*sin(time*.9+p.z+2.1),
            .65+.35*sin(time*.7+p.y+4.2));
        c*=mix(vec3(1.0),psychedelic*1.20,.42*shift);
        float treeLike = smoothstep(0.04,0.20,albedo.g-albedo.b) *
                         (1.0-smoothstep(0.72,0.95,albedo.r+albedo.g+albedo.b));
        float vein = pow(abs(sin((p.x+p.z)*13.0 + sin(p.y*8.0-time)*2.2)),18.0);
        vein += pow(abs(cos(p.y*11.0 + p.x*7.0 + time*.7)),24.0)*0.65;
        vec3 veinColor=mix(vec3(.10,1.25,.48),vec3(1.25,.52,.08),
                           .5+.5*sin(time*.6+p.y*2.5));
        c += veinColor*vein*treeLike*0.50;
        float flowerLike=smoothstep(.16,.34,albedo.r+albedo.b-albedo.g*1.25);
        float petalPattern=pow(abs(sin((p.x-p.z)*18.0+p.y*13.0-time*1.2)),22.0);
        vec3 flowerGlow=mix(vec3(.12,.8,1.3),vec3(1.35,.12,.85),.5+.5*sin(time*.8+p.y*5.0));
        c+=flowerGlow*flowerLike*(.08+.55*focusStrength)*(.35+.65*petalPattern);
    }
    FragColor=vec4(c,1.0);
}

