mkdir ROOT
cd ROOT

mkdir a_1 a_2
cd a_1
mkdir b_0

cd ..
cd a_2

mkdir b_1
mkdir b_2

cd b_1
truncate -s 227 c_1.bin
touch c_3.bin

cd ../b_2
ln ../b_1/c_3.bin c_2.bin

cd ../../a_1/b_0
ln -s ../../a_2/b_1 c_0
cd ..
cd ..
ln -s a_2/b_1/c_3.bin a_0.bin

../..