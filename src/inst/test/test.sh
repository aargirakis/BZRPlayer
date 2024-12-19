#!/bin/bash
#
# NAME
#     test.sh - test BZR2 XDG desktop entry association or XDG MIME type association
#     against a target file or directory content
#
# SYNOPSIS
#     ./test.sh [-h|--help|--show-passed] target [MIME type]
#

set -e

hide_tests_pass=true
requirements=(realpath xdg-mime)
bold=$'\e[1m'
bold_reset=$'\e[0m'

check_requirements() {
  for requirement in "${requirements[@]}"; do
    if ! type "$requirement" &>/dev/null; then
      echo -e "\nplease install ${bold}$requirement${bold_reset}"
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
      echo -e "[ ${bold}OK${bold_reset} ][expected: $mime_expected][actual: $mime_actual][$file]"
    fi
  else
    ((test_query_filetype_failed += 1))
    echo -e "[${bold}FAIL${bold_reset}][expected: $mime_expected][actual: $mime_actual][$file]"
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
      echo -e "[ ${bold}OK${bold_reset} ][$mime_type][$query_default_result][$(basename "$file")]"
      return 0
    fi
  else
    echo -e "[${bold}FAIL${bold_reset}][$mime_type][expected: $desktop_expected]\
[actual: $query_default_result][$(basename "$file")]"
    return 1
  fi
}

check_requirements

help_string="Usage: $(basename "$0") [-h|--help|--show-passed] target [MIME type]"

opts_pointer=1

if [ -z "${!opts_pointer}" ]; then
  echo "target arg is required"
  echo "$help_string"
  exit 1
fi

if [ "${!opts_pointer}" == "-h" ] || [ "${!opts_pointer}" == "--help" ]; then
  echo "$help_string"
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

if [ $is_target_dir = true ]; then
  readarray -d '' files < <(find "$target" -type f -print0)
else
  files=("$target")
fi

((opts_pointer += 1))
mime_provided="${!opts_pointer}"

if [ -z "$mime_provided" ]; then
  echo -e "\nTesting ${bold}BZR2 desktop entry association with provided target${bold_reset}...\n"

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

  echo -e "\nTest results [${bold}xdg-mime query default${bold_reset}]:"
  echo -e "Run ${bold}$((test_query_default_passed + test_query_default_failed))${bold_reset}, \
  Passed ${bold}$((test_query_default_passed))${bold_reset}, \
  Failed ${bold}$((test_query_default_failed))${bold_reset}"
else
  echo -e "\nTesting ${bold}MIME type association with provided target${bold_reset}...\n"

  mime_expected=$mime_provided
  test_query_filetype_passed=0
  test_query_filetype_failed=0

  for file in "${files[@]}"; do
    test_query_filetype
  done

  echo -e "\nTest results [${bold}xdg-mime query filetype${bold_reset}]:"
  echo -e "Run ${bold}$((test_query_filetype_passed + test_query_filetype_failed))${bold_reset}: \
  Passed ${bold}$test_query_filetype_passed${bold_reset}, Failed ${bold}$test_query_filetype_failed${bold_reset}"
fi
