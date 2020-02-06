#! /bin/bash

if [ "$1" == "" ]; then
    echo "Usage: $0 <distro>"
    exit 1
fi

set -x
set -e

export DOCKER_DIST="$1"

cd $(readlink -f $(dirname "$0"))

IMAGE=quay.io/appimagelauncher-build/"$DOCKER_DIST"
DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"

if [ "$ARCH" == "x86_64" ]; then
    # no action required
    true
elif [ "$ARCH" == "i386" ]; then
    IMAGE="$IMAGE"-i386-cross
    DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"-i386-cross
elif [ "$ARCH" == "arm64" ]; then
    IMAGE="$IMAGE"-arm64
    DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"-arm64
elif [ "$ARCH" == "armhf" ]; then
    IMAGE="$IMAGE"-armhf-cross
    DOCKERFILE=Dockerfile.build-"$DOCKER_DIST"-armhf-cross
else
    echo "Unknown architecture: $ARCH"
    exit 1
fi

if [ ! -f "$DOCKERFILE" ]; then
    echo "Error: $DOCKERFILE could not be found"
    exit 1
fi

# speed up build by pulling last built image from quay.io and building the docker file using the old image as a base
docker pull "$IMAGE" || true
# if the image hasn't changed, this should be a no-op
IMAGE="$IMAGE":latest
docker build --cache-from "$IMAGE" -t "$IMAGE" -f "$DOCKERFILE" .

if [[ "$BUILD_LITE" == "" ]]; then
    build_script=travis-build.sh
else
    build_script=build-lite.sh
fi

# push built image as cache for future builds to registry
# we can do that immediately once the image has been built successfully; if its definition ever changes it will be
# rebuilt anyway
# credentials shall only be available on (protected) master branch
if [[ "$DOCKER_USERNAME" != "" ]]; then
    echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin quay.io
    docker push "$IMAGE"
fi

DOCKER_OPTS=()
# fix for https://stackoverflow.com/questions/51195528/rcc-error-in-resource-qrc-cannot-find-file-png
if [ "$TRAVIS" != "" ]; then
    DOCKER_OPTS+=("--security-opt" "seccomp:unconfined")
fi

# run build
docker run -e ARCH -e TRAVIS_BUILD_NUMBER --rm -i "${DOCKER_OPTS[@]}" -v $(readlink -f ..):/ws "$IMAGE" \
    bash -xc "export CI=1 && export DEBIAN_DIST=\"$DOCKER_DIST\" && cd /ws && source travis/$build_script"

