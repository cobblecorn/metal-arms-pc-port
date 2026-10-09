"""Build a local, offline vanilla/HD comparison page. Does not install assets."""
from pathlib import Path
import json
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / 'HD Texture Pack'


def load(path, default):
    return json.loads(path.read_text(encoding='utf-8')) if path.exists() else default


def main():
    usage = {item['stem']: item for item in load(PACK / 'catalog/usage.json', {}).get('textures', [])}
    rows = []
    for generated in sorted((PACK / 'generated').glob('*.png')):
        original = PACK / 'vanilla/color' / generated.name
        if not original.exists():
            continue
        source_size = list(Image.open(original).size)
        record = load(PACK / 'catalog/processed' / (generated.stem + '.json'), {})
        hd = PACK / 'hd/color' / generated.name
        item = usage.get(generated.stem, {})
        rows.append(dict(name=generated.stem, category=item.get('category', 'unclassified'),
                         original=original.relative_to(PACK).as_posix(),
                         hd=(hd if hd.exists() else generated).relative_to(PACK).as_posix(),
                         source_size=source_size,
                         hd_size=list(Image.open(hd if hd.exists() else generated).size),
                         worlds=item.get('world_count', 0),
                         roles=item.get('material_roles', {}),
                         status=record.get('review_status', 'pending_visual_and_runtime_review'),
                         holds=record.get('release_holds', []),
                         job='jobs/' + generated.stem + '.json'))
    data = json.dumps(rows).replace('<', '\\u003c')
    html = TEMPLATE.replace('__ASSET_DATA__', data)
    target = PACK / 'Compare Textures.html'
    target.write_text(html, encoding='utf-8')
    # Check every linked image exists; this page needs neither a server nor downloads.
    for row in rows:
        assert (PACK / row['original']).is_file()
        assert (PACK / row['hd']).is_file()
    print(f'Comparison gallery: {len(rows)} textures, all image paths verified')


