#!/bin/bash
# This script builds, installs, and checks the IOFSL software.  It pulls the latest edition of the IOFSL software
# from the git repo.  The options -c and -m refer to the cunit test framework and the mpich2 distribution 
# respectively.  The script uses these options to locate the those installed dependencies.  The boost and cunit 
# dependencies are located automatically since they are not optional.  The dependencies must all be installed in 
# the ~/opt directory.  
# After installation and construction the script runs a "make check" on the build.  Errors from the make check will
# be emailed to the user-email specified in the git global user.email variable.  Configure errors are also mailed 
# to the same address. This can be changed easily if the IOFSL-group should be emailed instead.    


git pull


get_user_email(){
  echo "Locating git user email."
  git config -l > user_email.tmp
  grep 'user.email=*' user_email.tmp > user_email.tmp2 ; rm -r user_email.tmp
  sed 's/user.email=//g'  user_email.tmp2 > user_email.mod ; rm -f user_email.tmp2
  user_email=$(cat user_email.mod)
  rm -f user_email.mod
  if [[ $user_email == *@* ]]
  then
    echo "User email found ----> "   $user_email
  else
    echo "Have you configured your git global email? It's needed for error reporting"
  fi
  prep
}

find_cunit(){
  echo " Searching for cunit dependency in ~/opt"
  wcunit=$(find ~/opt -type d -name "cunit*")  
  if [[ -n "$wcunit" ]]
  then
    echo "found cunit! ----> " $wcunit
  else  
    echo "can't find cunit i ~/opt, or it doesn't have the format \"cunit*\", aborting build" ; exit 0
  fi
  echo
}
find_boost(){
  echo " Searching for boost dependency in ~/opt"
  wboost=$(find ~/opt -type d -name "boost-*")
  if [[ -n "$wboost" ]] 
  then
    echo "found boost! ----> " $wboost
  else
    echo "can't find boost in ~/opt, or it doesn't have the format \"boost-*\", aborting build" ; exit 0
  fi
  echo
}
find_bmi(){
  echo " Searching for bmi dependency in ~/opt"
  wbmi=$(find ~/opt -type d -name "bmi-*")
  if [[ -n "$wbmi" ]]
  then
    echo "found bmi! ----> " $wbmi
  else
    echo "can't find bmi in ~/opt, or it doesn't have the format \"bmi-*\", aborting build" ; exit 0
  fi
  echo
}
find_mpi(){
  echo " Searching for mpi dependency in ~/opt"
  wmpi=$(find ~/opt -type d -name "mpich2*")
  if [[ -n "$wmpi" ]]
  then
    echo "found mpi! ----> " $wmpi
  else
    echo "can't find mpi in ~/opt, or it doesn't have the format \"mpich2*\", aborting build" ; exit 0
  fi
  echo
}


while getopts "cm" opt; do
  case $opt in
    c)
        echo "You've selected cunit."
        find_cunit
        ;;
    m)
        echo "You've selected mpich's mpi."
        find_mpi
        ;;
    [?])
        echo "invalid option, only cunit "-c", and mpi "-m" are options! bmi and boost are located automatically."
        exit 0
        ;;
  esac
done
find_boost
find_bmi

prep(){
  ./prepare
  if [ -e ~/iofsl/configure ] 
  then 
    config
  fi
}

config(){
  prefix=--prefix=$HOME/iofsl-install 
  conf_stmt1="./configure --with-cunit=$wcunit --with-boost=$wboost --with-bmi=$wbmi $prefix"
  conf_stmt2="./configure --with-boost=$wboost --with-bmi=$wbmi --with-mpi=$wmpi $prefix"
  conf_stmt3="./configure --with-cunit=$wcunit --with-boost=$wboost --with-bmi=$wbmi --with-mpi=$wmpi $prefix"
 
  if [[ -n "$wcunit" ]] && [[ -z "$wmpi" ]]
  then
    echo $conf_stmt1
    $conf_stmt1
  elif [[ -z "$wcunit" ]] && [[ -n "$wmpi" ]]
  then
    echo $conf_stmt2
    $conf_stmt2
  elif [[ -n "$wcunit" ]] && [[ -n "$wmpi" ]]
  then
    echo $conf_stmt3
    $conf_stmt3
  fi
  if [ -e ~/iofsl/Makefile ]
  then
    build
  else
    mail -s "IOFSL configure report" $user_email < ~/iofsl/config.log 
  fi
}

build(){
  make
  make install
  if [ -e ~/iofsl-install ]
  then
    check
  fi
}
check(){
date > ~/iofsl/autotest/make_check
make check >> ~/iofsl/autotest/make_check
mail -s "IOFSL build and make_check result" $user_email < ~/iofsl/autotest/make_check
}
get_user_email

exit 0
