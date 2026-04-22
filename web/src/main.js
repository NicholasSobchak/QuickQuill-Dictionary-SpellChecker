import './style.css';

const input = document.getElementById('word');
const result = document.getElementById('result');
const spinner = document.getElementById('spinner');
const statusText = document.getElementById('statusText');
const alternativeSearches = document.getElementById('alternativeSearches');
const button = document.getElementById('go');
const historyToggle = document.getElementById('historyToggle');
const historyDrawer = document.getElementById('historyDrawer');
const historyList = document.getElementById('historyList');
const historyEmpty = document.getElementById('historyEmpty');
const clearHistoryButton = document.getElementById('clearHistory');
const suggestToggle = document.getElementById('suggestToggle');
const suggestDrawer = document.getElementById('suggestDrawer');
const suggestList = document.getElementById('suggestList');
const suggestEmpty = document.getElementById('suggestEmpty');
const clearSuggestionsButton = document.getElementById('clearSuggestions');
const suggestedBox = document.getElementById('suggestedBox');
const suggestedList = document.getElementById('suggestedList');
const mainResultBox = document.getElementById('resultBox');
const SUGGESTED_LIMIT = 100;
const HISTORY_KEY = 'quickquill-history';
const HISTORY_LIMIT = 100;
const SUGGESTED_KEY = 'quickquill-suggested-words';
const TOGGLE_GAP = 12;
let spinnerTimer = null;
let drawerSuggestedWords = loadSuggestedWords();
let liveSuggestedWords = [];
let lastSuggestedQuery = '';

function loadHistory() {
  const stored = localStorage.getItem(HISTORY_KEY);
  return stored ? JSON.parse(stored) : [];
}

function saveHistory(words) {
  localStorage.setItem(HISTORY_KEY, JSON.stringify(words.slice(0, HISTORY_LIMIT)));
}

function loadSuggestedWords() {
  const stored = localStorage.getItem(SUGGESTED_KEY);
  return stored ? JSON.parse(stored) : [];
}

function saveSuggestedWords(words) {
  localStorage.setItem(SUGGESTED_KEY, JSON.stringify(words.slice(0, SUGGESTED_LIMIT)));
}

function addSearchedSuggestion(word) {
  const cleanedWord = displayWord(word);
  if (!cleanedWord) return;
  mergeDrawerSuggestedWords([cleanedWord]);
}

function mergeDrawerSuggestedWords(words) {
  if (!Array.isArray(words) || !words.length) return;

  const merged = [
    ...words.map((word) => displayWord(word)).filter(Boolean),
    ...drawerSuggestedWords,
  ];
  const unique = [];
  const seen = new Set();

  merged.forEach((item) => {
    const normalized = item.toLowerCase();
    if (seen.has(normalized)) return;
    seen.add(normalized);
    unique.push(item);
  });

  drawerSuggestedWords = unique.slice(0, SUGGESTED_LIMIT);
  saveSuggestedWords(drawerSuggestedWords);
}

async function storeSuggestionsForQuery(word) {
  const cleanedWord = displayWord(word);
  if (!cleanedWord) return;

  if (lastSuggestedQuery.toLowerCase() === cleanedWord.toLowerCase()) {
    mergeDrawerSuggestedWords(liveSuggestedWords);
    renderSuggest();
    syncSimilarSearchesForEmptyInput();
    return;
  }

  try {
    const res = await fetch(`/api/suggest/${encodeURIComponent(cleanedWord)}`);
    if (!res.ok) return;

    const words = (await res.json()).map((item) => displayWord(item)).filter(Boolean);
    mergeDrawerSuggestedWords(words);
    renderSuggest();
    syncSimilarSearchesForEmptyInput();
  } catch (err) {
    console.error('Error storing suggestions:', err);
  }
}

function renderHistory() {
  const items = loadHistory();
  historyList.innerHTML = '';
  if (!items.length) {
    historyEmpty.style.display = 'block';
    historyList.style.display = 'none';
    return;
  }
  historyEmpty.style.display = 'none';
  historyList.style.display = 'flex';
  items.forEach((w) => {
    const li = document.createElement('li');
    li.className = 'history-item';
    li.textContent = w;
    li.addEventListener('click', () => {
      li.style.textDecoration = 'underline';
      input.value = w;
      lookup();
    });
    historyList.appendChild(li);
  });
}

