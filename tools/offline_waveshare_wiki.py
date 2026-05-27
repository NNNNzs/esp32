#!/usr/bin/env python3
import argparse
import html
import os
import re
import sys
import time
import urllib.parse
import urllib.request
from html.parser import HTMLParser
from pathlib import Path


DEFAULT_URL = "https://www.waveshare.net/wiki/ESP32-S3-Touch-LCD-1.83"


class WikiMarkdownExtractor(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.in_content = False
        self.capture_depth = 0
        self.skip_depth = 0
        self.tag_stack = []
        self.current_link = None
        self.current_heading = None
        self.heading_text = []
        self.list_stack = []
        self.lines = []
        self.cell_text = []
        self.in_table_cell = False
        self.in_pre = False
        self.pre_buf = []
        self.started = False

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        self.tag_stack.append(tag)

        if tag == "div" and attrs.get("id") == "mw-content-text":
            self.in_content = True
            self.capture_depth = 1
            return

        if not self.in_content:
            return

        if tag == "div":
            self.capture_depth += 1

        classes = attrs.get("class", "")
        if tag in {"script", "style"} or "printfooter" in classes or "catlinks" in classes:
            self.skip_depth += 1
            return
        if self.skip_depth:
            return

        if tag in {"h1", "h2", "h3", "h4"}:
            self.flush_line()
            self.current_heading = tag
            self.heading_text = []
        elif tag == "p":
            self.flush_line()
        elif tag in {"ul", "ol"}:
            self.list_stack.append(tag)
        elif tag == "li":
            prefix = "1. " if self.list_stack and self.list_stack[-1] == "ol" else "- "
            self.lines.append(prefix)
        elif tag == "br":
            self.flush_line()
        elif tag == "a":
            href = attrs.get("href")
            if href:
                self.current_link = href
        elif tag == "img":
            src = attrs.get("src")
            alt = attrs.get("alt") or "image"
            if src:
                self.flush_line()
                self.lines.append(f"![{alt}]({src})")
                self.flush_line()
        elif tag in {"td", "th"}:
            self.in_table_cell = True
            self.cell_text = []
        elif tag == "tr":
            self.flush_line()
        elif tag in {"pre", "code"}:
            self.in_pre = True
            self.pre_buf = []

    def handle_endtag(self, tag):
        if not self.in_content:
            if tag in self.tag_stack:
                self.tag_stack.pop()
            return

        if self.skip_depth:
            if tag in {"script", "style", "div"}:
                self.skip_depth -= 1
            return

        if tag == self.current_heading:
            level = int(tag[1])
            text = clean_text("".join(self.heading_text))
            if text:
                self.flush_line()
                self.lines.append("#" * level + " " + text)
                self.flush_line()
            self.current_heading = None
            self.heading_text = []
        elif tag == "p":
            self.flush_line()
        elif tag in {"ul", "ol"}:
            if self.list_stack:
                self.list_stack.pop()
            self.flush_line()
        elif tag == "li":
            self.flush_line()
        elif tag == "a":
            self.current_link = None
        elif tag in {"td", "th"}:
            text = clean_text("".join(self.cell_text))
            if text:
                self.lines.append(text + " | ")
            self.in_table_cell = False
            self.cell_text = []
        elif tag == "tr":
            self.flush_line()
        elif tag in {"pre", "code"}:
            text = "".join(self.pre_buf).strip()
            if text:
                self.flush_line()
                self.lines.append("```")
                self.lines.extend(text.splitlines())
                self.lines.append("```")
                self.flush_line()
            self.in_pre = False
            self.pre_buf = []

        if tag == "div":
            self.capture_depth -= 1
            if self.capture_depth <= 0:
                self.in_content = False

        if self.tag_stack:
            self.tag_stack.pop()

    def handle_data(self, data):
        if not self.in_content or self.skip_depth:
            return
        if self.in_pre:
            self.pre_buf.append(data)
            return

        text = clean_text(data)
        if not text:
            return
        if self.current_heading:
            self.heading_text.append(text + " ")
            return
        if self.in_table_cell:
            self.cell_text.append(text + " ")
            return
        if self.current_link:
            self.lines.append(f"[{text}]({self.current_link})")
        else:
            self.lines.append(text + " ")

    def flush_line(self):
        if not self.lines:
            return
        if self.lines[-1] != "":
            self.lines.append("")

    def markdown(self):
        content = "\n".join(self.lines)
        content = re.sub(r"[ \t]+\n", "\n", content)
        content = re.sub(r"\n{3,}", "\n\n", content)
        return content.strip() + "\n"


def clean_text(value):
    return re.sub(r"\s+", " ", html.unescape(value)).strip()


def fetch(url):
    req = urllib.request.Request(
        url,
        headers={
            "User-Agent": "Mozilla/5.0 offline-doc-builder",
            "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.6",
        },
    )
    with urllib.request.urlopen(req, timeout=60) as response:
        return response.read(), response.headers.get_content_charset() or "utf-8"


def decode_html(page_bytes, fallback):
    head = page_bytes[:4096].decode("ascii", errors="ignore")
    match = re.search(r"charset=[\"']?([A-Za-z0-9._-]+)", head, flags=re.I)
    charset = match.group(1) if match else fallback
    return page_bytes.decode(charset, errors="replace")


def absolutize(url, base):
    return urllib.parse.urljoin(base, html.unescape(url))


def filename_from_url(url):
    parsed = urllib.parse.urlparse(url)
    name = Path(urllib.parse.unquote(parsed.path)).name or "asset"
    name = re.sub(r"[^A-Za-z0-9._-]", "_", name)
    return name


def download_assets(raw_html, base_url, out_dir):
    assets_dir = out_dir / "assets"
    assets_dir.mkdir(parents=True, exist_ok=True)
    replacements = {}
    candidates = set()

    for attr in ("src", "href"):
        for match in re.finditer(rf'{attr}=["\']([^"\']+)["\']', raw_html, flags=re.I):
            target = match.group(1)
            if target.startswith(("data:", "mailto:", "javascript:")):
                continue
            absolute = absolutize(target, base_url)
            lower = urllib.parse.urlparse(absolute).path.lower()
            if any(lower.endswith(ext) for ext in (".png", ".jpg", ".jpeg", ".gif", ".webp", ".svg", ".pdf", ".zip", ".7z", ".rar")):
                candidates.add((target, absolute))

    manifest = []
    for original, absolute in sorted(candidates, key=lambda item: item[1]):
        name = filename_from_url(absolute)
        dest = assets_dir / name
        stem = dest.stem
        suffix = dest.suffix
        counter = 1
        while dest.exists() and replacements.get(original) != f"assets/{dest.name}":
            if dest.stat().st_size > 0:
                break
            dest = assets_dir / f"{stem}-{counter}{suffix}"
            counter += 1
        try:
            data, _ = fetch(absolute)
            dest.write_bytes(data)
            local = f"assets/{dest.name}"
            replacements[original] = local
            manifest.append((absolute, local, len(data)))
            time.sleep(0.15)
        except Exception as exc:
            manifest.append((absolute, "DOWNLOAD_FAILED: " + str(exc), 0))

    offline_html = raw_html
    for original, local in replacements.items():
        offline_html = offline_html.replace(original, local)
    return offline_html, manifest


def extract_links(raw_html, base_url):
    links = []
    for match in re.finditer(r'<a\b[^>]*href=["\']([^"\']+)["\'][^>]*>(.*?)</a>', raw_html, flags=re.I | re.S):
        href = absolutize(match.group(1), base_url)
        text = clean_text(re.sub(r"<[^>]+>", " ", match.group(2)))
        if href and not href.startswith("javascript:"):
            links.append((text or href, href))
    seen = set()
    unique = []
    for text, href in links:
        key = (text, href)
        if key not in seen:
            unique.append(key)
            seen.add(key)
    return unique


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", default=DEFAULT_URL)
    parser.add_argument("--out", default="docs/waveshare-esp32-s3-touch-lcd-1.83")
    args = parser.parse_args()

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    page_bytes, charset = fetch(args.url)
    raw_html = decode_html(page_bytes, charset)
    (out_dir / "source.html").write_text(raw_html, encoding="utf-8")

    offline_html, manifest = download_assets(raw_html, args.url, out_dir)
    (out_dir / "index.html").write_text(offline_html, encoding="utf-8")

    extractor = WikiMarkdownExtractor()
    extractor.feed(offline_html)
    md = [
        "# ESP32-S3-Touch-LCD-1.83 离线文档",
        "",
        f"- 来源: {args.url}",
        f"- 离线化时间: {time.strftime('%Y-%m-%d %H:%M:%S')}",
        "",
        extractor.markdown(),
    ]
    (out_dir / "README.md").write_text("\n".join(md), encoding="utf-8")

    with (out_dir / "links.md").open("w", encoding="utf-8") as f:
        f.write("# 页面链接清单\n\n")
        for text, href in extract_links(raw_html, args.url):
            f.write(f"- [{text}]({href})\n")

    with (out_dir / "assets-manifest.tsv").open("w", encoding="utf-8") as f:
        f.write("source\tlocal\tsize_bytes\n")
        for source, local, size in manifest:
            f.write(f"{source}\t{local}\t{size}\n")

    print(f"Offline wiki saved to: {out_dir.resolve()}")
    print(f"Assets attempted: {len(manifest)}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
