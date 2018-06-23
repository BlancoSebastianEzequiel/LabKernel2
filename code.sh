#!/bin/bash

ROOT=./
LIB=lib/

function create_file () {
    for case in $(ls $1); do
        filename=$(basename ${case%%.*})
        extension="${case##*.}"

        if [[ $extension != 'S'  && $extension != 'c'  &&  $extension != 'h' ]];
        then
            continue
        fi

        string="# $case"   # El numeral en markdown implica titulo
        echo $string >> code.md
        echo '```' >> code.md
        cat $1/$case >> code.md
        echo '  ' >> code.md
        echo '```' >> code.md
    done
}

if [[ -e code.md ]]; then
    rm code.md
fi
touch code.md

echo "## Libs" >> code.md
create_file $LIB
echo "## Src" >> code.md
create_file $ROOT


pandoc code.md -t latex -o code.pdf
rm code.md
