#!/bin/bash

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
  prepare_config
}

find_cunit(){
  echo " Searching for cunit dependency in ~/opt"
  wcunit=$(find ~/opt -type d -name "cunit*")
  echo "found cunit! ----> " $wcunit
  echo
}
find_boost(){
  echo " Searching for boost dependency in ~/opt"
  wboost=$(find ~/opt -type d -name "boost-*")
  echo "found boost! ----> " $wboost
  echo
}
find_bmi(){
  echo " Searching for bmi dependency in ~/opt"
  wbmi=$(find ~/opt -type d -name "bmi-*")
  echo "found bmi! ----> " $wbmi
  echo
}
find_mpi(){
  echo " Searching for mpi dependency in ~/opt"
  wmpi=$(find ~/opt -type d -name "mpich2*")
  echo "found mpi! ----> " $wmpi
  echo
}


while getopts "cbim" opt; do
  case $opt in
    c)
        echo "You've selected cunit."
        find_cunit
        with_deps=(1)
        ;;
    b)
        echo "You've selected boost."
        find_boost
	with_deps=(1 2)
        ;;
    i)
        echo "You've selected bmi."
        find_bmi
        with_deps=(1 2 3)
        ;;
    m)
        echo "You've selected mpich's mpi."
        find_mpi
        with_deps=(1 2 3 4)
        ;;
    [?])
        echo "invalid option, only cunit "-c", bmi "-i", boost "-b" and mpi "-m" are options!"
        ;;
  esac
done

prepare_config(){
  prefix=--prefix=$HOME/iofsl-install 

  if [[ ${#with_deps[@]} -eq 1 ]] ; then
    echo "./configure --with-cunit=$wcunit $prefix"
  else
  if [[ ${#with_deps[@]} -eq 2 ]] ; then
    echo "./configure --with-cunit=$wcunit --with-boost=$wboost $prefix"
  else
  if [[ ${#with_deps[@]} -eq 3 ]] ; then
   echo "./configure --with-cunit=$wcunit --with-boost=$wboost --with-bmi=$wbmi $prefix"
  else 
  if [[ ${#with_deps[@]} -eq 4 ]] ; then
    echo "./configure --with-cunit=$wcunit --with-boost=$wboost --with-bmi=$wbmi --with-mpi=$wmpi $prefix"
  fi
  fi
  fi
  fi
}

get_user_email

exit 0

