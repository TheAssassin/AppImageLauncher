#! /bin/bash

if [[ "$DIST" == "" ]] || [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... DIST=... bash $0"
    exit 1
fi

set -x
set -e

# the other script sources this script, therefore we have to support that use case
if [[ "${BASH_SOURCE[*]}" != "" ]]; then
    this_dir="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
else
    this_dir="$(readlink -f "$(dirname "$0")")"
fi

# needed to keep user ID in and outside Docker in sync to be able to write to workspace directory
image=quay.io/appimagelauncher/build:"$DIST"-"$ARCH"
dockerfile="$this_dir"/Dockerfile."$DIST"-"$ARCH"

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
docker build --pull --cache-from "$image" -t "$image" -f "$dockerfile" "$this_dir"

# push built image as cache for future builds to registry
# we can do that immediately once the image has been built successfully; if its definition ever changes it will be
# rebuilt anyway
# credentials shall only be available on (protected) master branch
set +x
if [[ "$DOCKER_USERNAME" != "" ]]; then
    echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin quay.io
    docker push "$image"
fi
