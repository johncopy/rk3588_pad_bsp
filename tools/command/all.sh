# get branch name from "repo branch"
BRANCHNUM=$(repo branch | grep -c "*  ")
if [ "$BRANCHNUM" == "1" ]
then
    BRANCHNAME=$(repo branch | grep ^* | grep "in all projects" | cut -f3 -d " ")
if [ "$BRANCHNAME" == "" ]
then
    echo -e "\033[34mPlease check the branch status, need to comfirm check out to \033[31msame branch in all projects.\033[0m"
else

./revert.sh

# get kernel version
if [ ! -f "kernel/Makefile" ]; then
        kernel_version="general"
else
        version=$(grep '^VERSION =' kernel/Makefile | awk '{print $NF}')
        patchlevel=$(grep '^PATCHLEVEL =' kernel/Makefile | awk '{print $NF}')
        sublevel=$(grep '^SUBLEVEL =' kernel/Makefile | awk '{print $NF}')
        extraversion=$(grep '^EXTRAVERSION =' kernel/Makefile | awk '{print $NF}')
        kernel_version="$version.$patchlevel.$sublevel"
fi

#export RK_ROOTFS_SYSTEM=buildroot
export RK_ROOTFS_SYSTEM=debian

# get create image command name
IMGNAME=$0
# get image compile date time
DATATIME=`date +%Y%m%d`
# create file name, add date time
FILENAME=${RK_ROOTFS_SYSTEM}_kernel${kernel_version}_rk3588_$(echo $BRANCHNAME)_$(echo ${IMGNAME%.*} | cut -c 3-)_$(echo $DATATIME)

# create image name file
echo update_$(echo $FILENAME)_release.img > filename.txt

./build.sh

echo -e "\033[34mCompile finished, do you upload this manifest? \033[33m(y/N)\033[0m"
read CHECK
if [ "$CHECK" == "y" ] || [ "$CHECK" == "Y" ]
then
echo -e "\033[34mYou select upload the manifest!\033[0m"
# create manifest xml file
mkdir -p tools/pubxml/
repo manifest -r -o tools/pubxml/$FILENAME.xml
else
echo -e "\033[32mYou select NOT upload the manifest!\033[0m"
fi

fi

else
echo -e "\033[34mPlease check the branch status, need to comfirm check out to \033[31msame branch in all projects.\033[0m"
fi
