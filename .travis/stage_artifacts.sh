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
#      Travis CI script to stage artifacts after a successful build of
#      the OpenWeave ESP32 Demo application.
#

set -x

# Create a directory for build artifacts
mkdir -p ${TRAVIS_BUILD_DIR}/build_artifacts || exit 1

# Copy select build output into the artifacts directory
cp ${TRAVIS_BUILD_DIR}/build/openweave-esp32-demo.bin ${TRAVIS_BUILD_DIR}/build_artifacts
cp ${TRAVIS_BUILD_DIR}/build/openweave-esp32-demo.elf ${TRAVIS_BUILD_DIR}/build_artifacts
cp ${TRAVIS_BUILD_DIR}/build/openweave-esp32-demo.map ${TRAVIS_BUILD_DIR}/build_artifacts

set +x