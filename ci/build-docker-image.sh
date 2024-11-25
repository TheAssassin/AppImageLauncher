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

docker_command=(
    docker buildx build
    --pull
    --platform "$DOCKER_PLATFORM"
    --build-arg DOCKER_PLATFORM="$DOCKER_PLATFORM"

    # we can always cache from the image
    --cache-from type=registry,ref="$image"

    --tag "$image"
)

if [[ "${GITHUB_CI:-}" != "" ]]; then
    docker_command+=(
        --cache-to inline
    )
else
    echo ci
fi


# depending on the type of the build (and thus caching), we need different flags to be set


docker_command+=(
   "$this_dir"
)

# using inline cache to speed up builds by fetching the image from the GitHub registry first
# this should speed up local builds as well
# if the image hasn't changed, this should be a no-op
"${docker_command[@]}"

# push built image as cache for future builds to registry
# we can do that immediately once the image has been built successfully; if its definition ever changes it will be
# rebuilt anyway
# credentials shall only be available on (protected) master branch
set +x
if [[ "${DOCKER_USERNAME:-}" != "" ]]; then
    echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin quay.io
    docker push "$image"
fi
