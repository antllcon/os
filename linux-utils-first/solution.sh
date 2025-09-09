#!/usr/bin/env bash

set -e

DIR="out"

tar -czf proj.tar.gz -C proj .

if [ -d "$DIR" ]; then
    echo "Folder '$DIR' already exist"
    read -p "Recreate folder? (y/n): " answer

    if [ "${answer,,}" = "y" ]; then
        rm -rf "$DIR"
        mkdir "$DIR"
    else
        echo "Cancel creating folder"
        exit 0
    fi
else
    mkdir "$DIR"
fi

mv proj.tar.gz out/

tar -xzf out/proj.tar.gz -C out/ --strip-components=1
rm out/proj.tar.gz

cd out/
mkdir -p include src build
mv -f *.h include/
mv *.cpp src/

g++ -I./include -Wall -Wextra -std=c++17 -O2 src/*.cpp -o build/start
cd ../

echo -e "$1\n$2" | ./out/build/start > stdout.txt