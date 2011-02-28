#!/bin/bash
# This script installs the IOFSL software, and executes distcheck on the IOFSL software.  
# dependencies are located through ../scripts/configoption.${HOSTNAME}. 
# After installation and construction the script runs a "distcheck" on the build.  
# test reports are mailed to io-fwd-discuss@lists.mcs.anl.gov
# This script has two options -n and -b.  -n followed by an integer identifies the number of 
# commits you wish to test from your present position backwards through history.
# The -b option followed by a branch name identifies which branch you would wish the test harness to exe# on.  If you wish to use the master branch you can simply leave the -b option out of your script invocation.

checkout_branch(){
  if
    [ "$branch" != "before-resourcework-thread" ] && [ "$branch" != "iofsl_vampir" ] && [ "$branch" != "master-merge" ] && [ "$branch" != "clientrpc" ] && [ "$branch" != "paper-freeze" ] && [ "$branch" != "resourcework" ] && [ "$branch" != "tranform" ] 
  then 
    :
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
        branch="$OPTARG"
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
  for ((i=0; i<$num_tests; i++))
  do
    destroy_touch
  done
}

destroy_touch(){
  #make clean
  #make distclean
  touch test_report.txt
  touch failure_report.txt
  touch make_error_report.txt  
  touch runtest_results.txt
  set_to_variable
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
  echo $(date) >> runtest_results.txt
  echo "testing commit $commit" >> runtest_results.txt
  echo "commit $commit committed by $committer_email" >> runtest_results.txt
  run_tests
}

run_tests(){
  git checkout $commit
  scripts/runtest.sh 2>&1 | tee -a runtest_results.txt 
  echo "This concludes the test of commit $commit" >> runtest_results.txt
  error_report
}

error_report(){
  rm -f make_error_report.txt
  touch make_error_report.txt
  if 
    egrep ' Error ' runtest_results.txt
  then
    echo "test results for commit $commit" >> make_error_report.txt
    echo "this commit was finished testing at $(date)" >> make_error_report.txt
    echo >> make_error_report.txt
    awk 'NR<=3' runtest_results.txt >> make_error_report.txt; awk '/Error/{c=5}c&&c--' runtest_results.txt >> make_error_report.txt; tail -5 runtest_results.txt >> make_error_report.txt
    echo "=========================================================" >> make_error_report.txt
  comm=${commit:0:7}
  echo | mutt -a make_error_report.txt -s "make error: Make FAIL for commit $comm" rjdamore@gmail.com
  #echo | mutt -c drieskimpe@gmail.com -a make_error_report.txt -s "make error: Make FAIL for commit $comm" rjdamore@lanl.gov
  else test_report 
  fi
  cat make_error_report.txt >> make_error_report-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
      echo "$commit" >> autotest/tested_commits.txt
  fi
  failure_report
}

failure_report(){
  rm -f failure_report.txt
  touch failure_report.txt
  if
    egrep 'FAIL|non-zero' runtest_results.txt
  then
    echo "test results for commit $commit" >> failure_report.txt
    echo "this commit was finished testing at $(date)" >> failure_report.txt
    echo >> failure_report.txt
    awk 'NR<=5' runtest_results.txt >> failure_report.txt ; awk '/FAIL/{c=5}c&&c--' runtest_results.txt >> failure_report.txt ; awk '/non-zero/{c=3}c&&c--' runtest_results.txt >> failure_report.txt; tail -5 runtest_results.txt >> failure_report.txt
    echo "==========================================================" >> failure_report.txt
  comm=${commit:0:7}   
  echo | mutt -a failure_report.txt -s "failure report for commit $comm FAIL" rjdamore@gmail.com
  else :
  fi
  cat failure_report.txt >> failure_report-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
    echo "$commit" >> autotest/tested_commits.txt
  fi
  server_report
}
    
server_report(){
  rm -f server_report.txt
  touch server_report.txt
  echo "test results for commit $commit" >> server_report.txt
  echo >> server_report.txt
  echo "this commit was finished testing at $(date)" >> server_report.txt
  awk 'NR<=5' runtest_results.txt >> server_report.txt ; egrep 'Running|PASS|All' runtest_results.txt >> server_report.txt ; awk '/--Run/{c=4}c&&c--' runtest_results.txt >> server_report.txt; awk '/non-zero/{c=3}c&&c--' runtest_results.txt >> server_report.txt; tail -5 runtest_results.txt >> server_report.txt
  echo "==========================================================" >> server_report.txt
  cat server_report.txt >> server_report-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
      echo "$commit" >> autotest/tested_commits.txt
  fi
  comm=${commit:0:7}
  echo | mutt -a server_report.txt -s "server report for commit $comm" rjdamore@gmail.com
  
}

test_report(){
  rm -f test_report.txt
  touch test_report.txt
  echo "test results for commit $commit" >> test_report.txt
  echo >> test_report.txt
  echo "this commit was finished testing at $(date)" >> test_report.txt
  which gcc >> test_report.txt 
  awk 'NR<=6' runtest_results.txt >> test_report.txt ; egrep 'Running|PASS|All' runtest_results.txt >> test_report.txt ; awk '/--Run/{c=4}c&&c--' runtest_results.txt >> test_report.txt; tail -5 runtest_results.txt >> test_report.txt
  echo "==========================================================" >> test_report.txt
  cat test_report.txt >> test_report-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
      echo "$commit" >> autotest/tested_commits.txt
  fi
  comm=${commit:0:7}
  echo | mutt -a test_report.txt -s "test report for commit $comm PASS" rjdamore@gmail.com
  #echo | mutt -c drieskimpe@gmail.com -a test_report.txt -s "test report for commit $comm PASS" rjdamore@gmail.com
  #echo | mutt -c drieskimpe@gmail.com -a test_report.txt -s "test report for commit $comm PASS" rjdamore@lanl.gov
  #echo | mutt -c dkimpe@mcs.anl.gov -a test_report.txt -s "test report for commit $comm PASS" io-fwd-discuss@lists.mcs.anl.gov 
  edit_files  
}  

edit_files(){
  echo "==========================================================" >> ~/repo-testing-results-pile.txt
  echo $(date) >> ~/repo-testing-results-pile.txt
  cat ~/iofsl/runtest_results.txt >>  ~/repo-testing-results-pile.txt
  echo "==========================================================" >> ~/repo-testing-results-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
      echo "$commit" >> autotest/tested_commits.txt
  fi
  rm -f ~/iofsl/runtest_results.txt
  chmod -R 777 iofwd-0.1
  rm -rf iofwd-0.1
  rm -rf iofwd-0.1.tar.gz
  make clean
  make distclean
  git checkout master
}


get_user_email
