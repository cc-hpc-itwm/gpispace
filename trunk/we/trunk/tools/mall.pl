# accumulate the output of l.cpp, mirko.rahn@itwm.fraunhofer.de

use strict;

my $m=0;
my $f=0;
my %s;

while (<>)
{
  if (/^malloc/)
    {
      my (undef,$byte,$addr) = split;
      $m += $byte;
      $s{$addr} = $byte;
      print "~~ m = $m, f = $f, d = ", $m-$f,"\n";
    }
  elsif (/^free/)
    {
      my (undef,$addr) = split;
      my $b = $s{$addr} or die "strange";
      $f += $b;
      print "~~ m = $m, f = $f, d = ", $m-$f, "\n";
    }
  else
    {
      print;
    }
}
