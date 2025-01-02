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
# append platform to Docker image tag since we can't push separate (multi-arch) images to the same name without
# creating the manifest manually
# see https://github.com/docker/build-push-action/issues/671
platform_suffix="$(echo -n "$DOCKER_PLATFORM" | tr -c 'A-Za-z0-9-' '_')"

current_branch_tag="${image}:${branch}_${platform_suffix}"
master_branch_tag="${image}:master_${platform_suffix}"

docker_command=(
    docker buildx build
    --builder "$builder_name"
    --load  # --output=type=docker
    --pull
    --platform "$DOCKER_PLATFORM"
    --build-arg DOCKER_PLATFORM="$DOCKER_PLATFORM"

    # cache from the current branch's image
    --cache-from type=registry,ref="$current_branch_tag"

    # we can always cache from the master branch's image
    --cache-from type=registry,ref="$master_branch_tag"

    --tag "$image:$branch"
)

# if we are building on GitHub actions, we can also push the resulting image
if [[ "${GITHUB_ACTIONS:-}" != "" ]]; then
    echo "Going to push built image"
    docker_command+=(
        --cache-to type=inline
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
