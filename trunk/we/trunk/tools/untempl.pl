# strip template arguments, mirko.rahn@itwm.fraunhofer.de

use strict;

for (<>)
{
  my $depth = 0;

  for (split //)
    {
      if (/</)
        {
          print; $depth++;
        }
      elsif (/>/)
        {
          print; $depth--;
        }
      elsif ($depth == 0)
        {
          print;
        }
    }
}
