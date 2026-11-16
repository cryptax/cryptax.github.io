---
layout: post
title: "GreHack 2026: HP 85 challenges"
author: cryptax
date: 2026-11-15
tags:
- retrocomputing
- hp
- BASIC
- CTF
---

# GreHack 2026: HP 85 challenges

![](/images/2026-grehack-hp85.jpg)

At GreHack 2026 CTF, there were lots of challenges in the *Hardware* category, with a couple of challenges on operational **HP 85** devices. HP 85 are desktop computers released in 1980 with an integrated 5-inch CRT, built-in printer, tape drive and BASIC interpreter.

![](/images/2026-grehack-hardware.png)

The machines are restored and maintained by [ACONIT](https://aconit.org). It's a real pleasure to use antique devices which are running smoothly - yes, even the printer.

## Overview

I did 4 challenges on the HP 85: while the flag is hidden in different places and with different techniques, there is a common base.

The first step was to get acquainted with the keyboard: there is a "LIST" key, arrows to navigate, a "SCRATCH" key to restart, a "RUN" key to run the current program etc. The challenge resources provide a link to the [Owner's Manual](https://archived.hpcalc.org/greendyk/hp85/index.html)🔗, but to be honest, it was easier to ask the helpful guy around the machines 😉.

We list files on the tape with `CAT`. There are 2 program files `2DEDT` and `2DPLT`, and 2 data files `GHLOG` and `HPLOG`.

![](/images/grehack26-cat.jpg)

We retrieve the BASIC source code of `2DPLT`, but not `2DEDT`:

```
Error 22: Secured
```

This means the program has been protected against listing. 
And, of course, Data files don't have source code.

A quick parse at the source code of `2DPLT` shows that it draws a logo on the screen.

The second program `2DEDT` can be run, but not listed. It provides several menus to modify the display, e.g load a logo file, add a line, edit a text label.

```
*** GRAPH EDITOR ***
1. Add element
2. View elements
3. Edit element
4. Save to file
5. Load from file
6. Test render
7. Exit
CHOICE (1-7)?
```

## HP 85 DATA FILES 1/2

### Source code

We analyze the source code of `2DEDT` (see *Appendix*).

1. It reads data from a given file: the code mentions `GRAPH`, but we will want to read `GHLOG` (GreHack's logo), or `HPLOG` (HP's logo).

```
80 ASSIGN# 1 TO "GRAPH"
```

2. The code loops from 1 to `N`, and for each loop, we end up in case 1, 2 or 3. The interesting part is in `GOSUB 690`.


```
140 B$=""
150 FOR I=1 TO N
151 READ# 1 ; T(I)
160 IF T(I)=1 THEN READ# 1 ; V1(I),V2(I),V3(I),V4(I)
170 IF T(I)=2 THEN READ# 1 ; V1(I),V2(I),V3(I),V4(I),V5(I)
180 IF T(I)=3 THEN GOSUB 690
190 NEXT I
```

3. In the subroutine, the program reads a character (`S$`) and appends it to `B$`

```
690 REM - READ LABEL TO BUFFER
700 READ# 1 ; V1(I),V2(I),S$
710 P(I)=LEN(B$)+1
720 L(I)=LEN(S$)
730 B$=B$&S$
```

### Data file structure

At first, it isn't so clear what `N` is. I create the following program to read `N` for `GHLOG`. It answers **20**.

```
ASSIGN# 1 TO "GHLOG"
READ# 1;N
DISP N
```

Then, with the help of AI, I read the type of the next value: `DISP TYP(1)`. The answer is **1**, which means we have a *numeric* value.

> In this writeup, I try to reproduce my (non linear!) path to the solution! At this stage, the values we have are unclear, same for what `N` is.

I launch the other program, `2DEDT`, and decide to load the `GHLOG` file in the menu. It reads it and tell me it has **20 elements**.

> So `N` is the number of elements 🎉

When we create new elements, or try to edit an element, there is usually: 2 coordinates (x, y) and then, if it's a "label" element, a string.

> Probably, the first numeric value we read after `N` was an `x` coordinate.

### Reading "after the end"

We have 20 elements in the file. What if there is actually something more, something beyond those 20 elements?

> I can't remember exactly how I came up with the idea. I think through a discussion with the guy handling the challenges 🙂

I started writing my own program, but got a bit lost. The guy suggested I simply *modified* the existing program. Indeed, that's easier. Those are the changes I did:

- `80 ASSIGN# 1 TO "GHLOG"`. Read the `GHLOG` file.
- `150 FOR I=1 TO 50` : we replace `N` with 50. Just a guess, but bigger than 20.
- `735 DISP B$`: in the case that handles labels, we make sure to display it. We can display it character by character (`S$`), but it's easier to use the concatenated string `B$`.

![](/images/grehack26-flag1.jpg)

The flag is `GH[hp_85_FILEstream]`.

## HP 85 DATA FILES 2/2

The description of the challenge says:

```
Oh look, there is a second file HPLOG that seems to be of the same structure of the first one !
```

I try the same trick on the file `HPLOG`. There are somewhat 40 elements in that file - I can't remember exactly the amount, but it doesn't really matter. I do the same fix and read until 50 + display the label, but the program fails with an syntax error on this line:

```
700 READ# 1 ; V1(I),V2(I),S$
```

The error we get complains that the **data format is wrong**.

> I search for a long time. I was given the hint that the file "is glitched", so maybe we just need to get past that error.

The probable issue is that, for a given element, the value we read in `S$` is not an (expected) character but a numeric value. So, let's assume an extra numeric value was inserted and we just need to read the character as fourth parameter:

- Replace with `700 READ# 1 ; V1(I),V2(I),V3(I),S$`

![](/images/grehack26-flag2.jpg)

> IMHO, this challenge was slightly guessy. There are many possibilities for a wrong data format...


## HP 85 BASIC 1/2

After a while, we figure out that `2DPLT` is a roughly the subpart of `2DEDT` that renders the file.

`2DEDT` is protected against listing, but we can still *execute it* without any problem, and with BASIC, this also means we can **debug** it.

How about we display `B$`?

1. Run the program
2. `B$`: get the value of B (or `DISP B$`)


![](/images/grehack26-flag3.jpg)

## HP 85 BASIC 2/2

This time, we work with the program `2DEDT` and we get the hint there is *yet another flag to find without writing code*.

- There is a hidden menu `8`, asking for a password, but I am told not to go in that direction. Lol. I so much wanted to break the admin password 😂
- The program is able to load a graph file, so I try to load the program itself `2DEDT`, hoping to see its code. "Nice try" says the guy, but it does not work.

No code to write? Okay, I look into every menu, and try hidden options.
This is when I try option "5" in the following menu.

![](/images/grehack26-menu.jpg)

I don't even notice at first that it has printed the flag - the printer didn't make that much noise 🖨️

![](/images/grehack26-flag4.jpg)

## Conclusion

I enjoyed:

- **Working on the machines themselves**. There is an awesome HP85 simulator, provided by ACONIT, but I prefer to be physically on the machine 🙂
- **The guy was very helpful**, guiding us through the keyboard, but also providing hints when we were stuck. That's probably the positive aspect of no longer having any scoreboard at GreHack CTF: there was no competition around those challenges, and they were created for participants to learn and have fun, but if someone needed more help than an other, he/she'd get more help, and that would be fine.
- The **first challenge was technically my favorite**.

As drawback, I'd just say that the challenges were **a bit guessy**. There are many possible solutions, and **there aren't enough indications to guide us with certainty in a given direction**. Nevertheless, **I had fun, and learned a lot**. Thanks!



## Appendix `2DEDT`

```basic

10 REM GRAPH INTERPRETER
20 GCLEAR
30 DEG
40 SCALE 0,1000,0,750
50 REM - SKIP IF CACHED
60 IF F=1 THEN 220
70 REM - LOAD TO RAM
80 ASSIGN# 1 TO "GRAPH"
90 READ# 1 ; N
100 DIM T(50),V1(50),V2(50)
110 DIM V3(50),V4(50),V5(50)
120 DIM P(50),L(50)
130 DIM B$[500],S$[80]
140 B$=""
150 FOR I=1 TO N
151 READ# 1 ; T(I)
160 IF T(I)=1 THEN READ# 1 ; V1(I),V2(I),V3(I),V4(I)
170 IF T(I)=2 THEN READ# 1 ; V1(I),V2(I),V3(I),V4(I),V5(I)
180 IF T(I)=3 THEN GOSUB 690
190 NEXT I
200 ASSIGN# 1 TO *
210 F=1
220 REM -- RENDER
230 FOR I=1 TO N
240 T=T(I)
250 IF T=1 THEN GOSUB 300 ! LINE
260 IF T=2 THEN GOSUB 390 ! ARC
270 IF T=3 THEN GOSUB 600 ! LABEL
280 NEXT I
290 END
300 REM -- LINE --
310 X1=V1(I)
320 Y1=V2(I)
330 X2=V3(I)
340 Y2=V4(I)
350 MOVE X1,Y1
360 PENUP
370 DRAW X2,Y2
380 RETURN
390 REM -- ARC --
400 X1=V1(I)
410 Y1=V2(I)
420 R=V3(I)
430 A1=V4(I)
440 A2=V5(I)
450 IF A1=A2 THEN A2=A1+360
460 IF A2<A1 THEN A2=A2+360
470 A=A1
480 X2=X1+R*COS(A)
490 Y2=Y1+R*SIN(A)
500 MOVE X2,Y2
510 FOR A=A1+5 TO A2 STEP 15
520 X2=X1+R*COS(A)
530 Y2=Y1+R*SIN(A)
540 DRAW X2,Y2
550 NEXT A
560 X2=X1+R*COS(A2)
570 Y2=Y1+R*SIN(A2)
580 DRAW X2,Y2
590 RETURN
600 REM --- LABEL ---
610 X=V1(I)
620 Y=V2(I)
630 P1=P(I)
640 P2=P1+L(I)-1
650 MOVE X,Y
660 PENUP
670 LABEL B$[P1,P2]
680 RETURN
690 REM - READ LABEL TO BUFFER
700 READ# 1 ; V1(I),V2(I),S$
710 P(I)=LEN(B$)+1
720 L(I)=LEN(S$)
730 B$=B$&S$
740 RETURN
```