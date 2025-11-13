#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"
#include <cmath>

using namespace std;

// How many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return sequence_numbers_in_flight_;
}

// How many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  if ( reader().has_error() ) {
    transmit( make_empty_message() );
    return;
  }

  // we assume that the window size is 1 for push when set to 0
  uint64_t effective_window_size = ( window_size == 0 ) ? 1 : window_size;
  uint64_t boundary_abs_seqno = absolute_ackno + effective_window_size;
  uint64_t last_valid_index
    = max( static_cast<uint64_t>( 0 ), boundary_abs_seqno - ( reader().bytes_popped() + 1 ) );
  string buffer = string( reader().peek().substr( 0, last_valid_index ) );

  if ( buffer.size() == 0 ) {
    TCPSenderMessage new_segment;
    new_segment.seqno = isn_ + ( SYN_sent ? 1 : 0 ) + reader().bytes_popped();

    if ( !SYN_sent ) {
      new_segment.SYN = true;
      SYN_sent = true;
      sequence_numbers_in_flight_ += 1;
    }

    // we are ready to send FIN if the ByteStream has completed
    if ( reader().is_finished()
         && ( new_segment.seqno.unwrap( isn_, reader().bytes_popped() ) + new_segment.sequence_length() )
              < boundary_abs_seqno
         && !FIN_sent ) {
      new_segment.FIN = true;
      sequence_numbers_in_flight_ += 1;
      FIN_sent = true;
    }

    // transmit if have seqnos to send
    if ( new_segment.sequence_length() > 0 ) {
      transmit( new_segment );
      outstanding_messages_.push_back( new_segment );
      timer_active = true;
    }
  }

  uint64_t segment_begin = 0;
  while ( segment_begin < buffer.size() ) {
    uint64_t segment_end = min( segment_begin + TCPConfig::MAX_PAYLOAD_SIZE, buffer.size() );
    TCPSenderMessage new_segment;
    new_segment.payload = buffer.substr( segment_begin, segment_end - segment_begin );

    // the +1 holds if the SYN has already been sent
    new_segment.seqno = isn_ + reader().bytes_popped() + 1;

    if ( !SYN_sent ) {
      new_segment.SYN = true;
      new_segment.seqno = isn_;
      sequence_numbers_in_flight_ += 1;
      SYN_sent = true;
    }

    reader().pop( new_segment.payload.size() );
    if ( reader().is_finished() && ( reader().bytes_popped() + 1 ) < boundary_abs_seqno ) {
      new_segment.FIN = true;
      sequence_numbers_in_flight_ += 1;
      FIN_sent = true;
    }

    sequence_numbers_in_flight_ += new_segment.payload.size();
    transmit( new_segment );
    outstanding_messages_.push_back( new_segment );
    timer_active = true;
    segment_begin = segment_end;
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage empty_message;
  empty_message.seqno = isn_ + reader().bytes_popped() + ( SYN_sent ? 1 : 0 ) + ( FIN_sent ? 1 : 0 );
  empty_message.RST = reader().has_error();
  return empty_message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.RST || reader().has_error() ) {
    reader().set_error();
    return;
  }

  auto curr_message = outstanding_messages_.begin();
  uint64_t largest_absolute_ackno = reader().bytes_popped() + ( SYN_sent ? 1 : 0 ) + ( FIN_sent ? 1 : 0 );

  // catches cases when the ackno claims to have seen seqnos not sent yet
  if ( msg.ackno.has_value() && largest_absolute_ackno < ( msg.ackno->unwrap( isn_, reader().bytes_popped() ) ) )
    return;

  absolute_ackno = msg.ackno->unwrap( isn_, reader().bytes_popped() );
  while ( curr_message != outstanding_messages_.end() ) {
    uint64_t last_seqno
      = curr_message->sequence_length() + curr_message->seqno.unwrap( isn_, reader().bytes_popped() );
    if ( absolute_ackno >= last_seqno ) {
      sequence_numbers_in_flight_ -= curr_message->sequence_length();
      curr_message = outstanding_messages_.erase( curr_message );
      consecutive_retransmissions_ = 0;
      time_elapsed = 0;

    } else {
      ++curr_message;
    }
  }

  window_size = msg.window_size;
  if ( outstanding_messages_.size() == 0 ) {
    timer_active = false;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  uint64_t curr_RTO_ms_ = initial_RTO_ms_ * pow( 2, consecutive_retransmissions_ );
  if ( timer_active ) {
    time_elapsed += ms_since_last_tick;
  }

  if ( time_elapsed >= curr_RTO_ms_ ) {
    if ( outstanding_messages_.size() > 0 ) {
      transmit( outstanding_messages_.front() );
    }

    if ( window_size > 0 ) {
      consecutive_retransmissions_ += 1;
    }

    time_elapsed = 0;
  }
}
