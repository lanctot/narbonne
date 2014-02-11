#!/bin/sh

archive="narbonne.zip"

rm -f $archive
zip -r $archive CMakeLists.txt README.txt *.cpp *.hpp scripts/*.sh scripts/*.pl 

echo "Re-created $archive"

