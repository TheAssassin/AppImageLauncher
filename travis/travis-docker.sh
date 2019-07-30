#! /bin/bash

if [ "$1" == "" ]; then
    echo "Usage: $0 <distro>"
    exit 1
fi

set -x
set -e

export DOCKER_DIST="$1"

case "$DOCKER_DIST" in
    xenial|bionic)
        ;;
    *)
        echo "Error: invalid/unsupported distro: $DOCKER_DIST"
        exit 1
esac

cd $(readlink -f $(dirname "$0"))

IMAGE=appimagelauncher-build:"$DOCKER_DIST"
DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"

if [ "$BUILD_LITE" != "" ]; then
    IMAGE="$IMAGE"-lite
    DOCKERFILE="$DOCKERFILE"-lite
fi

if [ "$ARCH" == "i386" ]; then
    IMAGE="$IMAGE"-i386-cross
    DOCKERFILE="$DOCKERFILE"-i386-cross
fi

if [ ! -f "$DOCKERFILE" ]; then
    echo "Error: $DOCKERFILE could not be found"
    exit 1
fi

docker build -t "$IMAGE" -f "$DOCKERFILE" .

if [[ "$BUILD_LITE" == "" ]]; then
    build_script=travis-build.sh
else
    build_script=build-lite.sh
fi

docker run -e ARCH -e TRAVIS_BUILD_NUMBER --rm -it -v $(readlink -f ..):/ws "$IMAGE" \
    bash -xc "export CI=1 && export DEBIAN_DIST=\"$DOCKER_DIST\" && cd /ws && source travis/$build_script"
