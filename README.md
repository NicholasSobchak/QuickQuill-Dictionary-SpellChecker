<p align="center"><img src="QuickQuill-logo.png" alt="QuickQuill Logo" width=600 style="background: transparent;" /></p>
<h4 align="center">A Quick Lookup Dictionary at your service.</h4>

#
### Description

QuickQuill is a C++ dictionary + spell-check backend with a lightweight web UI.
It supports fast word lookup, spell correction, and rich dictionary data (definitions, examples, synonyms, antonyms, forms, etymology).

You can find the QuickQuill website here.

### Features
  - Trie-based lookup and autocomplete behavior
  - HTTP API via Crow (`dict_crow`)
  - Local console test mode (`dict`)
  - JSONL import pipeline for Kaikki/Wiktionary-style data
  - SQLite-backed dictionary storage that includes:
    - Multi-sense entries with POS and definitions
    - Synonyms and antonyms per sense (when present in source data)
    - Examples
    - Forms/inflections and etymology

### Technical Highlights
  - Trie-based autocomplete supporting 1.4M+ words
  - Sub-XXms prefix search queries
  - Memory-efficient dictionary indexing
  - REST API backend written in C++
  - Unit tested with Catch2
 
#
## Setting Up / Building this Project Locally

### Database Download
If you want to run this with the full prebuilt database, download:

```https://www.dropbox.com/home/dictionary-db-sql/dictionary-db?preview=dictionary.db```

Then place `dictionary.db` in the project root.

### This Project Uses
  - C++17
  - SQLite3 (https://sqlite.org/cintro.html)
  - Crow (HTTP) (https://crowcpp.org/master/)
  - Catch2 (https://github.com/catchorg/Catch2)
  - nlohmann/json

### Project Layout

- `src/app/main.cpp`: local console test entrypoint
- `src/app/main_crow.cpp`: web server entrypoint
- `src/http/*`: routes, handlers, DTOs
- `src/core/*`: dictionary, trie, spell checker
- `src/data/*`: SQLite persistence layer
- `web/index.html`: frontend
- `scripts/import_kaikki.py`: database import script

### Configuration

QuickQuill can be configured via a `config.json` file in the project root.
If this file is not present, the application will use default values.

Example `config.json`:
```json
{
  "database_path": "dictionary.db",
  "server_port": 8080,
}
```

- `database_path`: The path to the SQLite database file.
- `server_port`: The port for the web server to listen on.
- `max_suggestions`: The maximum number of suggestions to return for a prefix.

### Build

This project uses **CMake** + **vcpkg** (manifest mode via `vcpkg.json`) to fetch/build dependencies.

#### 1) Install vcpkg (one-time)

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
```

#### 2) Configure

From the project root:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
```

> If you see errors like `Could not find CrowConfig.cmake` or `nlohmann_jsonConfig.cmake`,
> it usually means you forgot `-DCMAKE_TOOLCHAIN_FILE=...`.

#### 3) Build

```bash
cmake --build build -j
```

### Run

After building, the executables are located under `build/src/`.

#### 1) Console test mode

```bash
./build/src/dict
```

Commands:

- `lookup <word>`
- `correct <word>`
- `exit`

#### 2) Web server

```bash
./build/src/dict_crow
```

Open:

- `http://localhost:8080/`

#
## API

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

#
## Academia Use & Data Attribution

_This project is developed for academic and educational purposes. QuickQuill is an independent project and has no affiliation with any organizations._ _All marks remain the property of their respective owners._

_The dictionary data used to build this system is derived from Wiktionary content processed through Wiktextract._

_If this project or its data is referenced in academic work, please cite:_
```
Tatu Ylonen. Wiktextract: Wiktionary as Machine-Readable Structured Data.
Proceedings of the 13th Conference on Language Resources and Evaluation (LREC),
pp. 1317–1325, Marseille, 20–25 June 2022.
Linking to the Wiktextract project website is also appreciated:
```

_Linking to the Wiktextract project website is also appreciated:_
```
https://kaikki.org/
```

