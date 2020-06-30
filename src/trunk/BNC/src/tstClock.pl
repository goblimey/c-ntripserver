#!/usr/bin/perl -w

use strict;

# List of Parameters
# ------------------
my($fileName) = @ARGV;

if (!defined($fileName)) {
  die "Usage: tstClock.pl fileName\n";
}

open(inFile,  "<$fileName") || die "Cannot open file $fileName";

my %firstClk;

while ( defined(my $line=<inFile>) ) {
  if ($line =~ /Full Clock/) {
    my @p = split(/\s+/, $line);
    my $dateStr = $p[0];
    my $timeStr = $p[1];
    my $prn     = $p[4];
    my $iod     = $p[5];
    my $clk     = $p[6];
    if (!defined($firstClk{$prn})) {
      $firstClk{$prn} = $clk;
    }
    printf("%s %s %s  %14.4f\n", 
           $dateStr, $timeStr, $prn, $clk - $firstClk{$prn});
  }
}

close(inFile);

