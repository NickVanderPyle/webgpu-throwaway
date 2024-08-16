FROM debian:trixie-backports

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get -y install --no-install-recommends \
    # Project uses CMake
    cmake \
    # Git needed to clone the newest emscripten tag
    git \
    # Emscripten requirements & nice-to-have
    build-essential \
    ca-certificates \
    python3 \
    python3-pip \
    pipx \
    # Needed for rustup install
    curl \
    # Needed for vscode-clangd
    clangd

RUN ln -s /usr/bin/python3 /usr/bin/python \
    && pipx --version \
    && pipx ensurepath \
    && pipx ensurepath --global

# Install emscripten. Mostly copied from: https://github.com/emscripten-core/emsdk/blob/master/docker/Dockerfile
ENV EMSDK=/opt/emsdk \
    EMSCRIPTEN_VERSION=latest

# Stop popup when new terminal is opened.
RUN echo "export EMSDK_QUIET=1" >> /etc/bash.bashrc

# Assuming tagged versions of Emscripten to be more stable than latest.
RUN cd /opt \
    && git clone --depth=1 https://github.com/emscripten-core/emsdk.git \
    && cd emsdk \
    && git fetch --tags \
    && latest_tag=$(git describe --tags `git rev-list --tags --max-count=1`) \
    && git checkout $latest_tag \
    && rm -rf .git \
    && echo ". $EMSDK/emsdk_env.sh" >> /etc/bash.bashrc
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
    # # strip out symbols from clang (~extra 50MB disc space)
    # && find ${EMSDK}/upstream/bin -type f -exec strip -s {} + || true \
    && echo "## Done"

# Needed for wgsl-analyzer.wgsl-analyzer
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y \
    && echo '. "$HOME/.cargo/env"' >> $HOME/.bashrc \
    && . "$HOME/.cargo/env"
RUN $HOME/.cargo/bin/cargo install --git https://github.com/wgsl-analyzer/wgsl-analyzer wgsl_analyzer

ENV PATH=${EMSDK}/upstream/bin:$PATH