TEMPLATE = r'''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Metal Arms — Texture comparisons</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#10151b;color:#e7edf4;font:15px system-ui,sans-serif}
header{padding:24px 28px;border-bottom:1px solid #303c49}h1{font-size:25px;margin:0 0 8px}p{margin:0;color:#aab8c8;line-height:1.6}
main{display:grid;grid-template-columns:270px minmax(0,1fr);min-height:80vh}aside{padding:20px;border-right:1px solid #303c49}
label{display:block;margin:0 0 8px;color:#b8c6d5}input,select,button{font:inherit;color:#eef4fa;background:#1c2632;border:1px solid #425167;border-radius:5px;padding:9px}
aside input,aside select{width:100%;margin-bottom:12px}#list{display:flex;flex-direction:column;gap:5px;max-height:65vh;overflow:auto}
#list button{text-align:left;font-family:ui-monospace,monospace;cursor:pointer}button:hover,button[aria-current=true]{background:#30435c;border-color:#81adcf}
article{padding:24px;min-width:0}h2{font-size:22px;margin:0 0 8px;font-family:ui-monospace,monospace}#status{color:#d4b77c;margin-bottom:18px}
.toolbar{display:flex;gap:8px;margin:18px 0;flex-wrap:wrap}.toolbar button{cursor:pointer}.panes{display:grid;grid-template-columns:1fr 1fr;gap:20px}
figure{margin:0;min-width:0}figcaption{color:#b8c6d5;margin:0 0 10px}.panes img{display:block;width:100%;height:auto;background:#05080b}
.original{image-rendering:pixelated}#wipe{position:relative;width:100%;max-width:1050px;background:#05080b;overflow:hidden}
#wipe img{position:absolute;inset:0;width:100%;height:100%}#line{position:absolute;top:0;bottom:0;left:50%;width:2px;background:#e7edf4;pointer-events:none}
#slider{width:100%;max-width:1050px;display:block;margin:12px 0;accent-color:#88bbdf}#repeat{max-width:1050px;background-repeat:repeat;background-size:50% 50%;background-color:#05080b}
#notes{margin-top:20px;line-height:1.7;color:#bbc7d5}a{color:#98c9ef}#count{font-size:13px;margin-bottom:12px}section[hidden],div[hidden],input[hidden]{display:none!important}
@media(max-width:760px){main{grid-template-columns:1fr}aside{border-right:0;border-bottom:1px solid #303c49}#list{max-height:180px}.panes{grid-template-columns:1fr}}
</style>
<header><h1>Metal Arms texture comparisons</h1><p>Original textures and HD drafts. These files are separate from the regular game. Generation and decoder checks do not confirm in-game UV or material fit.</p></header>
<main><aside><label for="search">Find a texture</label><input id="search" placeholder="RAT, metal, grunt…" type="search"><label for="category">Category</label><select id="category"><option value="">All categories</option></select><p id="count"></p><nav id="list" aria-label="Textures"></nav></aside>
<article><h2 id="name"></h2><p id="status"></p><p id="dimensions"></p>
<div class="toolbar"><button id="sideBtn">Side by side</button><button id="wipeBtn">Comparison slider</button><button id="repeatBtn">HD repeated 2 × 2</button></div>
<section class="panes" id="panes"><figure><figcaption>Vanilla</figcaption><a id="originalLink"><img id="original" class="original" alt="Original texture"></a></figure><figure><figcaption>HD draft</figcaption><a id="hdLink"><img id="hd" alt="Generated HD texture"></a></figure></section>
<div id="wipe" hidden><img id="wipeOriginal" class="original" alt="Original texture"><img id="wipeHd" alt="HD texture"><div id="line"></div></div><input id="slider" type="range" min="0" max="100" value="50" aria-label="HD portion of comparison" hidden>
<div id="repeat" role="img" aria-label="HD texture repeated four times" hidden></div><p id="notes"></p></article></main>
<script>
const assets=__ASSET_DATA__;
const $=id=>document.getElementById(id);let selected=assets[0];let mode='side';
const categories=[...new Set(assets.map(a=>a.category))].sort();
for(const c of categories){const option=document.createElement('option');option.value=c;option.textContent=c.replaceAll('_',' ');$('category').append(option)}
function list(){const query=$('search').value.toLowerCase();const category=$('category').value;const matches=assets.filter(a=>(!category||a.category===category)&&a.name.toLowerCase().includes(query));$('count').textContent=matches.length+' of '+assets.length+' generated textures';$('list').replaceChildren();for(const a of matches){const b=document.createElement('button');b.textContent=a.name;b.setAttribute('aria-current',a===selected);b.onclick=()=>{selected=a;show();list()};$('list').append(b)}}
function show(){if(!selected)return;const a=selected;$('name').textContent=a.name;$('dimensions').textContent='Vanilla '+a.source_size.join(' × ')+' · Stored HD '+a.hd_size.join(' × ')+' · Used in '+a.worlds+' mapped worlds';$('status').textContent=a.holds.length?'Held for material or graphic review':a.status==='user_tested_on_vehicle'?'User tested on the RAT vehicle':'Draft — visual and runtime review pending';
for(const id of ['original','wipeOriginal'])$(id).src=a.original;for(const id of ['hd','wipeHd'])$(id).src=a.hd;$('originalLink').href=a.original;$('hdLink').href=a.hd;$('wipe').style.aspectRatio=a.source_size[0]+'/'+a.source_size[1];$('repeat').style.aspectRatio=a.source_size[0]+'/'+a.source_size[1];$('repeat').style.backgroundImage='url("'+a.hd+'")';
$('notes').replaceChildren();const roles=document.createElement('div');roles.textContent='Material uses: '+Object.entries(a.roles).map(([role,n])=>role.replaceAll('_',' ')+' ('+n+')').join(', ');$('notes').append(roles);for(const hold of a.holds){const p=document.createElement('div');p.textContent=hold;$('notes').append(p)}const link=document.createElement('a');link.href=a.job;link.textContent='Saved generation prompt and review record';$('notes').append(link);setMode(mode)}
function setMode(next){mode=next;$('panes').hidden=mode!=='side';$('wipe').hidden=mode!=='wipe';$('slider').hidden=mode!=='wipe';$('repeat').hidden=mode!=='repeat';updateSlider()}
function updateSlider(){const v=$('slider').value;$('wipeHd').style.clipPath='inset(0 '+(100-v)+'% 0 0)';$('line').style.left=v+'%'}
$('search').oninput=list;$('category').onchange=list;$('sideBtn').onclick=()=>setMode('side');$('wipeBtn').onclick=()=>setMode('wipe');$('repeatBtn').onclick=()=>setMode('repeat');$('slider').oninput=updateSlider;show();list();
</script></html>'''


if __name__ == '__main__':
    main()
