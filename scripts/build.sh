#!/bin/sh

SCRIPT=$(readlink -f $0)
SCRIPTS_PATH=$(dirname $SCRIPT)

ROOT_DIR="$(dirname $SCRIPTS_PATH)"
SOURCE_DIR="$ROOT_DIR/datasrc"
if [ x"$BUILD_DIR" = "x" ]; then
  BUILD_DIR="$ROOT_DIR/build"
fi

echo "Build dir: \"$BUILD_DIR\""

echo "Check the environment..."
if [ -x "$(command -v rsvg-convert)" ]; then
  CONVERTER_COMMAND="rsvg-convert -o "
elif [ -x "$(command -v inkscape)" ]; then
  CONVERTER_COMMAND="inkscape --export-filename="
elif [ -x "$(command -v flatpak)" ]; then
  if [ "$(flatpak list --app|grep org.inkscape.Inkscape|wc -l)" = "1" ]; then
    CONVERTER_COMMAND="flatpak run --command=inkscape org.inkscape.Inkscape --export-filename="
  fi
fi

if [ x"$CONVERTER_COMMAND" = x ]; then
  echo "Unable to find an svg converter."
  echo "Please install rsvg-convert or Inkscape and try again."
  # Note: 'convert' gave a worse result
  exit 1
fi

convert_svg2png()
{
  INPUT=$1
  OUTPUT=$2

  ${CONVERTER_COMMAND}${OUTPUT} $INPUT
}

echo "List the source data..."
cd "$SOURCE_DIR"
GUI_DIRS=$(find gui -mindepth 1 -type d)
SVG_FILES=$(find -type f -name '*.svg')
PLANET_TEXTURES=$(find models -type f -name 'for-planets.txt')

# Do the job
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

for GUI_DIR in $GUI_DIRS
do
  mkdir -p "$GUI_DIR"
done

echo "Copy the textures..."
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

echo "Scale the UI images..."
for SVG_FILE in $SVG_FILES
do
  BASE_PATH=$(echo "$SVG_FILE"|sed 's/^..// ; s/....$//')
  convert_svg2png "$SOURCE_DIR/$BASE_PATH.svg" "${BASE_PATH}_2x.png"
done
