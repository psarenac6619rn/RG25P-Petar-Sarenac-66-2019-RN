#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "GBuffer.h"
#include "SSAO.h"
#include "Particles.h"
#include "PostProcess.h"
#include "GUI.h"
static const int W=1280,H=720; static Camera cam; static bool firstMouse=true;static double lastX=W/2,lastY=H/2;static bool mouseLocked=true;
static void mouse(GLFWwindow*,double x,double y){if(!mouseLocked)return;if(firstMouse){lastX=x;lastY=y;firstMouse=false;}double dx=x-lastX,dy=lastY-y;lastX=x;lastY=y;cam.look((float)dx,(float)dy);}
static GUI* pGui=nullptr;
static void resizeCB(GLFWwindow* win, int w, int h) { glViewport(0,0,w,h); if(pGui) pGui->resize(w,h); }
static GLuint proceduralNormal(){const int N=128;std::vector<unsigned char>p(N*N*3);for(int y=0;y<N;y++)for(int x=0;x<N;x++){float fx=x*.20f,fy=y*.20f;float nx=.5f+.18f*std::sin(fx)*std::cos(fy),ny=.5f+.18f*std::cos(fx*.7f+fy*.3f),nz=1;int k=(y*N+x)*3;p[k]=(unsigned char)(nx*255);p[k+1]=(unsigned char)(ny*255);p[k+2]=(unsigned char)(nz*255);}GLuint t;glGenTextures(1,&t);glBindTexture(GL_TEXTURE_2D,t);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,N,N,0,GL_RGB,GL_UNSIGNED_BYTE,p.data());glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);return t;}
static void object(const Mesh&m,const Shader&s,const Mat4&model,Vec3 color,float normalMap){s.setMat4("model",model);s.setVec3("baseColor",color);s.setFloat("useNormalMap",normalMap);m.draw();}
static void drawEnergyTree(const Mesh& cube,const Shader& g,float t,bool bhc){
    const Vec3 trunkColor{.18f,.115f,.055f};
    const Vec3 branchColor{.16f,.095f,.045f};
    const Vec3 leafColor{.075f,.30f,.105f};
    float breathe=bhc?(0.025f*std::sin(t*1.35f)):0.0f;
    Mat4 root=translate({0.0f,0.10f,-2.80f});
    for(int i=0;i<5;i++){
        float y=.45f+i*.62f;
        float sway=bhc?std::sin(t*.85f+i*.7f)*.035f:0.0f;
        Mat4 m=multiply(root,multiply(translate({sway,y,0}),multiply(rotateZ(sway*.8f),scale({.34f-i*.035f,.38f,.32f-i*.03f}))));
        object(cube,g,m,trunkColor,1);
    }
    for(int level=0;level<3;level++){
        float y=2.15f+level*.52f;
        float len=1.10f-level*.10f;
        for(int b=0;b<8;b++){
            float ang=(float)b*PI_F/4.0f + level*.34f;
            float wob=bhc?std::sin(t*.95f+b*1.7f+level)*.055f:0.0f;
            Mat4 hub=multiply(root,translate({0,y,0}));
            Mat4 yaw=multiply(hub,rotateY(ang+wob));
            Mat4 tilt=multiply(yaw,rotateZ(.78f+breathe+wob*.25f));
            Mat4 branch=multiply(tilt,multiply(translate({0,len*.56f,0}),scale({.10f,len*.62f,.095f})));
            object(cube,g,branch,branchColor,1);
            Mat4 tip=multiply(tilt,translate({0,len*1.06f,0}));
            float side=(b%2==0?1.0f:-1.0f);
            Mat4 twig=multiply(tip,multiply(rotateZ(side*(.48f+.08f*std::sin(t*.8f+b))),multiply(translate({0,.42f,0}),scale({.055f,.48f,.055f}))));
            object(cube,g,twig,branchColor,1);
            Vec3 lc=leafColor;
            if(bhc){
                lc={.08f+.05f*(.5f+.5f*std::sin(t+b)), .34f+.15f*(.5f+.5f*std::sin(t*1.2f+b)), .13f+.10f*(.5f+.5f*std::cos(t*.8f+b))};
            }
            Mat4 crown=multiply(tip,multiply(translate({0,.82f,0}),scale({.46f,.34f,.46f})));
            object(cube,g,crown,lc,1);
        }
    }
    if(bhc){
        float pulse=.18f+.045f*(.5f+.5f*std::sin(t*3.2f));
        Mat4 core=multiply(root,multiply(translate({0,1.85f,.02f}),scale({pulse,.42f,pulse})));
        object(cube,g,core,{.10f,.82f,.48f},0);
    }
}
static float terrainHeight(float,float){ return 0.0f; }
static void drawFlower(const Mesh& cube,const Shader& g,Vec3 p,float t,bool bhc,float focus){
    float pulse=bhc?(1.0f+0.10f*std::sin(t*2.5f+p.x)):1.0f;
    Mat4 stem=multiply(translate({p.x,p.y+.25f,p.z}),scale({.035f,.25f,.035f}));
    object(cube,g,stem,{.045f,.22f,.07f},0);
    for(int i=0;i<8;i++){
        float a=i*(2.0f*PI_F/8.0f)+0.12f*std::sin(t*.9f+p.z);
        Mat4 petal=multiply(translate({p.x,p.y+.56f,p.z}),multiply(rotateY(a),multiply(rotateZ(.72f),multiply(translate({0,.16f,0}),scale({.085f,.22f,.035f})))));
        Vec3 c=bhc?Vec3{.55f+.30f*std::sin(t*.7f+i),.08f+.10f*focus,.68f+.28f*std::cos(t*.9f+i)}:Vec3{.42f,.10f,.48f};
        object(cube,g,petal,c*pulse,0);
    }
    float core=.10f+.035f*focus+.018f*std::sin(t*3.0f+p.x);
    Mat4 center=multiply(translate({p.x,p.y+.58f,p.z}),scale({core,core,core}));
    object(cube,g,center,bhc?Vec3{1.0f,.40f+.50f*focus,.12f}:Vec3{.72f,.34f,.08f},0);
}
enum class InteractType { Chair, Table, Tree, Pond, Newspaper, Flower, Duck };
struct Interactable {
    InteractType type;
    Vec3 pos;
    float radius;
    int index;
    const char* name;
};
static bool flowerPlacementAllowed(float x,float z){
    float pdx=(x+4.25f)/2.75f;
    float pdz=(z+5.25f)/2.15f;
    if(pdx*pdx+pdz*pdz < 1.0f) return false;
    if(x>0.15f && x<2.75f && z>0.10f && z<3.20f) return false;
    if(x>-2.60f && x<-0.65f && z>-7.10f && z<-4.30f) return false;
    return true;
}
static int nearestInteractable(const std::vector<Interactable>& items,const Vec3& player){
    int best=-1; float bestD=9999.0f;
    for(int i=0;i<(int)items.size();i++){
        float d=length(items[i].pos-player);
        if(d<=items[i].radius && d<bestD){ best=i; bestD=d; }
    }
    return best;
}
static std::vector<Vec3> makeFlowers(){
    std::vector<Vec3> f;
    const float candidates[][2]={
        {-5.8f,2.9f},{-4.2f,3.9f},{-2.8f,4.8f},{3.7f,4.4f},{5.5f,3.0f},
        {-5.7f,.7f},{-3.7f,.1f},{3.8f,-.2f},{5.6f,.8f},
        {-3.0f,-2.0f},{2.9f,-2.0f},{-1.9f,-4.3f},{1.8f,-4.6f},
        {4.8f,-3.7f},{5.9f,-1.6f},{-6.0f,-1.4f},
        {-6.2f,4.8f},{5.9f,5.0f},{-1.0f,5.6f},{1.1f,5.7f},
        {3.6f,-5.5f},{6.0f,-5.0f}
    };
    for(const auto &q:candidates){
        if(flowerPlacementAllowed(q[0],q[1])) f.push_back({q[0],.02f,q[1]});
    }
    return f;
}
static void drawTableChairNewspaper(const Mesh&c,const Shader&g,float t){
 Vec3 w{.25f,.14f,.07f},m{.12f,.16f,.18f}; object(c,g,multiply(translate({1.45f,.78f,1.15f}),scale({1.05f,.10f,.70f})),w,1);
 for(float x:{.72f,2.18f})for(float z:{.70f,1.60f})object(c,g,multiply(translate({x,.39f,z}),scale({.07f,.39f,.07f})),m,0);
 object(c,g,multiply(translate({1.45f,.45f,2.38f}),scale({.42f,.08f,.42f})),w,1);
 object(c,g,multiply(translate({1.45f,.92f,2.76f}),scale({.42f,.48f,.08f})),w,1);
 // Newspaper on table: two folded halves
 Vec3 npaper{.88f,.82f,.04f}; // beige/newsprint
 Vec3 nline{.20f,.15f,.10f}; // dark print lines
 float fold=0.18f*std::sin(t*0.22f)+0.05f; // gentle page flutter
 // Left half
 Mat4 nleft=multiply(translate({1.25f,.835f,1.10f}),multiply(rotateY(.12f),multiply(rotateZ(-fold),scale({.20f,.008f,.28f}))));
 object(c,g,nleft,npaper,0);
 // Right half (slightly raised — open)
 Mat4 nright=multiply(translate({1.65f,.840f,1.10f}),multiply(rotateY(.12f),multiply(rotateZ(fold),scale({.20f,.008f,.28f}))));
 object(c,g,nright,npaper,0);
 // Spine crease
 Mat4 nspine=multiply(translate({1.45f,.838f,1.10f}),multiply(rotateY(.12f),scale({.025f,.012f,.28f})));
 object(c,g,nspine,nline,0);
 // A couple of "print" decorative strips on left page
 for(int row=0;row<3;row++){
 float rz=1.10f-0.07f+row*0.06f;
 Mat4 strip=multiply(translate({1.22f+0.03f*std::sin(t*.3f+row),.843f,rz}),multiply(rotateY(.12f),scale({.14f,.003f,.015f})));
 object(c,g,strip,nline,0);
 }
}

