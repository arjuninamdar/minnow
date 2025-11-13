Checkpoint 5 Writeup
====================

My name: Arjun Inamdar 

My SUNet ID: arjun05 

I collaborated with: Ganesh Venu 

I would like to thank/reward these classmates for their help: ganeshve 

This checkpoint took me about 6 hours to do. I did not attend the lab session.

## Program Structure and Design of the NetworkInterface ## 

I maintained an unordered map to store the mappings of the IP Addresses to the
Ethernet/MAC addresses. This has various advantages, including O(1) average time
lookup, removal, and addition (in stark contrast to storing a vector of pairs, which 
would be linear for any of these operations). Implementation was also simple, 
as all I required was a simple key-value store and this provided it easily, wheras
with a vector more complex implementation (relying upon iteration/etc.) for basic
operations would be necessary.

It was also required that I store a data structure of the IP addresses to a 
pair, which stored a vector of Internet datagrams and the oldest time (in MS)
that they were transmitted. This was required to keep track of the ARP requests
previously sent (to avoid flooding the network with such requests) and to additionally
remember the respective datagrams previously sent to avoid losing previous requests
and so that they could be re-sent in the future. I only stored the oldest time sent for 
memory related efficiencies: I initially created a vector of the pairs of (datagram, time sent)
pairs, but soon realized that we needed to invalidate the entire cache on expire, meaning we 
only needed to know the time at which the first datagram was sent.

Another state variable required was the total accumulated milliseconds 
from which the program began running. This was to maintain a general sense of 
time and so that we can invalidate both of the caches whenever necessary.

I also generally found myself repeating the same code whenever I wanted to create
an ARPMessage (create the ARP message itself, the Ethernet header, and then the frame).
To address this and to simplify the code, I created a helper function (called
`construct_arp_message`) to assist in the creation of ARPMessages and constructing
the entire Ethernet frame.

## Implementation Challenges ##

One significant challenge was understanding how/when to invalidate the cache
for ARP messages and also how to handle the ARP requests not intended for us. 
These were especially difficult to handle because much of the intended behavior 
wasn't too explicitly listed in the handout (for example, that we should learn from
ARPRequests and flush the cache even if they aren't intended for us, and that we 
should generaly flush the queue after sending an ARPRequest). I handled these sorts
of errors by caarefully tracing through the test cases to understand the intended
behavior (these were in great detail), and implementing the respective changes in 
my code. This required lots of refactoring throughout. 


## Remaining Bugs ##

All of the tests passed, and I couldn't find any remaining bugs in the code
that would significantly diverge from desired behavior. 

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:

1.) When you call tick, which datagrams do you invalidate? Do you invalidate all of them
if you can send another ARP message? 

2.) What data structures did you consider at a high level and what are some trade-offs
between them (specifically related to queue vs. a vector for the ARP messaging cache). 

3.) How simple is deletion implementation with an unordered set in C++?
