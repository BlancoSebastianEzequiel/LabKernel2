#!/bin/bash

ROOT=./

if [[ -e code.md ]]; then
    rm code.md
fi
touch code.md

for case in $(ls $ROOT); do
    filename=$(basename ${case%%.*})
    extension="${case##*.}"

    if [[ $extension != 'c'  &&  $extension != 'h' ]]; then
        continue
    fi

    string="# $case"   # El numeral en markdown implica titulo
    echo $string >> code.md
    echo '```' >> code.md
    cat $case >> code.md
    echo '  ' >> code.md
    echo '```' >> code.md

done

pandoc code.md -t latex -o code.pdf
rm code.md
