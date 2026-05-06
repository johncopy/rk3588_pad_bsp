REMOTE=$1
if [ "$REMOTE" == "-r" ]
then
BRANCH=$2
else
BRANCH=$1
fi

#get remote list
repo forall -c "git remote" > temp ; sort temp | uniq >remote ;rm temp

if [ "$BRANCH" == "" ]
then
echo -e "\033[31mBranch Name is NULL!\033[0m"
echo -e "\033[32m./branch_sync.sh \033[33m<BRANCH>\033[0m or \033[32m./branch_sync.sh \033[34m-r \033[33m<BRANCH>\033[0m"
else
if [ "$REMOTE" == "-r" ]
then
echo -e "\033[34mSync a Branch from remote (\033[33m$BRANCH\033[34m) to local.\033[0m"
repo forall -c "pwd;git remote update"
	for i in $(cat remote)
	do
	repo forall -c "pwd; git merge $i/$BRANCH"
	done
else
echo -e "\033[34mSync a Branch from local (\033[33m$BRANCH\033[34m) to local.\033[0m"
repo forall -c "pwd; git remote update; git merge $BRANCH"
fi
fi
