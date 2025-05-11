#!/bin/bash
set -e

# Set SDK
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path --toolchain com.apple.dt.toolchain.XcodeDefault)
echo $SDKROOT

# Ensure Conan is ready
export PATH="$HOME/.local/bin:$PATH"  # Add user pip bin to path if necessary

# Check if conan is installed
if ! command -v conan &> /dev/null
then
    echo ">>> Conan not found. Installing Conan via pip..."
    pip install --user conan
    echo ">>> Conan installed successfully."
else
    echo ">>> Conan is already installed."
fi

# Check if Conan default profile exists
if [ ! -f "$HOME/.conan2/profiles/default" ]; then
    echo ">>> Conan default profile not found. Running 'conan profile detect'..."
    conan profile detect
fi

conan install . --output-folder=build --build=missing

# Configure with CMake (specify Release mode explicitly!)
cmake -Bbuild -S. \
  -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
  -DCMAKE_PREFIX_PATH="$(pwd)/build" \
  -DCMAKE_BUILD_TYPE=Release
# Build
cmake --build build

# Run
./build/ruby_life
