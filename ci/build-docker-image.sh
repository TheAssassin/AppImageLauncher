#! /bin/bash

if [[ "$DOCKER_PLATFORM" == "" ]]; then
    echo "Usage: env $DOCKER_PLATFORM=... bash $0"
    exit 1
fi

set -euo pipefail

# the other script sources this script, therefore we have to support that use case
if [[ "${BASH_SOURCE[*]}" != "" ]]; then
    this_dir="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
else
    this_dir="$(readlink -f "$(dirname "$0")")"
fi

image=ghcr.io/theassassin/appimagelauncher-build

if [[ "${BUILD_LITE:-}" != "" ]]; then
    image="${image}:lite"
fi

# we need a "docker-container" type builder to make use of all the buildx features regarding caching and multi-arch
# support
builder_name="appimagelauncher-builder"
if ! docker buildx inspect "$builder_name" &>/dev/null; then
    echo "Docker builder $builder_name not found, creating"
    docker buildx create --name="$builder_name" --driver=docker-container --bootstrap
else
    echo "Using existing Docker builder $builder_name found"
fi

docker_command=(
    docker buildx build
    --builder "$builder_name"
    --load  # --output=type=docker
    --pull
    --platform "$DOCKER_PLATFORM"
    --build-arg DOCKER_PLATFORM="$DOCKER_PLATFORM"

    # we can always cache from the image
    --cache-from type=registry,ref="$image"

    --tag "$image"
)

# if we are building on GitHub actions, we can also push the resulting image
if [[ "${GITHUB_CI:-}" != "" ]]; then
    echo "Going to push built image"
    docker_command+=(
        --cache-to inline
        --push
    )
fi

docker_command+=(
   "$this_dir"
)

# using inline cache to speed up builds by fetching the image from the GitHub registry first
# this should speed up local builds as well
# if the image hasn't changed, this should be a no-op
"${docker_command[@]}"
