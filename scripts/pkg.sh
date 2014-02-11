#!/bin/sh

archive="narbonne.zip"

rm -f $archive
zip -r $archive *.txt *.cpp *.hpp scripts/*.sh scripts/*.pl 

echo "Re-created $archive"

