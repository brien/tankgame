import ctypes as C, time, os, subprocess, sys, struct, zlib, json
out=os.environ.get('TANKGAME_REVIEW_OUT', '/home/deck/Dropbox/work/Programming/tankgame/documentation/manual-review-2026-09-29')
os.makedirs(out,exist_ok=True)
x=C.CDLL('libX11.so.6'); t=C.CDLL('libXtst.so.6')
def sig(lib,name,args,res):
 f=getattr(lib,name); f.argtypes=args; f.restype=res; return f
P=C.c_void_p; U=C.c_ulong; I=C.c_int
@C.CFUNCTYPE(I,P,P)
def xerror(display,event):
 print('X11 error; game process status',p.poll(),flush=True)
 return 0
x.XSetErrorHandler.argtypes=[C.CFUNCTYPE(I,P,P)];x.XSetErrorHandler(xerror)
sig(x,'XOpenDisplay',[C.c_char_p],P); d=x.XOpenDisplay(None)
if not d: raise RuntimeError('No X display')
sig(x,'XDefaultRootWindow',[P],U); root=x.XDefaultRootWindow(d)
sig(x,'XQueryTree',[P,U,C.POINTER(U),C.POINTER(U),C.POINTER(C.POINTER(U)),C.POINTER(C.c_uint)],I)
sig(x,'XFetchName',[P,U,C.POINTER(C.c_char_p)],I);sig(x,'XFree',[P],I)
sig(x,'XRaiseWindow',[P,U],I);sig(x,'XSetInputFocus',[P,U,I,U],I);sig(x,'XFlush',[P],I)
sig(x,'XStringToKeysym',[C.c_char_p],U);sig(x,'XKeysymToKeycode',[P,U],C.c_uint)
sig(t,'XTestFakeKeyEvent',[P,C.c_uint,I,U],I);sig(t,'XTestFakeButtonEvent',[P,C.c_uint,I,U],I)
sig(t,'XTestFakeRelativeMotionEvent',[P,I,I,U],I)
def windows(w):
 r=U();p=U();a=C.POINTER(U)();n=C.c_uint(); x.XQueryTree(d,w,C.byref(r),C.byref(p),C.byref(a),C.byref(n)); children=[a[i] for i in range(n.value)];x.XFree(a)
 for c in children:
  name=C.c_char_p();x.XFetchName(d,c,C.byref(name)); s=name.value;x.XFree(name)
  if s==b'tankgame':return c
  found=windows(c)
  if found:return found
 return None
def key(name,secs=.35):
 if p.poll() is not None: raise RuntimeError('Game exited: '+str(p.returncode))
 x.XSetInputFocus(d,win,1,0);x.XFlush(d)
 code=x.XKeysymToKeycode(d,x.XStringToKeysym(name.encode()));t.XTestFakeKeyEvent(d,code,1,0);x.XFlush(d);time.sleep(secs);t.XTestFakeKeyEvent(d,code,0,0);x.XFlush(d);time.sleep(.25)
sig(x,'XGetGeometry',[P,U,C.POINTER(U),C.POINTER(I),C.POINTER(I),C.POINTER(C.c_uint),C.POINTER(C.c_uint),C.POINTER(C.c_uint),C.POINTER(C.c_uint)],I)
sig(x,'XGetImage',[P,U,I,I,C.c_uint,C.c_uint,U,I],P);sig(x,'XGetPixel',[P,I,I],U);sig(x,'XDestroyImage',[P],I)
class XI(C.Structure):
 _fields_=[('width',I),('height',I),('xoffset',I),('format',I),('data',P),('byte_order',I),('bitmap_unit',I),('bitmap_bit_order',I),('bitmap_pad',I),('depth',I),('bytes_per_line',I),('bits_per_pixel',I)]
def shot(label):
 r=U();xx=I();yy=I();w=C.c_uint();h=C.c_uint();b=C.c_uint();depth=C.c_uint();x.XGetGeometry(d,win,C.byref(r),C.byref(xx),C.byref(yy),C.byref(w),C.byref(h),C.byref(b),C.byref(depth))
 im=x.XGetImage(d,win,0,0,w.value,h.value,0xffffffffffffffff,2);v=C.cast(im,C.POINTER(XI)).contents
 raw=C.string_at(v.data,v.bytes_per_line*v.height);rows=[]
 for y in range(v.height):
  row=raw[y*v.bytes_per_line:y*v.bytes_per_line+v.width*4]; rgb=bytearray(v.width*3);rgb[0::3]=row[2::4];rgb[1::3]=row[1::4];rgb[2::3]=row[0::4];rows.append(b'\0'+rgb)
 def chunk(k,data):return struct.pack('!I',len(data))+k+data+struct.pack('!I',zlib.crc32(k+data)&0xffffffff)
 data=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('!2I5B',v.width,v.height,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(b''.join(rows)))+chunk(b'IEND',b'')
 open(out+'/'+mode+'-'+label+'.png','wb').write(data);x.XDestroyImage(im);print(label,flush=True)
mode=sys.argv[1];scenario=sys.argv[2];modeLabel=mode;mode=mode+'-fresh-'+scenario
env=os.environ.copy();env['SDL_VIDEODRIVER']='x11'
if modeLabel=='modern':env['TANKGAME_RENDERER']='modern'
else:env.pop('TANKGAME_RENDERER',None)
with open(out+'/'+mode+'.log','w') as log:
 p=subprocess.Popen(['./tankgame-linux'],cwd='/home/deck/Dropbox/work/Programming/tankgame/runtime',env=env,stdout=log,stderr=subprocess.STDOUT)
 try:
  time.sleep(3);win=windows(root)
  if not win:raise RuntimeError('No game window; exit='+str(p.poll()))
  x.XRaiseWindow(d,win);x.XSetInputFocus(d,win,1,0);x.XFlush(d)
  key('Right')
  if scenario=='versus':key('Down')
  key('Return');time.sleep(1);shot('idle')
  key('d',.4);key('w',.5);shot('moved');key('Up',.7);shot('camera')
  t.XTestFakeButtonEvent(d,1,1,0);x.XFlush(d);time.sleep(.7);shot('firing');time.sleep(2);shot('later');t.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d)
  key('Escape');key('Left');shot('single-selected');key('Return');time.sleep(1);shot('single-after-split')
  key('Escape');key('Escape');p.wait(timeout=8);print('exit',p.returncode,flush=True)
 finally:
  print('process status before cleanup',p.poll(),flush=True)
  if p.poll() is None:p.terminate();p.wait()
