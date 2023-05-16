FROM mcr.microsoft.com/devcontainers/cpp:0-debian-11

# Everything below this is needed for installing MariaDB
# Instructions are copied and modified from: https://mariadb.com/docs/clients/mariadb-connectors/connector-cpp/install/
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install curl

COPY ./install-mariadb.sh /tmp/
RUN chmod +x /tmp/install-mariadb.sh && /tmp/install-mariadb.sh && rm -f /tmp/install-mariadb.sh

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.21.5"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# install google protobuf
ARG PROTOBUF_BRANCH="3.19.x"
COPY ./install-protobuf.sh /tmp/
RUN chmod +x /tmp/install-protobuf.sh && /tmp/install-protobuf.sh && rm -f /tmp/install-protobuf.sh

# install tiny xml
ARG NUM_COMPILE_JOBS="3"
COPY ./tinyxml.sh /tmp/
RUN chmod +x /tmp/tinyxml.sh && /tmp/tinyxml.sh && rm -f /tmp/tinyxml.sh