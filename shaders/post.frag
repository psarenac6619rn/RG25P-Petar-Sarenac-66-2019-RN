#version 330 core
out vec4 FragColor;
in vec2 UV;

uniform sampler2D scene;
uniform int effect;
uniform int dmtMode;
uniform int launchMode;
uniform float launchProgress;
uniform float focusStrength;
uniform float time;
uniform vec2 resolution;

#define PI 3.14159265359

float hash21(vec2 p){
    p=fract(p*vec2(123.34,456.21));
    p+=dot(p,p+45.32);
    return fract(p.x*p.y);
}

vec3 hueShift(vec3 c,float a){
    const mat3 toYIQ=mat3(0.299,0.587,0.114,0.596,-0.274,-0.322,0.211,-0.523,0.312);
    const mat3 toRGB=mat3(1.0,0.956,0.621,1.0,-0.272,-0.647,1.0,-1.106,1.703);
    vec3 yiq=toYIQ*c;float h=atan(yiq.z,yiq.y)+a;float chroma=length(yiq.yz);
    yiq.y=chroma*cos(h);yiq.z=chroma*sin(h);return max(toRGB*yiq,vec3(0.0));
}

float sdLine(float x,float w){return 1.0-smoothstep(w,w*2.0,abs(x));}

// Kaleidoscopic rosette used for flower focus and the F2 tunnel walls.
float fractalRosette(vec2 p,float spin,float petals){
    float r=length(p);float a=atan(p.y,p.x)+spin;
    float rings=0.0;
    for(int i=1;i<=5;i++){
        float rr=float(i)*.105;
        rings+=1.0-smoothstep(.005,.016,abs(r-rr));
    }
    float spokes=pow(abs(cos(a*petals)),30.0)*(1.0-smoothstep(.62,.78,r));
    vec2 q=p;float rec=0.0;
    for(int i=0;i<4;i++){
        q=abs(q)/max(dot(q,q),.18)-vec2(.72,.55);
        rec+=exp(-17.0*abs(length(q)-.52));
    }
    return clamp(rings*.35+spokes*.9+rec*.10,0.0,1.0)*(1.0-smoothstep(.72,.92,r));
}

vec3 tunnelLayer(vec2 uv,float progress){
    vec2 p=uv-.5;p.x*=resolution.x/max(resolution.y,1.0);
    float r=length(p);float a=atan(p.y,p.x);

    // Perspective tunnel rings accelerate as launch progresses.
    float speed=mix(1.2,6.5,progress);
    float z=1.0/max(r,.025);
    float rings=pow(.5+.5*cos(z*3.3-time*speed),10.0);
    rings*=smoothstep(.08,.22,r)*(1.0-smoothstep(.90,1.35,r));

    // Radial streaks make the viewer feel pulled upward/forward.
    float rays=pow(abs(cos(a*18.0+sin(time*.8+r*12.0))),30.0);
    rays*=smoothstep(.06,.20,r)*(1.0-smoothstep(.75,1.2,r));

    // Several independent fractal realms orbit around the tunnel.
    float fs=0.0;
    for(int i=0;i<6;i++){
        float fi=float(i);
        float aa=time*(.18+.04*fi)+fi*1.047;
        vec2 c=vec2(cos(aa),sin(aa))*mix(.28,.62,.5+.5*sin(time*.22+fi));
        float sc=.55+.18*sin(time*.35+fi*2.1);
        fs+=fractalRosette((p-c)/sc,-time*(.15+.025*fi),5.0+mod(fi,4.0));
    }
    fs=clamp(fs,0.0,1.0);

    vec3 cyan=vec3(.04,.95,1.55),mag=vec3(1.40,.08,1.15),gold=vec3(1.25,.62,.10);
    vec3 col=mix(cyan,mag,.5+.5*sin(a*3.0+time*.7));
    col+=gold*fs*.55;
    col+=mix(cyan,mag,.5+.5*sin(time*.6))*rings*1.35;
    col+=col*rays*.55;

    // Bright destination/source at the center.
    float source=exp(-r*r*mix(35.0,130.0,progress));
    col+=vec3(.8,1.1,1.4)*source*(.35+1.7*progress);
    return col;
}

