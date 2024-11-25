FROM ubuntu:focal

# tell scripts that we're building in an automated release scenario
ENV CI=1

# unfortunately, this needs to be redeclared, as everything ahead of FROM gets discarded
ARG DIST_RELEASE=focal

# we re-use the DOCKER_PLATFORM to tell the build scripts about the build target architecture without the need to use uname
# the build scripts will have to map the architecture to names Debian/RPM/AppImage support
ARG DOCKER_PLATFORM

ENV DOCKER_PLATFORM="${DOCKER_PLATFORM}"

# let the install script do the heavy lifting (easier than replicating the complex behavior in a Dockerfile)
COPY install-deps.sh /
RUN bash -xe install-deps.sh
