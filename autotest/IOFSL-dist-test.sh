#!/bin/bash
# This script installs the IOFSL software, and executes distcheck on the IOFSL software.  
# dependencies are located through ../scripts/configoption.${HOSTNAME}. 
# After installation and construction the script runs a "distcheck" on the build.  
# test reports are mailed to io-fwd-discuss@lists.mcs.anl.gov
# This script has two options -n and -b.  -n followed by an integer identifies the number of 
# commits you wish to test from your present position backwards through history.
# The -b option followed by a branch name identifies which branch you would wish the test harness to# execute on.  

checkout_branch(){
  if
    [ "$branch" != "before-resourcework-thread" ] && [ "$branch" != "iofsl_vampir" ] && [ "$branch" != "master-merge" ] && [ "$branch" != "clientrpc" ] && [ "$branch" != "paper-freeze" ] && [ "$branch" != "resourcework" ] && [ "$branch" != "tranform" ] 
  then 
    git checkout master
  else
    git checkout $branch
  fi
}

while getopts "n:b:" opt; do
  case $opt in
    n)
        num_tests="$OPTARG"
	echo "Running tests on $OPTARG commits"
        ;;
    b) 
        export branch="$OPTARG"
	checkout_branch
	;;
    [?])
	echo "invalid option! only -n followed by an integer and -b followed by a branch name are valid options. See autotest/README for more info. Thanks."
	;;
  esac
done

get_user_email(){
  echo "Locating git user email."
  export user_email=$(git config -l | grep user.email=* | sed 's/user.email=//g')  
  if [[ $user_email == *@* ]]
  then
    echo "User email found ----> "   $user_email
  else
    echo "Have you configured your git global email? It's needed for error reporting"
  fi
  commits_to_txt
}
  
commits_to_txt(){
  git log | grep "commit" | awk '{print $2}'| awk 'length > 39'  > ~/commits_to.txt
  read_commits
}

read_commits(){
  commits_IFS=$IFS
  IFS=$'\n'
  lines=($(cat ~/commits_to.txt))
  IFS=$commits_IFS 
  execute_tests
}

execute_tests(){
  for ((i=0; i<$num_tests; i++))
  do
    set_to_variable
  done
}


set_to_variable(){
  touch runtest_results.txt
  export commit=$(echo ${lines[$i]})
  echo "testing commit $commit"
  if
    grep $commit ~/tested_commits/tested_commits.txt
  then
    echo "This commit has been tested previously...exiting testing schedule..."
    echo | mutt -s "exiting $branch: already tested $commit" $EMAIL2  ; exit 0
  fi
  export committer_email=$(git cat-file commit $commit | grep committer | awk '{print $4}' | sed 's/<//g' | sed 's/>//g')
  echo "committer_email= $committer_email"
  echo $(date) >> runtest_results.txt
  echo "testing commit $commit" >> runtest_results.txt
  echo "commit $commit committed by $committer_email" >> runtest_results.txt
  run_tests
}

run_tests(){
  git checkout $commit
  ./scripts/runtest.sh 2>&1 | tee -a runtest_results.txt 
  echo "This concludes the test of commit $commit" >> runtest_results.txt
  email
}

email(){
  ~/email_results.sh
  remove
}
remove(){
rm -f make_error_report.txt
rm -f runtest_results.txt
rm -f test_report.txt
git reset --hard $branch
}
hostname=$(hostname)
source ~/configoptions.$hostname
get_user_email
