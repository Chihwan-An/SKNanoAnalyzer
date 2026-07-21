from __future__ import annotations

import html
import json
import os
from pathlib import Path
from typing import Any


def write_gallery(output: str | Path, records: list[dict[str, Any]]) -> Path:
    output_path = Path(output)
    cards: list[str] = []
    for record in records:
        png = record.get("files", {}).get("png")
        if not png:
            continue
        rel = Path(png).relative_to(output_path).as_posix()
        title = html.escape(record.get("title", record.get("task_id", rel)))
        search = html.escape(" ".join([title, json.dumps(record.get("facets", {}), sort_keys=True)]))
        cards.append(
            f'<article data-search="{search.lower()}"><a href="{rel}"><img loading="lazy" src="{rel}" alt="{title}"></a>'
            f"<h2>{title}</h2><code>{html.escape(json.dumps(record.get('facets', {}), sort_keys=True))}</code></article>"
        )
    document = f"""<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>SKNano plots</title><style>
body{{font-family:system-ui,sans-serif;margin:1.5rem;background:#f6f7f8;color:#202124}}
input{{width:min(42rem,95%);padding:.7rem;font-size:1rem;margin-bottom:1.2rem}}
main{{display:grid;grid-template-columns:repeat(auto-fill,minmax(320px,1fr));gap:1rem}}
article{{background:white;padding:.8rem;border-radius:.4rem;box-shadow:0 1px 4px #0002}}
img{{width:100%;height:auto}}h2{{font-size:1rem;margin:.5rem 0}}code{{font-size:.72rem;word-break:break-all}}
</style></head><body><h1>SKNano plots</h1><input id="q" placeholder="Filter plots" autofocus>
<main>{''.join(cards)}</main><script>
const q=document.querySelector('#q');q.addEventListener('input',()=>{{const v=q.value.toLowerCase();
document.querySelectorAll('article').forEach(x=>x.hidden=!x.dataset.search.includes(v));}});
</script></body></html>"""
    destination = output_path / "index.html"
    temporary = output_path / f".index.{os.getpid()}.tmp"
    temporary.write_text(document, encoding="utf-8")
    temporary.replace(destination)
    return destination
