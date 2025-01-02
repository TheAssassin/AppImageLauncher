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

# we need a "docker-container" type builder to make use of all the buildx features regarding caching and multi-arch
# support
builder_name="appimagelauncher-builder"
if ! docker buildx inspect "$builder_name" &>/dev/null; then
    echo "Docker builder $builder_name not found, creating"
    docker buildx create --name="$builder_name" --driver=docker-container --bootstrap
else
    echo "Using existing Docker builder $builder_name found"
fi

image=ghcr.io/theassassin/appimagelauncher-build
branch="$(git rev-parse --abbrev-ref HEAD)"

docker_command=(
    docker buildx build
    --builder "$builder_name"
    --load  # --output=type=docker
    --pull
    --platform "$DOCKER_PLATFORM"
    --build-arg DOCKER_PLATFORM="$DOCKER_PLATFORM"

    # cache from the current branch's image
    --cache-from type=registry,ref="$image:$branch"

    # we can always cache from the master branch's image
    --cache-from type=registry,ref="$image:master"

    --tag "$image:$branch"
)

# if we are building on GitHub actions, we can also push the resulting image
if [[ "${GITHUB_ACTIONS:-}" != "" ]]; then
    echo "Going to push built image"
    docker_command+=(
        --cache-to type=inline
        --output type=registry,ref="$image:$branch"
    )
fi

docker_command+=(
   "$this_dir"
)

# using inline cache to speed up builds by fetching the image from the GitHub registry first
# this should speed up local builds as well
# if the image hasn't changed, this should be a no-op
"${docker_command[@]}"
