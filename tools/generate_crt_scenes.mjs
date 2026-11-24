// Original deterministic procedural grayscale scene renderer, no external art.
// Run: node tools/generate_crt_scenes.mjs
import {writeFileSync} from 'node:fs';
function renderScene(index) {
 const W=224,H=196, boxes=[];
 const add=(a,b,c,d,e,f,g,mat=0)=>boxes.push({lo:[a,b,c],hi:[d,e,f],gray:g,mat});
 const room=(width=3.2,depth=9)=>{
  add(-width,-.15,-1,width,0,depth,74,2);
  add(-width,3.3,-1,width,3.5,depth,54,1);
  add(-width-.2,0,-1,-width,3.3,depth,95,1);
  add(width,0,-1,width+.2,3.3,depth,90,1);
  add(-width,0,depth,width,3.3,depth+.2,83,1);
 };
 const door=(x,z,side=0)=>{
  if(side){const w=side*3.18;add(Math.min(w,w-side*.10),0,z,Math.max(w,w-side*.10),2.5,z+1.4,24,3);add(Math.min(w,w-side*.16),0,z-.08,Math.max(w,w-side*.16),2.6,z,124,3);}
  else{add(x,0,z,x+1.1,2.45,z+.05,24,3);add(x-.07,0,z-.06,x,2.52,z,125,3);add(x+1.1,0,z-.06,x+1.17,2.52,z,125,3);add(x-.07,2.45,z-.06,x+1.17,2.52,z,125,3);add(x+.88,1.02,z-.09,x+.98,1.08,z,195,3);}
 };
 const bench=(x,z)=>{add(x,.69,z,x+1.85,.84,z+.65,121,3);for(const X of [x+.08,x+1.68])for(const Z of [z+.08,z+.53])add(X,0,Z,X+.065,.7,Z+.065,100,3);add(x,.83,z+.57,x+1.85,1.42,z+.65,91,3)};
 const cabinet=(x,z,w=1.1,h=2.1)=>{add(x,0,z,x+w,h,z+.52,89,3);for(let i=0;i<4;i++){const y=.09+i*(h-.17)/4;add(x+.04,y,z-.04,x+w-.04,y+(h-.21)/4,z,122,3);add(x+w*.35,y+(h-.21)/8,z-.09,x+w*.65,y+(h-.21)/8+.05,z-.04,53,3);}};
 const device=(x,y,z,w,h)=>{add(x,y,z,x+w,y+h,z+.42,104,3);add(x+.05,y+.06,z-.035,x+w-.05,y+h-.06,z,32,3);add(x+.11,y+h*.48,z-.05,x+w-.11,y+h-.13,z-.035,150,3);for(let j=0;j<4;j++)add(x+.12+j*(w-.24)/4,y+.12,z-.085,x+.18+j*(w-.24)/4,y+.22,z-.05,183,3);};
 const bed=(x,z)=>{add(x,.64,z,x+1.05,.82,z+2.2,135,4);add(x,.82,z+.07,x+1.05,.99,z+.47,158,4);for(const X of [x+.05,x+.93])for(const Z of [z+.05,z+2.04])add(X,0,Z,X+.055,1.15,Z+.055,121,3);for(const Z of [z,z+2.16])add(x,.98,Z,x+1.05,1.05,Z+.045,140,3);};
 let camera=[0,1.55,-.3],aim=[0,1.5,6.5],light=[-.8,2.85,3.1];
 room(index===0?2.1:3.2,index===0?13:9);
 if(index===0){camera=[.08,1.58,-.25];aim=[0,1.37,11];light=[0,3.0,5.6];door(-.57,12.94);for(let z=2;z<12;z+=2.35){for(const k of [-1,1]){const xx=k*2.08;add(Math.min(xx,xx-k*.06),0,z,Math.max(xx,xx-k*.06),2.45,z+1.06,23,3);add(Math.min(xx,xx-k*.10),2.39,z-.08,Math.max(xx,xx-k*.10),2.54,z+1.14,130,3);}add(-.24,3.16,z,.24,3.27,z+.76,225,5);}}
 if(index===1){camera=[-1.75,1.74,.45];aim=[.45,1.4,5.7];light=[1.8,3,4];add(-2.5,.2,8.9,2.7,2.85,8.96,27,3);for(let i=0;i<4;i++)add(-2.5+i*1.3,.2,8.83,-2.43+i*1.3,2.85,8.9,130,3);bench(-1.65,5.2);cabinet(1.3,4.65,.95);device(1.37,2.1,4.7,.8,.64);}
 if(index===2){camera=[-.55,1.6,.15];aim=[0,1.55,5.25];light=[-1,2.8,3];cabinet(-2.2,6.4);cabinet(1.1,6.4);device(-1.65,.65,3.9,1.12,1.65);device(.3,.65,4.18,1.12,1.65);add(-2.4,.52,3.85,2.0,.66,5.1,100,3);for(const x of [-2.2,1.8])add(x,0,4.5,x+.11,.52,4.61,110,3);}
 if(index===3){camera=[.1,1.65,.12];aim=[0,1.6,4.3];light=[1.5,2.8,2.5];for(let j=0;j<3;j++)device(-1.68+j*1.1,.72,3.7,1,1.8);add(-1.9,.58,3.72,1.9,.72,4.3,117,3);for(const x of [-1.8,1.7])add(x,0,3.8,x+.1,.58,4.0,110,3);cabinet(-2.9,6.8);}
 if(index===4){camera=[-1.4,1.6,.35];aim=[.35,1.25,5.7];light=[.8,2.9,3.7];bed(-.4,4.1);device(1.15,.87,4.7,.9,1.0);add(1.55,0,4.95,1.65,.87,5.05,145,3);add(-.1,2.6,4.3,.65,2.75,4.8,196,5);add(.22,2.75,4.51,.28,3.3,4.57,110,3);}
 if(index===5){camera=[.1,1.5,.15];aim=[0,1.4,7.2];light=[-.9,2.9,4.4];door(-.54,8.94);add(-1.25,.88,8.91,-.9,1.65,8.94,14,3);bench(-2.2,6.4);add(-.3,3.17,5,.3,3.28,5.7,211,5);}
 if(index===6){camera=[-.85,1.58,.1];aim=[.4,1.45,6.8];light=[-.6,2.8,3];for(let j=0;j<5;j++)cabinet(-2.7+j*1.04,6.9,.98,2.48);for(let j=0;j<3;j++){add(-2.7,.65+j*.64,3.65,-1.9,.72+j*.64,6.2,102,3);for(let k=0;k<8;k++)add(-2.6,.72+j*.64,3.73+k*.29,-2.03,1.12+j*.64,3.95+k*.29,98+k*7,4);}}
 if(index===7){camera=[1.05,1.52,.1];aim=[-.1,1.3,7.1];light=[-1.9,2.9,6.4];bed(-1.5,4.2);bed(.42,5.9);add(-2.1,1.35,8.91,-.6,2.76,8.96,165,5);for(let k=0;k<4;k++)add(-2.08+k*.5,1.35,8.85,-2.03+k*.5,2.76,8.91,43,3);}
 if(index===8){camera=[.28,1.42,.1];aim=[0,1.45,4.7];light=[-.8,2.9,3.25];device(-1.55,.52,3.95,1.5,1.8);device(.15,.52,4.22,1.42,1.8);for(let i=0;i<5;i++)add(-1.37+i*.25,.79,3.84,-1.31+i*.25,1.01,3.95,200,3);add(-1.8,.40,3.9,1.8,.52,4.9,114,3);}
 const sub=(a,b)=>a.map((v,i)=>v-b[i]),dot=(a,b)=>a.reduce((s,v,i)=>s+v*b[i],0),unit=a=>{const l=Math.hypot(...a);return a.map(v=>v/l)},cross=(a,b)=>[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];
 const front=unit(sub(aim,camera)),right=unit(cross([0,1,0],front)),up=cross(front,right);
 function hit(o,d,max=100,ignore=-1){let best=null;for(let k=0;k<boxes.length;k++){if(k===ignore)continue;const b=boxes[k];let t0=.0001,t1=max,axis=0,sign=0;for(let i=0;i<3;i++){if(Math.abs(d[i])<1e-10){if(o[i]<b.lo[i]||o[i]>b.hi[i]){t0=1;t1=0;break;}continue;}let x=(b.lo[i]-o[i])/d[i],y=(b.hi[i]-o[i])/d[i],sg=-1;if(x>y){[x,y]=[y,x];sg=1;}if(x>t0){t0=x;axis=i;sign=sg;}t1=Math.min(t1,y);if(t0>t1)break;}if(t0<=t1&&t0<max){max=t0;best={t:t0,k,axis,sign};}}return best;}
 function noise(x,y,z){let n=Math.imul(x|0,374761393)+Math.imul(y|0,668265263)+Math.imul(z|0,1442695041);n=Math.imul(n^(n>>>13),1274126177);return ((n^(n>>>16))>>>0)/4294967295;}
 const raw=new Uint8Array((W+1)*H);
 for(let y=0;y<H;y++)for(let x=0;x<W;x++){
 const u=(x+.5-W/2)/H*1.05,v=(H/2-y-.5)/H*1.05;
 const d=unit(front.map((q,i)=>q+u*right[i]+v*up[i])),h=hit(camera,d);
 let c=10;
 if(h){const b=boxes[h.k],p=camera.map((q,i)=>q+h.t*d[i]),n=[0,0,0];n[h.axis]=h.sign;const ld=sub(light,p),dist=Math.hypot(...ld),l=unit(ld),diff=Math.max(0,dot(l,n));const shadow=hit(p.map((q,i)=>q+n[i]*.003),l,dist-.025,h.k);const direct=shadow?.12:1;const fleck=noise(p[0]*180,p[1]*180,p[2]*180);let texture=(fleck-.5)*14;
 if(b.mat===1){texture+=(noise(p[0]*8,p[1]*9,p[2]*7)-.5)*23;const line=p[1]>.86&&p[1]<.89; if(line)texture-=34;}
 if(b.mat===2){const line=Math.min(Math.abs(p[0]*2-Math.round(p[0]*2)),Math.abs(p[2]*2-Math.round(p[2]*2)));if(line<.024)texture-=22;texture+=(noise(p[0]*7,0,p[2]*7)-.5)*15;}
 if(b.mat===3)texture+=(noise(p[0]*3,p[1]*230,p[2]*3)-.5)*12;
 if(b.mat===4)texture+=(Math.sin(p[0]*120+p[2]*40)*2.8);
 const illum=.29+direct*diff*2.5/(1+.13*dist*dist);
 c=b.mat===5?b.gray:(b.gray+texture)*illum;
 c*=Math.exp(-h.t*.014);
 }
 const vig=Math.max(.36,1-.47*(u*u*1.5+v*v*1.7));
 c=Math.pow(Math.max(0,c/255),.83)*255*vig+(noise(x,y,index+44)-.5)*17;
 if(noise(x,0,index+92)>.993)c+=8;
 raw[y*(W+1)+x+1]=Math.round(Math.max(0,Math.min(255,c)));
 }
 return {W,H,raw};
}
function png({W,H,raw}) {
 const u32=n=>[(n>>>24)&255,(n>>>16)&255,(n>>>8)&255,n&255];
 function crc(a){let c=0xffffffff;for(const b of a){c^=b;for(let i=0;i<8;i++)c=(c>>>1)^((c&1)?0xedb88320:0);}return (c^0xffffffff)>>>0;}
 function chunk(name,data){const p=[...name].map(c=>c.charCodeAt(0)).concat(Array.from(data));return [...u32(data.length),...p,...u32(crc(p))];}
 let z=[0x78,0x01],a=1,b=0;for(let i=0;i<raw.length;i+=65535){const part=raw.slice(i,i+65535),n=part.length;z.push(i+n===raw.length?1:0,n&255,n>>>8,(~n)&255,((~n)>>>8)&255,...part);for(const k of part){a=(a+k)%65521;b=(b+a)%65521;}}z.push(...u32((b<<16)|a));
 return new Uint8Array([137,80,78,71,13,10,26,10,...chunk("IHDR",[...u32(W),...u32(H),8,0,0,0,0]),...chunk("IDAT",z),...chunk("IEND",[])]);
}
for(let i=0;i<9;i++)writeFileSync(new URL('../resource/scene-'+(i+1)+'.png',import.meta.url),png(renderScene(i)));
