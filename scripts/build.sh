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

echo "Check the environment..."
if [ -x "$(command -v rsvg-convert)" ]; then
  CONVERTER_COMMAND="rsvg-convert -o "
  echo "-- Found $(rsvg-convert --version)"
elif [ -x "$(command -v inkscape)" ]; then
  CONVERTER_COMMAND="inkscape --export-filename="
  echo "-- Found $(inkscape --version)"
elif [ -x "$(command -v flatpak)" ]; then
  INKSCAPE_ID="org.inkscape.Inkscape"
  if [ "$(flatpak list --app|grep $INKSCAPE_ID|wc -l)" = "1" ]; then
    CONVERTER_COMMAND="flatpak run --command=inkscape $INKSCAPE_ID --export-filename="
    INKSCAPE_VERSION=$(flatpak list --app --columns application,version|grep $INKSCAPE_ID|cut -f2)
    echo "-- Found Inkscape $INKSCAPE_VERSION (Flatpak)"
  fi
fi

if [ x"$CONVERTER_COMMAND" = x ]; then
  echo "Unable to find an svg converter."
  echo "Please install rsvg-convert or Inkscape and try again."
  # Note: 'convert' gave a worse result
  exit 1
fi

if [ -x "$(command -v gimp)" ]; then
  GIMP_COMMAND="gimp"
  echo "-- Found GIMP$(gimp --version|cut -d n -f 4-)"
else
  echo "Please install GIMP (needed e.g. to generate grayscale icons)"
  exit 1
fi

convert_svg2png()
{
  INPUT=$1
  OUTPUT=$2

  ${CONVERTER_COMMAND}${OUTPUT} $INPUT
}

echo "Collecting the source data..."
cd "$SOURCE_DIR"
GUI_DIRS=$(find gui -mindepth 1 -type d)
SVG_FILES=$(find -type f -name '*.svg')
PLANET_TEXTURES=$(find models -type f -name 'for-planets.txt')

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

  gimp -idf -b "(define (do-grayscale-icon input output) (let* ((image (car (gimp-file-load RUN-NONINTERACTIVE input input) ) ) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-grayscale image) (gimp-drawable-brightness-contrast drawable $BRIGHTNESS $CONTRAST) (gimp-file-save RUN-NONINTERACTIVE image drawable output output) (gimp-image-delete image)) ) $GIMP_COMMAND_PART" -b '(gimp-quit 0)' 2> /dev/null
  # Grayscale icons are done
}

grayscale_icons

cd "$BUILD_DIR"
for GUI_DIR in $GUI_DIRS
do
  mkdir -p "$GUI_DIR"
done

echo "Copying the textures..."
for TEXTURES_LIST in $PLANET_TEXTURES
do
  PLANETS=$(cat $SOURCE_DIR/$TEXTURES_LIST)
  TEXTURES_SOURCE_DIR=$(dirname $TEXTURES_LIST)

  for PLANET in $PLANETS
  do
    mkdir -p "models/planet/$PLANET"
    cp -rf $SOURCE_DIR/$TEXTURES_SOURCE_DIR/texture4 "models/planet/$PLANET/"
  done
done

echo "Scaling the UI images..."
for SVG_FILE in $SVG_FILES
do
  BASE_PATH=$(echo "$SVG_FILE" | sed 's/^..// ; s/....$//')
  convert_svg2png "$SOURCE_DIR/$BASE_PATH.svg" "${BASE_PATH}_2x.png"
done
