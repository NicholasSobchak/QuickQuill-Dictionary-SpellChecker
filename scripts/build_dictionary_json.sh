#!/usr/bin/env bash
set -euo pipefail

WORD_LIST_PATH="nlohmann/words_alpha.txt"
OUTPUT_PATH="nlohmann/dictionary_words.json"

if [[ ! -f "$WORD_LIST_PATH" ]]; then
  echo "Word list not found at $WORD_LIST_PATH" >&2
  exit 1
fi

python3 - "$WORD_LIST_PATH" "$OUTPUT_PATH" <<'PY'
import sys
from pathlib import Path
import json
import re

if len(sys.argv) != 3:
    print("Usage: script WORD_LIST OUTPUT", file=sys.stderr)
    sys.exit(1)

src = Path(sys.argv[1])
dst = Path(sys.argv[2])

alpha_re = re.compile(r'[A-Za-z]+')

with src.open('r', encoding='utf-8') as fin, dst.open('w', encoding='utf-8') as fout:
    for line in fin:
        word = ''.join(alpha_re.findall(line))
        if not word:
            continue
        entry = {
            "word": word,
            "etymology_text": "",
            "forms": [],
            "senses": [
                {
                    "pos": "",
                    "glosses": [""],
                    "examples": [],
                    "synonyms": [],
                    "antonyms": []
                }
            ]
        }
        json.dump(entry, fout)
        fout.write('\n')

print(f"Dictionary JSON written to {dst}")
PY
