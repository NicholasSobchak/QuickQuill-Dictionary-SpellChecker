  ____        _      _      ____        _ _ _
 / __ \      (_)    | |    / __ \      (_) | |
| |  | |_   _ _  ___| | __| |  | |_   _ _| | |
| |  | | | | | |/ __| |/ /| |  | | | | | | | |
| |__| | |_| | | (__|   < | |__| | |_| | | | |
 \___\_\\__,_|_|\___|_|\_\ \___\_\\__,_|_|_|_|

# QuickQuill Dictionary & Spell Checker

QuickQuill is a C++ dictionary + spell-check backend with a lightweight web UI.
It supports fast word lookup, spell correction, and rich dictionary data (definitions, examples, synonyms, antonyms, forms, etymology).

## 📦 Database Download (Run Locally)

If you want to run this with the full prebuilt database, download:

`https://www.dropbox.com/scl/fi/ydtk44m0pi5e745qi6uii/dictionary-db.zip?rlkey=gpaelm6fxwlyqqhtxmlmij48s&st=w0ldr97i&dl=0`

Then place `dictionary.db` in the project root.

## 🚀 What This Project Includes

- Trie-based lookup and autocomplete behavior
- SQLite-backed dictionary storage
- HTTP API via Crow (`dict_crow`)
- Local console test mode (`dict`)
- JSONL import pipeline for Kaikki/Wiktionary-style data

## ✨ Core Features

- `lookup <word>` returns structured word data
- `correct <word>` returns spelling corrections
- Multi-sense entries with POS and definitions
- Synonyms and antonyms per sense (when present in source data)
- Forms/inflections and etymology

## 🏛️ Project Layout

- `src/app/main.cpp`: local console test entrypoint
- `src/app/main_crow.cpp`: web server entrypoint
- `src/http/*`: routes, handlers, DTOs
- `src/core/*`: dictionary, trie, spell checker
- `src/data/*`: SQLite persistence layer
- `web/index.html`: frontend
- `scripts/import_kaikki.py`: database import script

## 🛠️ Build

```bash
make dict
make dict_crow
```

## ▶️ Run

### 1) Console test mode

```bash
./dict
```

Commands:

- `lookup <word>`
- `correct <word>`
- `exit`

### 2) Web server

```bash
./dict_crow
```

Open:

- `http://localhost:8080/`

## 🔌 API

### Health

```http
GET /api/health
```

### Word lookup

```http
GET /api/word/<word>
```

Response shape:

```json
{
  "id": 123,
  "lemma": "word",
  "forms": [{ "form": "words", "tag": "plural" }],
  "senses": [
    {
      "pos": "noun",
      "definition": "...",
      "examples": ["..."],
      "synonyms": ["..."],
      "antonyms": ["..."]
    }
  ],
  "etymology": ["..."]
}
```

## 🔄 Data Import / Refresh

Use the import script to build or refresh `dictionary.db` from Kaikki JSONL:

```bash
python3 scripts/import_kaikki.py --json nlohmann/kaikki.org-dictionary-English-words.jsonl --db dictionary.db
```

If you recently changed parsing behavior (for example synonym/antonym extraction), re-import so the DB reflects the fix.

## ⚠️ Notes on Antonyms Coverage

Not every lemma has antonyms in source data. Missing antonyms for many words is expected. The UI/API returns empty arrays when unavailable.

## 🧰 Tech Stack

- C++17
- SQLite3
- Crow (HTTP)
- nlohmann/json

## ⚠️ Academia Use & Data Attribution

Dictionary source data is derived from Wiktionary content processed by Wiktextract/Kaikki.

If this project or its data is referenced in academic work, please cite:

Tatu Ylonen. *Wiktextract: Wiktionary as Machine-Readable Structured Data.*
Proceedings of LREC 2022.

Wiktextract / Kaikki project site:

`https://kaikki.org/`
