#!/bin/sh

SCRIPT=$(readlink -f $0)
SCRIPTS_PATH=$(dirname $SCRIPT)

ROOT_DIR="$(dirname $SCRIPTS_PATH)"
DATA_DIR="$ROOT_DIR/data"
if [ x"$BUILD_DIR" = "x" ]; then
  BUILD_DIR="$ROOT_DIR/build"
fi

if [ x"$INSTALL_DIR" = "x" ]; then
  INSTALL_DIR="$ROOT_DIR/install"
fi

$SCRIPTS_PATH/build.sh || exit 1

echo "Installing to $INSTALL_DIR"

INSTALL_MACHINES_DIR="$INSTALL_DIR/machines"
mkdir -p "$INSTALL_MACHINES_DIR"

cp -rf $DATA_DIR/* "$INSTALL_MACHINES_DIR/"
cp -rf $BUILD_DIR/* "$INSTALL_MACHINES_DIR/"
