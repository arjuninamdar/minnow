// NOTE: You must run this as root!

#include "socket.hh"

using namespace std;

int main()
{
  string d;

  d += 0b0100'0101;
  d += string( 7, 0 );

  d += 64;
  d += 17;

  d += string( 6, 0 );
  d += char( 10 );
  d += char( 144 );
  d += char( 0 );
  d += char( 143 );

  d += 4;
  d += 1;

  d += 4;
  d += char( 0 );

  string message = "Ganesh is goated";

  d += char( 0 );
  d += char( message.length() + 8 );
  d += string( 2, 0 );

  d += message;

  RawSocket {}.send( d, Address { "10.144.0.143" } );
  return 0;
}
