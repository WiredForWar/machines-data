#!/bin/sh

SCRIPT=$(readlink -f $0)
SCRIPTS_PATH=$(dirname $SCRIPT)

ROOT_DIR="$(dirname $SCRIPTS_PATH)"
DATA_DIR="$ROOT_DIR/data"
SOURCE_DIR="$ROOT_DIR/datasrc"
if [ x"$BUILD_DIR" = "x" ]; then
  BUILD_DIR="$ROOT_DIR/build"
fi

echo "Build dir: \"$BUILD_DIR\""

if [ -x "$(command -v gimp)" ]; then
  GIMP_COMMAND="gimp"
  echo "-- Found GIMP$(gimp --version|cut -d n -f 4-)"
else
  echo "Please install GIMP (needed e.g. to generate grayscale icons)"
  exit 1
fi

echo "Collecting the source data..."

cd "$ROOT_DIR"
cd data/gui/navigate

cd construc/red
NAVIGATE_CONSTRUCTION_ICONS=$(find . -type f -name '*.png'| sed 's/^..// ; s/....$//')
cd ../..

cd machines/red
NAVIGATE_MACHINES_ICONS=$(find . -type f -name '*.png'| sed 's/^..// ; s/....$//')
cd ../..

# Do the job
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

mkdir -p gui/navigate
cd gui/navigate/

mkdir -p construc/grey
mkdir -p machines/grey
cp -rf $DATA_DIR/gui/navigate/construc/red/*.png construc/grey/
cp -rf $DATA_DIR/gui/navigate/machines/red/*.png machines/grey/

grayscale_icons()
{
  echo "Creating grayscale icons from colorful... (this will take a while)"
  MACHINES_ORIGIN_BRIGHTNESS=0.155
  CONSTRUCTIONS_ORIGIN_BRIGHTNESS=0.275

  # Use the average brightness value for both Machines and Constructions
  BRIGHTNESS=0.215
  CONTRAST=0.0

  GIMP_COMMAND_PART=""
  for ICON in $NAVIGATE_CONSTRUCTION_ICONS
  do
    GIMP_COMMAND_PART="$GIMP_COMMAND_PART (do-grayscale-icon \"construc/grey/$ICON.png\" \"construc/grey/$ICON.png\" )"
  done

  for ICON in $NAVIGATE_MACHINES_ICONS
  do
    GIMP_COMMAND_PART="$GIMP_COMMAND_PART (do-grayscale-icon \"machines/grey/$ICON.png\" \"machines/grey/$ICON.png\" )"
  done

  gimp -idf -b "(define (do-grayscale-icon input output) (let* ((image (car (gimp-file-load RUN-NONINTERACTIVE input input) ) ) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-grayscale image) (gimp-drawable-brightness-contrast drawable $BRIGHTNESS $CONTRAST) (gimp-file-save RUN-NONINTERACTIVE image drawable output output) (gimp-image-delete image)) ) $GIMP_COMMAND_PART" -b '(gimp-quit 0)' 2> /dev/null || exit 2
  # Grayscale icons are done
}

grayscale_icons
