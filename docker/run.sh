#!/bin/bash

CONTAINER_NAME="toolchain"
IMAGE_NAME="toolchain:latest"

PROJECT_DIR_CONTAINER="/home/devel/bzr2"
PROJECT_DIR_HOST="$(realpath ../)"

if [ -z "$BUILD_TYPE" ]; then
  BUILD_TYPE="Debug"
fi

BUILD_TYPE_LOWCASE="$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

BUILD_DIR_CONTAINER="$PROJECT_DIR_CONTAINER/cmake-build-docker-$BUILD_TYPE_LOWCASE"
BUILD_DIR_HOST="$PROJECT_DIR_HOST/cmake-build-docker-$BUILD_TYPE_LOWCASE"

# TODO allow external profile set

if [ -z "$(docker images -q "$IMAGE_NAME" 2>/dev/null)" ]; then
  docker build -t "$IMAGE_NAME" .
fi

if [ "$(docker ps -a -q -f name="$CONTAINER_NAME")" ]; then
  docker rm -f "$CONTAINER_NAME"
fi

run_docker_build() {
  local COMMAND_CONTAINER="su devel -c 'mkdir -p \"$BUILD_DIR_CONTAINER\" && cd \"$BUILD_DIR_CONTAINER\" && \
        i686-w64-mingw32-cmake -DCMAKE_PREFIX_PATH=/usr/i686-w64-mingw32 -DCMAKE_BUILD_TYPE=\"$BUILD_TYPE\" \
        -DCPACK_EXECUTABLE=/usr/bin/cpack -G Ninja .. && ninja'"

  docker run --rm -v "$PROJECT_DIR_HOST:$PROJECT_DIR_CONTAINER" --name "$CONTAINER_NAME" \
    $IMAGE_NAME bash -c "$COMMAND_CONTAINER"
}

if run_docker_build && [ "$BUILD_TYPE" == "Debug" ]; then
  for tmp_dir in "$XDG_RUNTIME_DIR" "$TMPDIR" "$(dirname "$(mktemp -u --tmpdir)")" "/tmp" "/var/tmp" "/var/cache"; do
    if [ -w "$tmp_dir" ]; then
      break
    fi
  done

  #export WINEDEBUG=warn #TODO
  export WINEPREFIX="$tmp_dir/bzr2_$BUILD_TYPE_LOWCASE"
  export WINEDLLOVERRIDES="mscoree="
  wine "$BUILD_DIR_HOST/output/BZRPlayer.exe" "$@"
fi
