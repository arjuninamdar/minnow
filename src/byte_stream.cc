#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push( string data )
{
  string to_push = data.substr( 0, min( available_capacity(), data.length() ) );
  for ( unsigned char b : to_push ) {
    dq.push_back( b );
  }

  tot_pushed += to_push.length();
  dq_len = dq.size();
  peek_contents.assign( dq.begin(), dq.begin() + min( dq_len, static_cast<uint64_t>( PEEK_CONTENTS_SIZE ) ) );
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  closed_ = true;
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  return closed_;
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  return capacity_ - dq_len;
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  return tot_pushed;
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  return string_view( peek_contents );
}

// Remove `len` bytes from the buffer.
void Reader::pop( uint64_t len )
{
  int to_pop = min( len, dq_len );
  for ( int i = 0; i < to_pop; ++i ) {
    dq.pop_front();
  }

  tot_popped += to_pop;
  dq_len -= to_pop;
  peek_contents.assign( dq.begin(), dq.begin() + min( dq_len, static_cast<uint64_t>( PEEK_CONTENTS_SIZE ) ) );
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  return closed_ && ( bytes_buffered() == 0 );
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  return dq_len;
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  return tot_popped;
}
