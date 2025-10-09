#!/bin/sh -e

Temp_Dir=$(mktemp -d)
Path=$PWD
File=$1

exit_handler()
{
    local rc=$?
    trap - EXIT
    rm -rf -- $Temp_Dir
    exit $rc
}
trap exit_handler EXIT HUP INT QUIT PIPE TERM


cd "$Temp_Dir"

File_Name=$(grep '&Output:' "$Path/$File" | cut -d: -f2)

g++ "$Path/$File" -o $File_Name

mv $File_Name $Path

echo "Сборка выполнена во временном каталоге: $Temp_Dir"
ls -l "$Temp_Dir"

cd / || exit 1
rm -rf "$Temp_Dir"