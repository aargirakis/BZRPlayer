#!/bin/bash
#
# NAME
#     bzr2-wine_setup.sh - distribution-agnostic BZR Player 2.x (BZR2) installer for linux with wine support
#
# SYNOPSIS
#     ./bzr2-wine_setup.sh
#
# DESCRIPTION
#     download, install and configure BZR2 using wine, also providing the way to remove it
#
#     handle multiple BZR2 versions (useful for testing purposes) in separated
#     wine prefixes at ~/.bzr-player/<version>
#
#     also install icons and generates an XDG desktop entry for launching the player,
#     eventually associated to supported MIME types
#
# NOTES
#     - versions older than 2.0.19 are not supported
#

set -e

main() {

  if [ "$(id -u)" -ne 0 ]; then
    echo "Root privileges are required"
    exit 1
  fi

  USER=${SUDO_USER}
  HOME=$(eval echo ~"$SUDO_USER")

  requirements=(
    cat install mktemp realpath sed sort sudo uname unzip update-desktop-database update-mime-database wget wine
    xdg-desktop-menu xdg-icon-resource xdg-mime xrdb
  )
  force_reinstall_default="n"
  url_latest_version="https://api.github.com/repos/aargirakis/BZRPlayer/releases/latest"
  urls_download=(
    "https://bzrplayer.blazer.nu/getFile.php?id="
    "https://github.com/aargirakis/BZRPlayer/releases/download"
    "https://raw.githubusercontent.com/aargirakis/BZRPlayer/binaries_archive/binaries"
  )
  download_tries=2
  bzr2_zip_dir_default="binaries"
  bzr2_xml_dir_default="."
  bzr2_xml_url="https://github.com/aargirakis/BZRPlayer/raw/refs/heads/main/src/inst/x-bzr-player.xml"
  mime_types_supported=(
    application/ogg audio/flac audio/midi audio/mp2 audio/mpeg audio/prs.sid audio/vnd.wave audio/x-adlib-ims
    audio/x-adlib-raw audio/x-ahx audio/x-aon audio/x-amf audio/x-cust audio/x-ddmf audio/x-deflemask audio/x-dsmi-amf
    audio/x-dsmi-amf-hack audio/x-dw audio/x-dz audio/x-fc audio/x-fc-bsi audio/x-flac+ogg audio/x-fp audio/x-fur
    audio/x-hip audio/x-hip-7v audio/x-hip-coso audio/x-hip-mcmd audio/x-hip-st audio/x-hip-st-coso audio/x-hvl
    audio/x-ims audio/x-it audio/x-lds audio/x-m2 audio/x-mdx audio/x-minipsf audio/x-ml audio/x-mmdc audio/x-mo3
    audio/x-mod audio/x-mpegurl audio/x-mptm audio/x-nintendo-ds-strm audio/x-nintendo-ds-strm-ffta2 audio/x-np
    audio/x-ntk audio/x-okt audio/x-prun audio/x-psf audio/x-psm audio/x-pt3 audio/x-ptk audio/x-s3m audio/x-sc2
    audio/x-sc68 audio/x-scl audio/x-scn audio/x-sidmon audio/x-sndh audio/x-sonix audio/x-soundmon audio/x-spc
    audio/x-spl audio/x-stk audio/x-stm audio/x-sun audio/x-sunvox audio/x-symmod audio/x-tfmx audio/x-tfmx-st
    audio/x-umx audio/x-v2m audio/x-vgm audio/x-vorbis+ogg audio/x-xad audio/x-xm
  )

  bold=$'\e[1m'
  bold_reset=$'\e[0m'

  invalid_value_inserted_message="please insert a valid value"

  bzr2_name="BZR Player"
  bzr2_pkgname="bzr-player"
  bzr2_wineprefix_dir_unversioned="$HOME/.$bzr2_pkgname"
  bzr2_exe_filename="BZRPlayer.exe"
  bzr2_launcher_filename="$bzr2_pkgname.sh"
  bzr2_xml_filename="x-$bzr2_pkgname.xml"
  bzr2_desktop_filename="$bzr2_pkgname.desktop"

  check_requirements
  set_temp_dir

  icon_sizes=(16 32 48 64 128 256 512)
  icons_hicolor_path="/usr/share/icons/hicolor"
  mime_dir_system="/usr/share/mime"
  mime_packages_dir_system="$mime_dir_system/packages"
  desktop_apps_dir_system="/usr/share/applications"

  get_action

  if [ "$action" == "setup" ]; then
    setup
  else
    remove
  fi
}