void main(){
    vec2 uv=UV;

    if(dmtMode==1){
        vec2 centered=uv-.5;float r=length(centered);
        vec2 drift=vec2(sin(time*.55),cos(time*.47))*.0022;
        vec2 ripple=vec2(sin(uv.y*18.0+time*1.20)*.0055,cos(uv.x*16.0-time*1.05)*.0045);
        float sacredPulse=sin(time*.9+r*28.0);
        centered*=1.0+.012*sin(time*.75+r*12.0)+.003*sacredPulse;
        float angle=atan(centered.y,centered.x);
        centered+=vec2(cos(angle*6.0),sin(angle*6.0))*.0018*sin(time+r*40.0);

        // Looking directly at a flower creates a real optical zoom, not just brighter colors.
        centered*=mix(1.0,.58,focusStrength);
        uv=clamp(.5+centered+drift+ripple,vec2(.002),vec2(.998));
    }

    vec3 c;
    if(dmtMode==1){
        vec2 dir=normalize((uv-.5)+vec2(.0001));float aberr=.0007+.0012*length(uv-.5)+focusStrength*.0012;
        c.r=texture(scene,clamp(uv+dir*aberr,0.0,1.0)).r;
        c.g=texture(scene,uv).g;
        c.b=texture(scene,clamp(uv-dir*aberr,0.0,1.0)).b;
        c=hueShift(c,.20*sin(time*.55)+focusStrength*.16*sin(time*1.7));
        c*=1.05+.08*sin(time*1.35+UV.y*8.0);
    }else c=texture(scene,uv).rgb;

    if(effect==1){c=vec3(1.0)-exp(-c*1.25);c=pow(max(c,vec3(0.0)),vec3(1.0/2.2));}
    else if(effect==2){float g=dot(c,vec3(.2126,.7152,.0722));c=vec3(g);}
    else if(effect==3)c=vec3(1.0)-c;

    if(dmtMode==1){
        float vignette=smoothstep(.78,.24,length(UV-.5));c*=.82+.18*vignette;

        // Flower focus: fractals emerge from the viewed flower and intensify continuously.
        if(focusStrength>.001){
            vec2 fp=UV-.5;fp.x*=resolution.x/max(resolution.y,1.0);
            float f1=fractalRosette(fp*1.45,time*.18,7.0);
            float f2=fractalRosette(fp*2.15,-time*.24,11.0);
            vec3 fc=mix(vec3(.08,.9,1.3),vec3(1.3,.12,1.0),.5+.5*sin(time*.55+length(fp)*12.0));
            c+=fc*(f1*.24+f2*.15)*focusStrength;
            c+=fc*exp(-dot(fp,fp)*18.0)*focusStrength*.18;
        }
    }

    if(launchMode==1){
        float p=clamp(launchProgress,0.0,1.0);
        vec2 q=UV-.5;float rr=length(q);

        // F2 starts with dense violet fog around the player; it clears as acceleration builds.
        float fogPhase=(1.0-smoothstep(.05,.42,p));
        float noise=hash21(floor(UV*vec2(180.0,100.0)+time*vec2(7.0,-4.0)));
        float clouds=.5+.5*sin(UV.x*9.0+sin(UV.y*11.0-time*.8)*2.0+time*.45);
        vec3 fog=vec3(.17,.07,.29)*(1.2+.55*clouds)+vec3(.06,.18,.25)*noise*.35;
        c=mix(c,fog,clamp(fogPhase*(.58+.24*clouds),0.0,.86));

        // Tunnel takeover progressively replaces the ordinary scene.
        vec3 tunnel=tunnelLayer(UV,p);
        float takeover=smoothstep(.08,.34,p)*(.45+.35*p);
        c+=tunnel*takeover;

        // Speed-line radial blur approximation from several samples.
        if(p>.12){
            vec2 center=vec2(.5);vec2 v=UV-center;
            vec3 blur=vec3(0.0);
            blur+=texture(scene,clamp(center+v*.96,0.0,1.0)).rgb;
            blur+=texture(scene,clamp(center+v*.91,0.0,1.0)).rgb;
            blur+=texture(scene,clamp(center+v*.84,0.0,1.0)).rgb;
            blur/=3.0;
            c=mix(c,blur+tunnel*(.45+.45*p),smoothstep(.18,.75,p)*.24);
        }

        float flash=exp(-rr*rr*90.0)*smoothstep(.72,1.0,p);
        c+=vec3(.75,1.0,1.3)*flash*2.2;
    }

    FragColor=vec4(c,1.0);
}
