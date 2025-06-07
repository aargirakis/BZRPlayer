#!/bin/bash
#
# NAME
#     test.sh - test BZR2 XDG desktop entry association or XDG MIME type association
#     against a target file or directory content
#
# SYNOPSIS
#     ./test.sh [-h|--help|--show-passed] target [--filter-ext=file extension (case insens)] [MIME type]
#

set -e

hide_tests_pass=true
requirements=(realpath xdg-mime)
text_bold=$'\e[1m'
text_underline=$'\e[4m'
text_reset=$'\e[0m'

check_requirements() {
  for requirement in "${requirements[@]}"; do
    if ! type "$requirement" &>/dev/null; then
      echo -e "\nplease install ${text_bold}$requirement${text_reset}"
      exit 1
    fi
  done
}

test_query_filetype() {
  local mime_actual
  mime_actual=$(xdg-mime query filetype "$file")

  if [ "$mime_actual" == "$mime_expected" ]; then
    ((test_query_filetype_passed += 1))
    if [ "$hide_tests_pass" = false ]; then
      echo -e "[ ${text_bold}OK${text_reset} ][expected: $mime_expected][actual: $mime_actual][$file]"
    fi
  else
    ((test_query_filetype_failed += 1))
    echo -e "[${text_bold}FAIL${text_reset}][expected: $mime_expected][actual: $mime_actual][$file]"
  fi
}

test_query_default_on_query_filetype() {
  local mime_actual
  mime_type=$(xdg-mime query filetype "$file")

  if [ -z "$mime_type" ]; then
    mime_type="NONE"
  fi

  local query_default_result
  query_default_result=$(xdg-mime query default "$mime_type")

  if [ -z "$query_default_result" ]; then
    query_default_result="NONE"
  fi

  if [ "$query_default_result" = "$desktop_expected" ]; then
    if [ "$hide_tests_pass" = false ]; then
      echo -e "[ ${text_bold}OK${text_reset} ][$mime_type][$query_default_result][$(basename "$file")]"
      return 0
    fi
  else
    echo -e "[${text_bold}FAIL${text_reset}][$mime_type][expected: $desktop_expected]\
[actual: $query_default_result][$(basename "$file")]"
    return 1
  fi
}

check_requirements

help_string="Usage: ${text_bold}$(basename "$0")${text_reset} \
[${text_bold}-h${text_reset}|${text_bold}--help${text_reset}|${text_bold}--show-passed${text_reset}] \
${text_underline}target${text_reset} [--filter-ext=${text_underline}file extension (case insens)${text_reset}] \
[${text_underline}MIME type${text_reset}]\nExample: ./$(basename "$0") --show-passed . --filter-ext=mdx audio/x-mdx "

opts_pointer=1

if [ -z "${!opts_pointer}" ]; then
  echo "target arg is required"
  echo -e "$help_string"
  exit 1
fi

if [ "${!opts_pointer}" == "-h" ] || [ "${!opts_pointer}" == "--help" ]; then
  echo -e "$help_string"
  exit 0
fi

if [ "${!opts_pointer}" == "--show-passed" ]; then
  hide_tests_pass=false
  ((opts_pointer += 1))
fi

target="${!opts_pointer}"

if [ ! -e "$target" ]; then
  echo "provided target does not exists"
  exit 1
fi

if [ -d "$target" ]; then
  if [ -z "$(ls -A "$target")" ]; then
    echo "provided target directory is empty"
    exit 1
  fi
  is_target_dir=true
else
  if [ ! -r "$target" ]; then
    echo "unable to read provided target file"
    exit 1
  fi
  is_target_dir=false
fi

target=$(realpath -s "$target")
((opts_pointer += 1))

if [[ "${!opts_pointer}" =~ --filter-ext=([^[:space:]]+) ]]; then
  extension="${BASH_REMATCH[1]}"
  ((opts_pointer += 1))
fi

if [ $is_target_dir = true ]; then
  if [ -z "$extension" ]; then
    readarray -d '' files < <(find "$target" -type f -print0)
  else
    readarray -d '' files < <(find "$target" -type f -iname "*.$extension" -print0)
  fi
else
  files=("$target")
fi

mime_provided="${!opts_pointer}"

if [ -z "$mime_provided" ]; then
  echo -e "\nTesting ${text_bold}BZR2 desktop entry association with provided target${text_reset}...\n"

  test_query_default_passed=0
  test_query_default_failed=0
  desktop_expected="bzr-player.desktop"

  for file in "${files[@]}"; do
    if test_query_default_on_query_filetype 2>/dev/null; then
      ((test_query_default_passed += 1))
    else
      ((test_query_default_failed += 1))
    fi
  done

  echo -e "\nTest results [${text_bold}xdg-mime query default${text_reset}]:"
  echo -e "Run ${text_bold}$((test_query_default_passed + test_query_default_failed))${text_reset}, \
  Passed ${text_bold}$((test_query_default_passed))${text_reset}, \
  Failed ${text_bold}$((test_query_default_failed))${text_reset}"
else
  echo -e "\nTesting ${text_bold}MIME type association with provided target${text_reset}...\n"

  mime_expected=$mime_provided
  test_query_filetype_passed=0
  test_query_filetype_failed=0

  for file in "${files[@]}"; do
    test_query_filetype
  done

  echo -e "\nTest results [${text_bold}xdg-mime query filetype${text_reset}]:"
  echo -e "Run ${text_bold}$((test_query_filetype_passed + test_query_filetype_failed))${text_reset}: \
  Passed ${text_bold}$test_query_filetype_passed${text_reset}, Failed ${text_bold}$test_query_filetype_failed${text_reset}"
fi