setup() {
  check_bzr2_latest_version
  get_bzr2_version

  bzr2_wineprefix_dir="$bzr2_wineprefix_dir_unversioned/$bzr2_version"
  bzr2_dir="$bzr2_wineprefix_dir/drive_c/Program Files/$bzr2_name"
  bzr2_exe="$bzr2_dir/$bzr2_exe_filename"
  bzr2_launcher="$bzr2_wineprefix_dir/$bzr2_launcher_filename"
  bzr2_desktop="$bzr2_wineprefix_dir/$bzr2_desktop_filename"

  if is_ge_than "$bzr2_version" "2.0.71"; then
    bzr2_icon="$bzr2_dir/data/resources/icon.png"
  else
    bzr2_icon="$bzr2_dir/resources/icon.png"
  fi

  if [ -f "$bzr2_exe" ]; then
    is_already_installed=true

    echo -e "\nBZR2 ${bold}$bzr2_version${bold_reset} installation detected at ${bold}$bzr2_wineprefix_dir${bold_reset}"
    get_force_reinstall
  else
    is_already_installed=false
    force_reinstall="$force_reinstall_default"
  fi

  if ! $is_already_installed || [ "$force_reinstall" = y ]; then

    if is_ge_than "$bzr2_version" "2.0.80"; then
      bzr2_zip_filename="bzr-player-$bzr2_version-win64.zip"
    elif is_ge_than "$bzr2_version" "2.0.78"; then
      bzr2_zip_filename="BZR-Player-$bzr2_version-win64.zip"
    else
      bzr2_zip_filename="BZR-Player-$bzr2_version.zip"
    fi

    download_bzr2

    if [ "$is_zip_downloaded" == false ]; then
      get_bzr2_local_zip_dir
    fi
  fi

  get_dpi

  if set_bzr2_xml; then
    get_mime_types_association
  fi

  if ! $is_already_installed || [ "$force_reinstall" = y ]; then
    if [ "$force_reinstall" = y ]; then
      sudo -u "$USER" rm -rf "$bzr2_wineprefix_dir"
    fi

    echo
    setup_bzr2
  fi

  setup_dpi
  setup_launcher_script
  setup_icon
  setup_desktop_entry

  if [ "$mime_types_association" = y ]; then
    setup_mime_types
  fi

  echo -e "\nAll done, enjoy ${bold}BZR2 $bzr2_version${bold_reset}!"
}

check_requirements() {
  for requirement in "${requirements[@]}"; do
    if ! type "$requirement" &>/dev/null; then
      echo -e "\nplease install ${bold}$requirement${bold_reset}"
      exit 1
    fi
  done
}

set_temp_dir() {
  for tmp_dir in "$XDG_RUNTIME_DIR" "$TMPDIR" "$(dirname "$(mktemp -u --tmpdir)")" "/tmp" "/var/tmp" "/var/cache"; do
    if [ -w "$tmp_dir" ]; then
      temp_dir="$tmp_dir"
      break
    fi
  done

  if [ -z "$temp_dir" ]; then
    temp_dir="$HOME"
    echo -e "\nunable to find a writable temp directory: ${bold}$temp_dir${bold_reset} will be used"
  fi
}

show_message_and_read_input() {
  if [ -z "$2" ]; then
    local message=$'\n'"$1: "
  else
    local message=$'\n'"$1 (${bold}$2${bold_reset}): "
  fi

  IFS= read -r -p "$message" input
  if [ -n "$input" ]; then
    echo "$input"
  else
    echo "$2"
  fi
}

get_action() {
  while :; do
    local input
    input=$(show_message_and_read_input "do you want to ${bold}setup${bold_reset} or ${bold}remove${bold_reset} \
BZR2?" "setup")

    case $input in
    setup | remove)
      break
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  action="$input"
}

validate_version() {
  # matches 2. >=0 AND <=9 . >=19 AND <=999
  local versioning_pattern="^2\.[0-9]\.{1}+(19|[2-9][0-9]|[1-9][0-9]{2})$"

  if [[ "$1" =~ $versioning_pattern ]]; then
    return 0
  fi
  return 1
}

