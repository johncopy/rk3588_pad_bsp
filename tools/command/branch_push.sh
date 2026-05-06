#gen push.sh 
echo "BRANCH=\$1" >push.sh
echo "REMOTE=\`git remote\`" >> push.sh
echo "git push \$REMOTE \$BRANCH > result.txt 2>&1 " >>push.sh
echo "LOCAL_COMMIT=\`git log \$BRANCH -1 --oneline | cut -c -7\`" >> push.sh
echo "REMOTE_COMMIT=\`git log \$REMOTE/\$BRANCH -1 --oneline | cut -c -7\`" >> push.sh
echo "if [ \"\$LOCAL_COMMIT\" = \"\$REMOTE_COMMIT\"  ];then" >> push.sh
echo "echo -e \"\033[32m Everything up-to-date . \033[0m\"" >> push.sh
echo "else" >> push.sh
echo "echo -e \"\033[31m current branch push faill, please doublecheck XXXXXXXXXXXXXXX \033[0m\"" >> push.sh
echo "echo -e \"\033[31m local=\$LOCAL_COMMIT  != remote \$REMOTE_COMMIT \033[0m\"" >> push.sh
echo "fi" >> push.sh

BRANCH=$1
FILE_PATH=`pwd`;


if [ "$BRANCH" == "" ]
then
echo -e "\033[31mBranch Name is NULL!\033[0m"
echo "./branch_push.sh <BRANCH>"
else
echo -e "\033[34mPush a Branch from local to remote: \033[33m$BRANCH\033[34m.\033[0m"
repo forall -c "pwd;cp $FILE_PATH/push.sh push.sh;chmod +x push.sh;./push.sh $BRANCH;rm push.sh;rm result.txt"
fi