function renderSuggest() {
  if (!suggestList) return;
  drawerSuggestedWords = drawerSuggestedWords.slice(0, SUGGESTED_LIMIT);
  suggestList.innerHTML = '';
  if (!drawerSuggestedWords.length) {
    if (suggestEmpty) suggestEmpty.style.display = 'block';
    suggestList.style.display = 'none';
    return;
  }
  if (suggestEmpty) suggestEmpty.style.display = 'none';
  suggestList.style.display = 'flex';
  drawerSuggestedWords.forEach((w) => {
    const li = document.createElement('li');
    li.className = 'history-item';
    li.textContent = w;
    li.addEventListener('click', () => {
      input.value = w;
      hideSuggestedBox();
      lookup();
    });
    suggestList.appendChild(li);
  });
}

function addToHistory(word) {
  if (!word) return;
  const items = loadHistory();
  const filtered = items.filter((w) => w.toLowerCase() !== word.toLowerCase());
  filtered.unshift(word);
  saveHistory(filtered.slice(0, HISTORY_LIMIT));
  renderHistory();
}

if (clearHistoryButton) {
  clearHistoryButton.addEventListener('click', () => {
    localStorage.removeItem(HISTORY_KEY);
    renderHistory();
  });
}

if (clearSuggestionsButton) {
  clearSuggestionsButton.addEventListener('click', () => {
    drawerSuggestedWords = [];
    localStorage.removeItem(SUGGESTED_KEY);
    renderSuggest();
    syncSimilarSearchesForEmptyInput();
  });
}

function updateHistoryLayout() {
  const viewport = { w: window.innerWidth };
  const sidePad = 16;
  historyDrawer.style.left = `${sidePad}px`;
  historyDrawer.style.right = `${sidePad}px`;
  historyDrawer.style.width = `${Math.max(viewport.w - sidePad * 2, 0)}px`;

  const arrow = historyToggle.querySelector('.toggle-arrow');
  if (arrow) arrow.textContent = historyDrawer.classList.contains('open') ? '^' : 'v';
}

function updateSuggestLayout() {
  const viewport = { w: window.innerWidth };
  const sidePad = 16;
  suggestDrawer.style.left = `${sidePad}px`;
  suggestDrawer.style.right = `${sidePad}px`;
  suggestDrawer.style.width = `${Math.max(viewport.w - sidePad * 2, 0)}px`;

  const arrow = suggestToggle.querySelector('.toggle-arrow');
  if (arrow) arrow.textContent = suggestDrawer.classList.contains('open') ? '^' : 'v';
}

function positionToggleGroup() {
  const vw = window.innerWidth;
  const sugWidth = suggestToggle.getBoundingClientRect().width || 0;
  const histWidth = historyToggle.getBoundingClientRect().width || 0;
  const total = sugWidth + histWidth + TOGGLE_GAP;
  const start = Math.max((vw - total) / 2, 8);

  suggestToggle.style.left = `${start}px`;
  suggestToggle.style.right = 'auto';
  historyToggle.style.left = `${start + sugWidth + TOGGLE_GAP}px`;
  historyToggle.style.right = 'auto';
  suggestToggle.style.transform = 'translate(0, 0)';
  historyToggle.style.transform = 'translate(0, 0)';
}

function resolveToggleOverlap() {
  positionToggleGroup();
  suggestToggle.dataset.manual = '0';
  historyToggle.dataset.manual = '0';
}

function toggleHistory(open) {
  const shouldOpen = open ?? !historyDrawer.classList.contains('open');
  if (shouldOpen) {
    toggleSuggest(false);
  }
  historyDrawer.classList.toggle('open', shouldOpen);
  historyDrawer.setAttribute('aria-hidden', shouldOpen ? 'false' : 'true');
  historyToggle.setAttribute('aria-expanded', shouldOpen ? 'true' : 'false');
  requestAnimationFrame(() => {
    updateHistoryLayout();
    resolveToggleOverlap();
  });
  if (shouldOpen) renderHistory();
}

historyToggle.addEventListener('click', () => toggleHistory());
window.addEventListener('resize', () => {
  updateHistoryLayout();
  updateSuggestLayout();
  resolveToggleOverlap();
});
requestAnimationFrame(() => {
  updateHistoryLayout();
  updateSuggestLayout();
  resolveToggleOverlap();
});

function toggleSuggest(open) {
  const shouldOpen = open ?? !suggestDrawer.classList.contains('open');
  if (shouldOpen) {
    toggleHistory(false);
  }
  suggestDrawer.classList.toggle('open', shouldOpen);
  suggestDrawer.setAttribute('aria-hidden', shouldOpen ? 'false' : 'true');
  suggestToggle.setAttribute('aria-expanded', shouldOpen ? 'true' : 'false');
  if (shouldOpen) renderSuggest();
  requestAnimationFrame(() => {
    updateSuggestLayout();
    resolveToggleOverlap();
  });
}

