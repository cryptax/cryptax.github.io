---
layout: post
title: "OSINT Industries Christmas 2025 CTF"
author: cryptax
date: 2026-01-13
tags:
- OSINT
- CTF
- 2025
---

# OSINT Industries Christmas 2025 CTF

## Find the Masked Man 

### Description

In this first challenge, a photograph was taken on December 3rd, 2023 
around 18:00 and we must identify the nearest metro station in Paris.
We also get a hint that the scene is located near the intersection of a Rue and an Avenue.

![](/images/osindus2025-mask.jpg)

### Solution

I crop on the part of the image that shows shops.

![](/images/osindus2025-mask-cropped.jpg)

I can distinctively read "Julien", and then perhaps "Brasserie" or "Creperie" or something like that, and a shop name ending "Aid" (maybe an English name).

I upload the image in Google images, but it doesn't find anything that strikes me as interesting. Then, I ask ChatGPT we answer bulshit (it reads Brasserie Aid and Creperie Julien etc).

There is a brasserie named *Chez Julien*, but it doesn't look the same, and the street doesn't look the same either (the picture was taken 2 years ago, shops may change, but not buildings !).

Then I realize this is maybe "patisserie". And there is a [Patisserie Julien](https://fr.mappy.com/poi/4d6c77b9fc692507bc1010d7). So, I get the address: 
*73 av Franklin D Roosevelt, 75008 Paris*. 

Now, I head to Google Maps and find the [nearest metro station](https://www.google.fr/maps/place/73+Av.+Franklin+Delano+Roosevelt,+75008+Paris/@48.8729335,2.3098318,17z/data=!3m1!4b1!4m6!3m5!1s0x47e66fc66432bee5:0x95bf5172ffe2f39f!8m2!3d48.8729335!4d2.3098318!16s%2Fg%2F11rgdjn2z_?entry=ttu&g_ep=EgoyMDI1MTEyMy4xIKXMDSoASAFQAw%3D%3D) which is *Saint Philippe du Roule*

According to flag format, the flag is `OSINT{SAINT_PHILIPPE_DU_ROULE}`.