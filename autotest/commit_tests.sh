#!/bin/bash
# This script builds, installs, and checks the IOFSL software.  It pulls the latest edition of the IOFSL software
# from the git repo.  The options -c and -m refer to the cunit test framework and the mpich2 distribution 
# respectively.  The script uses these options to locate the those installed dependencies.  The boost and cunit 
# dependencies are located automatically since they are not optional.  The dependencies must all be installed in 
# the ~/opt directory.  
# After installation and construction the script runs a "make check" on the build.  Errors from the make check will
# be emailed to the user-email specified in the git global user.email variable.  Configure errors are also mailed 
# to the same address. This can be changed easily if the IOFSL-group should be emailed instead.    


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
  for ((i=0; i<${#lines[3]}; i++))
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
  echo "testing commit $commit" > runtest_results.txt
  echo "commit $commit committed by $committer_email" >> runtest_results.txt
  git checkout $commit
  scripts/runtest.sh | tee -a runtest_results.txt 
  echo "This concludes the test of commit $commit" >> runtest_results.txt
  echo "$commit" >> autotest/tested_commits.txt
  echo "mailing results"
  ./autotest/runtest_result.mail 
  git checkout master  
}

get_user_email

