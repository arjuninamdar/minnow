#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] sender_ip_address the sender's IP address
//! \param[in] target_ip_address the target IP address
//! \param[in] opcode the ARP message type, either reply or request
//! \param[in] sender_ethernet_address the sender's Ethernet address
//! \param[in] target_ethernet_address the target's Ethernet address (empty if a ARP Request)
EthernetFrame NetworkInterface::construct_arp_message( uint32_t sender_ip_address,
                                                       uint32_t target_ip_address,
                                                       uint16_t opcode,
                                                       EthernetAddress sender_ethernet_address,
                                                       EthernetAddress target_ethernet_address = {} )
{
  ARPMessage arp_message;
  arp_message.sender_ip_address = sender_ip_address;
  arp_message.target_ip_address = target_ip_address;
  arp_message.sender_ethernet_address = sender_ethernet_address;

  // will be empty/default value for ARP Requests (address not known)
  arp_message.target_ethernet_address = target_ethernet_address;
  arp_message.opcode = opcode;

  EthernetHeader ethernet_header;
  ethernet_header.type = EthernetHeader::TYPE_ARP;
  ethernet_header.src = sender_ethernet_address;

  // always broadcast if this is an ARP Request
  ethernet_header.dst = ( opcode == ARPMessage::OPCODE_REQUEST ) ? ETHERNET_BROADCAST : target_ethernet_address;
  EthernetFrame ethernet_frame;
  ethernet_frame.header = ethernet_header;
  ethernet_frame.payload = serialize( arp_message );
  return ethernet_frame;
}

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( InternetDatagram dgram, const Address& next_hop )
{
  EthernetFrame ethernet_frame;

  if ( ethernet_to_ip_.find( next_hop.ipv4_numeric() ) != ethernet_to_ip_.end() ) {
    EthernetHeader ethernet_header;
    ethernet_header.src = ethernet_address_;
    ethernet_header.type = EthernetHeader::TYPE_IPv4;
    ethernet_header.dst = ethernet_to_ip_[next_hop.ipv4_numeric()].first;
    ethernet_frame.header = ethernet_header;
    ethernet_frame.payload = serialize( dgram );
  } else if ( arp_message_times_.find( next_hop.ipv4_numeric() ) != arp_message_times_.end() ) {
    arp_message_times_[next_hop.ipv4_numeric()].second.push_back( dgram );
    return;
  } else {
    // we know that no ARP request has been sent in past 5 seconds, so send
    ethernet_frame = construct_arp_message(
      ip_address_.ipv4_numeric(), next_hop.ipv4_numeric(), ARPMessage::OPCODE_REQUEST, ethernet_address_ );

    arp_message_times_[next_hop.ipv4_numeric()].first = curr_MS;
    arp_message_times_[next_hop.ipv4_numeric()].second = { dgram };
  }

  transmit( ethernet_frame );
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return;
  }

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram internet_datagram;
    if ( parse( internet_datagram, frame.payload ) ) {
      datagrams_received_.push( internet_datagram );
    }

    return;
  }

  ARPMessage arp_message;
  if ( !parse( arp_message, frame.payload ) ) {
    return;
  }

  // always learn the mapping and flush the cache, even if unintended for us
  ethernet_to_ip_[arp_message.sender_ip_address] = { arp_message.sender_ethernet_address, curr_MS };
  if ( arp_message.opcode == ARPMessage::OPCODE_REQUEST
       && arp_message.target_ip_address == ip_address_.ipv4_numeric() ) {
    EthernetFrame ethernet_frame_reply = construct_arp_message( ip_address_.ipv4_numeric(),
                                                                arp_message.sender_ip_address,
                                                                ARPMessage::OPCODE_REPLY,
                                                                ethernet_address_,
                                                                arp_message.sender_ethernet_address );
    transmit( ethernet_frame_reply );
  }

  if ( arp_message_times_.find( arp_message.sender_ip_address ) != arp_message_times_.end() ) {
    for ( InternetDatagram ip_datagram : arp_message_times_[arp_message.sender_ip_address].second ) {
      send_datagram( ip_datagram, Address::from_ipv4_numeric( arp_message.sender_ip_address ) );
    }

    arp_message_times_.erase( arp_message.sender_ip_address );
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  curr_MS += ms_since_last_tick;

  for ( auto it = ethernet_to_ip_.begin(); it != ethernet_to_ip_.end(); ) {
    if ( ( curr_MS - it->second.second ) >= ( 1000 * 30 ) ) {
      it = ethernet_to_ip_.erase( it );
    } else {
      ++it;
    }
  }

  for ( auto it = arp_message_times_.begin(); it != arp_message_times_.end(); ) {
    // we always clear the entire cache if the oldest datagram transmission >= 5 seconds
    if ( ( curr_MS - it->second.first ) >= ( 1000 * 5 ) ) {
      it = arp_message_times_.erase( it );
    } else {
      ++it;
    }
  }
}
