#! /bin/bash

if [[ "$DIST" == "" ]] || [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... DIST=... bash $0"
    exit 1
fi

set -x
set -e

cd "$(readlink -f "$(dirname "$0")")"

# needed to keep user ID in and outside Docker in sync to be able to write to workspace directory
uid="$(id -u)"
image=quay.io/appimagelauncher/build:"$DIST"-"$ARCH"-uid"$uid"
dockerfile=Dockerfile."$DIST"-"$ARCH"

if [[ "$BUILD_LITE" != "" ]]; then
    image="$image"-lite
    dockerfile="$dockerfile"-lite
fi

if [ ! -f "$dockerfile" ]; then
    echo "Error: $dockerfile could not be found"
    exit 1
fi

# speed up build by pulling last built image from quay.io and building the docker file using the old image as a base
docker pull "$image" || true
# if the image hasn't changed, this should be a no-op
docker build --cache-from "$image" -t "$image" -f "$dockerfile" --build-arg UID="$uid" .

# push built image as cache for future builds to registry
# we can do that immediately once the image has been built successfully; if its definition ever changes it will be
# rebuilt anyway
# credentials shall only be available on (protected) master branch
if [[ "$DOCKER_USERNAME" != "" ]]; then
    echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin quay.io
    docker push "$image"
fi

# figure out which build script to use
if [[ "$BUILD_LITE" == "" ]]; then
    build_script=build.sh
else
    build_script=build-lite.sh
fi

DOCKER_OPTS=()
# fix for https://stackoverflow.com/questions/51195528/rcc-error-in-resource-qrc-cannot-find-file-png
if [ "$CI" != "" ]; then
    DOCKER_OPTS+=("--security-opt" "seccomp:unconfined")
fi

# only if there's more than 3G of free space in RAM, we can build in a RAM disk
if [[ "$(free -m  | grep "Mem:" | awk '{print $4}')" -gt 3072 ]]; then
    echo "Host system has enough free memory -> building in RAM disk"
    DOCKER_OPTS+=("--tmpfs /docker-ramdisk:exec,mode=777")
else
    echo "Host system does not have enough free memory -> building on regular disk"
fi

# run build
docker run -e DIST -e ARCH -e GITHUB_RUN_NUMBER -e GITHUB_RUN_ID --rm -i "${DOCKER_OPTS[@]}" -v "$(readlink -f ..):/ws" \
     "$image" \
     bash -xc "export CI=1 && cd /ws && source ci/$build_script"

