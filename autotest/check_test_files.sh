#!/bin/bash

read_failures(){
if [[ -s ~/iofsl/failure_report.txt ]]; then
grep "committed" ../failure_report.txt | awk '{print $5}' > failure_emails.txt
else
echo "failure_report.txt is empty."
fi;
failures_to_array
}

failures_to_array(){
failures_IFS=$IFS
IFS=$'\n'
failures=($(cat failure_emails.txt))
IFS=$failures_IFS
mail_failures
}

mail_failures(){
for i in "${failures[@]}"
do
mail -s "IOFSL commit test failure report" $i < ~/iofsl/failure_report.txt 
done  
}

if [[ -s ~/iofsl/test_report.txt ]]; then
mail -s "IOFSL commits test report" rjdamore@lanl.gov < ~/iofsl/test_report.txt
#mail -s "IOFSL commits test report" io-fwd-commits@lists.mcs.anl.gov < ~/iofsl/test_report.txt
else
echo "test_report.txt is empty"
fi;
read_failures
