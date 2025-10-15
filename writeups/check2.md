Checkpoint 2 Writeup
====================

My name: Arjun Inamdar 

My SUNet ID: arjun05 

I collaborated with: ganeshve 

I would like to thank/reward these classmates for their help: Ganesh Venu 

This lab took me about 10 hours to do. I did not attend the lab session.

Program Structure and Design: When designing Wrap32, I took advantages of the properties of
integer overflow. I know that upon overflow causes the result to wrap around re-starting from
0, so I simply added the jump to the starting point after casting. 

For unwrap, I took advantage of the fact that once we found the appropriate "initial jump"
from the current number to the zero point, we simply had to find the approprite bit
segment in the upper 32 bits of the number. Being more mathematically specific, 
2^(32)c + (the computed offset) is a valid absolute seqno given any c >= 0. From here, 
finding this c given the checkpoint becomes easier, as we only care about the same or 
neighboring (+/- 1) relative to the checkpoints' c. As a result, I considered all of the 
numbers and returned the one closest to the checkpoint.

In TCPReceiver, there are several important design decisions I made. I first chose to defer the
RST and error state to the "reader" after initially implementing it as a separate state
variable. This avoids redundancy in the state. I also chose to catch invalid indices by
largely deferring this to the Reassembler: if we have an out of bounds seqno, the Reassembler
will catch this and simply ignore this seqno. 

Alternative design choices: I initially used a state variable for RST status.
However, I decided to use "set_error" and "has_error" to manage this. I also decided to 
allow the Reassembler to handle out-of-bounds indices, as previously described. 

Implementation Challenges: One challenge was making the "unwrap" function. 
I had to mathematically think what defined the valid absolute numbers, 
and writing down the resulting sum (from binary) and simplifying it
into the offset + 2^(32)c form helped. I also used GDB to extensively
test and resolve cases resulting from index errors in the "receive" function,
especially as they related to the SYN flag and the sequence number this
occupies. 


Remaining Bugs: If we have a large underlying buffer (for the Reassembler), 
and we insert a character at sequence number 0 (which is invalid and taken
by SYN), my code subtracts 1 which can potentially go to the very last
index in the Bytestream, thus leading to a write which is unintended
behavior. 

- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:

I discussed general high-level design principles with Ganesh Venu. This 
included questions including: 

1.) What are trade-offs you considered in handling RST flags? 
2.) At a high-level, how did you initially prune the search
space for the closest absolute sequence number? 

I also asked code-related questions to LLMs, including: 
1.) Does min require both arguments to be the same? 
2.) How does cast work if the value is beyond the maximum? 
Etc.