suggestToggle.addEventListener('click', () => toggleSuggest());

function hideSuggestedBox() {
  if (suggestedBox) {
    suggestedBox.classList.add('hidden');
  }
}

function showSuggestedBox() {
  if (!suggestedBox) return;
  suggestedBox.classList.remove('hidden');
  renderSuggestedSearches();
}

function syncSimilarSearchesForEmptyInput() {
  if (input.value.trim()) return;
  liveSuggestedWords = drawerSuggestedWords.slice(0, 10);
  renderSuggestedSearches();
}

function clearAlternativeSearches() {
  if (!alternativeSearches) return;
  alternativeSearches.innerHTML = '';
  alternativeSearches.classList.add('hidden');
}

function renderAlternativeSearches(words) {
  if (!alternativeSearches) return;

  const alternatives = Array.isArray(words) ? words.filter(Boolean) : [];
  alternativeSearches.innerHTML = '';

  if (!alternatives.length) {
    alternativeSearches.classList.add('hidden');
    return;
  }

  const label = document.createElement('span');
  label.className = 'alternative-searches-label';
  label.textContent = 'Alternative Searches:';
  alternativeSearches.appendChild(label);

  alternatives.forEach((word, index) => {
    if (index > 0) {
      const separator = document.createElement('span');
      separator.className = 'alternative-searches-separator';
      separator.textContent = ',';
      alternativeSearches.appendChild(separator);
    }

    const link = document.createElement('a');
    link.href = '#';
    link.className = 'alternative-search-link';
    link.textContent = word;
    link.addEventListener('click', (event) => {
      event.preventDefault();
      input.value = word;
      lookup();
    });
    alternativeSearches.appendChild(link);
  });

  alternativeSearches.classList.remove('hidden');
}

function renderSuggestedSearches() {
  if (!suggestedList || !suggestedBox) return;
  liveSuggestedWords = liveSuggestedWords.slice(0, SUGGESTED_LIMIT);
  const words = liveSuggestedWords.slice(0, Math.min(10, SUGGESTED_LIMIT));
  suggestedList.innerHTML = '';
  if (!words.length) {
    suggestedBox.classList.remove('hidden');
    const empty = document.createElement('div');
    empty.className = 'history-empty';
    empty.textContent = 'No suggestions yet.';
    suggestedList.appendChild(empty);
    return;
  }

  suggestedBox.classList.remove('hidden');
  words.forEach((word) => {
    const row = document.createElement('div');
    row.className = 'suggested-row';

    const bullet = document.createElement('span');
    bullet.className = 'suggested-bullet';
    bullet.textContent = '>';

    const chip = document.createElement('button');
    chip.type = 'button';
    chip.className = 'chip suggested-chip';
    chip.textContent = word;
    chip.addEventListener('click', () => {
      input.value = word;
      hideSuggestedBox();
      lookup();
    });

    row.appendChild(bullet);
    row.appendChild(chip);
    suggestedList.appendChild(row);
  });
}

renderSuggestedSearches();
clearResult();
clearAlternativeSearches();
syncSimilarSearchesForEmptyInput();

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape' && historyDrawer.classList.contains('open')) {
    toggleHistory(false);
  }
  if (e.key === 'Escape' && suggestDrawer.classList.contains('open')) {
    toggleSuggest(false);
  }
});

function clearResult() {
  while (result.firstChild) result.removeChild(result.firstChild);
  clearAlternativeSearches();
  if (mainResultBox) {
    mainResultBox.classList.remove('has-results');
    mainResultBox.classList.add('hidden');
  }
}

function setStatus(text, isError = false) {
  statusText.innerHTML = text || '';
  statusText.style.color = isError ? '#ff6666' : 'var(--muted)';
}

function setSuggestionStatus(word, label = word) {
  statusText.innerHTML = '';
  statusText.style.color = '#ff6666';

  const message = document.createElement('span');
  message.textContent = 'Did you mean ';

  const link = document.createElement('a');
  link.href = '#';
  link.className = 'suggestion-link';
  link.addEventListener('click', (event) => {
    event.preventDefault();
    input.value = word;
    lookup();
  });

  const emphasis = document.createElement('em');
  emphasis.className = 'corrected-word';
  emphasis.textContent = label;
  link.appendChild(emphasis);

  message.appendChild(link);
  message.append('?');
  statusText.appendChild(message);
}

function addSection(title) {
  const section = document.createElement('div');
  section.className = 'section';
  const label = document.createElement('div');
  label.className = 'label';
  label.textContent = title;
  section.appendChild(label);
  result.appendChild(section);
  return section;
}

