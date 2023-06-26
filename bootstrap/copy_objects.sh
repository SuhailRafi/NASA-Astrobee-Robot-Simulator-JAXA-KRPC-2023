#! /bin/sh

INPUT_PATH=/tmp/bootstrap/files
OUTPUT_PATH=/src/astrobee/src

# copy files
while read file_name || [ -n "${file_name}" ]; do
    cp ${INPUT_PATH}/${file_name} ${OUTPUT_PATH}/${file_name}
done < /tmp/bootstrap/filelist.txt

echo copy objects done!
