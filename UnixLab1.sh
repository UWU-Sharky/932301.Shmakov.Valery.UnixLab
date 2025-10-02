#!/bin/sh

Temp_Dir=$(mktemp -d)
Path=$PWD
File=$1


if [ ! -d "$Temp_Dir" ]; then
    echo "Не удалось создать Temp_Dir выход с ошибкой." >&2
    exit 1
fi

if ! cd "$Temp_Dir"; then
    echo "Не удалось перейти в каталог $Temp_Dir" >&2
    exit 1
fi

File_Name=$(grep '&Output:' "$Path/$File" | cut -d: -f2)

g++ "$Path/$File" -o $File_Name

mv $File_Name $Path

echo "Сборка выполнена во временном каталоге: $Temp_Dir"
ls -l "$Temp_Dir"

cd / || exit 1
rm -rf "$Temp_Dir"


