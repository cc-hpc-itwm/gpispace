import java.math.BigInteger;
import java.nio.ByteBuffer;

public class main {
  static
  {
    System.loadLibrary ("gpi");
  }

  public static void main(String argv[]) {
    int rank = 0;

    gpi.gpi_init ( rank == 0 ? 1 : 0
                 , "/var/tmp/S-gpi-space.1000.0"
                 , "/home/alex/.local/sdpa/libexec/fhg/plugins"
                 , "/home/alex/src/sdpa/build/application/ufbmig/xml/ufbmig_prepare.pnet" // prepare
                 , "/home/alex/src/sdpa/build/application/ufbmig/xml/ufbmig_init.pnet"    // initialize
                 , "/home/alex/src/sdpa/build/application/ufbmig/xml/ufbmig_calc.pnet"    // calc
                 , "/home/alex/src/sdpa/build/application/ufbmig/xml/ufbmig_calc.pnet"    // update salt mask
                 , "/home/alex/src/sdpa/build/application/ufbmig/xml/ufbmig_done.pnet"    // finalize
                 , "/etc/passwd" // input xml
                 );

    if (rank == 0)
    {
      gpi.gpi_wait (rank);

      // barrier

      int fd = gpi.gpi_open ("data.output");
      int rc = 0;

      if (fd >= 0)
      {
        System.out.println ("opened handle:" + fd);
        ByteBuffer buffer = ByteBuffer.allocateDirect (4096);

        // should probably seek to be able to do parallel reads
        rc = gpi.gpi_read (fd, buffer, 1000);
        System.out.println ("rc = " + rc);

        if (rc <= 0)
        {
          System.err.println ("nothing read");
          return;
        }

        while (rc > 0 && buffer.hasRemaining ())
        {
          System.out.print ((char)buffer.get ());
          --rc;
        }

        System.out.flush ();
      }
      else
      {
        System.err.println ("could not open test handle");
      }
    }
    else
    {
      // barrier

      // open output handle
    }
  }
}