check_bzr2_latest_version() {
  while :; do
    echo -en "\nchecking latest version online... "

    set +e
    local latest_version
    latest_version=$(wget -qO- "$url_latest_version" | grep '"tag_name":' | sed 's/.*"tag_name": "//;s/",.*//')
    local wget_result=$?
    set -e

    if [ $wget_result -eq 0 ] && validate_version "$latest_version"; then
      echo "${bold}$latest_version${bold_reset} found"
      bzr2_version=$latest_version
      break
    fi

    echo "FAIL"
    while :; do
      local input
      input=$(show_message_and_read_input "do you want to ${bold}retry${bold_reset} online latest version check \
or ${bold}skip${bold_reset} it?" "retry")

      case $input in
      retry) break ;;

      skip)
        break 2
        ;;
      *)
        echo -e "\n$invalid_value_inserted_message"
        ;;
      esac
    done
  done
}

get_bzr2_version() {

  while :; do
    local input
    input=$(show_message_and_read_input "select the version to manage" "$bzr2_version")

    if validate_version "$input"; then
      break
    fi

    echo -e "\n$invalid_value_inserted_message"
  done

  bzr2_version="${input,,}"
}

is_ge_than() {
  printf '%s\n' "$2" "$1" | sort -C -V
}

get_force_reinstall() {
  while :; do
    local input
    input=$(show_message_and_read_input "force reinstall (fresh installation, does not keep settings), \
otherwise only the configuration will be performed" ${force_reinstall_default})

    case $input in
    y | n)
      break
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  force_reinstall="$input"
}

bzr2_zip_sanity_check() {
  echo -n "sanity check... "

  if unzip -tq "$1" >/dev/null 2>&1 && [ "$(unzip -l "$1" | grep -c "$bzr2_exe_filename")" -eq 1 ] >/dev/null 2>&1; then
    echo "OK"
    return 0
  else
    echo -n "FAIL"
    return 1
  fi
}

download_bzr2() {
  echo -e "\nrelease archive will be downloaded to ${bold}$temp_dir${bold_reset}"

  while :; do
    for ((i = 0; i < ${#urls_download[@]}; i++)); do
      local url_download="${urls_download[$i]}"

      case $i in
      0) local query_string="$bzr2_version" ;;
      1) local query_string="/$bzr2_version/$bzr2_zip_filename" ;;
      2) local query_string="/$bzr2_zip_filename" ;;
      esac

      echo -en "\ndownloading ${bold}$bzr2_zip_filename${bold_reset} from $url_download$query_string... "

      set +e
      wget -q --tries=$download_tries -P "$temp_dir" -O "$temp_dir/$bzr2_zip_filename" \
        "$url_download$query_string"

      local wget_result=$?
      set -e

      bzr2_zip="$temp_dir/$bzr2_zip_filename"

      if [ $wget_result -eq 0 ] && unzip -tq "$bzr2_zip" >/dev/null 2>&1; then
        set +e
        bzr2_zip_sanity_check "$bzr2_zip"
        local is_zip_sane=$?
        set -e

        if [ $is_zip_sane -eq 0 ]; then
          is_zip_downloaded=true
          return
        fi
      else
        echo -n "FAIL"
      fi
    done

    echo -e "\n\nunable to download the release archive"

    while :; do
      local input
      input=$(show_message_and_read_input "do you want to ${bold}retry${bold_reset} downloading it \
or select a ${bold}local${bold_reset} release archive?" "retry")

      case $input in
      retry) break ;;

      local)
        is_zip_downloaded=false
        break 2
        ;;
      *)
        echo -e "\n$invalid_value_inserted_message"
        ;;
      esac
    done
  done
}

get_bzr2_local_zip_dir() {
  while :; do
    local bzr2_zip_dir
    bzr2_zip_dir=$(show_message_and_read_input "specify the release archive folder path" \
      "$(realpath -s "$bzr2_zip_dir_default")")

    bzr2_zip="$bzr2_zip_dir/$bzr2_zip_filename"

    if [ -f "$bzr2_zip" ]; then
      echo -en "\nrelease archive ${bold}$bzr2_zip${bold_reset} for version \
${bold}$bzr2_version${bold_reset} found... "

      set +e
      bzr2_zip_sanity_check "$bzr2_zip"
      local is_zip_sane=$?
      set -e

      if [ "$is_zip_sane" -eq 0 ]; then
        break
      fi
    fi

    echo -e "\nvalid ${bold}$bzr2_zip${bold_reset} file not found... $invalid_value_inserted_message"
  done
}

