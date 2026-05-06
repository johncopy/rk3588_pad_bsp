HELP=$1

if [ "$HELP" == "-h" ]
then
echo -e "\033[34mBranch Merge Tool help:\033[0m"
echo -e "\033[32m./branch_mergetool.sh\033[34m, user command: \033[33mgit mergetool\033[34m, to check merge status, after run command: \033[32m./branch_sync.sh\033[34m.\033[0m"
else
echo -e "\033[34muse Merge Tool to merge your Branch at local, to check merge status.\033[0m"
repo forall -c "pwd; git mergetool"
fi
