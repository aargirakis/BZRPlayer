#!/bin/bash

set -e

CONTAINER_NAME="toolchain"
IMAGE_NAME="toolchain:latest"

PROJECT_DIR="$(realpath ../)"

if [ -z "$BUILD_TYPE" ]; then
  BUILD_TYPE="Debug"
fi

BUILD_TYPE_LOWCASE="$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"
BUILD_DIR="$PROJECT_DIR/cmake-build-$BUILD_TYPE_LOWCASE"

if [ -z "$(docker images -q "$IMAGE_NAME" 2>/dev/null)" ]; then
  docker build -t "$IMAGE_NAME" .
fi

if [ "$(docker ps -a -q -f name="$CONTAINER_NAME")" ]; then
  docker rm -f "$CONTAINER_NAME"
fi

CONTAINER_COMMAND="\
mkdir -p $BUILD_DIR && \
chown -R devel:devel $BUILD_DIR && \
cd $BUILD_DIR"

if [ "$CONFIG" == 1 ]; then
  CONTAINER_COMMAND="${CONTAINER_COMMAND} && \
                                          su devel -c '\
                                          i686-w64-mingw32-cmake \
                                          -DCMAKE_PREFIX_PATH=/usr/i686-w64-mingw32 \
                                          -DCMAKE_BUILD_TYPE=\"$BUILD_TYPE\" \
                                          -DCPACK_EXECUTABLE=/usr/bin/cpack \
                                          -DOFFLINE_MODE=\"$OFFLINE_MODE\" \
                                          -G Ninja ..'"
fi

if [ "$BUILD" == 1 ]; then
  CONTAINER_COMMAND="${CONTAINER_COMMAND} && su devel -c 'ninja'"
fi

docker run --rm -v "$PROJECT_DIR:$PROJECT_DIR" --name "$CONTAINER_NAME" \
  $IMAGE_NAME bash -c "$CONTAINER_COMMAND"

if [ "$RUN_BZR2" == 1 ]; then
  for tmp_dir in "$XDG_RUNTIME_DIR" "$TMPDIR" "$(dirname "$(mktemp -u --tmpdir)")" "/tmp" "/var/tmp" "/var/cache"; do
    if [ -w "$tmp_dir" ]; then
      break
    fi
  done

  #export WINEDEBUG=warn #TODO
  export WINEPREFIX="$tmp_dir/bzr2_$BUILD_TYPE_LOWCASE"
  export WINEDLLOVERRIDES="mscoree="

  echo "executing BZR Player 2 with WINEPREFIX: $WINEPREFIX"

  if [ ! -d "$WINEPREFIX" ]; then
    # disable wine crash dialog (winetricks nocrashdialog)
    wine reg add "HKEY_CURRENT_USER\Software\Wine\WineDbg" /v ShowCrashDialog /t REG_DWORD /d 0 /f
  fi

  wine "$BUILD_DIR/output/BZRPlayer.exe" #TODO "$@"

fi
