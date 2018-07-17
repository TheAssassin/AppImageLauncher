#! /bin/bash

set -x
set -e

cd $(readlink -f $(dirname "$0"))


IMAGE=appimagelauncher-build:bionic
DOCKERFILE=Dockerfile.build-bionic

if [ "$ARCH" == "i386" ]; then
    IMAGE=appimagelauncher-build:bionic-i386-cross
    DOCKERFILE=Dockerfile.build-bionic-i386-cross
fi

docker build -t "$IMAGE" -f "$DOCKERFILE" .

docker run -e ARCH -e TRAVIS_BUILD_NUMBER --rm -it -v $(readlink -f ..):/ws "$IMAGE" bash -xc "export BIONIC=true && cd /ws && travis/travis-build.sh"
