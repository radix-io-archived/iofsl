#!/usr/bin/env perl

use strict;
use warnings;


use Getopt::Long;
use Pod::Usage;

my $opt_help =0; 
my $opt_man = 0; 
my %opt_params; 

my $res = GetOptions ('help|?' => \$opt_help, 'man' => \$opt_man, 
        'param=s' => \%opt_params)
     or pod2usage (2); 

pod2usage(1) if $opt_help;
pod2usage(-existstatus => 0, -verbose => 2) if $opt_man;

die "Need template file!" unless scalar @ARGV; 

sub getvar ($)
{
   shift =~ /((\S+):)?(\S+)/;
   my ($op, $name) = ($2,$3); 
   
   $op |= ""; 

   die "Unknown param: '$name' (Op: '$op')" unless defined ($opt_params{$name}); 

   my $value = $opt_params{$name}; 
   if ($op eq "UP")
   {
      $value = uc($value); 
   }
   if ($op eq "DEF")
   {
      $value = uc($value);
      $value =~ s/::/_/g;
   }
   return $value; 
}


for my $templatefile (@ARGV)
{
   my $file; 
   open $file, "<", $templatefile
     or die "Error opening template $templatefile: $!"; 
   while (my $line = <$file>)
   {
      unless ($line =~ /@[a-z,A-Z,0-9,:]+@/)
      {
         print $line;
         next; 
      }

      while ($line =~ /@([a-z,A-Z,0-9,:]+)@/)
      {
         print "$`";
         print getvar ($1); 
         $line = $';
      }
      print $line; 

   }
   close $file; 
}


__END__

=head1 NAME

filltemplate.pl - Tool to generate C++ .hh/.cpp skeletons

=head1 SYNOPSIS

filltemplate.pl [ --param varname=varval ... ] templatefile

=cut
