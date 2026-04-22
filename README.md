<p align="center"><img src="QuickQuill-logo.png" alt="QuickQuill Logo" width=600 style="background: transparent;" /></p>

<h4 align="center">A Quick Lookup Dictionary at your service.</h4>
<p align="center">
<a href="https://github.com/NicholasSobchak/QuickQuill-Dictionary-SpellChecker/actions"><img src="https://github.com/NicholasSobchak/QuickQuill-Dictionary-SpellChecker/actions/workflows/ci.yml/badge.svg" alt="Build and test"></a>
<!-- Placeholder -->
<a href="https://github.com/NicholasSobchak/QuickQuill-Dictionary-SpellChecker/releases"><img src="https://img.shields.io/badge/version-0.0.0-black" alt="Version"></a>
</p>

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

### Analytics
```
Import Complete:
  | Entries     : 1,441,164
  | Words       : 1,249,942
  | Senses      : 1,721,645
  | Forms       : 951,202
  | Examples    : 718,930
  | Synonyms    : 550,274
  | Antonyms    : 27,485
  | Etymologies : 550,369
```

### Technical Highlights
  - Trie-based autocomplete supporting 1.2M+ words
  - Sub-0.18ms cached search queries for words with extensive data
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
  - [SQLite3](https://sqlite.org/cintro.html) 
  - [Crow (HTTP)](https://crowcpp.org/master/)
  - [Catch2](https://github.com/catchorg/Catch2)
  - nlohmann/json
  - python3
  - Node.js
  - Vite
> Check out dependencies in vcpkg.json

### Project Layout

- `src/app/main.cpp`: local console test entrypoint
- `src/app/main_crow.cpp`: web server entrypoint
- `src/http/*`: routes, handlers, DTOs
- `src/core/*`: dictionary, trie, spell checker
- `src/data/*`: SQLite persistence layer
- `tests/*`: contains all C++ unit and integration tests.
- `web/*`: directory for frontend development.
- `web/index.html`: main entry point for the frontend web application
- `src/app/main_import.cpp`: JSONL-to-SQLite import tool (`dict_import`)

### Configuration

QuickQuill can be configured via a `config.json` file in the project root.
If this file is not present, the application will use default values.

Example `config.json`:
```json
{
  "database_path": "dictionary.db",
  "server_port": 8080
}
```

- `database_path`: The path to the SQLite database file.
- `server_port`: The port for the web server to listen on.

### Code Formatting (Pre-commit Hook)
To ensure consistent code formatting across the project, a `pre-commit` hook is configured. This hook automatically runs `clang-format` on your staged C++ files before each commit.

CI uses `clang-format-17` by default. To match CI locally on Ubuntu/Debian:

**Setup Instructions:**

1.  **Install `pre-commit`:** If you don't have it already, install `pre-commit`:
    ```bash
    sudo apt install pre-commit
    ```
2.  **Install Git Hooks:** From the project root directory, install the Git hooks:
    ```bash
    pre-commit install
    ```

Once installed, `clang-format` will automatically run on your staged C++ files every time you run `git commit`. If `clang-format` makes any changes, the commit will be aborted. You will then need to `git add` the newly formatted files and run `git commit` again. This helps maintain a clean and consistently formatted codebase.

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
> it usually means you forgot `-DCMAKE_TOOLCHAIN_FILE=...` (this is a problem I was having too).

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
- `suggest <word>`
- `exit`

#### 2) Web server

```bash
./build/src/dict_crow
```

Open:
- Frontend (Vite dev server): `cd web && npm install && npm run dev` then open http://localhost:5173
- Backend API (Crow): http://localhost:8080 (default) — keep this running so the frontend can load data


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
## Autocomplete Query Functionality
<p align="center"><img width="350" height="350" alt="QueryPyramid" src="https://github.com/user-attachments/assets/8da371ab-d159-4994-b001-dbbf5647adc2" />


#
## Academia Use & Data Attribution

_This project is developed for academic and educational purposes. QuickQuill is an independent project and has no affiliation with any organizations._ _All marks remain the property of their respective owners._

_The dictionary data used to build this system is derived from Wiktionary content processed through Wiktextract._

_If this project or its data is referenced in academic work, please cite:_
```
Tatu Ylonen. Wiktextract: Wiktionary as Machine-Readable Structured Data.
Proceedings of the 13th Conference on Language Resources and Evaluation (LREC),
pp. 1317–1325, Marseille, 20–25 June 2022.
```

_Linking to the Wiktextract project website is also appreciated:_
```
https://kaikki.org/
```
