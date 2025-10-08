Checkpoint 1 Writeup
====================

My name: Arjun Inamdar 

My SUNet ID: arjun05 

I collaborated with: ganeshve 

I would like to thank/reward these classmates for their help: ganeshve (Ganesh Venu) 

This lab took me about 10 hours to do. I did not attend the lab session.

I was surprised by or edified to learn that: So much of the transimission (especially 
related to the reciever's state) is dependent upon the sender. For example, if the
reciever doesn't have the capacity to store a part of the substring, it's the 
responsibility of the sender to re-send this: this requires an interesting 
flavor of abstraction. 

Report from the hands-on component of the lab checkpoint: 

Question 2.1.)

4.a.) The average round trip delay was 24.839 ms. 

4.b.) The delivery rate was 99.8%, and the loss was 0.2%. 

4.c.) There were no duplicates that I detected on the 1000 ping run.

4.d.) Yes, Wireshark clearly shows some matching in the structures of the datagrams. 
The following are each of the fields of the datagram and the respective values I
observed: 

Version: 4
IHL: 20 bytes
Type of Service: 0
Total Length: 84
Identification: 7980
Flags: 0
Fragent Offset: 0
Time To Live: 63
Protocol: ICMP
Header Checksum: 0x464f
Source Address: 10.144.0.143
Destination Address: 10.144.0.128

However, I couldn't find any "Options" or "Padding" values.

4.e.) There were some differences in the sending and recieving packet. This is because
the TTL is decremented every time it hits a node, so the TTL field will be different. Also, 
the checksum is different as a result because the TTL is a part of it. 

Describe Reassembler structure and design: 

My Reassembler internally maintains two vectors which represent the unassemebled
part of the Bytestream. The first vector stores the actual characters (vector<char>), 
and the second vector stores boolean values indicating whether or not the value is 
set or not. My implementation first truncates the passed string/index pair to fit 
within the bounds of the unassembled indices. We then loop over the vector of booleans
to get the first "k" characters of the stream, if they exist, and push it to the 
Bytestream if so. 

An alternate approach considered was storing tuple pairs of the unassembled indices in a
vector (sorted by index). However, there's complexity with inserting and then merging the
indices algorithmically, and creating and concatenating the strings would've taken linear time
(with respect to the size of the strings) iterating over the array. My approach is strictly linear
with respect to the size of the buffer per-call.

I also considered using a tree. Each node would store an interval [min, max] which would represent
the minimum and maximum intervals of the unassembled strings as children, and the leaves would store
the strings themselves. Insertion implementation is non-trivial and can be complex. Additionally, 
finding the leftmost string (in the case we want to push it to the ByteStream), would require you to
iterate the height of the tree in the worst-case. 

Implementation Challenges:

Handling/checking strings in bounds (and truncating them appropriately) was difficult. This required 
difficult index math and verifying my results. I broke up the problem carefully and separated it into
checking/handling out of bounds to the left and to the right. Drawing out the cases and then running GDB
on the provided test cases while inspecting variables was incredibly helpful. 

There were non-trivial edge cases. Handling empty strings in bounds was difficult because it conflicted
with handling non-empty strings which were out of bounds. I created a separate check condition for 
empty strings as a result. 

Remaining Bugs:

There are edge cases not-appropriately handled because we defer so much reponsibility to the caller
of the function. If we are provided the last substring but don't have the capacity to store it (within 
the available_capacity) at that time, then we rely on the fact that the writer will provide this again
in the future. If they don't, we cannot ever fully store/push the substring to the Bytestream: some
coordination is necessary.


- If applicable: I received help from a former student in this class,
  another expert, or a chatbot or other AI system (e.g. ChatGPT,
  Gemini, Claude, etc.), with the following questions or prompts:
  [please list questions/prompts]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
