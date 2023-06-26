#!/bin/bash

SRC_PATH='../../../astrobee/src/astrobee'
PATCH_SRC_PATH='./src/astrobee'

while read patch_file; do
    src_file_path=${SRC_PATH}/${patch_file}
    copy_file_path=${PATCH_SRC_PATH}/${patch_file}
    copy_dir_path=$(dirname ${copy_file_path})

    mkdir -p ${copy_dir_path}

    cp ${src_file_path} ${copy_file_path}
    cp ${src_file_path} ${copy_file_path}.new

    if [ -e ${copy_file_path}.patch ]; then
        patch ${copy_file_path}.new ${copy_file_path}.patch
    fi

done < ./filelist.txt
