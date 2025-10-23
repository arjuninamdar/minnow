Checkpoint 3 Writeup
====================

My name: Arjun Inamdar 

My SUNet ID: arjun05     

I collaborated with: Ganesh Venu

I would like to thank/reward these classmates for their help: ganeshve (Ganesh Venu)

This checkpoint took me about 10 hours to do. I did not attend the lab session.


## Program Structure and Design ##

Note: I ended up changing my Bytestream implementation and made it based off of
Keith's solution from lab. This is due to implementation difficulties with the 
peek() function (I needed to see the entire buffer). 

I stored the outstanding messages list as a "std::list," which 
is internally maintained as a doubly-linked list. This supports 
constant time insertion and removal at the tradeoff of linear random element
access. However, for this use case, this made sense because we don't require
random element access. We just need the ability to iterate over the list 
and remove segments which have been acknowledged (which fits the doubly
linked list use case perfectly). Additionally, appending to the end is
constant time, which makes implementation simple. 

I also chose to maintain the timer with a minimal set of state variables (the 
initial RTO, consecutive retransmissions, and whether or not it was active)
for simplicity in state management. This entailed computing other necessary
variables (like the current RTO) using math (from the understanding that 
the timer doubles anytime the consecutive retransmissions goes increments).

## Alternative design choices ##

I considered other data structures including a vector to represent
the internal outstanding messages list. Implementation
would've been similarly complex to the list data structure. One advantage is 
the data structure has O(1) random element access, but this isn't particularly compelling for this 
use case. Additionally, removing an element from middle of the list potentially takes
linear time: a significant slowdown, as this is necessary in the receive function. I also 
considered using a deque, but this approach has similar trade-offs to the vector implementation.
Complexity would've been similar, but the O(n) removal would've been a significant 
cost.

I also considered creating separate state variables for the clock variables
that I computed (for example, the current RTO), but chose not to due to the
simplicity of the computation and cleanliness of the minimalist state implementation.

## Implementation Challenges ##

Indexing logic was difficult to handle. There were many edge cases which I had to find difficult 
and manually debug (oftentimes manually, using GDB). For example, I found it difficult to determine 
whether or not an ackno provided (in a receive call) was valid or not, and this required lots of manual
debugging and hand tracing test-cases to validate my logic. Handling cases when to include/omit SYN
in the calculation of the current sequence number was difficult aswell, especially in the push function
when it's called before and after the SYN has been sent.

## Remaining Bugs ##

All of the tests passed, and I couldn't think of any specific bugs.

## Hands On Activity ##

4.1.1.) I successfully setup TCP connection between two instances of Linux's TCP 
implementation. However, in order to configure the connection, I had to run
"sudo ./scripts/tun.sh start 144" prior. 

4.1.2.) the tcp_ipv4 implementation needs to wait when 
it's the first one to close (when I set it up as the client). 

4.1.3.) This ran properly and the SHA-256 hashes matched up. 1000000 bytes (1 MB)

4.2.) I did this part with Ganesh Venu. Here are the respective largest file sizes and
their hashes: 

Sender SHA-256: 7393814d02449d5e1b1c23034ea8b3dc909486f78f4084dfac64f05d55c9c1fa
Size: 1MB

Receiver SHA-256: 350d8635c16e456b9fd5aeb92817909ae655bae3adcede18fcf4aee70ce6acbf
Size: 1000005 bytes (approximately 1MB)

4.3.) This functions properly, and here are the contents of the HTTP 
message: QWx0NhMPkoM/bJr/ohvHXlviFhOyYrYb+qqdOnwLYo4.


- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

I primarily asked programming related questions and high-level implementation related ones, 
such as: 

1. How does substring work in C++? 
2. Does "string_view" mutate with the underlying string?
3. How does std::list work in C++? Is it a doubly-linked list, and if so, is the 
removal time complexity O(1)? 
4. What are the tradeoffs between generally using a vector and a list? 
Etc.
