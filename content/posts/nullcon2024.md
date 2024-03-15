---
layout: post
title: "Nullcon Berlin CTF 2024"
author: cryptax
date: 2024-03-15
tags:
- Nullcon
- CTF
- 2024
- Hardware
---

# HackMe Hardware Challenges - Nullcon Berlin CTF 2024

The 6 challenges below use the same PCB.

## HackMe Fix the Board (5 solves)

The PCB we are given does not work as such: the screen does not light up and the device does not boot correctly. We've got to repair it.

### Fix 1

As the screen does not light up, there has to be a power issue. From VCC, we notice there is a Diode, U14, which is in the wrong direction, thus blocking current.

![](/images/nullcon24-tofix1.png)

We let the current pass by simply bypassing the diode. I solder a wire from VCC to the other end of U14.

![](/images/nullcon24-fixed1.png)

### Fix 2

We notice that the track by U17 has been (intentionally) cut by the organizers ;)
We just need to solder that again.

![](/images/nullcon24-tofix2.png)

### Fix 3

The same also occurs on a track below the screen: intentionally cut, you just need to add solder. I forgot to take a picture.

### Flag

Once all those fixes are done, you can power the device using a USB-TTL, and the screen lights up :) You might also have to turn a potentiometer to see the message which gives you the first flag `LCD`.

![](/images/nullcon24-contrast.png)

The screen shows several menus, each one consisting in the next challenges to unlock.

## HackMe Dump the memory (3 solves)

The next 2 challenges consist in dumping the 2 EEPROMs labeled "MEMORY 1" and "MEMORY 2".

I follow the beginning of this [tutorial](https://www.youtube.com/watch?v=M4-1FqxVJjw) for the wiring, but actually it's quite simple: VCC goes to 5V, SDA goes to A4 and SCL goes to A5. The rest goes to the ground.

The, I use [the code of this blog post to read an I2C memory](http://chrisgreenley.com/projects/eeprom-dumping/). I just modify the output to break lines every x characters.

```c
#include <Wire.h>
#include <stdint.h>
#define CHIP_ADDR 0x50
// http://chrisgreenley.com/projects/eeprom-dumping/
//  SDA is A4 and SCL is A5
void setup() {
  uint8_t dataAddr;
  Serial.begin(9600);
  Serial.println("Setting up serial");
  Wire.begin();
  //Wire.setClock(31000L); //31 kHz
  Wire.beginTransmission(CHIP_ADDR);
  Serial.println("Begin transmission");
  Wire.write(0x00); //Sets the start address for use in the upcoming reads
  Wire.endTransmission();
  
  for (int chipAddr=0; chipAddr<4096; chipAddr++) { 
    for(uint8_t i=0;i<8;++i){ //cycle through enough times to capture entire EEPROM
      Wire.requestFrom(CHIP_ADDR,32,1); //read 32 bytes at a time
      uint8_t counter = 0;
      while (Wire.available()){
        uint8_t c = Wire.read();
        Serial.write(c); //Send raw data over serial to 
        counter++;
        if (counter>=32) {
          Serial.println("");
          counter=0;
        }
      }
    }
  }
  Serial.println("Done");
}

void loop() {
  
}
```

I am actually quite lucky: I guessed the I2C memory's address: 0x50. I should have used an [I2C scanner](https://playground.arduino.cc/Main/I2cScanner/).

The memory dump provides lots of garbage, and in the middle:

```
l\K5@F[lpRNAUgr6UBMmKVMuXHP1dw;<
E3Ia@V@<=0L2Kf1A62KA0lMWiu_PHBtg
u1=aYfl=FcAb2DDZcQtWav64rLGwVl=@
4@BkHFCbGLFLAG FLAG FLAG...----.
.. LOW ON MEMORY ...-- -... FLAG
 FLAG FLAGpZjZS8YpR177dFTFl:mtTW
```

The flag is `LOW ON MEMORY`.


## HackMe Dump memory 2 (2 solves)

*TO COME*

## HackMe UART Password (1 solve)

*TO COME*

## HackMe Write 129 at address 800 (1 solve)

*TO COME*

## HackMe Hidden in plain sight (1 solve)

* TO COME*

![](/images/nullcon24-hiddeninplain.png)

