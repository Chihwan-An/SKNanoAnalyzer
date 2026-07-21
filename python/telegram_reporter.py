#!/usr/bin/env python3
"""Small, failure-tolerant Telegram client used by SKNano job reporting."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path
from typing import Optional, Tuple

import requests


TELEGRAM_MESSAGE_LIMIT = 4096
_CONFIG_PATTERN = re.compile(r"^\[(?P<key>[^]]+)\](?:\s+(?P<value>.*))?$")
_EMOJI_PATTERN = re.compile(
    "["
    "\U0001F000-\U0001FAFF"
    "\u2600-\u27BF"
    "\uFE0F"
    "\u200D"
    "]+"
)


def _strip_emoji(text: str) -> str:
    return _EMOJI_PATTERN.sub("", text)


def _read_config(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    try:
        with path.open(encoding="utf-8") as handle:
            for raw_line in handle:
                match = _CONFIG_PATTERN.match(raw_line.strip())
                if match:
                    values[match.group("key")] = (match.group("value") or "").strip()
    except OSError:
        pass
    return values


def load_telegram_credentials(
    config_path: Optional[str] = None,
) -> Tuple[Optional[str], Optional[str]]:
    """Load credentials from the environment, falling back to the user config.

    Reading the config in the final Condor job avoids embedding the bot token in
    generated shell scripts or the DAGMan environment ClassAd.
    """

    token = os.environ.get("TOKEN_TELEGRAMBOT", "").strip()
    chat_id = os.environ.get("USER_CHATID", "").strip()
    if token and chat_id:
        return token, chat_id

    candidates = []
    if config_path:
        candidates.append(Path(config_path))
    home = os.environ.get("SKNANO_HOME")
    user = os.environ.get("USER")
    if home and user:
        candidates.append(Path(home) / "config" / f"config.{user}")

    for candidate in candidates:
        config = _read_config(candidate)
        token = token or config.get("TOKEN_TELEGRAMBOT", "")
        chat_id = chat_id or config.get("USER_CHATID", "")
        if token and chat_id:
            return token, chat_id
    return None, None


def _split_message(text: str, limit: int = TELEGRAM_MESSAGE_LIMIT) -> list[str]:
    text = text.strip()
    if not text:
        return []

    chunks = []
    while len(text) > limit:
        split_at = text.rfind("\n", 0, limit + 1)
        if split_at < limit // 2:
            split_at = limit
        chunks.append(text[:split_at].rstrip())
        text = text[split_at:].lstrip("\n")
    if text:
        chunks.append(text)
    return chunks


def send_telegram_message(
    text: str,
    *,
    config_path: Optional[str] = None,
    timeout: Tuple[float, float] = (5.0, 15.0),
) -> bool:
    """Send plain-text Telegram messages without affecting analysis status."""

    token, chat_id = load_telegram_credentials(config_path)
    if not token or not chat_id:
        print("Telegram reporting is disabled: credentials are not configured.", file=sys.stderr)
        return False

    url = f"https://api.telegram.org/bot{token}/sendMessage"
    text = _strip_emoji(text.replace(token, "[REDACTED]"))
    try:
        for chunk in _split_message(text):
            response = requests.post(
                url,
                data={
                    "chat_id": chat_id,
                    "text": chunk,
                    "disable_web_page_preview": "true",
                },
                timeout=timeout,
            )
            response.raise_for_status()
            payload = response.json()
            if not payload.get("ok", False):
                raise RuntimeError(payload.get("description", "Telegram API returned ok=false"))
    except (requests.RequestException, RuntimeError, ValueError) as exc:
        safe_error = str(exc).replace(token, "[REDACTED]")
        print(f"Telegram report could not be delivered: {safe_error}", file=sys.stderr)
        return False
    return True


def submission_message(manifest: dict) -> str:
    options = manifest.get("options", {})
    samples = manifest.get("samples", [])
    submit = manifest.get("submit", {})
    eras = sorted({str(item.get("era")) for item in samples if item.get("era")})
    total_jobs = sum(int(item.get("jobs", 0)) for item in samples)
    git = manifest.get("git", {})
    commit = git.get("commit") or git.get("hash") or git.get("revision") or "unknown"
    dirty = git.get("dirty")
    git_text = str(commit)[:12]
    if dirty:
        git_text += " (dirty)"

    return "\n".join(
        [
            "SKNano job submitted",
            "",
            f"Analyzer: {options.get('Analyzer') or 'unknown'}",
            f"Era: {', '.join(eras) or 'unknown'}",
            f"Samples: {len(samples)}",
            f"Analyzer jobs: {total_jobs}",
            f"Cluster: {submit.get('cluster_id', 'unknown')}",
            f"Git: {git_text}",
            f"Master directory: {manifest.get('master_dir', 'unknown')}",
        ]
    )
