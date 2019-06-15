#! /bin/bash

if [ "$1" == "" ]; then
    echo "Usage: $0 <distro>"
    exit 1
fi

set -x
set -e

export DOCKER_DIST="$1"

cd $(readlink -f $(dirname "$0"))

IMAGE=appimagelauncher-build:"$DOCKER_DIST"
DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"

if [ ! -f "$DOCKERFILE" ]; then
    echo "Error: $DOCKERFILE could not be found"
    exit 1
fi

if [ "$ARCH" == "i386" ]; then
    IMAGE=appimagelauncher-build:"$DOCKER_DIST"-i386-cross
    DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"-i386-cross
fi

docker build -t "$IMAGE" -f "$DOCKERFILE" .

docker run -e ARCH -e TRAVIS_BUILD_NUMBER --rm -it -v $(readlink -f ..):/ws "$IMAGE" \
    bash -xc "export CI=1 && export DOCKER_DIST=\"$DOCKER_DIST\" && cd /ws && source travis/travis-build.sh"
