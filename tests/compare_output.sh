#!/usr/bin/env bash
set -euo pipefail

binary=${1:?missing binary path}
case_name=${2:?missing case name}

tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/buniq-compare.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT

input_file="$tmp_dir/input.txt"
output_file="$tmp_dir/output.txt"

case "$case_name" in
  empty)
    : > "$input_file"
    ;;
  small)
    printf 'alpha\nbeta\ngamma\n' > "$input_file"
    ;;
  cross_chunk)
    perl -e 'print "A" x 250, "\n", "B" x 10, "\n"' > "$input_file"
    ;;
  no_final_newline)
    perl -e 'print "C" x 120, "\n", "D" x 80' > "$input_file"
    ;;
  *)
    printf 'unknown test case: %s\n' "$case_name" >&2
    exit 2
    ;;
esac

"$binary" "$input_file" > "$output_file"

if cmp -s "$input_file" "$output_file"; then
  printf '%s: match\n' "$case_name"
else
  printf '%s: mismatch\n' "$case_name" >&2
  diff -u "$input_file" "$output_file" || true
  exit 1
fi