static void drawPond(const Mesh&c,const Shader&g,float t,bool d){
 Vec3 pc{-4.25f,.05f,-5.25f}; for(int z=-2;z<=2;z++)for(int x=-3;x<=3;x++){float nx=x/3.f,nz=z/2.f;if(nx*nx+nz*nz>1.08f)continue;
 Vec3 wc=d?Vec3{.045f,.30f,.42f}:Vec3{.035f,.18f,.25f}; object(c,g,multiply(translate({pc.x+x*.48f,pc.y+.018f*std::sin(t*1.8f+x+z),pc.z+z*.48f}),scale({.27f,.025f,.27f})),wc,0);}
 for(int i=0;i<24;i++){float q=i*2*PI_F/24;object(c,g,multiply(translate({pc.x+2.05f*std::cos(q),.16f,pc.z+1.48f*std::sin(q)}),multiply(rotateY(-q),scale({.24f,.16f,.16f}))),{.24f,.20f,.17f},1);}
}
static Vec3 duckPosition(float t){
 return {-4.25f+1.10f*std::cos(t*.42f),.25f,-5.25f+.68f*std::sin(t*.42f)};
}
static void drawDuck(const Mesh&c,const Shader&g,float t){
 Vec3 p=duckPosition(t); Mat4 r=translate(p);
 object(c,g,multiply(r,scale({.28f,.16f,.42f})),{.76f,.62f,.18f},0); object(c,g,multiply(r,multiply(translate({0,.23f,-.28f}),scale({.17f,.18f,.17f}))),{.84f,.70f,.22f},0);
 object(c,g,multiply(r,multiply(translate({0,.22f,-.48f}),scale({.08f,.045f,.16f}))),{.82f,.34f,.06f},0);
}
static void drawGhostBody(const Mesh& cube,const Shader& g,Vec3 pos,float yaw,float t,bool bhc){
    Mat4 root=multiply(translate({pos.x,pos.y-0.78f,pos.z}),rotateY(-yaw*PI_F/180.0f));
    Vec3 dark{.055f,.075f,.095f}, mid{.075f,.105f,.135f}, skin{.42f,.31f,.24f};
    object(cube,g,multiply(root,multiply(translate({0,1.18f,0}),scale({.30f,.43f,.18f}))),dark,1);
    object(cube,g,multiply(root,multiply(translate({0,.77f,0}),scale({.25f,.16f,.17f}))),mid,1);
    object(cube,g,multiply(root,multiply(translate({0,1.78f,0}),scale({.19f,.22f,.18f}))),skin,1);
    for(int side=-1;side<=1;side+=2){
        float q=(float)side;
        object(cube,g,multiply(root,multiply(translate({q*.40f,1.20f,0}),scale({.105f,.39f,.105f}))),dark,1);
        object(cube,g,multiply(root,multiply(translate({q*.43f,.82f,0}),scale({.095f,.27f,.095f}))),mid,1);
        object(cube,g,multiply(root,multiply(translate({q*.15f,.43f,0}),scale({.13f,.36f,.14f}))),dark,1);
        object(cube,g,multiply(root,multiply(translate({q*.15f,.05f,0}),scale({.12f,.32f,.13f}))),mid,1);
    }
    object(cube,g,multiply(root,multiply(translate({0,1.25f,.22f}),scale({.22f,.30f,.08f}))),mid,1);
    float pulse=bhc?(.65f+.35f*std::sin(t*2.4f)):.55f;
    object(cube,g,multiply(root,multiply(translate({0,1.25f,.315f}),scale({.035f,.19f,.018f}))),
           {.04f*pulse,.55f*pulse,.95f*pulse},0);
}
 int main(){
 if(!glfwInit()){std::cerr<<"GLFW init failed\n";return 1;}glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
 GLFWwindow*win=glfwCreateWindow(W,H,"RAF RG - Sci-Fi Outpost",nullptr,nullptr);if(!win){glfwTerminate();return 1;}glfwMakeContextCurrent(win);glewExperimental=GL_TRUE;if(glewInit()!=GLEW_OK){std::cerr<<"GLEW init failed\n";return 1;}glGetError();glfwSwapInterval(1);glfwSetCursorPosCallback(win,mouse);glfwSetFramebufferSizeCallback(win,resizeCB);glfwSetInputMode(win,GLFW_CURSOR,GLFW_CURSOR_DISABLED);glEnable(GL_DEPTH_TEST);
 try{
  Shader g("shaders/gbuffer.vert","shaders/gbuffer.frag"),ss("shaders/fullscreen.vert","shaders/ssao.frag"),light("shaders/fullscreen.vert","shaders/lighting.frag"),part("shaders/particle.vert","shaders/particle.frag"),post("shaders/fullscreen.vert","shaders/post.frag");
  Mesh terrain=Mesh::plane(96,28),cube=Mesh::cube(),quad=Mesh::quad();GBuffer gb(W,H);SSAO ao(W,H);PostProcess pp(W,H);Particles particles(220),treeEnergy(420),launchMist(520);GLuint nmap=proceduralNormal();
  GUI gui(W,H); pGui=&gui;
  std::vector<Vec3> flowers=makeFlowers();
  std::vector<Interactable> interactables;
  interactables.push_back({InteractType::Chair,{1.45f,.55f,2.38f},2.0f,-1,"Chair"});
  interactables.push_back({InteractType::Table,{1.45f,.75f,1.15f},1.9f,-1,"Table"});
  interactables.push_back({InteractType::Newspaper,{1.45f,.88f,1.10f},1.6f,-1,"Newspaper"});
  interactables.push_back({InteractType::Tree,{0.0f,1.0f,-2.80f},2.2f,-1,"Energy Tree"});
  interactables.push_back({InteractType::Pond,{-4.25f,.10f,-5.25f},2.8f,-1,"Pond"});
  for(int i=0;i<(int)flowers.size();i++)
      interactables.push_back({InteractType::Flower,flowers[i]+Vec3{0,.55f,0},1.35f,i,"Flower"});
  g.use();g.setInt("normalMap",0);ss.use();ss.setInt("gPosition",0);ss.setInt("gNormal",1);ss.setInt("noiseTex",2);light.use();light.setInt("gPosition",0);light.setInt("gNormal",1);light.setInt("gAlbedo",2);light.setInt("ssaoTex",3);post.use();post.setInt("scene",0);
  bool aoOn=true;int effect=1;bool bhcMode=false;bool launchMode=false;bool ghostMode=false;
  bool key1=false,key2=false,keyF=false,keyF1=false,keyF2=false,keyF3=false,keyF4=false,keyE=false;
  float launchProgress=0.0f,focusStrength=0.0f;
  Vec3 launchStartPos{}; float launchStartYaw=0,launchStartPitch=0;
  Vec3 ghostReturnPos{}; float ghostReturnYaw=0,ghostReturnPitch=0;
  bool seated=false; Vec3 seatReturnPos{}; float seatReturnYaw=0,seatReturnPitch=0;
  bool newspaperMenu=false; int newspaperSelected=0; bool keyR=false;
  const Vec3 chairSeatPos{1.45f,1.18f,2.20f};
  int currentInteractable=-1;
  float interactionPulse=0.0f;
  const float playerEyeHeight=1.72f;
  const float gravity=-18.0f;
  float verticalVelocity=0.0f;
  float last=(float)glfwGetTime();
  std::cout<<"Controls: WASD move, mouse look, 1 SSAO, 2 post effect, F cursor, F1 bhc, F2 tunnel, F3 ghost flight, F4 materialize, ESC exit\n";
  while(!glfwWindowShouldClose(win)){
   float now=(float)glfwGetTime(),dt=std::min(now-last,.05f);last=now;if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)glfwSetWindowShouldClose(win,1);
   float fw=(glfwGetKey(win,GLFW_KEY_W)==GLFW_PRESS)-(glfwGetKey(win,GLFW_KEY_S)==GLFW_PRESS);
   float rt=(glfwGetKey(win,GLFW_KEY_D)==GLFW_PRESS)-(glfwGetKey(win,GLFW_KEY_A)==GLFW_PRESS);
   if(!launchMode && !seated){
       if(ghostMode){
           float up=(glfwGetKey(win,GLFW_KEY_SPACE)==GLFW_PRESS)-(glfwGetKey(win,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS);
           float boost=(glfwGetKey(win,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS)?2.4f:1.0f;
           Vec3 ghostForward=cam.front();
           Vec3 ghostRight=normalize(cross(ghostForward,{0,1,0}));
           if(length(ghostRight)<0.001f) ghostRight={1,0,0};
           Vec3 ghostUp={0,1,0};
           float ghostSpeed=cam.speed*1.20f*boost;
           cam.pos=cam.pos+ghostForward*(fw*ghostSpeed*dt)+ghostRight*(rt*ghostSpeed*dt)+ghostUp*(up*ghostSpeed*dt);
       }else{
           cam.move(fw,rt,dt);
       }
   }
   if(!ghostMode && !launchMode && !seated){
       float groundY=terrainHeight(cam.pos.x,cam.pos.z)+playerEyeHeight;
       if(cam.pos.y>groundY+0.002f){
           verticalVelocity += gravity*dt;
           cam.pos.y += verticalVelocity*dt;
           if(cam.pos.y<=groundY){
               cam.pos.y=groundY;
               verticalVelocity=0.0f;
           }
       }else{
           cam.pos.y=groundY;
           verticalVelocity=0.0f;
       }
   }else if(ghostMode){
       verticalVelocity=0.0f;
   }
   currentInteractable=nearestInteractable(interactables,cam.pos);
   Vec3 duckNow=duckPosition(now);
   bool duckNear=length(duckNow-cam.pos)<1.55f;
   bool ke=glfwGetKey(win,GLFW_KEY_E)==GLFW_PRESS;
   if(ke&&!keyE&&!ghostMode&&!launchMode){
       if(seated){
           seated=false;
           cam.pos=seatReturnPos; cam.yaw=seatReturnYaw; cam.pitch=seatReturnPitch;
           cam.pos.y=playerEyeHeight;
           std::cout<<"Standing.\n";
       }else if(duckNear){
           interactionPulse=1.0f;
           std::cout<<"Interaction: Duck\n";
       }else if(currentInteractable>=0){
           const Interactable &it=interactables[currentInteractable];
           interactionPulse=1.0f;
           switch(it.type){
               case InteractType::Chair:
                   seatReturnPos=cam.pos; seatReturnYaw=cam.yaw; seatReturnPitch=cam.pitch;
                   seated=true; cam.pos=chairSeatPos; cam.yaw=-118.0f; cam.pitch=-4.0f;
                   std::cout<<"Interaction: Chair - seated. E to stand.\n";
                   break;
               case InteractType::Table:
                   std::cout<<"Interaction: Table\n";
                   break;
               case InteractType::Tree:
                   focusStrength=std::max(focusStrength,.72f);
                   std::cout<<"Interaction: Energy Tree\n";
                   break;
               case InteractType::Pond:
                   std::cout<<"Interaction: Pond\n";
                   break;
               case InteractType::Newspaper:
                   newspaperMenu=!newspaperMenu;
                   if(newspaperMenu){ glfwSetInputMode(win,GLFW_CURSOR,GLFW_CURSOR_NORMAL); mouseLocked=false; firstMouse=true; std::cout<<"Newspaper menu opened.\n"; }
                   else             { glfwSetInputMode(win,GLFW_CURSOR,GLFW_CURSOR_DISABLED); mouseLocked=true; firstMouse=true; std::cout<<"Newspaper menu closed.\n"; }
                   break;
               case InteractType::Flower:
                   focusStrength=1.0f;
                   std::cout<<"Interaction: Flower\n";
                   break;
               default: break;
           }
       }
   }
   keyE=ke;
   interactionPulse=std::max(0.0f,interactionPulse-dt*1.6f);
   // ---- Newspaper menu keyboard handling ----
   if(newspaperMenu){
       bool kr=glfwGetKey(win,GLFW_KEY_R)==GLFW_PRESS;
       if(kr&&!keyR){
           // Reset: restore camera to start position
           newspaperMenu=false;
           seated=false;
           cam.pos={0.0f,1.72f,4.5f}; cam.yaw=-90.0f; cam.pitch=0.0f;
           bhcMode=false; launchMode=false; ghostMode=false;
           launchProgress=0.0f; focusStrength=0.0f;
           glfwSetWindowTitle(win,"RAF RG - Sci-Fi Outpost");
           std::cout<<"Game Reset.\n";
       }
       keyR=kr;
       // ESC to exit from newspaper menu handled below
       if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(win,1);
   }
   bool k1=glfwGetKey(win,GLFW_KEY_1)==GLFW_PRESS;if(k1&&!key1)aoOn=!aoOn;key1=k1;bool k2=glfwGetKey(win,GLFW_KEY_2)==GLFW_PRESS;if(k2&&!key2)effect=(effect+1)%4;key2=k2;bool kf=glfwGetKey(win,GLFW_KEY_F)==GLFW_PRESS;if(kf&&!keyF){mouseLocked=!mouseLocked;firstMouse=true;glfwSetInputMode(win,GLFW_CURSOR,mouseLocked?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}keyF=kf;
   bool kf1=glfwGetKey(win,GLFW_KEY_F1)==GLFW_PRESS;if(kf1&&!keyF1){bhcMode=!bhcMode;std::cout<<"Brain HyperConnectivity mode: "<<(bhcMode?"ON":"OFF")<<"\n";glfwSetWindowTitle(win,bhcMode?"RAF RG - Sci-Fi Outpost [Brain HyperConnectivity]":"RAF RG - Sci-Fi Outpost");}keyF1=kf1;
   bool kf2=glfwGetKey(win,GLFW_KEY_F2)==GLFW_PRESS;if(kf2&&!keyF2){
       launchMode=!launchMode;
       if(launchMode){bhcMode=true;launchProgress=0.0f;launchStartPos=cam.pos;launchStartYaw=cam.yaw;launchStartPitch=cam.pitch;std::cout<<"Launch tunnel: ON\n";}
       else {cam.pos=launchStartPos;cam.yaw=launchStartYaw;cam.pitch=launchStartPitch;launchProgress=0.0f;std::cout<<"Launch tunnel: OFF\n";}
   }keyF2=kf2;
   bool kf3=glfwGetKey(win,GLFW_KEY_F3)==GLFW_PRESS;
   if(kf3&&!keyF3){
       if(!ghostMode){
           if(launchMode){
               cam.pos=launchStartPos; cam.yaw=launchStartYaw; cam.pitch=launchStartPitch;
               launchMode=false; launchProgress=0.0f;
           }
           cam.pos.y=terrainHeight(cam.pos.x,cam.pos.z)+playerEyeHeight;
           verticalVelocity=0.0f;
           ghostReturnPos=cam.pos;
           ghostReturnYaw=cam.yaw;
           ghostReturnPitch=cam.pitch;
           ghostMode=true;
           std::cout<<"Ghost mode: ON\n";
           glfwSetWindowTitle(win,"RAF RG - Sci-Fi Outpost [GHOST MODE]");
       }else{
           cam.pos=ghostReturnPos;
           cam.yaw=ghostReturnYaw;
           cam.pitch=ghostReturnPitch;
           verticalVelocity=0.0f;
           ghostMode=false;
           std::cout<<"Ghost mode: OFF\n";
           glfwSetWindowTitle(win,bhcMode?"RAF RG - Sci-Fi Outpost [Brain HyperConnectivity]":"RAF RG - Sci-Fi Outpost");
       }
   }
   keyF3=kf3;
   bool kf4=glfwGetKey(win,GLFW_KEY_F4)==GLFW_PRESS;
   if(kf4&&!keyF4&&ghostMode){
       ghostMode=false;
       verticalVelocity=0.0f;
       ghostReturnPos=cam.pos;
       ghostReturnYaw=cam.yaw;
       ghostReturnPitch=cam.pitch;
       std::cout<<"Ghost position: Saved\n";
       glfwSetWindowTitle(win,bhcMode?"RAF RG - Sci-Fi Outpost [Brain HyperConnectivity]":"RAF RG - Sci-Fi Outpost");
   }
   keyF4=kf4;
   if(launchMode){
       launchProgress=std::min(1.0f,launchProgress+dt/10.0f);
       float ease=launchProgress*launchProgress*(3.0f-2.0f*launchProgress);
       cam.pitch=launchStartPitch+(82.0f-launchStartPitch)*ease;
       cam.yaw=launchStartYaw+std::sin(now*.8f)*2.2f*(1.0f-ease);
       cam.pos=launchStartPos+Vec3{std::sin(now*.65f)*.12f*ease,7.5f*ease,-1.4f*ease};
   }
   float bestFlowerDot=-1.0f;
   if(bhcMode && !launchMode){
       Vec3 cf=cam.front();
       for(const Vec3& fp:flowers){
           Vec3 target=fp+Vec3{0,.58f,0}; Vec3 to=target-cam.pos; float dist=length(to);
           if(dist<14.0f)bestFlowerDot=std::max(bestFlowerDot,dot(cf,normalize(to)));
       }
   }
   float focusTarget=(bestFlowerDot>.982f)?std::clamp((bestFlowerDot-.982f)/.016f,0.0f,1.0f):0.0f;
   focusStrength += (focusTarget-focusStrength)*std::min(1.0f,dt*4.2f);
   float fov=60.0f-30.0f*focusStrength;
   Mat4 V=cam.view(),P=perspective(radians(fov),W/(float)H,.1f,100);
   gb.bind();glViewport(0,0,W,H);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);g.use();g.setMat4("view",V);g.setMat4("projection",P);g.setFloat("time",now);g.setInt("bhcMode",bhcMode?1:0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,nmap);
   object(terrain,g,identity(),{.075f,.13f,.12f},1); drawTableChairNewspaper(cube,g,now); drawPond(cube,g,now,bhcMode); drawDuck(cube,g,now);
   drawEnergyTree(cube,g,now,bhcMode);
   if(ghostMode){
       Vec3 groundedBody=ghostReturnPos;
       groundedBody.y=terrainHeight(groundedBody.x,groundedBody.z)+playerEyeHeight;
       drawGhostBody(cube,g,groundedBody,ghostReturnYaw,now,bhcMode);
   }
   for(const Vec3& fp:flowers) drawFlower(cube,g,fp,now,bhcMode,focusStrength);
   Mat4 droneRoot=multiply(translate({std::sin(now*.55f)*3.2f,2.7f+std::sin(now*1.7f)*.18f,-1.5f+std::cos(now*.55f)*2.0f}),rotateY(now*.55f+.6f));
   object(cube,g,multiply(droneRoot,scale({.65f,.18f,.45f})),{.32f,.36f,.40f},1);
   Vec3 arms[4]={{.72f,0,.52f},{-.72f,0,.52f},{.72f,0,-.52f},{-.72f,0,-.52f}};for(auto a:arms){Mat4 arm=multiply(droneRoot,translate(a));object(cube,g,multiply(arm,scale({.28f,.06f,.05f})),{.18f,.21f,.24f},0);Mat4 rotor=multiply(arm,rotateY(now*14));object(cube,g,multiply(rotor,scale({.42f,.025f,.045f})),{.05f,.7f,.85f},0);}
   Vec3 emitterWorld=transformPoint(droneRoot,{0,-.25f,0});
   glBindFramebuffer(GL_FRAMEBUFFER,ao.fbo);glClear(GL_COLOR_BUFFER_BIT);ss.use();ss.setMat4("projection",P);ss.setVec3("samples[0]",ao.kernel[0]);for(int i=0;i<32;i++){std::string n="samples["+std::to_string(i)+"]";ss.setVec3(n.c_str(),ao.kernel[i]);}GLint ns=glGetUniformLocation(ss.id,"noiseScale");glUniform2f(ns,W/4.0f,H/4.0f);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,gb.position);glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,gb.normal);glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,ao.noise);quad.draw();
   pp.bind();glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);light.use();light.setFloat("time",now);light.setInt("bhcMode",bhcMode?1:0);light.setFloat("focusStrength",focusStrength);light.setFloat("launchProgress",launchMode?launchProgress:0.0f);Vec3 worldLights[4]={{0,5,2},{-6,2,-4},{6,2,-4},emitterWorld};Vec3 cols[4]={{1.3f,.55f,.25f},{.1f,.55f,1.2f},{.7f,.2f,1.2f},{.1f,1.0f,1.4f}};for(int i=0;i<4;i++){std::string ln="lights["+std::to_string(i)+"]",cn="lightColors["+std::to_string(i)+"]";light.setVec3(ln.c_str(),transformPoint(V,worldLights[i]));light.setVec3(cn.c_str(),cols[i]);}light.setInt("ssaoEnabled",aoOn?1:0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,gb.position);glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,gb.normal);glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,gb.albedo);glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,ao.texture);quad.draw();
   glBindFramebuffer(GL_READ_FRAMEBUFFER,gb.fbo);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,pp.fbo);glBlitFramebuffer(0,0,W,H,0,0,W,H,GL_DEPTH_BUFFER_BIT,GL_NEAREST);
   glBindFramebuffer(GL_FRAMEBUFFER,pp.fbo);glEnable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);particles.update(dt,emitterWorld);part.use();part.setMat4("view",V);part.setMat4("projection",P);particles.draw();
   if(bhcMode){
       treeEnergy.update(dt,{0.0f,1.75f,-2.80f});
       treeEnergy.draw();
   }
   if(launchMode){
       Vec3 fogEmitter=cam.pos+cam.front()*1.2f+Vec3{0,-.65f,0};
       launchMist.update(dt,fogEmitter);launchMist.draw();
   }
   glDisable(GL_BLEND);
   glBindFramebuffer(GL_FRAMEBUFFER,0);glDisable(GL_DEPTH_TEST);glClear(GL_COLOR_BUFFER_BIT);post.use();post.setInt("effect",effect);post.setInt("bhcMode",bhcMode?1:0);post.setInt("launchMode",launchMode?1:0);post.setFloat("launchProgress",launchProgress);post.setFloat("focusStrength",focusStrength);post.setFloat("time",now);GLint rloc=glGetUniformLocation(post.id,"resolution");int fbw,fbh;glfwGetFramebufferSize(win,&fbw,&fbh);glUniform2f(rloc,(float)fbw,(float)fbh);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,pp.color);quad.draw();glEnable(GL_DEPTH_TEST);
   
   // Draw GUI on top
   gui.begin();
   gui.drawCrosshair();
   if (!ghostMode && !launchMode && currentInteractable >= 0) {
       const Interactable& it = interactables[currentInteractable];
       std::string text = std::string("[E] Interact: ") + it.name;
       float scale = 2.0f;
       float textW = text.length() * 8.0f * scale;
       gui.drawText(text, fbw/2.0f - textW/2.0f, fbh/2.0f + 20.0f, scale, {1.0f, 0.8f, 0.2f});
   }
   if (ghostMode) {
       std::string text = "GHOST MODE ACTIVE [F4 to Materialize]";
       float scale = 1.5f;
       float textW = text.length() * 8.0f * scale;
       gui.drawText(text, fbw/2.0f - textW/2.0f, 20.0f, scale, {0.3f, 0.8f, 1.0f});
   }
   if (seated) {
       std::string text = "[E] Stand up";
       float scale = 1.5f;
       float textW = text.length() * 8.0f * scale;
       gui.drawText(text, fbw/2.0f - textW/2.0f, fbh - 40.0f, scale, {1.0f, 1.0f, 1.0f});
   }
   // ---- Newspaper menu overlay ----
   if(newspaperMenu){
       // Dark overlay
       gui.drawRect(0,0,(float)fbw,(float)fbh,{0.06f,0.04f,0.02f},0.88f);
       // Newspaper page background
       float pw=680.f, ph=460.f;
       float px=(fbw-pw)*0.5f, py=(fbh-ph)*0.5f;
       gui.drawRect(px,py,pw,ph,{0.91f,0.87f,0.76f},1.0f);
       // Newspaper header bar
       gui.drawRect(px+8,py+8,pw-16,36,{0.12f,0.08f,0.04f},1.0f);
       // Left column divider
       gui.drawRect(px+pw*0.5f-2,py+52,4,ph-64,{0.30f,0.22f,0.14f},0.6f);
       // Header text
       gui.drawText("OUTPOST GAZETTE", px+14, py+14, 2.8f, {0.91f,0.87f,0.76f});
       // Subtitle
       gui.drawText("RAF RG Special Edition", px+14, py+52, 1.3f, {0.25f,0.18f,0.10f});
       // Decorative rule
       gui.drawRect(px+8,py+72,pw-16,2,{0.30f,0.22f,0.14f},0.8f);
       // Left column: RESET option
       gui.drawRect(px+pw*0.5f/2.f-120,py+102,240,44,{0.20f,0.14f,0.08f},0.18f);
       gui.drawText("[R] Restart Game", px+pw*0.25f-120, py+114, 1.8f, {0.10f,0.07f,0.03f});
       // Decorative filler lines (simulate news text)
       for(int row=0;row<8;row++){
           float lw=(row%3==2)?120.f:180.f;
           gui.drawRect(px+30,py+160+row*24,lw,6,{0.55f,0.44f,0.32f},0.55f);
       }
       // Right column: EXIT option
       gui.drawRect(px+pw*0.5f+pw*0.25f-120,py+102,240,44,{0.55f,0.08f,0.06f},0.25f);
       gui.drawText("[ESC] Quit Game", px+pw*0.5f+pw*0.25f-118, py+114, 1.8f, {0.55f,0.08f,0.06f});
       // Filler lines right column
       for(int row=0;row<8;row++){
           float lw=(row%2==1)?140.f:170.f;
           gui.drawRect(px+pw*0.5f+30,py+160+row*24,lw,6,{0.55f,0.44f,0.32f},0.55f);
       }
       // Footer
       gui.drawRect(px+8,py+ph-26,pw-16,2,{0.30f,0.22f,0.14f},0.8f);
       gui.drawText("Press [R] to restart or [ESC] to exit", px+14, py+ph-22, 1.2f, {0.35f,0.25f,0.15f});
   }
   gui.end();

   glfwSwapBuffers(win);glfwPollEvents();
  }
  glDeleteTextures(1,&nmap);
 }catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<"\n";glfwDestroyWindow(win);glfwTerminate();return 2;}
 glfwDestroyWindow(win);glfwTerminate();return 0;
}


