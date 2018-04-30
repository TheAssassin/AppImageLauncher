#! /bin/bash

set -x
set -e

cd $(readlink -f $(dirname "$0"))

IMAGE=appimagelauncher-build:bionic

docker build -t "$IMAGE" -f Dockerfile.build-bionic .

docker run --rm -it -v $(readlink -f ..):/ws "$IMAGE" bash -xc "export BIONIC=true && cd /ws && travis/travis-build.sh"
