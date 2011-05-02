#!/bin/bash
# This script gathers relevant information from the IOFSL-dist-test.sh script test results and mails them to the desired parties.  The mutt mail commands can be edited for the desired paties to be mailed to.  


error_report(){
  rm -f make_error_report.txt
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
  echo | mutt -c $committer_email -c dkimpe@mcs.anl.gov -c drieskimpe@gmail.com -c rjdamore@gmail.com -c maxadam@mcs.anl.gov -c max@trefonides.com -a make_error_report.txt -s "clientrpc make error: for commit $comm" io-fwd-discuss@lists.mcs.anl.gov

  #echo | mutt -a make_error_report.txt -s "clientrpc make error: for commit $comm" rjdamore@gmail.com
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
  echo | mutt -c $committer_email -c dkimpe@mcs.anl.gov -c drieskimpe@gmail.com -c rjdamore@gmail.com -c maxadam@mcs.anl.gov -c max@trefonides.com -a test_report.txt -s "clientrpc test PASS for commit $comm" io-fwd-discuss@lists.mcs.anl.gov  

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
  rm -f runtest_results.txt
  rm -f make_error_report.txt
  make clean
  make distclean
  git checkout clientrpc
}
error_report
