#!/usr/bin/perl -w
use strict;

#
# variables
#
my $mountDir = "/tmp/sysio";

#
# setup the test directory
#
system("mkdir $mountDir");


#
# run the tests...
#
print "##### TEST 1: Do we follow symlinks to the original file? #####\n";
system("./zoidfsapi_symlink_test $mountDir");

print "##### TEST 2: Does the file handle for a renamed file change? #####\n";
system("./zoidfsapi_rename_test $mountDir");

print "##### TEST 3: What files can we access with restrictive permissions? #####\n";
system("./zoidfsapi_lookup_perm_test");

#
# cleanupthe test directory
#
system("rm -rf $mountDir");
