#!/bin/bash
sizes=(16 32 64 128 256 512)
if [ ! -e icon.iconset ]; then
  mkdir icon.iconset
fi
for i in "${sizes[@]}"; do
  echo "making $i..."
  convert logo.png -filter Mitchell -scale "$i"x"$i" icon.iconset/icon_"$i"x"$i".png
done
for i in "${sizes[@]}"; do
  echo "making $i@2x..."
  convert logo.png -filter Mitchell -scale "$((i*2))"x"$((i*2))" icon.iconset/icon_"$i"x"$i""@2x.png"
done

convert icon.iconset/icon_256x256.png icon.ico
