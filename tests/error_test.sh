#!/usr/bin/env bash
set -euo pipefail

binary=${1:?missing binary path}
case_name=${2:?missing case name}

tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/buniq-error.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT

input_file="$tmp_dir/input.txt"

case "$case_name" in
  to_be_determined)
    perl -e 'print "X" x 300, "\n"' > "$input_file"
    ;;
  *)
    printf 'unknown error test case: %s\n' "$case_name" >&2
    exit 2
    ;;
esac

if "$binary" "$input_file" 2>&1; then
  printf '%s: should have thrown but did not\n' "$case_name" >&2
  exit 1
else
  printf '%s: correctly threw\n' "$case_name"
fi