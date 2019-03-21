#! /bin/bash

set -x
set -e

cd $(readlink -f $(dirname "$0"))


IMAGE=appimagelauncher-build:cosmic
DOCKERFILE=Dockerfile.build-cosmic

if [ "$ARCH" == "i386" ]; then
    IMAGE=appimagelauncher-build:cosmic-i386-cross
    DOCKERFILE=Dockerfile.build-cosmic-i386-cross
fi

docker build -t "$IMAGE" -f "$DOCKERFILE" .

docker run -e ARCH -e TRAVIS_BUILD_NUMBER --rm -it -v $(readlink -f ..):/ws "$IMAGE" bash -xc "export COSMIC=true && cd /ws && travis/travis-build.sh"
