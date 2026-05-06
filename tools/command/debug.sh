BRANCHNAME=$(repo branch | grep ^* | grep "in all projects" | cut -f3 -d " ")

#export RK_ROOTFS_SYSTEM=buildroot
export RK_ROOTFS_SYSTEM=debian
./build.sh

# get create image command name
IMGNAME=$0
# get image compile date time
DATATIME=`date +%Y%m%d`
# create file name, add date time
FILENAME=${RK_ROOTFS_SYSTEM}_rk3588_$(echo $BRANCHNAME)_$(echo ${IMGNAME%.*} | cut -c 3-)_$(echo $DATATIME)

# create image name file
echo update_$(echo $FILENAME)_debug.img > rockdev/filename.txt
