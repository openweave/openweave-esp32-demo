#!/bin/sh

#
#    Copyright 2019 Google LLC All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      Travis CI build preparation script for the OpenWeave ESP32 Demo
#      application.
#

FetchURL() {
    local URL="$1"
    local LOCAL_FILE_NAME=$2
    local HASH=$3

    # NOTE: 2 spaces required between hash value and file name.
    if ! (echo "${HASH}  ${LOCAL_FILE_NAME}" | ${HASH_CMD} -c --status >/dev/null 2>&1); then
        rm -f ${LOCAL_FILE_NAME}
        wget -O ${LOCAL_FILE_NAME} -nv "${URL}" || exit 1
    fi
}

# --------------------------------------------------------------------------------
# General Configuration
# --------------------------------------------------------------------------------

TMPDIR=${TMPDIR-/tmp}
CACHEDIR=${TRAVIS_BUILD_DIR}/cache
HASH_CMD="shasum -a 256"


# --------------------------------------------------------------------------------
# Install dependencies
# --------------------------------------------------------------------------------

ESP_IDF_VERSION=release/v3.3
if test "${TRAVIS_OS_NAME}" = "linux"; then
    XTENSA_TOOL_CHAIN_URL=https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
    XTENSA_TOOL_CHAIN_HASH=3fe96c151d46c1d4e5edc6ed690851b8e53634041114bad04729bc16b0445156
elif test "${TRAVIS_OS_NAME}" = "osx"; then
    XTENSA_TOOL_CHAIN_URL=https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz
    XTENSA_TOOL_CHAIN_HASH=a4307a97945d2f2f2745f415fbe80d727750e19f91f9a1e7e2f8a6065652f9da
fi

set -x

mkdir -p ${CACHEDIR}

# Install dependent packages needed by the ESP32 development environment.
#
if test "${TRAVIS_OS_NAME}" = "linux"; then
    sudo apt-get update
    sudo apt-get -y install libncurses-dev flex bison gperf
elif test "${TRAVIS_OS_NAME}" = "osx"; then
    sudo pip install pyserial
fi

# Install ESP32 toolchain for linux
#
XTENSA_TOOL_CHAIN_FILE_NAME=${CACHEDIR}/`basename ${XTENSA_TOOL_CHAIN_URL}`
FetchURL "${XTENSA_TOOL_CHAIN_URL}" ${XTENSA_TOOL_CHAIN_FILE_NAME} ${XTENSA_TOOL_CHAIN_HASH}
tar -C ${TRAVIS_BUILD_DIR} -xzf ${XTENSA_TOOL_CHAIN_FILE_NAME} || exit 1

# Install the ESP32 development environment.
#
git -C ${TRAVIS_BUILD_DIR} clone --depth 1 https://github.com/espressif/esp-idf.git -b ${ESP_IDF_VERSION} || exit 1
git -C ${TRAVIS_BUILD_DIR}/esp-idf checkout ${ESP_IDF_VERSION} || exit 1
git -C ${TRAVIS_BUILD_DIR}/esp-idf submodule update --init --recursive || exit 1

set +x


# --------------------------------------------------------------------------------
# Prepare source tree
# --------------------------------------------------------------------------------

set -x

# Initialize and update all submodules within the example app.
#
git -C ${TRAVIS_BUILD_DIR} submodule init || exit 1
git -C ${TRAVIS_BUILD_DIR} submodule update || exit 1

set +x

# --------------------------------------------------------------------------------
# Prepare python
# --------------------------------------------------------------------------------
python -m pip install --user -r ${TRAVIS_BUILD_DIR}/esp-idf/requirements.txt


# --------------------------------------------------------------------------------
# Log build information.
# --------------------------------------------------------------------------------

echo '---------------------------------------------------------------------------'
echo 'Build Preparation Complete'
echo ''
echo "openweave-esp32-demo branch: ${TRAVIS_BRANCH}"
echo "ESP-IDF version: ${ESP_IDF_VERSION}"
echo "Xtensa Tool Chain: ${XTENSA_TOOL_CHAIN_URL}"
echo 'Commit Hashes'
echo '  openweave-esp32-demo: '`git -C ${TRAVIS_BUILD_DIR} rev-parse --short HEAD`
git -C ${TRAVIS_BUILD_DIR} submodule --quiet foreach 'echo "  openweave-esp32-demo/$path: "`git rev-parse --short HEAD`'
echo '---------------------------------------------------------------------------'
