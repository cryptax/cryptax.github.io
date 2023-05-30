---
layout: post
title: "Drone and secret message (THCon CTF 2023)"
author: lud0
date: 2023-04-22
tags:
- THCon
- CTF
- 2023
- Misc
---

## Description


This challenge was in the *Misc* category.

```
While travelling to a conference in Paris, Herlock Sholmès sees a troubled police inspector.

After asking him what was his problem, the inspector tells Herclock that he is trying to understand how two spies communicate with each other.

Once every week both spies get on the Eiffel tower, but never get in speaking distance of each other. Generally one of them flies a drone for around twenty minutes, sometimes both of them do it. When questonned about it, they told it was for taking pictures, and provided the pictures in question when asked. Of course, nothing helpful came from these.

The inspector hands Sholmès a file containing the last flight path of the drone.

The flag of this challenge is THCon23{**SECRETMESSAGE**}, where **SECRETMESSAGE** is the message sent in that interaction, written in all caps without spaces.
```

## What is provided

A trace file in text format, formatted as follows: date and time, GPS coordinate in degrees, minutes and seconds, and an altitude in feet. For instance, the beginning of the file:

```
2023/03/02-11:19:43     48°51'32.25"N   2°17'42.66"E    1002ft
2023/03/02-11:19:46     48°51'32.45"N   2°17'42.97"E    999ft
2023/03/02-11:19:49     48°51'32.57"N   2°17'43.38"E    1001ft
2023/03/02-11:19:52     48°51'32.57"N   2°17'43.82"E    1000ft
2023/03/02-11:19:55     48°51'32.45"N   2°17'44.23"E    1003ft
2023/03/02-11:19:58     48°51'32.25"N   2°17'44.54"E    995ft
2023/03/02-11:20:01     48°51'31.98"N   2°17'44.71"E    1003ft
2023/03/02-11:20:04     48°51'31.69"N   2°17'44.71"E    1002ft
2023/03/02-11:20:07     48°51'31.42"N   2°17'44.54"E    1001ft
2023/03/02-11:20:10     48°51'31.21"N   2°17'44.23"E    1000ft
2023/03/02-11:20:13     48°51'31.1"N    2°17'43.82"E    995ft
```

## First idea

My first idea was that the drone was tracing something interesting, or going to noticeable geographical points. So, I decided to convert the txt file to a GPX trace to be opened in google earth

The conversion code is not complex to be done, but takes time, so I decided to use ChatGPT 4. My question:

![My question](/images/thcon23-question-chatgpt.png)


The answer given by ChatGPT produces a fully operational code that produces a GPX trace.

```java
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

public class TextToGPX {

    public static void main(String[] args) {
        String inputFileName = "input.txt";
        String outputFileName = "output.gpx";

        try (BufferedReader br = new BufferedReader(new FileReader(inputFileName));
             BufferedWriter bw = new BufferedWriter(new FileWriter(outputFileName))) {

            bw.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            bw.write("<gpx version=\"1.1\" creator=\"TextToGPX\"\n");
            bw.write("    xmlns=\"http://www.topografix.com/GPX/1/1\"\n");
            bw.write("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
            bw.write("    xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");
            bw.write("<trk>\n");
            bw.write("<trkseg>\n");

            String line;
            SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd-HH:mm:ss");

            while ((line = br.readLine()) != null) {
                String[] parts = line.split("\\s+");

                Date dateTime = sdf.parse(parts[0]);
                String isoDateTime = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'").format(dateTime);

                double latitude = dmsToDecimal(parts[1]);
                double longitude = dmsToDecimal(parts[2]);
                double elevation = Double.parseDouble(parts[3].substring(0, parts[3].length() - 2)) * 0.3048;

                bw.write(String.format("<trkpt lat=\"%s\" lon=\"%s\">\n", latitude, longitude));
                bw.write(String.format("  <ele>%.2f</ele>\n", elevation));
                bw.write(String.format("  <time>%s</time>\n", isoDateTime));
                bw.write("</trkpt>\n");
            }

            bw.write("</trkseg>\n");
            bw.write("</trk>\n");
            bw.write("</gpx>\n");

        } catch (IOException | ParseException e) {
            e.printStackTrace();
        }
    }

    public static double dmsToDecimal(String dmsCoordinate) {
        String[] dmsElements = dmsCoordinate.split("[°'\"NSEW]+");
        double degrees = Double.parseDouble(dmsElements[0]);
        double minutes = Double.parseDouble(dmsElements[1]) / 60;
        double seconds = Double.parseDouble(dmsElements[2]) / 3600;

        double decimalCoordinate = degrees + minutes + seconds;

        char direction = dmsCoordinate.toUpperCase().charAt(dmsCoordinate.length() - 1);
        if (direction == 'S' || direction == 'W') {
            decimalCoordinate *= -1;
        }

        return decimalCoordinate;
    }
}

```

The next step was to open the GPX trace with google earth. 

![Trace for Google Earth](/images/thcon23-googleearth.png)

Different successives scheme of altitudes and circles are made by the drone, but nothing really relevant there. So, we decided to start investigating photos posted by users in the center of the circle of the drone. But analyzing dozens of images seems unrealistic, and no recent pictures with a "hacker" nickname could be found. And the lake around which the drone flies has not name ...

Dead end!

But yet, since we notice regular schemes, we decided to tackle the problem differently.

## Second idea

Since the GPS coordinates are totally regular, altitudes are not regular. So, we decided to trace the altitude as a function of time.

![Altitude as a function of time](/images/thcon23-morse.png)
Altitude as a function of time

Bingo, this is a morse!

We translated this, and made a final *rot13* on the text to get the flag.
