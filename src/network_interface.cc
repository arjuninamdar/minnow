#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

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
	EthernetHeader ethernet_header;
	ethernet_header.src = ethernet_address_; 
	dgram.header.src = ip_address_.ipv4_numeric();
	dgram.header.dst = next_hop.ipv4_numeric();

	if (ethernet_to_ip_.find(next_hop.ipv4_numeric()) != ethernet_to_ip_.end()) {
		ethernet_header.type = EthernetHeader::TYPE_IPv4;
		ethernet_header.dst = ethernet_to_ip_[next_hop.ipv4_numeric()].first;
		ethernet_frame.header = ethernet_header;
		ethernet_frame.payload = serialize(dgram);
		transmit(ethernet_frame);
		return;
	} else if (arp_message_times_.find(next_hop.ipv4_numeric()) != arp_message_times_.end()) {
		arp_message_times_[next_hop.ipv4_numeric()].push_back({ dgram, curr_MS });
		return;
	}

	ARPMessage arp_message;
	arp_message.opcode = ARPMessage::OPCODE_REQUEST;
	arp_message.sender_ethernet_address = ethernet_address_;
	arp_message.sender_ip_address = ip_address_.ipv4_numeric();
	arp_message.target_ip_address = next_hop.ipv4_numeric();
	
	ethernet_header.type = EthernetHeader::TYPE_ARP;
	ethernet_header.dst = ETHERNET_BROADCAST;
	ethernet_frame.header = ethernet_header;
	ethernet_frame.payload = serialize(arp_message);
	transmit(ethernet_frame);

	arp_message_times_[next_hop.ipv4_numeric()] = { { dgram, curr_MS } };
  /*
  if (known) {
    
  }

  debug( "unimplemented send_datagram called" );
  (void)dgram;
  (void)next_hop;
  */
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
	if (frame.header.type == EthernetHeader::TYPE_IPv4) {
		InternetDatagram internet_datagram;
		if (parse(internet_datagram, frame.payload)) {
			datagrams_received_.push(internet_datagram);
		}

		return;
	}
	
	ARPMessage arp_message;
	if (!parse(arp_message, frame.payload)) {
		return;		
	}

	ethernet_to_ip_[arp_message.sender_ip_address] = { arp_message.sender_ethernet_address, curr_MS };
	if (arp_message.opcode == ARPMessage::OPCODE_REQUEST) {
		ARPMessage arp_message_reply;
		arp_message_reply.opcode = ARPMessage::OPCODE_REPLY;
		arp_message_reply.sender_ip_address = ip_address_.ipv4_numeric();
		arp_message_reply.target_ip_address = arp_message.sender_ip_address;
		
		EthernetHeader ethernet_frame_reply_header;
		ethernet_frame_reply_header.dst = arp_message.sender_ethernet_address;
		ethernet_frame_reply_header.src = ethernet_address_;
		ethernet_frame_reply_header.type = EthernetHeader::TYPE_ARP;

		EthernetFrame ethernet_frame_reply;
		ethernet_frame_reply.header = ethernet_frame_reply_header;
		ethernet_frame_reply.payload = serialize(arp_message_reply);
		transmit(ethernet_frame_reply);
		return;
	}


	// [TODO]: we have an ARP reply. Is it true that sender will be the nexthop?
	auto vec = arp_message_times_[arp_message.sender_ip_address];
	for (auto itr = vec.begin(); itr != vec.end(); itr += 1) {
		send_datagram(itr->first, Address::from_ipv4_numeric(arp_message.sender_ip_address)); 
	}
	arp_message_times_.erase(arp_message.sender_ip_address);
	/*
	InternetDatagram dgram = arp_message_times_[arp_message.sender_ip_address].first;
	arp_message_times_.erase(arp_message.sender_ip_address);
	send_datagram(dgram, Address::from_ipv4_numeric(arp_message.sender_ip_address));
	*/

	/*
  debug( "unimplemented recv_frame called" );
  (void)frame;
	*/
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
	curr_MS += ms_since_last_tick;

	// [TODO]: How do I do this (now, these!!!) safely?
	for (auto it = ethernet_to_ip_.begin(); it != ethernet_to_ip_.end(); ) {
		// [TODO]: Store in constant? Also is the syntax correct?
		if ((curr_MS - it->second.second) >= (1000 * 30)) {
			it = ethernet_to_ip_.erase(it);
		} else {
			++it;
		}
	}
	
	for (auto outer_it = arp_message_times_.begin(); outer_it != arp_message_times_.end(); ) {
		auto vec = outer_it->second;

		for (auto it = vec.begin(); it != vec.end(); ) {
			if ((it->second - curr_MS) >= 5 * 1000) {
				it = vec.erase(it);
			} else {
				++it;
			}
		}


		if (outer_it->second.size() == 0) {
			outer_it = arp_message_times_.erase(outer_it);
		} else {
			++outer_it;
		}
	}
}
