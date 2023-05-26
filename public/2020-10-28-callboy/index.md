# Callboy - Hack.Lu CTF 2020


# Callboy

- 207 solves
- Baby
- Category: Misc
- Author: p4ck3to
- Description : "Have you ever called a Callboy? No!? Then you should definitely try it. To make it a pleasant experience for you, we have recorded a call with our Callboy to help you get started, so that there is no embarrassing silence between you.

PS: do not forget the to wrap flag{} around the secret "

- 127 points

# An overview of network packets

The file we download is a network packet capture. We see:

- Many packets coming from the leet IP address `10.13.37.86`
- Contacts to `theannoyingsite.com` (50.116.11.184). At first I thought that might be interesting, but it was just a false lead.
- VoIP communications with RTCP and RTP (coming from `sip:p4ck3t0@10.13.37.86.CLinphone Desktop`)

# VoIP communication

In Wireshark, you load the communication (Telephony > VoIP Calls). Then, you click on "Play Streams" (can be a little bit long to load).

In my case, it didn't work. See the screenshot below: Wireshark didn't recognize any output device to play the sound! Though I *do* have an output device...

![](/images/hacklu2020-callboy.png)

Another member of the team tried on his host. Same issue. In the end, the solution was to *download a more recent version of Wireshark* and it played like a charm. **A voice would spell the flag.**

Although it is a common advice: use up-to-date software, I didn't find it cool that the solution to this challenge only depended on the version of wireshark you had :(

