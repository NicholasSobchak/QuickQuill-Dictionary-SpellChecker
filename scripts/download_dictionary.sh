#!/usr/bin/env bash
set -euo pipefail

dict_dir="${1:-nlohmann}"
mkdir -p "$dict_dir"

word_list_url="https://raw.githubusercontent.com/dwyl/english-words/master/words_alpha.txt"
out_file="$dict_dir/words_alpha.txt"

if [[ ! -f "$out_file" ]]; then
  echo "Downloading word list..."
  curl -L "$word_list_url" -o "$out_file"
else
  echo "Word list already exists at $out_file"
fi
