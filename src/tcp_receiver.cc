#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
	if (message.RST) {
		reader().set_error();
		return;
	} else if (message.SYN) {
		initial_sequence_number = message.seqno;
	} else if (!initial_sequence_number.has_value()) {
		return;
	}

	uint64_t first_index = message.seqno.unwrap(*initial_sequence_number, writer().bytes_pushed());
	if (!message.SYN) {
		first_index -= 1;
	}

	reassembler_.insert(first_index, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const
{
	
	TCPReceiverMessage message;	
	if (initial_sequence_number.has_value()) {
		uint64_t absolute_seqno = writer().bytes_pushed() + 1;
		if (writer().is_closed()) {
			absolute_seqno += 1;
		}

		message.ackno = Wrap32::wrap(absolute_seqno, *initial_sequence_number);
	}

	message.window_size = static_cast<uint16_t>(min(writer().available_capacity(), static_cast<uint64_t>(UINT16_MAX)));
	message.RST = reader().has_error();
	return message;
}


