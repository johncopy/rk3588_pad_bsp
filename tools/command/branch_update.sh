BRANCH=$1
TRACK=$2
if [ "$TRACK" == "-r" ]
then
SRC_BRANCH=$3
else
SRC_BRANCH=$2
fi

if [ "$BRANCH" == "" ]
then

echo -e "\033[31mBranch Name is NULL!\033[0m"
echo -e "\033[32m./branch_update.sh \033[33m<BRANCH> <SRC_BRANCH>\033[0m"
echo -e "or \033[32m./branch_update.sh \033[33m<BRANCH> \033[34m-r \033[33m<SRC_BRANCH>\033[0m"

else

if [ "$TRACK" == "-r" ]
then
if [ "$SRC_BRANCH" == "" ]
then

echo -e "\033[31mSource Branch Name is NULL!\033[0m"
echo -e "\033[32m./branch_update.sh \033[33m<BRANCH> <SRC_BRANCH>\033[0m"
echo -e "or \033[32m./branch_update.sh \033[33m<BRANCH> \033[34m-r \033[33m<SRC_BRANCH>\033[0m"

else

echo -e "\033[34mUpdate a Branch at local: \033[33m$BRANCH\033[34m, from remote Branch: \033[33m$SRC_BRANCH\033[34m, and push local Branch: \033[33m$BRANCH\033[34m to remote.\033[0m"
./branch_checkout.sh $BRANCH
repo branch
echo -e "\033[34mPlease check only one branch in all projects: \033[33m(y/N)\033[0m"
read CHECK
if [ "$CHECK" == "y" ] || [ "$CHECK" == "Y" ]
then
./branch_sync.sh -r $SRC_BRANCH
./branch_mergetool.sh
echo -e "\033[34mPlease check no Merge Conflict in all projects, and commit the Conflict projects: \033[33m(y/N)\033[0m"
read CHECK
if [ "$CHECK" == "y" ] || [ "$CHECK" == "Y" ]
then
./branch_push.sh $BRANCH
else
echo -e "\033[31mPlease fix all projects Merge Conflict, and commit the Conflict projects!\033[0m"
fi
else
echo -e "\033[31mPlease switch to only one branch!\033[0m"
fi

fi
else

if [ "$SRC_BRANCH" == "" ]
then

echo -e "\033[31mSource Branch Name is NULL!\033[0m"
echo -e "\033[32m./branch_update.sh \033[33m<BRANCH> <SRC_BRANCH>\033[0m"
echo -e "or \033[32m./branch_update.sh \033[33m<BRANCH> \033[34m-r \033[33m<SRC_BRANCH>\033[0m"

else

echo -e "\033[34mUpdate a Branch at local: \033[33m$BRANCH\033[34m, from local Branch: \033[33m$SRC_BRANCH\033[34m, and push local Branch: \033[33m$BRANCH\033[34m to remote.\033[0m"
./branch_checkout.sh $BRANCH
repo branch
echo -e "\033[34mPlease check only one branch in all projects: \033[33m(y/N)\033[0m"
read CHECK
if [ "$CHECK" == "y" ] || [ "$CHECK" == "Y" ]
then
./branch_sync.sh $SRC_BRANCH
./branch_mergetool.sh
echo -e "\033[34mPlease check no Merge Conflict in all projects, and commit the Conflict projects: \033[33m(y/N)\033[0m"
read CHECK
if [ "$CHECK" == "y" ] || [ "$CHECK" == "Y" ]
then
./branch_push.sh $BRANCH
else
echo -e "\033[31mPlease fix all projects Merge Conflict, and commit the Conflict projects!\033[0m"
fi
else
echo -e "\033[31mPlease switch to only one branch!\033[0m"
fi

fi

fi

fi