get_dpi() {
  local dpi_pattern="^[1-9][0-9]*$"

  while :; do
    local input
    input=$(show_message_and_read_input "select the DPI, ${bold}auto${bold_reset} for using the current from xorg \
screen 0 or ${bold}default${bold_reset} for using the default one" "auto")

    case $input in
    default | auto)
      break
      ;;
    *)
      if ! [[ "$input" =~ $dpi_pattern ]]; then
        echo -e "\n$invalid_value_inserted_message"
      else
        break
      fi
      ;;
    esac
  done

  dpi="$input"
}

get_size_of_longer_array_entry() {
  local array=("$@")
  local longer_size=-1

  for entry in "${array[@]}"; do
    local length=${#entry}
    ((length > longer_size)) && longer_size=$length
  done

  echo "$longer_size"
}

set_bzr2_xml() {
  bzr2_xml="$(realpath -s "$bzr2_xml_dir_default")/$bzr2_xml_filename"

  if [ ! -f "$bzr2_xml" ]; then
    echo -e "\nfile ${bold}$bzr2_xml${bold_reset} not found: ${bold}$bzr2_xml_filename${bold_reset} will be downloaded to ${bold}$temp_dir${bold_reset}:"
    echo -en "downloading ${bold}$bzr2_xml_filename${bold_reset} from $bzr2_xml_url... "
    set +e
    wget -q --tries=$download_tries -P "$temp_dir" -O "$temp_dir/$bzr2_xml_filename" \
      "$bzr2_xml_url"

    local wget_result=$?
    set -e

    if [ $wget_result -eq 0 ]; then
      bzr2_xml="$temp_dir/$bzr2_xml_filename"
      echo "OK"
      return 0
    fi

    echo -e "FAIL: ${bold}MIME types association will be skipped${bold_reset}"
    return 1
  fi
}

get_mime_types_association() {
  while :; do
    local input
    input=$(show_message_and_read_input "associate to all suppported MIME types (enter ${bold}list${bold_reset} \
for listing all)" "y")

    case $input in
    y | n)
      break
      ;;

    list)
      local mime_length_max
      mime_length_max=$(get_size_of_longer_array_entry "${mime_types_supported[@]}")
      local mime_comments=()
      local mime_patterns=()
      local bzr2_xml_content
      bzr2_xml_content=$(cat "$bzr2_xml_filename")

      for mime_type in "${mime_types_supported[@]}"; do
        local sed_pattern="\|<mime-type type=\"$mime_type\">| , \|</mime-type>|{p; \|</mime-type>|q}"
        local mime_single
        mime_single=$(echo "$bzr2_xml_content" | sed -n "$sed_pattern")

        if [ -z "$mime_single" ]; then
          mime_single=$(sed -n "$sed_pattern" "$mime_packages_dir_system/freedesktop.org.xml")
        fi

        mime_comments+=("$(echo "$mime_single" | grep "<comment>" | sed 's:<comment>::;s:</comment>::;s:    ::' |
          sed 's/^ *//g')")
        local mime_pattern
        mime_pattern=$(echo "$mime_single" | grep "<glob " | sed -E -e 's:<glob ::g' -e 's: weight="[0-9]+"::g' \
          -e 's:pattern="::g' -e 's:"/>::g')
        local mime_pattern_split=()

        while read -r line; do
          mime_pattern_split+=("$line")
        done <<<"$mime_pattern"

        local mime_comment_length_max
        mime_comment_length_max=$(get_size_of_longer_array_entry "${mime_comments[@]}")
        local delimiter="  "
        local padding_size=$((mime_length_max + mime_comment_length_max + ${#delimiter} + ${#delimiter}))
        local padding_string=""

        for ((i = 0; i < "$padding_size"; i++)); do
          padding_string+=" "
        done

        local max_patterns_per_chunk=4
        local mime_pattern_chunks=()

        for ((i = 0; i < ${#mime_pattern_split[@]}; i++)); do
          local div=$((i / max_patterns_per_chunk))
          if [ $div -gt 0 ] && [ $((i % max_patterns_per_chunk)) -eq 0 ]; then
            mime_pattern_chunks[div]=${mime_pattern_chunks[div]}$padding_string"["${mime_pattern_split[$i]}]
          else
            if [ "$i" -eq 0 ]; then
              mime_pattern_chunks[div]="${mime_pattern_chunks[div]}[${mime_pattern_split[$i]}]"
            else
              mime_pattern_chunks[div]="${mime_pattern_chunks[div]}[${mime_pattern_split[$i]}]"
            fi
          fi
        done

        mime_pattern=""

        for ((i = 0; i < ${#mime_pattern_chunks[@]}; i++)); do
          mime_pattern="$mime_pattern${mime_pattern_chunks[$i]}"$'\n'
        done

        mime_pattern=$(sed -z 's/.$//' <<<"$mime_pattern")
        mime_patterns+=("$mime_pattern")
      done

      echo -e "\nBZR2 supports following MIME types:\n"

      for i in "${!mime_types_supported[@]}"; do
        printf "%${mime_length_max}s$delimiter%${mime_comment_length_max}s$delimiter%s\n" "${mime_types_supported[$i]}" \
          "${mime_comments[$i]}" "${mime_patterns[$i]}"
      done
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  mime_types_association="$input"
}

setup_bzr2() {
  sudo -u "$USER" mkdir -p "$bzr2_dir"
  sudo -u "$USER" unzip -oq "$bzr2_zip" -d "$bzr2_dir"

  # disable wine crash dialog (winetricks nocrashdialog)
  sudo -u "$USER" WINEDEBUG=-all WINEPREFIX="$bzr2_wineprefix_dir" WINEDLLOVERRIDES="mscoree=" \
    wine reg add "HKEY_CURRENT_USER\Software\Wine\WineDbg" /v ShowCrashDialog /t REG_DWORD /d 0 /f

  # disable wine debugger (winetricks autostart_winedbg=disabled)
  sudo -u "$USER" WINEDEBUG=-all WINEPREFIX="$bzr2_wineprefix_dir" WINEDLLOVERRIDES="mscoree=" \
    wine reg add "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\AeDebug" \
    /v Debugger /t REG_SZ /d "false" /f
}

setup_dpi() {
  local dpi_to_set

  case "$dpi" in
  default)
    dpi_to_set=96
    ;;

  auto)
    dpi_to_set=$(sudo -u "$USER" xrdb -query | grep dpi | sed 's/.*://;s/^[[:space:]]*//')
    if [ -z "$dpi_to_set" ]; then
      echo -e "\nunable to retrieve the screen ${bold}DPI${bold_reset}: the ${bold}default${bold_reset} will be used \
in wine"
      return
    fi
    ;;

  *)
    dpi_to_set=$dpi
    ;;
  esac

  echo -e "\nsetting wine ${bold}DPI${bold_reset} to ${bold}$dpi_to_set${bold_reset}\n"

  dpi_to_set='0x'$(printf '%x\n' "$dpi_to_set")

  sudo -u "$USER" WINEDEBUG=-all WINEPREFIX="$bzr2_wineprefix_dir" WINEDLLOVERRIDES="mscoree=" \
    wine reg add "HKEY_CURRENT_USER\Control Panel\Desktop" /v LogPixels /t REG_DWORD /d "$dpi_to_set" /f

  sudo -u "$USER" WINEDEBUG=-all WINEPREFIX="$bzr2_wineprefix_dir" WINEDLLOVERRIDES="mscoree=" \
    wine reg add "HKEY_CURRENT_USER\Software\Wine\Fonts" /v LogPixels /t REG_DWORD /d "$dpi_to_set" /f

  sudo -u "$USER" WINEDEBUG=-all WINEPREFIX="$bzr2_wineprefix_dir" WINEDLLOVERRIDES="mscoree=" \
    wine reg add "HKEY_CURRENT_CONFIG\Software\Fonts" /v LogPixels /t REG_DWORD /d "$dpi_to_set" /f
}

setup_launcher_script() {

  bzr2_launcher_content=$(
    cat <<EOF
#!/bin/bash
#
# NAME
#     bzr-player.sh - BZR Player 2.x (BZR2) launcher
#
# SYNOPSIS
#     ./bzr-player.sh [target(s)]
#
# EXAMPLES
#     ./bzr-player.sh
#         run BZR2
#
#     ./bzr-player.sh file1 file2 dir1 dir2
#         run BZR2 with selected files and/or directories as arguments
#

set -e

export WINEPREFIX="$bzr2_wineprefix_dir"
export WINEDLLOVERRIDES="mscoree=" # disable mono

wine "$bzr2_exe"
EOF
  )

  bzr2_launcher_content=$(echo "$bzr2_launcher_content" | sed '$s/$/ "$@" \&/')
  sudo -u "$USER" bash -c "echo '$bzr2_launcher_content' > '$bzr2_launcher'"
  sudo -u "$USER" chmod +x "$bzr2_launcher"
}

setup_icon() {
  if [ -f "$bzr2_icon" ]; then
    echo -e "\ninstalling ${bold}icons${bold_reset}"

    for size in "${icon_sizes[@]}"; do
      xdg-icon-resource install --noupdate --novendor --context apps --mode system --size "${size}" "$bzr2_icon" "$bzr2_pkgname"
    done

    xdg-icon-resource forceupdate --theme hicolor

    if type gtk-update-icon-cache &>/dev/null; then
      echo
      gtk-update-icon-cache -t -f "$icons_hicolor_path"
    fi
  else
    echo -e "\nskipping ${bold}icons${bold_reset} installation"
  fi
}

setup_desktop_entry() {
  echo -e "\ninstalling ${bold}desktop menu entry${bold_reset}"
  local desktop_entry_mime_types=""

  for mime_type in "${mime_types_supported[@]}"; do
    desktop_entry_mime_types="$desktop_entry_mime_types$mime_type;"
  done

  bzr2_desktop_content=$(
    cat <<EOF
[Desktop Entry]
Type=Application
Name=$bzr2_name
GenericName=Audio player
Comment=Audio player supporting a wide array of multi-platform exotic file formats
Exec=$bzr2_launcher %U
Icon=$bzr2_pkgname
Terminal=false
StartupNotify=false
Categories=AudioVideo;Audio;Music;Player;
MimeType=$desktop_entry_mime_types

EOF
  )

  sudo -u "$USER" bash -c "echo '$bzr2_desktop_content' > '$bzr2_desktop'"
  xdg-desktop-menu install --novendor --mode system "$bzr2_desktop"
}

setup_mime_types() {
  echo -e "\nassociating to all supported ${bold}MIME types${bold_reset}"

  install -Dm644 "$bzr2_xml" "$mime_packages_dir_system"
  sudo -u "$USER" xdg-mime default $bzr2_desktop_filename "${mime_types_supported[@]}"
  update-mime-database "$mime_dir_system"
  update-desktop-database "$desktop_apps_dir_system"
}

remove() {
  local nothing_to_remove=true

  if [ -d "$bzr2_wineprefix_dir_unversioned" ]; then
    local targets=()
    mapfile -t targets \
      < <(sudo -u "$USER" find "$bzr2_wineprefix_dir_unversioned" -maxdepth 1 -path "$bzr2_wineprefix_dir_unversioned*" -type d -print | sort -V)

    for target in "${targets[@]}"; do
      if [ -d "$target" ]; then
        while :; do
          local input
          input=$(show_message_and_read_input "remove ${bold}$target${bold_reset} ?" "y")

          case $input in
          y)
            nothing_to_remove=false
            sudo -u "$USER" rm -rf "$target"
            break
            ;;
          n)
            break
            ;;
          *)
            echo -e "\n$invalid_value_inserted_message"
            ;;
          esac
        done
      fi
    done
  fi

  while :; do
    local input
    input=$(show_message_and_read_input "remove ${bold}desktop menu entry${bold_reset} ?" "y")

    case $input in
    y)
      nothing_to_remove=false
      xdg-desktop-menu uninstall --mode system "$bzr2_desktop_filename"
      break
      ;;
    n)
      break
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  while :; do
    local input
    input=$(show_message_and_read_input "remove ${bold}icons${bold_reset} ?" "y")

    case $input in
    y)
      nothing_to_remove=false
      for size in "${icon_sizes[@]}"; do
        xdg-icon-resource uninstall --noupdate --context apps --mode system --size "${size}" "$bzr2_pkgname"
      done

      xdg-icon-resource forceupdate --theme hicolor

      if type gtk-update-icon-cache &>/dev/null; then
        echo
        gtk-update-icon-cache -t -f "$icons_hicolor_path"
      fi

      break
      ;;
    n)
      break
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  while :; do
    local input
    input=$(show_message_and_read_input "remove associated ${bold}MIME types${bold_reset} ?" "y")

    case $input in
    y)
      nothing_to_remove=false
      rm -f "$mime_packages_dir_system/$bzr2_xml_filename"
      update-mime-database "$mime_dir_system"
      update-desktop-database "$desktop_apps_dir_system"
      break
      ;;
    n)
      break
      ;;
    *)
      echo -e "\n$invalid_value_inserted_message"
      ;;
    esac
  done

  if [ "$nothing_to_remove" == true ]; then
    echo -e "\nnothing to remove"
  else
    echo -e "\nAll done"
  fi
}

main "$@" exit
