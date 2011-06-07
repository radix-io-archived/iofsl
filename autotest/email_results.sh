#!/bin/bash
# This script musters error reports from runtest_results.txt and mails test-harness results to the 
# io-fwd-discuss list.  If there are make_errors the results are also emailed to the committer of 
# the resultig error.

error_report(){
  touch make_error_report.txt
  if
    egrep ' Error|*** ' runtest_results.txt
  then
    echo "test results for commit $commit" >> make_error_report.txt
    echo "this commit was finished testing at $(date)" >> make_error_report.txt
    echo >> make_error_report.txt
    sed -i 's/checking*//g' runtest_results.txt
    awk '/error/{c=5}c&&c--' runtest_results.txt >> make_error_report.txt; tail -5 runtest_results.txt >> make_error_report.txt

    echo "=========================================================" >> make_error_report.txt
  comm=${commit:0:7}
  echo | mutt -c $committer_email -c rjdamore@gmail.com -s "iofsl_vampir make error: for commit $comm" -a make_error_report.txt -- io-fwd-discuss@lists.mcs.anl.gov
  #echo | mutt -a make_error_report.txt -s "make error: Make FAIL for commit $comm" rjdamore@gmail.com
  edit_files
  else test_report
  fi
  cat make_error_report.txt >> make_error_report-pile.txt
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
  echo | mutt -c rjdamore@gmail.com.com -s "iofsl_vampir test report: PASS for commit $comm" -a test_report.txt -- io-fwd-discuss@lists.mcs.anl.gov

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
  make clean
  make distclean
  git checkout iofsl_vampir
}
error_report
