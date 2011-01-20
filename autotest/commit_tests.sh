#!/bin/bash
# This script builds, installs, and checks the IOFSL software.  
# dependencies are located configoption.${HOSTNAME}. 
# After installation and construction the script runs a "make check" on the build.  
# the tests specified in the makefile are run and error reports will be mailed to the appropriate # committers address.



#git pull

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
  git log | grep "commit" | awk '{print $2}' > commits_to.txt
  read_commits
}

read_commits(){
  commits_IFS=$IFS
  IFS=$'\n'
  lines=($(cat commits_to.txt))
  IFS=$commits_IFS 
  execute_tests
}

execute_tests(){
  hostname=$(hostname)
  cp ~/iofsl/scripts/configoptions.tu-fe1.lanl.gov ~/iofsl/scripts/configoptions.$hostname
  touch test_report.txt
  for ((i=0; i<1; i++))
  do
    set_to_variable_and_test
  done
}

set_to_variable_and_test(){
  export commit=$(echo ${lines[$i]})
  echo "testing commit $commit"
  if
    grep $commit autotest/tested_commits.txt
  then
    echo "This commit has been tested previously...exiting testing schedule..." ; exit 0
  fi
  export committer_email=$(git cat-file commit $commit | grep committer | awk '{print $4}' | sed 's/<//g' | sed 's/>//g')
  echo "committer_email= $committer_email"
  echo $(date) > runtest_results.txt
  echo "testing commit $commit" >> runtest_results.txt
  echo "commit $commit committed by $committer_email" >> runtest_results.txt
  git checkout $commit
  scripts/runtest.sh | tee -a runtest_results.txt 
  echo "This concludes the test of commit $commit" >> runtest_results.txt
  awk 'NR<=5' runtest_results.txt > test_report.txt ; egrep 'Running|PASS' runtest_results.txt >> test_report.txt ; awk '/--Run/{c=4}c&&c--' runtest_results.txt >> test_report.txt; tail -1 runtest_results.txt >> test_report.txt
  echo "$commit" >> autotest/tested_commits.txt
  echo "mailing results"
  ./autotest/runtest_result.mail 
  rm -f ~/iofsl/configoptions.$hostname
  git checkout master  
}
get_user_email
