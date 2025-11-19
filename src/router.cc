#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
	routing_table_.push_back({ route_prefix, prefix_length, next_hop, interface_num });
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
	for (shared_ptr<NetworkInterface> curr_interface : interfaces_) {
		queue<InternetDatagram>& datagrams = curr_interface->datagrams_received();
		while (!datagrams.empty()) {
			InternetDatagram datagram = datagrams.front();
			datagrams.pop();

			uint32_t final_dest = datagram.header.dst;
			optional<size_t> routing_table_entry;

			for (size_t i = 0; i < routing_table_.size(); ++i) {
				const int INT_MAX_WIDTH = 32;
				uint32_t curr_route_prefix = get<0>(routing_table_[i]);
				uint8_t curr_prefix_length = get<1>(routing_table_[i]);
				uint8_t shift_amount = (INT_MAX_WIDTH - curr_prefix_length);

				// if we shift right 32, this is the 0/0 entry, so we always match
				if ((shift_amount == INT_MAX_WIDTH || (final_dest >> shift_amount) == (curr_route_prefix >> shift_amount)) && (!routing_table_entry.has_value() || get<1>(routing_table_[*routing_table_entry]) < curr_prefix_length))
					routing_table_entry = i;
			}

			if (!routing_table_entry.has_value() || datagram.header.ttl-- <= 1)
				continue;

			datagram.header.compute_checksum();

			size_t interface_num = get<3>(routing_table_[*routing_table_entry]);
			optional<Address> next_hop_optional = get<2>(routing_table_[*routing_table_entry]);
			Address next_hop = next_hop_optional.value_or(Address::from_ipv4_numeric(final_dest));

			interface(interface_num)->send_datagram(datagram, next_hop);
		}
	}
}
