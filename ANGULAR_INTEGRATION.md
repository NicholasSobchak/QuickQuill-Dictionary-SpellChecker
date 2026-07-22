# Angular TypeScript Frontend Migration

Complete guide to migrating the QuickQuill frontend from vanilla JS/Vite to Angular 19.

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Project Setup](#project-setup)
4. [Models](#models)
5. [Services](#services)
6. [Routing](#routing)
7. [Components](#components)
8. [Global Styles](#global-styles)
9. [Build & Configuration](#build--configuration)
10. [Backend Changes](#backend-changes)
11. [Infrastructure Updates](#infrastructure-updates)
12. [Verification](#verification)
13. [ng generate Commands Reference](#ng-generate-commands-reference)

---

## Overview

Replace the vanilla JS/Vite frontend (`web/`) with an Angular 19 standalone-component SPA.

### Current Frontend (Vanilla JS)
- 3 HTML pages: `index.html`, `history.html`, `suggestions.html`
- Vanilla JS with Vite 5.2 bundler
- No framework, all DOM manipulation
- Ghost autofill, localStorage persistence, dark theme

### Target Frontend (Angular 19)
- Angular 19 with standalone components
- New control flow syntax (`@if`, `@for`)
- 3 routes: `/`, `/history`, `/suggestions`
- Component-based architecture
- `HttpClient` for API calls
- `localStorage` for persistence (same as current)

### What Stays the Same
- API endpoints (Crow C++ backend)
- Dark theme CSS (migrated to Angular global styles)
- Static assets (logo, fonts, images in `public/assets/`)
- Deployment pipeline (Docker, nginx, CI/CD)

---

## Prerequisites

- Node.js 20+
- Angular CLI 19: `npm install -g @angular/cli@19`

---

## Project Setup

### 1. Save the existing assets

```bash
# The assets directory is in git history
git checkout HEAD -- web/public/assets/
```

### 2. Remove old frontend files

```bash
rm -f web/index.html web/history.html web/suggestions.html
rm -f web/vite.config.js web/.eslintrc.cjs web/.eslintignore web/.prettierrc
rm -rf web/src/
rm -f web/package.json web/package-lock.json
```

Keep `web/public/assets/` — it stays.

### 3. Create Angular project

```bash
# From the repo root
npx @angular/cli@19 new quickquill \
  --directory web \
  --style css \
  --routing \
  --ssr false \
  --skip-git \
  --skip-tests
```

This generates:
```
web/
├── angular.json
├── package.json
├── tsconfig.json
├── tsconfig.app.json
├── tsconfig.spec.json
├── public/
│   └── assets/          ← your existing assets
├── src/
│   ├── index.html
│   ├── main.ts
│   ├── styles.css
│   └── app/
│       ├── app.component.ts
│       ├── app.component.html
│       ├── app.component.css
│       ├── app.config.ts
│       └── app.routes.ts
```

### 4. Copy assets into Angular public directory

```bash
# Assets should already be in web/public/assets/ from step 1
# Angular serves everything in public/ at the root URL
ls web/public/assets/
# QuickQuill-logo.png  Quotex.otf  school_backdrop.jpg  ...
```

### 5. Install dependencies

```bash
cd web && npm install
```

---

## Models

Create `src/app/models/word.models.ts` with all TypeScript interfaces matching the backend API responses.

### Interface Definitions

```typescript
// src/app/models/word.models.ts

export interface WordSense {
  pos: string;
  definition: string;
  examples: string[];
  synonyms: string[];
  antonyms: string[];
}

export interface WordForm {
  form: string;
  tag: string;
}

/** Successful lookup — GET /api/word/{word} → 200 */
export interface WordResponse {
  id: number;
  lemma: string;
  display_lemma?: string;
  query?: string;
  forms: WordForm[];
  senses: WordSense[];
  etymology: string[];
  alternative_searches: string[];
}

/** Not found — GET /api/word/{word} → 404 */
export interface WordNotFound {
  query: string;
  found: false;
  suggestion?: string;
}

/** Error — GET /api/word/{word} → 400 */
export interface WordError {
  error: string;
}

/** Autofill completion — GET /api/autofill/{word} → 200 */
export interface AutofillResponse {
  completion: string;
}

/** All possible responses from GET /api/word/{word} */
export type WordResult = WordResponse | WordNotFound | WordError;
```

### Why These Shapes

Each interface maps directly to the C++ JSON output:

| Interface | Backend Source | HTTP Response |
|---|---|---|
| `WordResponse` | `WordResponse.cpp:14-45` | `GET /api/word/{word}` → 200 |
| `WordNotFound` | `WordService.cpp:127-128` | `GET /api/word/{word}` → 404 |
| `WordError` | `WordService.cpp:108-109` | `GET /api/word/{word}` → 400 |
| `AutofillResponse` | `WordService.cpp:168-169` | `GET /api/autofill/{word}` → 200 |
| `string[]` (suggest) | `WordService.cpp:149` | `GET /api/suggest/{word}` → 200 |
| `string[]` (synonym) | `WordService.cpp:182` | `GET /api/synonym/{word}` → 200 |

Note: `display_lemma` and `query` are optional (`?`) because the C++ backend only includes them conditionally.

---

## Services

### API Service

Create `src/app/services/api.service.ts`:

```typescript
import { Injectable, inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';
import {
  WordResponse,
  WordNotFound,
  AutofillResponse,
} from '../models/word.models';

@Injectable({ providedIn: 'root' })
export class ApiService {
  private http = inject(HttpClient);

  lookup(word: string): Observable<WordResponse> {
    return this.http.get<WordResponse>(`/api/word/${this.encodePath(word)}`);
  }

  suggest(word: string): Observable<string[]> {
    return this.http.get<string[]>(`/api/suggest/${this.encodePath(word)}`);
  }

  synonym(word: string): Observable<string[]> {
    return this.http.get<string[]>(`/api/synonym/${this.encodePath(word)}`);
  }

  autofill(
    word: string,
    history: string[],
    suggested: string[]
  ): Observable<AutofillResponse> {
    let url = `/api/autofill/${this.encodePath(word)}`;
    const params = new URLSearchParams();
    if (history.length) params.set('history', history.join(','));
    if (suggested.length) params.set('suggested', suggested.join(','));
    const qs = params.toString();
    if (qs) url += `?${qs}`;
    return this.http.get<AutofillResponse>(url);
  }

  private encodePath(value: string): string {
    return encodeURIComponent(value).replace(/%20/g, '+');
  }
}
```

### Storage Service

Create `src/app/services/storage.service.ts`:

```typescript
import { Injectable } from '@angular/core';

const HISTORY_KEY = 'quickquill-history';
const SUGGESTED_KEY = 'quickquill-suggested-words';
const HISTORY_LIMIT = 1000;
const SUGGESTED_LIMIT = 1000;

const DISPLAY_WORD_RE = /[^A-Za-z0-9''\s-]+/g;
const MULTISPACE_RE = /\s+/g;
const SEARCH_INPUT_RE = /[^A-Za-z'\s.-]+/g;

@Injectable({ providedIn: 'root' })
export class StorageService {
  // --- History ---

  getHistory(): string[] {
    const stored = localStorage.getItem(HISTORY_KEY);
    return stored ? JSON.parse(stored) : [];
  }

  saveHistory(words: string[]): void {
    localStorage.setItem(
      HISTORY_KEY,
      JSON.stringify(words.slice(0, HISTORY_LIMIT))
    );
  }

  addToHistory(word: string): void {
    if (!word) return;
    const items = this.getHistory();
    const filtered = items.filter(
      (w) => w.toLowerCase() !== word.toLowerCase()
    );
    filtered.unshift(word);
    this.saveHistory(filtered.slice(0, HISTORY_LIMIT));
  }

  clearHistory(): void {
    localStorage.removeItem(HISTORY_KEY);
  }

  // --- Suggested Words ---

  getSuggestedWords(): string[] {
    const stored = localStorage.getItem(SUGGESTED_KEY);
    return stored ? JSON.parse(stored) : [];
  }

  saveSuggestedWords(words: string[]): void {
    localStorage.setItem(
      SUGGESTED_KEY,
      JSON.stringify(words.slice(0, SUGGESTED_LIMIT))
    );
  }

  addSuggestedWords(words: string[]): void {
    if (!Array.isArray(words) || !words.length) return;

    const merged = [
      ...words.map((w) => this.displayWord(w)).filter(Boolean),
      ...this.getSuggestedWords(),
    ];
    const unique: string[] = [];
    const seen = new Set<string>();

    merged.forEach((item) => {
      const normalized = item.toLowerCase();
      if (seen.has(normalized)) return;
      seen.add(normalized);
      unique.push(item);
    });

    this.saveSuggestedWords(unique.slice(0, SUGGESTED_LIMIT));
  }

  clearSuggestedWords(): void {
    localStorage.removeItem(SUGGESTED_KEY);
  }

  // --- Utilities ---

  displayWord(text: string): string {
    if (!text) return '';
    return text
      .replace(DISPLAY_WORD_RE, ' ')
      .replace(MULTISPACE_RE, ' ')
      .trim();
  }

  sanitizeSearchInput(text: string): string {
    if (!text) return '';
    return text.replace(SEARCH_INPUT_RE, '');
  }

  encodePathSegment(value: string): string {
    return encodeURIComponent(value).replace(/%20/g, '+');
  }
}
```

---

## Routing

### Route Configuration

```typescript
// src/app/app.routes.ts
import { Routes } from '@angular/router';
import { DictionaryComponent } from './pages/dictionary/dictionary.component';
import { HistoryComponent } from './pages/history/history.component';
import { SuggestionsComponent } from './pages/suggestions/suggestions.component';

export const routes: Routes = [
  { path: '', component: DictionaryComponent },
  { path: 'history', component: HistoryComponent },
  { path: 'suggestions', component: SuggestionsComponent },
  { path: '**', redirectTo: '' },
];
```

### App Config

```typescript
// src/app/app.config.ts
import { ApplicationConfig } from '@angular/core';
import { provideRouter, withComponentInputBinding } from '@angular/router';
import { provideHttpClient } from '@angular/common/http';
import { routes } from './app.routes';

export const appConfig: ApplicationConfig = {
  providers: [
    provideRouter(routes, withComponentInputBinding()),
    provideHttpClient(),
  ],
};
```

### URL Query Param Sync

The dictionary page reads `?word=X` and syncs back:

```typescript
// In dictionary.component.ts
import { ActivatedRoute, Router } from '@angular/router';

constructor(
  private route: ActivatedRoute,
  private router: Router,
) {}

ngOnInit() {
  // Read word from URL on load
  this.route.queryParams.subscribe((params) => {
    const word = params['word'];
    if (word) {
      this.searchInput = word;
      this.lookup();
    }
  });
}

// On search, update URL
updateUrl(word: string) {
  this.router.navigate([], {
    queryParams: { word },
    queryParamsHandling: 'merge',
  });
}
```

---

## Components

### Project Structure

```
src/app/
├── models/
│   └── word.models.ts
├── services/
│   ├── api.service.ts
│   ├── storage.service.ts
│   └── search-state.service.ts
├── pages/
│   ├── dictionary/
│   │   ├── dictionary.component.ts
│   │   ├── dictionary.component.html
│   │   └── dictionary.component.css
│   ├── history/
│   │   ├── history.component.ts
│   │   ├── history.component.html
│   │   └── history.component.css
│   └── suggestions/
│       ├── suggestions.component.ts
│       ├── suggestions.component.html
│       └── suggestions.component.css
├── shared/
│   ├── header/
│   │   ├── header.component.ts
│   │   ├── header.component.html
│   │   └── header.component.css
│   ├── footer/
│   │   ├── footer.component.ts
│   │   ├── footer.component.html
│   │   └── footer.component.css
│   ├── chip/
│   │   └── chip.component.ts
│   └── expandable-list/
│       ├── expandable-list.component.ts
│       ├── expandable-list.component.html
│       └── expandable-list.component.css
├── app.component.ts
├── app.component.html
├── app.component.css
├── app.config.ts
└── app.routes.ts
```

### AppComponent (Shell)

```typescript
// src/app/app.component.ts
import { Component, HostListener } from '@angular/core';
import { Router, RouterOutlet } from '@angular/router';
import { HeaderComponent } from './shared/header/header.component';
import { FooterComponent } from './shared/footer/footer.component';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, HeaderComponent, FooterComponent],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css',
})
export class AppComponent {
  constructor(private router: Router) {}

  @HostListener('document:keydown.escape')
  onEscape() {
    this.router.navigate(['/']);
  }
}
```

```html
<!-- src/app/app.component.html -->
<app-header />
<main class="shell">
  <router-outlet />
</main>
<app-footer />
```

### HeaderComponent

```typescript
// src/app/shared/header/header.component.ts
import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';

@Component({
  selector: 'app-header',
  standalone: true,
  imports: [RouterLink, RouterLinkActive],
  templateUrl: './header.component.html',
  styleUrl: './header.component.css',
})
export class HeaderComponent {}
```

```html
<!-- src/app/shared/header/header.component.html -->
<header class="site-header">
  <a routerLink="/" routerLinkActive="active" [routerLinkActiveOptions]="{exact: true}"
     class="nav-button">Dictionary</a>
  <a routerLink="/suggestions" routerLinkActive="active"
     class="nav-button">Suggested Words</a>
  <a routerLink="/history" routerLinkActive="active"
     class="nav-button">History</a>
</header>
```

### FooterComponent

```typescript
// src/app/shared/footer/footer.component.ts
import { Component } from '@angular/core';

@Component({
  selector: 'app-footer',
  standalone: true,
  templateUrl: './footer.component.html',
  styleUrl: './footer.component.css',
})
export class FooterComponent {}
```

```html
<!-- src/app/shared/footer/footer.component.html -->
<footer class="site-footer">
  <div class="footer-content">
    <a href="https://github.com/NicholasSobchak/QuickQuill-Dictionary-SpellChecker"
       target="_blank" rel="noopener" class="github-link" aria-label="GitHub Repository">
      <svg class="github-icon" viewBox="0 0 24 24" fill="currentColor">
        <path d="M12 1.27a11 11 0 00-3.48 21.46c.55.09.73-.24.73-.53v-1.84c-3.03.66-3.67-1.46-3.67-1.46-.5-1.26-1.22-1.6-1.22-1.6-.98-.67.08-.66.08-.66 1.08.08 1.65 1.11 1.65 1.11.97 1.65 2.55 1.17 3.17.9.1-.7.38-1.17.69-1.44-2.42-.27-4.96-1.21-4.96-5.38 0-1.19.43-2.17 1.13-2.93-.11-.28-.49-1.39.11-2.89 0 0 .91-.29 3 1.12a10.33 10.33 0 015.51 0c2.09-1.41 3-1.12 3-1.12.6 1.5.22 2.61.11 2.89.71.76 1.13 1.74 1.13 2.93 0 4.18-2.55 5.1-4.98 5.37.39.34.73 1 .73 2.02v3.01c0 .3.18.63.74.53A11 11 0 0012 1.27"/>
      </svg>
    </a>
    <p class="footer-disclaimer">
      QuickQuill is an independent project and has no affiliation with any organizations.
      All marks remain the property of their respective owners.
    </p>
  </div>
</footer>
```

### ChipComponent (Shared)

```typescript
// src/app/shared/chip/chip.component.ts
import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-chip',
  standalone: true,
  template: `
    <a class="chip" href="#" (click)="handleClick($event)">
      {{ label }}
    </a>
  `,
})
export class ChipComponent {
  @Input() label = '';
  @Input() clickable = true;
  @Output() clicked = new EventEmitter<string>();

  handleClick(event: Event) {
    event.preventDefault();
    if (this.clickable) {
      this.clicked.emit(this.label);
    }
  }
}
```

### ExpandableListComponent (Shared)

```typescript
// src/app/shared/expandable-list/expandable-list.component.ts
import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-expandable-list',
  standalone: true,
  templateUrl: './expandable-list.component.html',
  styleUrl: './expandable-list.component.css',
})
export class ExpandableListComponent {
  @Input() items: string[] = [];
  @Input() maxVisible = 5;
  expanded = false;

  get visibleItems(): string[] {
    return this.expanded ? this.items : this.items.slice(0, this.maxVisible);
  }

  get hiddenCount(): number {
    return Math.max(0, this.items.length - this.maxVisible);
  }

  toggle() {
    this.expanded = !this.expanded;
  }
}
```

```html
<!-- src/app/shared/expandable-list/expandable-list.component.html -->
<ol class="list" [class.show-all]="expanded">
  @for (item of items; track $index) {
    <li [class.extra]="$index >= maxVisible">{{ item }}</li>
  }
</ol>
@if (hiddenCount > 0) {
  <div style="display: flex; justify-content: center;">
    <button type="button" class="toggle-btn" (click)="toggle()">
      {{ expanded ? 'see less' : 'see more' }}
      <span class="arrow">{{ expanded ? '^' : 'v' }}</span>
    </button>
  </div>
}
```

### DictionaryComponent (Main Page)

```typescript
// src/app/pages/dictionary/dictionary.component.ts
import { Component, OnInit, OnDestroy, inject } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { Subject, Subscription, debounceTime, switchMap, of } from 'rxjs';
import { ApiService } from '../../services/api.service';
import { StorageService } from '../../services/storage.service';
import { WordResponse, WordNotFound, WordError, WordResult } from '../../models/word.models';
import { ChipComponent } from '../../shared/chip/chip.component';
import { ExpandableListComponent } from '../../shared/expandable-list/expandable-list.component';

@Component({
  selector: 'app-dictionary',
  standalone: true,
  imports: [ChipComponent, ExpandableListComponent],
  templateUrl: './dictionary.component.html',
  styleUrl: './dictionary.component.css',
})
export class DictionaryComponent implements OnInit, OnDestroy {
  private api = inject(ApiService);
  private storage = inject(StorageService);
  private route = inject(ActivatedRoute);
  private router = inject(Router);

  searchInput = '';
  result: WordResponse | null = null;
  notFound: WordNotFound | null = null;
  error: WordError | null = null;
  isLoading = false;
  statusMessage = '';
  liveSuggestedWords: string[] = [];
  ghostCompletion = '';
  ghostTyped = '';
  ghostSuffix = '';
  showGhostHint = false;

  private suggest$ = new Subject<string>();
  private suggestSub?: Subscription;
  private ghostSub?: Subscription;
  private spinnerFrames = ['|', '/', '-', ''];
  private spinnerIdx = 0;
  private spinnerTimer: ReturnType<typeof setInterval> | null = null;

  ngOnInit() {
    // Debounced suggest
    this.suggestSub = this.suggest$
      .pipe(debounceTime(300))
      .subscribe((word) => this.fetchSuggestions(word));

    // Read URL params
    this.route.queryParams.subscribe((params) => {
      const word = params['word'];
      if (word) {
        this.searchInput = word;
        this.lookup();
      }
    });
  }

  ngOnDestroy() {
    this.suggestSub?.unsubscribe();
    this.ghostSub?.unsubscribe();
    this.stopSpinner();
  }

  onInputChange() {
    const word = this.searchInput.trim();
    this.suggest$.next(word);
    this.triggerGhostAutofill();
  }

  onKeyDown(event: KeyboardEvent) {
    if (event.key === 'Enter') this.lookup();
    if (event.key === 'Tab' && this.ghostCompletion) {
      event.preventDefault();
      this.acceptGhost();
    }
  }

  lookup() {
    const word = this.searchInput.trim();
    if (!word) return;

    // Update URL
    this.router.navigate([], {
      queryParams: { word },
      queryParamsHandling: 'merge',
    });

    this.isLoading = true;
    this.startSpinner();
    this.clearResult();

    this.api.lookup(word).subscribe({
      next: (data) => {
        if ('found' in data && data.found === false) {
          this.notFound = data as WordNotFound;
        } else if ('error' in data) {
          this.error = data as WordError;
        } else {
          this.result = data as WordResponse;
          // Canonicalize input
          const canonical = this.storage.displayWord(
            data.display_lemma || data.query || word
          );
          if (canonical) {
            this.searchInput = canonical;
            this.storage.addToHistory(canonical);
          }
        }
        this.stopSpinner();
        this.isLoading = false;
      },
      error: (err) => {
        this.statusMessage = `Network error: ${err.message || 'failed to fetch'}`;
        this.stopSpinner();
        this.isLoading = false;
      },
    });
  }

  searchWord(word: string) {
    this.searchInput = word;
    this.lookup();
  }

  acceptGhost() {
    if (this.ghostCompletion) {
      this.searchInput = this.ghostCompletion;
      this.clearGhostText();
    }
  }

  private triggerGhostAutofill() {
    const typed = this.searchInput;
    const lastSpace = typed.lastIndexOf(' ');
    const base = lastSpace >= 0 ? typed.slice(0, lastSpace + 1) : '';
    const segment = lastSpace >= 0 ? typed.slice(lastSpace + 1) : typed;
    const word = segment.trim();

    if (word.length < 1) {
      this.clearGhostText();
      return;
    }

    const history = this.storage.getHistory().slice(0, 20);
    const suggested = this.storage.getSuggestedWords().slice(0, 20);

    this.api.autofill(word, history, suggested).subscribe({
      next: (data) => {
        const completion = data.completion || '';
        if (completion && completion.toLowerCase().indexOf(typed.toLowerCase()) === 0) {
          this.ghostCompletion = base + completion;
          this.ghostTyped = typed;
          this.ghostSuffix = completion.substring(typed.length);
          this.showGhostHint = true;
        } else {
          this.clearGhostText();
        }
      },
      error: () => this.clearGhostText(),
    });
  }

  private clearGhostText() {
    this.ghostCompletion = '';
    this.ghostTyped = '';
    this.ghostSuffix = '';
    this.showGhostHint = false;
  }

  private fetchSuggestions(word: string) {
    if (word.length < 1) {
      this.liveSuggestedWords = [];
      return;
    }
    this.api.suggest(word).subscribe({
      next: (suggestions) => {
        this.liveSuggestedWords = (Array.isArray(suggestions) ? suggestions : [])
          .map((w) => this.storage.displayWord(w))
          .filter(Boolean);
      },
      error: () => {
        this.liveSuggestedWords = [];
      },
    });
  }

  private clearResult() {
    this.result = null;
    this.notFound = null;
    this.error = null;
    this.statusMessage = '';
  }

  private startSpinner() {
    this.stopSpinner();
    this.spinnerIdx = 0;
    this.spinnerTimer = setInterval(() => {
      this.spinnerIdx = (this.spinnerIdx + 1) % this.spinnerFrames.length;
    }, 120);
  }

  private stopSpinner() {
    if (this.spinnerTimer) {
      clearInterval(this.spinnerTimer);
      this.spinnerTimer = null;
    }
  }

  get spinnerChar(): string {
    return this.spinnerFrames[this.spinnerIdx];
  }

  // Helpers for template
  getSynonyms(): string[] {
    return this.result?.senses?.flatMap((s) => s.synonyms) ?? [];
  }

  getAntonyms(): string[] {
    return this.result?.senses?.flatMap((s) => s.antonyms) ?? [];
  }

  getExamples(): string[] {
    return this.result?.senses?.flatMap((s) => s.examples) ?? [];
  }

  getDefinitions(): string[] {
    return this.result?.senses?.map(
      (s) => `${s.pos ? `[${s.pos}] ` : ''}${s.definition}`
    ) ?? [];
  }

  formatLemma(): string {
    const raw = this.result?.display_lemma || this.result?.query || this.result?.lemma || '';
    let decoded = raw;
    try { decoded = decodeURIComponent(raw); } catch {}
    const heading = this.storage.displayWord(decoded);
    const capitalized = heading.charAt(0).toUpperCase() + heading.slice(1);
    return capitalized.toLowerCase() === 'lexicon levissimum'
      ? `${capitalized}!`
      : capitalized;
  }
}
```

```html
<!-- src/app/pages/dictionary/dictionary.component.html -->
<div class="logo-container">
  <div class="brand">
    <img class="brand-logo" src="/assets/QuickQuill-logo.png" alt="QuickQuill" />
  </div>
  <img src="/assets/school_line.png" alt="" class="header-line" />
</div>

<section class="card">
  <div class="row">
    <button class="prompt" type="button" aria-label="Look up word" (click)="lookup()">
      <img class="prompt-icon" src="/assets/magnifying-glass-svgrepo-com.svg" alt="" />
    </button>
    <div class="autofill-wrapper">
      <input
        [(ngModel)]="searchInput"
        placeholder="lookup &lt;word&gt;"
        autocomplete="off"
        (input)="onInputChange()"
        (keydown)="onKeyDown($event)"
      />
      <div class="ghost-layer" aria-hidden="true">
        <span>{{ ghostTyped }}</span><span>{{ ghostSuffix }}</span>
      </div>
    </div>
  </div>

  <div class="status">
    @if (isLoading) {
      <span class="spinner">{{ spinnerChar }}</span>
    }
    <span>{{ statusMessage }}</span>
    @if (showGhostHint) {
      <span class="ghost-hint" (click)="acceptGhost()">
        <span class="hint-tab">TAB</span> to complete
      </span>
    }
  </div>

  <!-- Not found with suggestion -->
  @if (notFound) {
    <div class="status" style="color: #ff6666">
      @if (notFound.suggestion) {
        <span>Did you mean
          <a href="#" class="suggestion-link" (click)="searchWord(notFound.suggestion!); $event.preventDefault()">
            <em class="corrected-word">{{ notFound.suggestion }}</em>
          </a>?
        </span>
      } @else {
        <span>Word '{{ searchInput }}' not found.</span>
      }
    </div>
  }

  <!-- Error -->
  @if (error) {
    <div class="status" style="color: #ff6666">
      @if (error.error === 'Enter a valid word') {
        400: Enter a valid word
      } @else {
        Error: {{ error.error }}
      }
    </div>
  }

  <!-- Result box -->
  @if (result) {
    <div class="result-box has-results">
      <h2 class="lemma">{{ formatLemma() }}</h2>

      <!-- Alternative searches -->
      @if (result.alternative_searches?.length) {
        <div class="alternative-searches">
          <span class="alternative-searches-label">Alternative Searches:</span>
          @for (alt of result.alternative_searches; track alt; let i = $index) {
            @if (i > 0) {
              <span class="alternative-searches-separator">,</span>
            }
            <a href="#" class="alternative-search-link" (click)="searchWord(alt); $event.preventDefault()">{{ alt }}</a>
          }
        </div>
      }

      <!-- Definitions -->
      <div class="section">
        <div class="label">Definition</div>
        @if (getDefinitions().length) {
          <app-expandable-list [items]="getDefinitions()" />
        } @else {
          <div class="empty">No definitions available.</div>
        }
      </div>

      <!-- Synonyms -->
      <div class="section">
        <div class="label">Synonyms</div>
        @if (getSynonyms().length) {
          <div>
            @for (syn of getSynonyms(); track syn) {
              <app-chip [label]="syn" (clicked)="searchWord($event)" />
            }
          </div>
        } @else {
          <div class="empty">No synonyms available.</div>
        }
      </div>

      <!-- Antonyms -->
      <div class="section">
        <div class="label">Antonyms</div>
        @if (getAntonyms().length) {
          <div>
            @for (ant of getAntonyms(); track ant) {
              <app-chip [label]="ant" (clicked)="searchWord($event)" />
            }
          </div>
        } @else {
          <div class="empty">No antonyms available.</div>
        }
      </div>

      <!-- Examples -->
      <div class="section">
        <div class="label">Examples</div>
        @if (getExamples().length) {
          <app-expandable-list [items]="getExamples()" />
        } @else {
          <div class="empty">No examples available.</div>
        }
      </div>

      <!-- Forms -->
      <div class="section">
        <div class="label">Forms</div>
        @if (result.forms?.length) {
          <div>
            @for (form of result.forms; track form.form) {
              <app-chip
                [label]="form.tag ? form.form + ' (' + form.tag + ')' : form.form"
                (clicked)="searchWord(form.form)"
              />
            }
          </div>
        } @else {
          <div class="empty">No forms available.</div>
        }
      </div>

      <!-- Etymology -->
      <div class="section">
        <div class="label">Etymology</div>
        @if (result.etymology?.length) {
          <app-expandable-list [items]="result.etymology" />
        } @else {
          <div class="empty">No etymology available.</div>
        }
      </div>
    </div>
  }

  <!-- Similar searches sidebar -->
  @if (liveSuggestedWords.length) {
    <div class="result-box suggested-box has-results">
      <div class="suggested-content">
        <p class="suggested-title">Similar Searches</p>
        <div class="suggested-list">
          @for (word of liveSuggestedWords.slice(0, 30); track word) {
            <div class="suggested-column">
              <div class="suggested-row">
                <img class="suggested-bullet" src="/assets/school_bullet_nobg.png" alt="" />
                <button type="button" class="chip suggested-chip" (click)="searchWord(word)">
                  {{ word }}
                </button>
              </div>
            </div>
          }
        </div>
      </div>
    </div>
  }
</section>
```

### HistoryComponent

```typescript
// src/app/pages/history/history.component.ts
import { Component, inject } from '@angular/core';
import { Router } from '@angular/router';
import { StorageService } from '../../services/storage.service';

@Component({
  selector: 'app-history',
  standalone: true,
  templateUrl: './history.component.html',
  styleUrl: './history.component.css',
})
export class HistoryComponent {
  private storage = inject(StorageService);
  private router = inject(Router);

  filterText = '';

  get filteredWords(): string[] {
    const words = this.storage.getHistory();
    if (!this.filterText.trim()) return words;
    const query = this.filterText.trim().toLowerCase();
    return words.filter((w) => w.toLowerCase().includes(query));
  }

  searchWord(word: string) {
    this.router.navigate(['/'], { queryParams: { word } });
  }

  clearAll() {
    this.storage.clearHistory();
  }
}
```

```html
<!-- src/app/pages/history/history.component.html -->
<div class="shell">
  <section class="card history-page">
    <div class="history-page-header">
      <h1 class="history-page-title">Search History</h1>
    </div>
    <div class="line-container">
      <img src="/assets/school_line.png" alt="" class="page-title-line" />
      <div class="button-row">
        <button class="back-button" routerLink="/">Back to Dictionary</button>
        <button class="clear-history-button" (click)="clearAll()">Clear All</button>
      </div>
    </div>
    <div class="row">
      <input [(ngModel)]="filterText" placeholder="filter history..." autocomplete="off" />
    </div>
    <div class="history-words-grid">
      @for (word of filteredWords; track word) {
        <button type="button" class="chip history-chip" (click)="searchWord(word)">
          {{ word }}
        </button>
      }
    </div>
    @if (!filteredWords.length) {
      <div class="history-empty">No search history yet.</div>
    }
  </section>
</div>
```

### SuggestionsComponent

```typescript
// src/app/pages/suggestions/suggestions.component.ts
import { Component, inject } from '@angular/core';
import { Router } from '@angular/router';
import { StorageService } from '../../services/storage.service';

@Component({
  selector: 'app-suggestions',
  standalone: true,
  templateUrl: './suggestions.component.html',
  styleUrl: './suggestions.component.css',
})
export class SuggestionsComponent {
  private storage = inject(StorageService);
  private router = inject(Router);

  filterText = '';

  get filteredWords(): string[] {
    const words = this.storage.getSuggestedWords();
    if (!this.filterText.trim()) return words;
    const query = this.filterText.trim().toLowerCase();
    return words.filter((w) => w.toLowerCase().includes(query));
  }

  searchWord(word: string) {
    this.router.navigate(['/'], { queryParams: { word } });
  }

  clearAll() {
    this.storage.clearSuggestedWords();
  }
}
```

```html
<!-- src/app/pages/suggestions/suggestions.component.html -->
<div class="shell">
  <section class="card suggested-page">
    <div class="suggested-page-header">
      <h1 class="suggested-page-title">Suggested Words</h1>
    </div>
    <div class="line-container">
      <img src="/assets/school_line.png" alt="" class="page-title-line" />
      <div class="button-row">
        <button class="back-button" routerLink="/">Back to Dictionary</button>
        <button class="clear-history-button" (click)="clearAll()">Clear All</button>
      </div>
    </div>
    <div class="row">
      <input [(ngModel)]="filterText" placeholder="filter words..." autocomplete="off" />
    </div>
    <div class="suggested-words-grid">
      @for (word of filteredWords; track word) {
        <button type="button" class="chip suggested-chip" (click)="searchWord(word)">
          {{ word }}
        </button>
      }
    </div>
    @if (!filteredWords.length) {
      <div class="history-empty">No suggested words yet.</div>
    }
  </section>
</div>
```

---

## Global Styles

Copy the existing `style.css` content into `src/styles.css`. The CSS is identical to the current vanilla frontend.

```css
/* src/styles.css */

:root {
  --bg-1: #000000;
  --bg-2: #000000;
  --ink: #ffffff;
  --muted: #b6b6b6;
  --accent: #ffffff;
  --card: #0a0a0a;
  --edge: #2a2a2a;
}

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: 'Times New Roman', Times, serif;
  font-size: 17px;
  letter-spacing: 0.5px;
  color: var(--ink);
  background-color: #000000;
  background-image:
    radial-gradient(
      ellipse at center,
      rgba(0, 0, 0, 0) 8%,
      rgba(0, 0, 0, 0.9) 30%,
      rgba(0, 0, 0, 0.97) 55%,
      rgba(0, 0, 0, 1) 72%,
      rgba(0, 0, 0, 1) 100%
    ),
    linear-gradient(
      to right,
      rgba(0, 0, 0, 1) 0%,
      rgba(0, 0, 0, 0) 12%,
      rgba(0, 0, 0, 0) 88%,
      rgba(0, 0, 0, 1) 100%
    ),
    linear-gradient(
      to bottom,
      rgba(0, 0, 0, 1) 0%,
      rgba(0, 0, 0, 0) 14%,
      rgba(0, 0, 0, 0) 86%,
      rgba(0, 0, 0, 1) 100%
    ),
    linear-gradient(rgba(0, 0, 0, 0.99), rgba(0, 0, 0, 0.99)),
    url('/assets/school_backdrop.jpg');
  background-size:
    260% 260%, 100% 100%, 100% 100%, 100% 100%, contain;
  background-repeat: no-repeat;
  background-position: center center;
  background-attachment: fixed;
  background-blend-mode: multiply, multiply, multiply, multiply, normal;
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

@media (max-width: 768px) {
  body {
    background-attachment: scroll, scroll, scroll, scroll, scroll;
  }
}

/* ... include ALL remaining CSS from the current style.css ... */
/* See git: git show HEAD:web/src/style.css */
```

The full CSS file is large (~500 lines). Copy it verbatim from the git-tracked version. Key sections to include:

- Logo container, shell, header
- Card and row layouts
- Ghost autofill layer
- Result box and sections
- Chip, list, and toggle styles
- Suggested and history page layouts
- Footer styles
- All `.hidden`, `.empty`, `.error` utility classes
- Spinner animation

---

## Build & Configuration

### angular.json (key settings)

The Angular CLI generates this, but verify these values:

```json
{
  "projects": {
    "quickquill": {
      "architect": {
        "build": {
          "options": {
            "outputPath": "dist",
            "index": "src/index.html",
            "assets": ["public"],
            "styles": ["src/styles.css"],
            "scripts": []
          },
          "configurations": {
            "production": {
              "budgets": [
                { "type": "initial", "maximumWarning": "500kB", "maximumError": "1MB" }
              ],
              "outputHashing": "all"
            },
            "development": {
              "optimization": false,
              "extractLicenses": false,
              "sourceMap": true
            }
          }
        },
        "serve": {
          "options": {
            "proxyConfig": "proxy.conf.json"
          }
        }
      }
    }
  }
}
```

### proxy.conf.json (dev server)

Create `web/proxy.conf.json`:

```json
{
  "/api": {
    "target": "http://localhost:8080",
    "secure": false,
    "changeOrigin": true
  }
}
```

### package.json scripts

```json
{
  "scripts": {
    "ng": "ng",
    "start": "ng serve --proxy-config proxy.conf.json",
    "build": "ng build --configuration production",
    "build:dev": "ng build --configuration development",
    "lint": "ng lint",
    "test": "ng test"
  }
}
```

### Environment Files

```typescript
// src/environments/environment.ts
export const environment = {
  production: false,
  apiBase: '',  // proxied via proxy.conf.json in dev
};
```

```typescript
// src/environments/environment.prod.ts
export const environment = {
  production: true,
  apiBase: '',  // nginx proxies /api to backend:8080
};
```

### index.html

```html
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Quick Quill</title>
  <link rel="icon" href="assets/quill.png" />
  <!-- Fonts CSS from public/assets/fonts.css is loaded via styles.css -->
</head>
<body>
  <app-root></app-root>
</body>
</html>
```

---

## Backend Changes

### C++ SPA Fallback Routes

Add catch-all routes for client-side routing in `src/http/routes/WordRoutes.cpp`:

```cpp
// After the existing "/" route, add:
CROW_ROUTE(app, "/history")
([] { return htmlResponseFromFile(kDistRoot + "/index.html"); });

CROW_ROUTE(app, "/suggestions")
([] { return htmlResponseFromFile(kDistRoot + "/index.html"); });
```

This ensures navigating directly to `/history` or `/suggestions` serves the Angular SPA, which then handles the route client-side.

### Dockerfile

No changes needed. The existing Dockerfile works as-is:

```dockerfile
FROM node:20-slim AS frontend
WORKDIR /web
COPY web/package*.json ./
RUN npm install
COPY web/ ./
RUN npm run build

# ... rest unchanged, copies web/dist ...
```

### CI/CD

No changes needed. The existing CI workflow works as-is:

```yaml
- name: Install Frontend Dependencies
  run: npm ci
  working-directory: web/

- name: Build Frontend
  run: npm run build
  working-directory: web/
```

---

## Infrastructure Updates

### .gitignore additions

Add these to the root `.gitignore`:

```
# Angular
web/dist/
web/.angular/
web/node_modules/
```

---

## Verification

### Dev Server

```bash
cd web
npm start
# Opens http://localhost:4200
# Proxy forwards /api to http://localhost:8080
```

### Production Build

```bash
cd web
npm run build
ls dist/browser/  # Angular outputs here
# Contains: index.html, main.*.js, polyfills.*.js, styles.*.css
```

### Docker Build

```bash
docker compose build
docker compose up
# Visit http://localhost:8080
```

### Test Checklist

- [ ] Search a word -> definitions, synonyms, examples render
- [ ] Ghost autofill appears as you type
- [ ] TAB accepts ghost completion
- [ ] Click synonym/antonym chip -> triggers new search
- [ ] Click form chip -> triggers new search
- [ ] Alternative searches appear as clickable links
- [ ] "Did you mean?" appears for misspelled words
- [ ] History page displays localStorage history
- [ ] History page filter works
- [ ] Clear All removes history
- [ ] Suggestions page displays localStorage suggested words
- [ ] Suggestions page filter works
- [ ] Clear All removes suggestions
- [ ] Click chip on history/suggestions -> navigates to dictionary
- [ ] Browser back/forward works
- [ ] Escape key returns to dictionary
- [ ] URL updates with search query (`/?word=serendipity`)
- [ ] Direct URL load (`/?word=serendipity`) works
- [ ] Dark theme renders correctly
- [ ] Mobile responsive layout works
- [ ] Logo and decorative images display
- [ ] Footer GitHub link works

---

## ng generate Commands Reference

### Scaffold all components and services

```bash
cd web

# Pages
ng g c pages/dictionary --standalone
ng g c pages/history --standalone
ng g c pages/suggestions --standalone

# Shared components
ng g c shared/header --standalone
ng g c shared/footer --standalone
ng g c shared/chip --standalone
ng g c shared/expandable-list --standalone

# Services
ng g s services/api
ng g s services/storage
ng g s services/search-state

# Models (no ng generate command - create manually)
mkdir -p src/app/models
touch src/app/models/word.models.ts
```

Note: In Angular 19, `--standalone` is the default. All generated components
are standalone unless you explicitly opt out.

### Individual component generation (if doing incrementally)

```bash
# Generate one at a time
ng g c pages/dictionary
# Creates:
#   src/app/pages/dictionary/dictionary.component.ts
#   src/app/pages/dictionary/dictionary.component.html
#   src/app/pages/dictionary/dictionary.component.css
#   src/app/pages/dictionary/dictionary.component.spec.ts (if tests enabled)
```

---

## File Summary

### Files Created
| File | Description |
|---|---|
| `web/angular.json` | Angular workspace config |
| `web/package.json` | Angular dependencies |
| `web/tsconfig*.json` | TypeScript configs |
| `web/proxy.conf.json` | Dev server API proxy |
| `web/src/index.html` | SPA entry HTML |
| `web/src/main.ts` | Angular bootstrap |
| `web/src/styles.css` | Global dark theme CSS |
| `web/src/app/app.component.*` | Shell layout |
| `web/src/app/app.config.ts` | App providers |
| `web/src/app/app.routes.ts` | Route definitions |
| `web/src/app/models/word.models.ts` | TypeScript interfaces |
| `web/src/app/services/api.service.ts` | HTTP client |
| `web/src/app/services/storage.service.ts` | localStorage wrapper |
| `web/src/app/pages/dictionary/*` | Main dictionary page |
| `web/src/app/pages/history/*` | History page |
| `web/src/app/pages/suggestions/*` | Suggestions page |
| `web/src/app/shared/header/*` | Navigation header |
| `web/src/app/shared/footer/*` | Footer |
| `web/src/app/shared/chip/*` | Reusable chip |
| `web/src/app/shared/expandable-list/*` | Expandable list |

### Files Modified (outside web/)
| File | Change |
|---|---|
| `src/http/routes/WordRoutes.cpp` | Add SPA fallback routes for `/history`, `/suggestions` |
| `.gitignore` | Add Angular-specific ignores |

### Files Deleted
| File | Reason |
|---|---|
| `web/src/main.js` | Replaced by Angular `main.ts` |
| `web/src/style.css` | Merged into Angular `styles.css` |
| `web/index.html` | Replaced by Angular `index.html` |
| `web/history.html` | Replaced by Angular route |
| `web/suggestions.html` | Replaced by Angular route |
| `web/vite.config.js` | Replaced by `angular.json` |
| `web/package.json` | Replaced by Angular `package.json` |
| `web/.eslintrc.cjs` | Replaced by Angular lint config |
| `web/.eslintignore` | No longer needed |
| `web/.prettierrc` | Can be re-added if desired |
