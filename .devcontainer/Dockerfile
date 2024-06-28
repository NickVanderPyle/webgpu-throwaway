FROM debian:stable-backports

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV DEBIAN_FRONTEND=noninteractive

RUN cat /etc/passwd

RUN apt-get update \
    && apt-get -y install --no-install-recommends \
        sudo \
    && groupadd -g $USER_GID $USERNAME \
    && useradd -u $USER_UID -s /bin/bash -m -g $USER_GID $USERNAME \
    && echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

RUN apt-get upgrade -y \
    && apt-get -y install --no-install-recommends \
        ca-certificates \
        sudo \
        curl \
        git \
        unzip \
        build-essential \
        cmake \
        ninja-build


# Install emscripten. Mostly copied from: https://github.com/emscripten-core/emsdk/blob/master/docker/Dockerfile
ARG EMSCRIPTEN_VERSION=latest
ENV EMSDK /emsdk
RUN apt-get -y install --no-install-recommends \
        curl \
        python3 \
    && mkdir -p $EMSDK \
    && curl -L https://github.com/emscripten-core/emsdk/archive/refs/heads/main.tar.gz | tar -xz --strip-components=1 -C $EMSDK emsdk-main/
RUN echo "## Install Emscripten" \
    && cd ${EMSDK} \
    && ./emsdk install ${EMSCRIPTEN_VERSION} \
    && echo "## Done"
# This generates configuration that contains all valid paths according to installed SDK
# TODO(sbc): We should be able to use just emcc -v here but it doesn't
# currently create the sanity file.
RUN cd ${EMSDK} \
    && echo "## Generate standard configuration" \
    && ./emsdk activate ${EMSCRIPTEN_VERSION} \
    && chmod 777 ${EMSDK}/upstream/emscripten \
    && chmod -R 777 ${EMSDK}/upstream/emscripten/cache \
    && echo "int main() { return 0; }" > hello.c \
    && ${EMSDK}/upstream/emscripten/emcc -c hello.c \
    && cat ${EMSDK}/upstream/emscripten/cache/sanity.txt \
    && echo "## Done"
# Cleanup Emscripten installation and strip some symbols
RUN echo "## Aggressive optimization: Remove debug symbols" \
    && cd ${EMSDK} && . ./emsdk_env.sh \
    # Remove debugging symbols from embedded node (extra 7MB)
    && strip -s `which node` \
    # Tests consume ~80MB disc space
    && rm -fr ${EMSDK}/upstream/emscripten/tests \
    # Fastcomp is not supported
    && rm -fr ${EMSDK}/upstream/fastcomp \
    # strip out symbols from clang (~extra 50MB disc space)
    && find ${EMSDK}/upstream/bin -type f -exec strip -s {} + || true \
    && echo "## Done"
RUN echo ". /emsdk/emsdk_env.sh" >> /etc/bash.bashrc

USER $USERNAME
WORKDIR /home/$USERNAME    
ENV PATH $EMSDK:$EMSDK/upstream/emscripten/:$PATH

