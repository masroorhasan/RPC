#!/bin/bash
clear
echo "Unpacking..."

unzip ece454a1.zip
cp ./ece454a1/* ./
rm -r ./ece454a1
make clean
if [ -e a.out ] || [ -e *.o ] || [ -e *.a ]
then
    echo "Warning: clean does not seem to work!";
fi;
make libstubs.a

echo "Unpacking done!"