function appendNumberedList(section, items) {
  const list = document.createElement('ol');
  list.className = 'list';

  items.forEach((text, idx) => {
    const item = document.createElement('li');
    item.textContent = text;
    if (idx >= 5) item.classList.add('extra');
    list.appendChild(item);
  });

  section.appendChild(list);

  if (items.length > 5) {
    const toggle = document.createElement('button');
    toggle.type = 'button';
    toggle.className = 'toggle-btn';
    const label = document.createTextNode('see more ');
    const arrow = document.createElement('span');
    arrow.className = 'arrow';
    arrow.textContent = 'v';
    toggle.appendChild(label);
    toggle.appendChild(arrow);
    toggle.setAttribute('aria-expanded', 'false');
    toggle.addEventListener('click', () => {
      const expanded = list.classList.toggle('show-all');
      label.textContent = expanded ? 'see less ' : 'see more ';
      arrow.textContent = expanded ? '^' : 'v';
      toggle.setAttribute('aria-expanded', expanded);
    });
    const toggleWrap = document.createElement('div');
    toggleWrap.style.display = 'flex';
    toggleWrap.style.justifyContent = 'center';
    toggleWrap.style.transform = 'translateX(0px)';
    toggleWrap.appendChild(toggle);
    section.appendChild(toggleWrap);
  }
}

