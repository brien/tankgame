from pathlib import Path
import sys
runtime=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2]).resolve()
out.mkdir(parents=True,exist_ok=True)
for name in ['tankgame-linux.js','tankgame-linux.wasm','tankgame-linux.data']:
 p=out/name
 if not p.exists():p.symlink_to(runtime/name)
html=(runtime/'tankgame-linux.html').read_text()
cases={
 'default':'',
 'missing':"FS.unlink('/gameplay.json');",
 'invalid':"FS.writeFile('/gameplay.json','{bad json');",
 'alternative':"FS.writeFile('/alternative.json',FS.readFile('/gameplay.json'));FS.writeFile('/gameplay.json','{}');",
 'invalid-alternative':"FS.writeFile('/alternative.json','{}');"
}
for name,hook in cases.items():
 args="Module.arguments=['--gameplay-config','alternative.json'];" if 'alternative' in name else ''
 injection='<script>'+args+'Module.onRuntimeInitialized=function(){'+hook+'};</script>'
 assert '<script async' in html
 (out/(name+'.html')).write_text(html.replace('<script async',injection+'<script async',1))
