# clean u-boot
cd u-boot
make clean -j$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print field+1}')
cd ..

# clean kernel
cd kernel
make clean -j$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print field+1}')
cd ..

# clean android
make clean -j$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print field+1}')

# clean not manager file
#repo forall -c "pwd; git clean -df"
