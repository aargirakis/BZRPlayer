#!/bin/bash
#
# NAME
#     test_samples.sh - test both XDG MIME 'query filetype' & 'query default'
#     against a selected set of sample files and MIME types
#
# SYNOPSIS
#     ./test_samples.sh
#

set -e

hide_mime_types_required=true
hide_mime_types_found=true
hide_mime_types_valid=true
hide_mime_types_invalid=false
hide_mime_types_missing=false
hide_tests_pass=true
requirements=(basename dirname sed sort tr xdg-mime)

bold=$'\e[1m'
bold_reset=$'\e[0m'

mime_dir_system="/usr/share/mime"
mime_packages_dir_system="$mime_dir_system/packages"

check_requirements() {
  for requirement in "${requirements[@]}"; do
    if ! type "$requirement" &>/dev/null; then
      echo -e "\nplease install ${bold}$requirement${bold_reset}"
      exit 1
    fi
  done
}

get_elements_of_array1_not_contained_in_array2() {
  local -n array1=$1
  local -n array2=$2
  local res=()

  for i in "${array1[@]}"; do
    skip=
    for j in "${array2[@]}"; do
      [[ $i == "$j" ]] && {
        skip=1
        break
      }
    done
    [[ -n $skip ]] || res+=("$i")
  done

  if [ ${#res[@]} -ne 0 ]; then
    printf '%s\n' "${res[@]}"
  fi
}

mime_types_scan() {
  local mime_types_required=()
  mapfile -t mime_types_required < <(sed -n "\|mime_types_supported=(| , \|)|{p; \|)|q}" "$bzr2_setup" |
    sed -e 's:mime_types_supported=(::g' -e 's:)::g' -e 's: :\n:g' | sed '/^[[:space:]]*$/d')
  IFS=" " read -r -a mime_types_required <<<"$(tr ' ' '\n' <<<"${mime_types_required[@]}" | sort -u | tr '\n' ' ')"
  local mime_types_found=()
  mapfile -t mime_types_found < <(ls -d -1 "$samples_path"/*/)

  for i in "${!mime_types_found[@]}"; do
    mime_types_found[i]=$(basename "${mime_types_found[$i]}")
  done

  mime_types_found=("${mime_types_found[@]///}")
  mime_types_found=("${mime_types_found[@]/-//}")
  local mime_types_missing=()
  mapfile -t mime_types_missing < <(get_elements_of_array1_not_contained_in_array2 mime_types_required mime_types_found)
  local mime_types_invalid=()
  mapfile -t mime_types_invalid < <(get_elements_of_array1_not_contained_in_array2 mime_types_found mime_types_required)
  local mime_types_valid=()
  mapfile -t mime_types_valid < <(get_elements_of_array1_not_contained_in_array2 mime_types_found mime_types_invalid)
  echo "${#mime_types_required[@]}"
  echo "${#mime_types_found[@]}"
  echo "${#mime_types_valid[@]}"
  echo "${#mime_types_invalid[@]}"
  echo "${#mime_types_missing[@]}"
  echo "${mime_types_valid[@]}"

  if [ "$hide_mime_types_required" = false ]; then
    for i in "${mime_types_required[@]}"; do
      echo "[${bold}REQUIRED${bold_reset}][$i]"
    done
  fi

  if [ "$hide_mime_types_found" = false ]; then
    for i in "${mime_types_found[@]}"; do
      echo "[${bold}FOUND${bold_reset}][$i]"
    done
  fi

  if [ "$hide_mime_types_valid" = false ]; then
    for i in "${mime_types_valid[@]}"; do
      echo "[${bold}VALID${bold_reset}][$i]"
    done
  fi

  if [ "$hide_mime_types_invalid" = false ]; then
    for i in "${mime_types_invalid[@]}"; do
      echo "[${bold}INVALID${bold_reset}][$i]"
    done
  fi

  if [ "$hide_mime_types_missing" = false ]; then
    for i in "${mime_types_missing[@]}"; do
      echo "[${bold}MISSING${bold_reset}][$i]"
    done
  fi
}

test_query_default() {
  local query_default_result
  query_default_result=$(xdg-mime query default "$mime_type")
  local exit_status=$?

  if [ "$exit_status" -ne 0 ] || [ -z "$query_default_result" ]; then
    echo -e "[${bold}FAIL${bold_reset}][$mime_type][invalid MIME type provided]"
    return 1
  else
    if [ "$query_default_result" = "$desktop_expected" ]; then
      if [ "$hide_tests_pass" = false ]; then
        echo -e "[ ${bold}OK${bold_reset} ][$mime_type]"
        return 0
      fi
    else
      echo -e "[${bold}FAIL${bold_reset}][$mime_type][actual: $query_default_result]\
[expected: $desktop_expected]"
      return 1
    fi
  fi
}

test_query_filetype() {
  local mime_type_dir_name=${mime_type////-}
  local mime_type_dir_path
  mime_type_dir_path="$samples_path/$mime_type_dir_name"
  local mime_type_dir_files=()
  mapfile -t mime_type_dir_files < <(ls -1 "$mime_type_dir_path"/*.* 2>/dev/null)
  local mime_type_dir_filenames=()

  for i in "${!mime_type_dir_files[@]}"; do
    mime_type_dir_filenames[i]=$(basename "${mime_type_dir_files[$i]}")
  done

  local mime_pattern_split=()

  while read -r line; do
    mime_pattern_split+=("$line")
  done <<<"$mime_pattern"

  local sample_filenames_valid=()
  local total_pass=0
  local total_fail=0
  local total_missing=0

  for mime_pattern_single in "${mime_pattern_split[@]}"; do
    local is_found=false

    for i in "${!mime_type_dir_files[@]}"; do

      if [[ ${mime_type_dir_filenames[$i],,} == $mime_pattern_single ]]; then
        is_found=true
        sample_filenames_valid+=("${mime_type_dir_filenames[$i]}")
        query_filetype_result=$(xdg-mime query filetype "${mime_type_dir_files[$i]}")

        if [ "$query_filetype_result" == "$mime_type" ]; then
          if [ "$hide_tests_pass" = false ]; then
            echo "[  ${bold}OK${bold_reset}   ][$mime_type][$mime_pattern_single]\
[${mime_type_dir_filenames[$i]}]"
          fi
          ((total_pass += 1))
        else
          echo "[ ${bold}FAIL${bold_reset}  ][$mime_type][$mime_pattern_single]\
[${mime_type_dir_filenames[$i]}][actual: $query_filetype_result][expected: $mime_type]"
          ((total_fail += 1))
        fi
      fi
    done
    if ! $is_found; then
      echo "[${bold}MISSING${bold_reset}][$mime_type][$mime_pattern_single] \
No sample file found"
      ((total_missing += 1))
    fi
  done

  local sample_filenames_invalid=()
  mapfile -t sample_filenames_invalid \
    < <(get_elements_of_array1_not_contained_in_array2 mime_type_dir_filenames sample_filenames_valid)

  for sample_filename_invalid in "${sample_filenames_invalid[@]}"; do
    echo "[${bold}INVALID${bold_reset}][$mime_type][$sample_filename_invalid]"
  done

  echo "$total_pass"
  echo "$total_fail"
  echo "$total_missing"
  echo "${#sample_filenames_invalid[@]}"
}

check_requirements

bzr2_setup_filename="bzr2-wine_setup.sh"
bzr2_setup="$(dirname "$0")/../$bzr2_setup_filename"
bzr2_xml_filename="x-bzr-player.xml"
bzr2_xml="$(dirname "$0")/../$bzr2_xml_filename"
samples_path="$(dirname "$0")/samples"

echo -e "\nMIME types ${bold}scanning${bold_reset}...\n"

mapfile -t scan_results < <(mime_types_scan)

mime_types=()
IFS=" " read -r -a mime_types <<<"${scan_results[5]}"

for i in "${scan_results[@]:6}"; do
  echo -e "$i"
done

echo -e "\nMIME types ${bold}scan${bold_reset} results:"
echo -e "Required ${bold}${scan_results[0]}${bold_reset}, Found ${bold}${scan_results[1]}${bold_reset} \
(Valid ${bold}${scan_results[2]}${bold_reset}, Invalid ${bold}${scan_results[3]}${bold_reset}), \
Missing ${bold}${scan_results[4]}${bold_reset}"
echo -e "\nTesting ${bold}xdg-mime query default${bold_reset} (MIME types association with BZR2 desktop entry)...\n"
test_query_default_passed=0
test_query_default_failed=0
desktop_expected="bzr-player.desktop"

for mime_type in "${mime_types[@]}"; do
  if test_query_default 2>/dev/null; then
    ((test_query_default_passed += 1))
  else
    ((test_query_default_failed += 1))
  fi
done

echo -e "\nTest results [${bold}xdg-mime query default${bold_reset}]:"
echo -e "Run ${bold}$((test_query_default_passed + test_query_default_failed))${bold_reset}, \
Passed ${bold}$((test_query_default_passed))${bold_reset}, \
Failed ${bold}$((test_query_default_failed))${bold_reset}"
echo -e "\nTesting ${bold}xdg-mime query filetype${bold_reset} \
(MIME types effectiveness against provided sample files)...\n"

bzr2_xml_content=$(cat "$bzr2_xml")
test_query_filetype_passed=0
test_query_filetype_failed=0
test_query_filetype_missing=0
test_query_filetype_invalid=0

for mime_type in "${mime_types[@]}"; do
  sed_pattern="\|<mime-type type=\"$mime_type\">| , \|</mime-type>|{p; \|</mime-type>|q}"
  mime_single=$(echo "$bzr2_xml_content" | sed -n "$sed_pattern")

  if [ -z "$mime_single" ]; then
    mime_single=$(sed -n "$sed_pattern" "$mime_packages_dir_system/freedesktop.org.xml")
  fi

  mime_pattern=$(echo "$mime_single" | grep "<glob " | sed -E -e 's:<glob ::g' -e 's: weight="[0-9]+"::g' \
    -e 's:pattern="::g' -e 's:"/>::g')

  mapfile -t test_query_filetype_results < <(test_query_filetype)

  result_length="${#test_query_filetype_results[@]}"
  test_query_filetype_passed=$((test_query_filetype_passed + ${test_query_filetype_results[$result_length - 4]}))
  test_query_filetype_failed=$((test_query_filetype_failed + ${test_query_filetype_results[$result_length - 3]}))
  test_query_filetype_missing=$((test_query_filetype_missing + ${test_query_filetype_results[$result_length - 2]}))
  test_query_filetype_invalid=$((test_query_filetype_invalid + ${test_query_filetype_results[$result_length - 1]}))

  for test_query_filetype_result in "${test_query_filetype_results[@]::${#test_query_filetype_results[@]}-4}"; do
    if [ "$test_query_filetype_result" ]; then
      echo "$test_query_filetype_result"
    fi
  done
done

echo -e "\nTest results [${bold}xdg-mime query filetype${bold_reset}]:"
echo -e "Run ${bold}$((test_query_filetype_passed + test_query_filetype_failed))${bold_reset}: \
Passed ${bold}$test_query_filetype_passed${bold_reset}, Failed ${bold}$test_query_filetype_failed${bold_reset}"
echo -e "Skipped (due to missing sample files) ${bold}$test_query_filetype_missing${bold_reset}"
echo -e "Invalid samples files found ${bold}$test_query_filetype_invalid${bold_reset}"
