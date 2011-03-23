#!/bin/bash
# This script gathers relevant information from the IOFSL-dist-test.sh script test results and mails them to the desired parties.  The mutt mail commands can be edited for the desired paties to be mailed to.  


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
  #echo | mutt -c  $committer_email dkimpe@mcs.anl.gov drieskimpe@gmail.com rjdamore@gmail.com -a make_error_report.txt -s "make error: Make FAIL for commit $comm" io-fwd-discuss@lists.mcs.anl.gov
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
  i#echo | mutt -c $committer_email dkimpe@mcs.anl.gov drieskimpe@gmail.com rjdamore@gmail.com -a failure_report.txt -s "failure report for commit $comm FAIL" io-fwd-discuss@lists.mcs.anl.gov 
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
  #echo | mutt -a test_report.txt -s "test report for commit $comm PASS" rjdamore@gmail.com
  #echo | mutt -c drieskimpe@gmail.com -a test_report.txt -s "test report for commit $comm PASS" rjdamore@gmail.com
  #echo | mutt -c drieskimpe@gmail.com -a test_report.txt -s "test report for commit $comm PASS" rjdamore@lanl.gov
  echo | mutt -c $committer_email dkimpe@mcs.anl.gov drieskimpe@gamail.com rjdamore@gmail.com -a test_report.txt -s "test report for commit $comm PASS" io-fwd-discuss@lists.mcs.anl.gov 
  edit_files
}

edit_files(){
  echo "==========================================================" >> repo-testing-results-pile.txt
  echo $(date) >> repo-testing-results-pile.txt
  cat runtest_results.txt >>  repo-testing-results-pile.txt
  echo "==========================================================" >> repo-testing-results-pile.txt
  if
    grep $commit autotest/tested_commits.txt
  then :
  else
      echo "$commit" >> autotest/tested_commits.txt
  fi
  #rm -f runtest_results.txt
  chmod -R 777 iofwd-1.0.0
  rm -rf iofwd-1.0.0
  rm -rf iofwd-1.0.0.tar.gz
  make clean
  make distclean
  git checkout master
}
error_report
