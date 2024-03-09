#!/bin/sh

SCRIPT=$(readlink -f $0)
SCRIPTS_PATH=$(dirname $SCRIPT)

ROOT_DIR="$(dirname $SCRIPTS_PATH)"

if [ x"$INSTALL_DIR" = "x" ]; then
  INSTALL_DIR="$ROOT_DIR/install"
fi

if [ ! -x "$(command -v 7z)" ]; then
  echo "Please install 7z (p7zip-full) to proceed"
  exit 1
fi

cd "$ROOT_DIR"
MACH_DATA_COMMIT=$(git rev-parse --short HEAD)
DATE_STR=$(date +%Y%m%d)

echo "Creating data package (commit $MACH_DATA_COMMIT)"

rm -rf "$INSTALL_DIR"

INSTALL_DIR="$INSTALL_DIR" $SCRIPTS_PATH/install.sh || exit 1

cd "$INSTALL_DIR"
7z a machines-data-$DATE_STR-$MACH_DATA_COMMIT.7z machines
#zip -r machines-data-$DATE_STR-$MACH_DATA_COMMIT.7z machines
