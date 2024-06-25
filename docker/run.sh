#!/bin/bash

CONTAINER_NAME="toolchain"
IMAGE_NAME="toolchain:latest"
BUILD_TYPE="Release"
BUILD_DIR=$(echo "/home/devel/bzr2/cmake-build-docker-$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
COMMAND="su devel -c 'mkdir -p $BUILD_DIR && cd $BUILD_DIR && i686-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G Ninja .. && ninja'"
#COMMAND="sleep 1000"
# TODO stop container if running (and delete it?)
# TODO x86_64-w64-mingw32-cmake
# TODO avoid updating cmake if cmake local or repo ver is higher than 3.29

if [ -z "$(docker images -q $IMAGE_NAME 2>/dev/null)" ]; then
  docker build -t $IMAGE_NAME .
fi

docker run --rm -v "$(realpath ../):/home/devel/bzr2" --name "$CONTAINER_NAME" "$IMAGE_NAME" bash -c "$COMMAND"