function displayWord(text) {
  if (!text) return '';
  return text
    .replace(/[^A-Za-z0-9'’ -]+/g, ' ')
    .replace(/\s+/g, ' ')
    .trim();
}

function renderWord(data) {
  clearResult();

  if (data.found === false) {
    if (data.suggestion && data.suggestion.toLowerCase() !== data.query.toLowerCase()) {
      setSuggestionStatus(data.suggestion, data.display_lemma || data.suggestion);
    } else {
      setStatus(`Word <em>'${input.value}'</em> not found.`, true);
    }
    return;
  }

  if (data.error) {
    if (data.error === 'Enter a valid word') {
      setStatus('400: Enter a valid word', true);
    } else {
      setStatus(`Error: ${data.error}`, true);
    }
    return;
  }

  if (mainResultBox) {
    mainResultBox.classList.remove('hidden');
    mainResultBox.classList.add('has-results');
  }

  renderAlternativeSearches(data.alternative_searches);

  const lemma = document.createElement('h2');
  lemma.className = 'lemma';
  const rawHeading = data.display_lemma || data.query || data.lemma || '';
  let decodedHeading = rawHeading;
  try {
    decodedHeading = decodeURIComponent(rawHeading);
  } catch (_) {
    decodedHeading = rawHeading;
  }
  const heading = displayWord(decodedHeading);
  const capitalizedHeading = heading.charAt(0).toUpperCase() + heading.slice(1);
  const punctuated =
    capitalizedHeading && capitalizedHeading.toLowerCase() === 'lexicon levissimum'
      ? `${capitalizedHeading}!`
      : capitalizedHeading;
  lemma.textContent = punctuated || 'Unknown';
  if (capitalizedHeading && capitalizedHeading.toLowerCase() === 'nevermore') {
    const link = document.createElement('a');
    link.href = 'https://www.nevermoreacademy.com/';
    link.target = '_blank';
    link.rel = 'noreferrer noopener';
    link.textContent = ' https://www.nevermoreacademy.com/';
    link.style.color = 'var(--ink)';
    link.style.fontFamily = '"Times New Roman", Times, serif';
    link.style.fontSize = '17px';
    link.style.fontWeight = '400';
    link.style.marginLeft = '8px';
    lemma.appendChild(link);
  }
  result.appendChild(lemma);

  const sensesSection = addSection('Definition');
  if (Array.isArray(data.senses) && data.senses.length) {
    const items = data.senses.map((s) => {
      const pos = s.pos ? `[${s.pos}] ` : '';
      return `${pos}${s.definition || ''}`.trim();
    });
    appendNumberedList(sensesSection, items);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No definitions available.';
    sensesSection.appendChild(empty);
  }

  const synSection = addSection('Synonyms');
  const syns = Array.isArray(data.senses)
    ? data.senses.flatMap((s) => (Array.isArray(s.synonyms) ? s.synonyms : []))
    : [];
  if (syns.length) {
    const wrap = document.createElement('div');
    syns.forEach((w) => {
      const chip = document.createElement('a');
      chip.className = 'chip';
      chip.textContent = w;
      chip.href = '#';
      chip.onclick = (e) => {
        e.preventDefault();
        input.value = w;
        lookup();
      };
      wrap.appendChild(chip);
    });
    synSection.appendChild(wrap);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No synonyms available.';
    synSection.appendChild(empty);
  }

  const antSection = addSection('Antonyms');
  const ants = Array.isArray(data.senses)
    ? data.senses.flatMap((s) => (Array.isArray(s.antonyms) ? s.antonyms : []))
    : [];
  if (ants.length) {
    const wrap = document.createElement('div');
    ants.forEach((w) => {
      const chip = document.createElement('a');
      chip.className = 'chip';
      chip.textContent = w;
      chip.href = '#';
      chip.onclick = (e) => {
        e.preventDefault();
        input.value = w;
        lookup();
      };
      wrap.appendChild(chip);
    });
    antSection.appendChild(wrap);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No antonyms available.';
    antSection.appendChild(empty);
  }

  const exSection = addSection('Examples');
  const examples = Array.isArray(data.senses)
    ? data.senses.flatMap((s) => (Array.isArray(s.examples) ? s.examples : []))
    : [];
  if (examples.length) {
    appendNumberedList(exSection, examples);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No examples available.';
    exSection.appendChild(empty);
  }

  const formsSection = addSection('Forms');
  if (Array.isArray(data.forms) && data.forms.length) {
    const wrap = document.createElement('div');
    data.forms.forEach((f) => {
      const chip = document.createElement('a');
      chip.className = 'chip';
      chip.textContent = f.tag ? `${f.form} (${f.tag})` : f.form;
      chip.href = '#';
      chip.onclick = (e) => {
        e.preventDefault();
        input.value = f.form;
        lookup();
      };
      wrap.appendChild(chip);
    });
    formsSection.appendChild(wrap);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No forms available.';
    formsSection.appendChild(empty);
  }

  const etySection = addSection('Etymology');
  if (Array.isArray(data.etymology) && data.etymology.length) {
    appendNumberedList(etySection, data.etymology);
  } else {
    const empty = document.createElement('div');
    empty.className = 'empty';
    empty.textContent = 'No etymology available.';
    etySection.appendChild(empty);
  }
}

async function lookup() {
  const word = input.value.trim();
  if (!word) {
    clearResult();
    showSuggestedBox();
    renderWord({ error: 'Enter a word' }, '');
    input.focus();
    return;
  }
  if (!/^[A-Za-z'\- .]+$/.test(word)) {
    clearResult();
    setStatus('400: Enter a valid word', true);
    input.focus();
    return;
  }

  button.disabled = true;
  spinner.style.display = 'inline-block';
  if (spinnerTimer) clearInterval(spinnerTimer);
  const frames = ['|', '/', '-', ''];
  let idx = 0;
  spinner.textContent = frames[idx];
  spinnerTimer = setInterval(() => {
    idx = (idx + 1) % frames.length;
    spinner.textContent = frames[idx];
  }, 120);
  clearResult();

  try {
    setStatus('Looking up…');
    const res = await fetch(`/api/word/${encodeURIComponent(word)}`);
    const data = await res.json();
    if (!res.ok) {
      renderWord(data, word);
      storeSuggestionsForQuery(word);
    } else {
      renderWord(data, word);
      setStatus('');
      addToHistory(displayWord(word));
      addSearchedSuggestion(word);
      storeSuggestionsForQuery(word);
      renderSuggest();
    }
  } catch (err) {
    clearResult();
    const reason = err && err.message ? err.message : 'failed to fetch';
    setStatus(`Network error: ${reason}`, true);
  } finally {
    button.disabled = false;
    spinner.style.display = 'none';
    if (spinnerTimer) {
      clearInterval(spinnerTimer);
      spinnerTimer = null;
    }
  }
}

let suggestTimer;

function debounce(fn, delay) {
  return function debounced(...args) {
    clearTimeout(suggestTimer);
    suggestTimer = setTimeout(() => fn.apply(this, args), delay);
  };
}

async function fetchSuggestions() {
  const word = input.value.trim();
  if (word.length < 2) {
    liveSuggestedWords = drawerSuggestedWords.slice(0, 10);
    lastSuggestedQuery = '';
    renderSuggest();
    renderSuggestedSearches();
    return;
  }

  try {
    const res = await fetch(`/api/suggest/${encodeURIComponent(word)}`);
    if (!res.ok) return;

    liveSuggestedWords = (await res.json()).map((item) => displayWord(item)).filter(Boolean);
    lastSuggestedQuery = displayWord(word);
    renderSuggestedSearches();
  } catch (err) {
    console.error('Error fetching suggestions:', err);
  }
}

input.addEventListener('input', debounce(fetchSuggestions, 300));

button.addEventListener('click', lookup);
input.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') lookup();
});
