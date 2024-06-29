#!/bin/bash

CONTAINER_NAME="toolchain"
IMAGE_NAME="toolchain:latest"
BUILD_TYPE="Release"
BUILD_DIR=$(echo "/home/devel/bzr2/cmake-build-docker-$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
COMMAND="su devel -c 'mkdir -p $BUILD_DIR && cd $BUILD_DIR && i686-w64-mingw32-cmake \
-DCMAKE_PREFIX_PATH=/usr/i686-w64-mingw32 -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCPACK_EXECUTABLE=/usr/bin/cpack \
-G Ninja .. && ninja'"

# TODO allow external profile set

if [ -z "$(docker images -q "$IMAGE_NAME" 2>/dev/null)" ]; then
  docker build -t "$IMAGE_NAME" .
fi

if [ "$(docker ps -a -q -f name="$CONTAINER_NAME")" ]; then
  docker rm -f "$CONTAINER_NAME"
fi

docker run --rm -v "$(realpath ../):/home/devel/bzr2" --name "$CONTAINER_NAME" "$IMAGE_NAME" bash -c "$COMMAND"
