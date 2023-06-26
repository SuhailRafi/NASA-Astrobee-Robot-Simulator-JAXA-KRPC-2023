#!/bin/bash
PATCH_SRC_PATH=./src/astrobee

while read patch_file; do
    org=${PATCH_SRC_PATH}/${patch_file}
    new=${org}.new
    patch=${org}.patch

    if [ -e ${org} ] && [ -e ${new} ]; then
            diff -up ${org} ${new} > ${patch}
            rm ${new}
        rm ${org}
    fi

done < ./filelist.txt
