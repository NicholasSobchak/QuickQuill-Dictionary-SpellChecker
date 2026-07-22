export interface WordResponse {
  id: number;
  lemma: string;
  display_lemma?: string;    // only present if non-empty (C++ line 16-19)
  query?: string;             // only present if provided (C++ line 21-24)
  forms: WordForm[];
  senses: WordSense[];
  etymology: string[];
  alternative_searches: string[]
}
