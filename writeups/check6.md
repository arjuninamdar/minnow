Checkpoint 6 Writeup
====================

My name: Arjun Inamdar 

My SUNet ID: arjun05 

I collaborated with: N/A 

I would like to thank/reward these classmates for their help: N/A 

This checkpoint took me about 3 hours to do. I did not attend the lab session.

 ## Program Structure and Design of the Router ##

There is one additional data structure I added to the router, which was the 
routing table. I maintained this as a list of tuples, each of which stored
the cooresponding prefix IP, the length of the prefix, an optional representing
the Address object of the next hop, and the corresponding interface number. I used
a vector because this was the simplest possible data structure implementation-wise: we 
required a data structure which stored the routing table entries such that we could
manually iterate over them and select the longest possible matching prefix. This resulted
in O(n) space complexity and a linear time complexity per-datagram in the queue. Some other
mechanisms which could speed this up could be a caching mechanism which stores a map of the some subset
of IP addresses to the respective routing table entry: however, there are complications when it
comes to expiring the correct entry at the right time (due to risk of outdated entries in the cache), 
which is why I didn't implement this. 

## Implementation Challenges ##

There were several challenges which were the result of misunderstanding specifics related to the assignment. 
I first misunderstood that the IP prefixes provided in the routing table didn't provide the IP addresses "shifted over,"
but rather in their full form, which was overcome with debugging and going to office hours to clarify this. I also
misunderstood/made memory-related mistakes; I didn't know that when we accessed the Queue via the 
`datagrams_received` function, I had to make the variable a reference. When I initially accessed it, I did it 
by copy, which resulted in errors when attempting to `pop` off of the queue. 

## Remaining Bugs ##
I passed all of the test cases - there weren't any remaining bugs in my program that I 
could reasonably find. 

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:

I primarily asked questions related to syntax, including:

1.) Is there an equivalent for tuple in C++?

I also went to office hours for debugging help. 
