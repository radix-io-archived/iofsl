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
  git log | grep "commit" | awk '{print $2}'| awk 'length > 39'  > commits_to.txt
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
  rm -f test_report.txt; touch test_report.txt
  rm -f failure_report.txt; touch failure_report.txt
  for ((i=0; i<3; i++))
  do
    set_to_variable
  done
}

set_to_variable(){
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
  run_tests 
}
  
  run_tests(){
  git checkout $commit
  scripts/runtest.sh 2>&1 | tee -a runtest_results.txt 
  echo "This concludes the test of commit $commit" >> runtest_results.txt
  if
    grep 'FAIL' runtest_results.txt
  then
    echo "test results for $commit" > failure_report.txt
    echo >> failure_report.txt
    awk 'NR<=5' runtest_results.txt >> failure_report.txt ; awk '/FAIL/{c=5}c&&c--' runtest_results.txt >> failure_report.txt ; tail -1 runtest_results.txt >> failure_report.txt
    echo "==========================================================" >> failure_report.txt   
    echo "test results for $commit" > test_report.txt
    echo >> test_report.txt
    awk 'NR<=5' runtest_results.txt >> test_report.txt ; egrep 'Running|PASS|All' runtest_results.txt >> test_report.txt ; awk '/--Run/{c=4}c&&c--' runtest_results.txt >> test_report.txt; tail -1 runtest_results.txt >> test_report.txt
    echo "==========================================================" >> test_report.txt
    echo "$commit" >>autotest/tested_commits.txt
    ./autotest/check_test_files.sh
    rm -f ~/iofsl/configoptions.$hostname
    git checkout master
  else
    echo "test results for $commit" > test_report.txt
    echo >> test_report.txt
    awk 'NR<=5' runtest_results.txt >> test_report.txt ; egrep 'Running|PASS|All' runtest_results.txt >> test_report.txt ; awk '/--Run/{c=4}c&&c--' runtest_results.txt >> test_report.txt; tail -1 runtest_results.txt >> test_report.txt
    echo "==========================================================" >> test_report.txt
    echo "$commit" >> autotest/tested_commits.txt
  fi
  rm -f ~/iofsl/configoptions.$hostname
  ./autotest/check_test_files.sh
  git checkout master  
}


get_user_email
