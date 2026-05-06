CREATE=$1
if [ "$CREATE" == "-b" ]
then
BRANCH=$2
TRACK=$3
if [ "$TRACK" == "-t" ]
then
REMOTE_BRANCH=$4
fi
else
BRANCH=$1
fi

#get remote list
repo forall -c "git remote" > temp ; sort temp | uniq >remote ;rm temp


if [ "$BRANCH" == "" ]
then
echo -e "\033[31mBranch Name is NULL!\033[0m"
echo -e "\033[32m./branch_checkout.sh \033[33m<BRANCH>\033[0m or \033[32m./branch_checkout.sh \033[34m-b \033[33m<BRANCH>\033[0m"
echo -e "or \033[32m./branch_checkout.sh \033[34m-b \033[33m<BRANCH> \033[34m-t \033[33m<REMOTE_BRANCH>\033[0m"
else
if [ "$CREATE" == "-b" ]
then
if [ "$TRACK" == "-t" ]
then
echo -e "\033[34mCreate a Branch at local: \033[33m$BRANCH\033[34m, based on remote: \033[33m$REMOTE_BRANCH\033[34m.\033[0m"
repo forall -c "pwd;git remote update"
	for i in $(cat remote)
	do
	repo forall -c "pwd; git checkout -b $BRANCH -t $i/$REMOTE_BRANCH"
	done
else
echo -e "\033[34mCreate a Branch at local: \033[33m$BRANCH\033[34m, based on local.\033[0m"
repo forall -c "pwd;git remote update"
repo forall -c "pwd;  git checkout -b $BRANCH"
fi
else
echo -e "\033[34mCheckout a Branch from remote to local: \033[33m$BRANCH\033[34m, and merge remote: \033[33m$BRANCH\033[34m.\033[0m"
repo forall -c "pwd;git remote update"	
	for i in $(cat remote)
	do
	repo forall -c "pwd;git checkout $BRANCH; git merge $i/$BRANCH"
	done
fi
fi
