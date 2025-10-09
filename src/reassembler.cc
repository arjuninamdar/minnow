#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t available_capacity = writer().available_capacity();
  if ( buffer.size() != available_capacity ) {
    buffer.resize( available_capacity );
    init.resize( available_capacity );
  }

  if ( is_last_substring ) {
    last_index = first_index + data.size();
    seen_last_substr = true;
  }

  // this checks and returns if the string is out of bounds
  if ( data == "" ) {
    // special index handling is required for checking if data is an empty string
    if ( first_index < start_index || first_index >= start_index + available_capacity )
      return;
  } else if ( ( first_index + data.size() <= start_index )
              || ( first_index >= start_index + available_capacity ) ) {
    return;
  }

  // if the substring is out of bounds to the right, we want to ensure that we discard irrelevant bytes
  if ( static_cast<int64_t>( first_index + data.size() ) - static_cast<int64_t>( start_index + available_capacity )
       > 0 ) {
    data = data.substr( 0, data.length() - ( first_index + data.size() ) + ( start_index + available_capacity ) );
  }

  // if the substring is before the start index, discard the beginning up to the start_index (we've already seen
  // these bytes)
  if ( first_index < start_index ) {
    data = data.substr( start_index - first_index );
    first_index = start_index;
  }

  copy( data.begin(), data.end(), buffer.begin() + ( first_index - start_index ) );
  fill( init.begin() + ( first_index - start_index ),
        init.begin() + ( first_index - start_index ) + data.length(),
        true );

  uint64_t to_pop_index = 0;
  while ( to_pop_index < buffer.size() and init[to_pop_index] ) {
    to_pop_index += 1;
  }

  string to_pop( buffer.begin(), buffer.begin() + to_pop_index );
  output_.writer().push( to_pop );

  buffer.erase( buffer.begin(), buffer.begin() + to_pop_index );
  init.erase( init.begin(), init.begin() + to_pop_index );

  start_index += to_pop_index;

  if ( seen_last_substr and start_index >= last_index ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t count = 0;
  for ( char c : buffer ) {
    if ( c != '\0' )
      count += 1;
  }

  return count;
}
