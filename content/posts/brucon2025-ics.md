---
layout: post
title: "BruCON 2025 ICS Hacking Village"
author: cryptax
date: 2025-09-26
tags:
- Brucon
- CTF
- 2025
- M5
- ICS
- water
- pump
- APT44
- sandworm
---

# BruCON 2025 ICS Hacking Village

The **ICS Hacking Village** at [**BruCON 2025**](https://brucon.org) was offering a *cool challenge*, 
where water flow in Hacktopia city was malfunctioning because of a 
cyber attack.

![](/images/brucon2025-ics-village.jpg)

The goal of the challenge was to stop the overflow in a water tower
and re-open water from a secondary water tank for water to 
flow again in the building.

Each participant was supplied with a control box consisting of a **M5 Core** connecting to a few sensors, buttons, potentiometers, and connected to 
a water tank. We were not allowed to hack physically with the hardware.

## Stage 1: Sensor

At first, we just had an ICS image. Playing a bit with buttons, I see that the blue and red buttons change the color of the screen and talk about "watch ur shadow". 

![](/images/brucon2025-red.jpg)
![](/images/brucon2025-blue.jpg)

There is a light sensor connected to the M5 Core. I cover the sensor, but nothing happens. I try to cover it more, still nothing, I'm 
out of clue and an organizer gives me a hint: "if it's not shadow, it's...". I get it. I flash the light of my smartphone on the sensor, and
a hidden button appears on the screen of the M5.

![](/images/brucon2025-light.jpg)


## Stage 2: OSINT

I hit the button. We get to another screen which shows a message in Russian, a PIN pad, the image of a *sandworm* and a logo I do not recognize. The organizers tell us this is kind of an **OSINT challenge** and we have to find out more about who's this organisation.

![](/images/brucon2025-pin.jpg)

I use *Google Lens*, and get the translation of the image "ha ha, who are we". I continue to search for the logo in Google Images. It tells me the text on the left translates to "Strike first" and that the logo is "the emblem of the [..]People's Cyber Army, which appears to be a pro-Russian hacking group, as suggested by the Russian flag colors and text".

We find the same logo in this [blog post](https://medium.com/@PJ04857920/cyber-army-of-russia-ddos-tool-3b3050419225), where it is attributed to the *Cyber Army of Russia*.

I search the net for ICS attacks and "sandworm", possibly related to water facilities and find [this paper: APT 44 unearthing Sandworm](https://services.google.com/fh/files/misc/apt44-unearthing-sandworm.pdf). On page 4, the paper details different units of the group, they are 5-digit numbers, and I think one of them could be the PIN code. The first one I try, 55111, doesn't work, but the second one, **74455 opens up the "door"**.

![](/images/brucon2025-apt44.png)

## Stage 3: Water System

We get access to the water system of Hacktopia. From the challenge description, I know I must first solve the overflow in the water tower. Using the potentiometers, I see I can open/close valves. I reduce the overflow in the water to 0, and that issue is solved.

![](/images/brucon2025-tower.png)

Then, I manipulate other potentiometers to get the water to flow from the water tank, and activate the pump.

![](/images/brucon2025-pump.png)

I hear the **water flowing from the second tank into the city**! Hurray!
The flag displays partly on the blue screen and the red screen (pressing blue and red buttons respectively).

![](/images/brucon2025-flagblue.png)
*The blue image discloses half of the flag. I get the other half from the red button: `HnH{worm_sign_in_the_hmi}`*

## Conclusion

> In terms of challenge, I liked the first part with the hidden menu and the OSINT to find the PIN.
> I was slightly disappointed the challenge did not involve any real device (water pumps or else). The water system was implemented with M5 Core and sensors: it worked (that was awesome, very "visual") but that's not what a real city would use.
> Overall though, it was excellent, educational and relatively easy challenge. I enjoyed it, and it's really nice work from the ICS Village!