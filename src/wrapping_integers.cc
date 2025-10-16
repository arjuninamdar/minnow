#include "wrapping_integers.hh"
#include "debug.hh"
#include <cstdint>
#include <vector>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = raw_value_ - zero_point.raw_value_;
  uint64_t checkpoint_wrap = checkpoint >> 32;
  vector<uint64_t> possible_values;

  if ( checkpoint_wrap > 0 ) {
    possible_values.push_back( ( ( checkpoint_wrap - 1 ) << 32 ) + offset );
  }

  if ( checkpoint_wrap < UINT32_MAX ) {
    possible_values.push_back( ( ( checkpoint_wrap + 1 ) << 32 ) + offset );
  }

  uint64_t closest_value = ( checkpoint_wrap << 32 ) + offset;
  for ( uint64_t possible_value : possible_values ) {
    uint64_t new_distance
      = ( possible_value > checkpoint ) ? possible_value - checkpoint : checkpoint - possible_value;
    uint64_t old_distance
      = ( closest_value > checkpoint ) ? closest_value - checkpoint : checkpoint - closest_value;
    if ( new_distance < old_distance ) {
      closest_value = possible_value;
    }
  }

  return closest_value;
}
