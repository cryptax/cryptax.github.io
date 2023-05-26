# draw.per - THCon 2021



# Draw APK - THCon 2021

- 14 solves
- 249 points

## First steps

As the challenge creator tells me this app is "based on a Trojan Horse" and it is "not recommended to install it on a real smartphone", even if there is actually "no malicious payload", I am very reluctant at first to test the app, even in an emulator.

So, I start off with my favorite static analysis combination: [DroidLysis](https://github.com/cryptax/droidlysis) and [JEB](https://www.pnfsoftware.com). The results are disappointing, I spot several *"suspicious"* parts (use of `com.andrognito` for example) but they are False Positives.

## Dexcalibur and Zip

I decide therefore to run the app in an Android emulator, instrumented with [Dexcalibur](https://github.com/FrenchYeti/dexcalibur/). I use the default set of hooks, and spawn the app. Nothing important happens until I click on the *Settings* menu. There, I notice the **creation of a `files.zip`**.

![](/images/thcon21-dexcalibur-files.png)

I find this suspicious: apart if configuration elements are zipped (but why? storing them in shared preferences seems easier), I see no reason why the app would need to create a Zip file.

I retrieve the zip file via ADB. It contains:

- ExtClass.enc: this file is obviously encrypted
- inf.enc: same, encrypted.
- theme.jpg: this file is a ... PNG image. I don't know why eog (Eye of Gnome) refuses to open it, but Gimp has no problem to open it.

It says we have been *pwned*. So, I am obviously in the right direction :)

![](/images/thcon21-pwned.png)

## Dexcalibur and Dex

Then, I notice in Dexcalibur that a Dalvik Executable is loaded (highly suspicious):

![](/images/thcon21-dexcalibur-dex.png)

I grab the DEX via ADB. It is a valid DEX file, that I immediately decompile. The code is simple, with a single class `ExtClass`, and methods such as `isInfected()` : exactly what I expect in a challenge.

## ExtClass code

The image of the Zip is used by the method `changeWallpaper()`. It sets it as wallpaper.

```java
public void changeWallpaper(Context arg7) {
        WallpaperManager wpm = WallpaperManager.getInstance(arg7);
        Bitmap bitmap = BitmapFactory.decodeFile(this.filesDir + "/theme.jpg");
        try {
            wpm.setBitmap(bitmap);
        }
        catch(Exception e) {
            e.printStackTrace();
        }
    }
```

I am interested in method `checkInf()`, which decrypts `inf.enc` file. As this file is small (48 bytes), it could very well hold the encrypted flag.

```java
private String checkInf() {
        String path = this.filesDir + "/inf.enc";
        try {
            SecretKeySpec key = new SecretKeySpec(MessageDigest.getInstance("SHA-256").digest("y3t_@n0th3r_p@ssw0rd".getBytes(StandardCharsets.UTF_8)), "AES");
            IvParameterSpec iv = new IvParameterSpec("@n0th3r_f1x3d_1v".getBytes(StandardCharsets.UTF_8));
            Cipher cipher = Cipher.getInstance("AES_256/CBC/PKCS7Padding");
            cipher.init(2, key, iv);
            return new String(cipher.doFinal(Files.readAllBytes(Paths.get(path, new String[0]))));
        }
        catch(Exception e) {
            e.printStackTrace();
            return null;
        }
    }
```

I write the matching standalone Java code, but when I run it, it fails with no available provider for AES_256/CBC/PKCS7Padding.

## How to lose time, silly

At this point thinking that if my computer does not have this provider, for sure Android phones and emulators have it, so I am going to hook the decryption method `doFinal` and display the decrypted text. 

I hook `doFinal` and ensure that it displays the decrypted result (`ret`).
![](/images/thcon21-dexcalibur-hook.png)

Then, I spawn the malware (note each time, I have to uninstall it and re-install it, to run it clean):

![](/images/thcon21-dexcalibur-decrypted-dex.png)

I considered hooking directly `checkInf()`, except I can't with Dexcalibur because it is from dynamically loaded code and Dexcalibur does not find it. It is possible to write a Frida hook instead (without using Dexcalibur), but I'm lazy and don't do it.

So, I get the result of the decrypted text and re-build the file... except this is useless, because those bytes are not from `inf.enc` but the decrypted value of `ExtClass.enc` !

And unfortunately, I register no other call to `doFinal()` for `inf.enc`... Dead end.

## Back on track

I consider my Java decryption code and search on Internet why it does not find a provider for this algorithm and where I can find one.

I bump into [this page](https://stackoverflow.com/questions/29232705/encrypt-text-to-aes-cbc-pkcs7padding) which says that PKCS7 padding is not supported by Java, only PKCS5 (naming issue see the stackoverflow page). So, I simply change PKCS7 padding to PKCS5 in my code.

```java
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.MessageDigest;
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class Decrypt {
    public static String path = "./inf.enc";

    public static void main(String args[]) {
        try {
                SecretKeySpec key = new SecretKeySpec(MessageDigest.getInstance("SHA-256").digest("y3t_@n0th3r_p@ssw0rd".getBytes(Standa
rdCharsets.UTF_8)), "AES");
                IvParameterSpec iv = new IvParameterSpec("@n0th3r_f1x3d_1v".getBytes(StandardCharsets.UTF_8));
                Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
                cipher.init(2, key, iv);
                System.out.println("Solution: "+ new String(cipher.doFinal(Files.readAllBytes(Paths.get(path, new String[0])))));
        }
        catch(Exception exp) {
            System.out.println("Exception: "+exp.getMessage());
        }
    }
}
```

## Solution

Bingo.

```
java Decrypt 
Solution: THCon21{Dyn@m1c_c0d3_l0@d1ng_1s_$c@ry}
```

## How does it work?

When the end-user selects Settings, this starts `SettingsActivity`.
At some point, this calls method `invoke` on the `SettingsActivity`.
This calls a method named `executePayload`.

```java
 private final void executePayload() {
        ObjectRef classname = new ObjectRef();
        classname.element = "com.example.extlib.ExtClass";
        ObjectRef thedex = new ObjectRef();
        StringBuilder v3 = new StringBuilder();
        File v4 = this.getFilesDir();
        Intrinsics.checkNotNullExpressionValue(v4, "filesDir");
        thedex.element = v3.append(v4.getAbsolutePath()).append("/").append("ExtClass.dex").toString();
        if((this.loadServerFiles()) && (this.integrityVerified(((String)thedex.element)))) {
            BuildersKt.runBlocking$default(null, ((Function2)new SettingsActivity.executePayload.1(this, thedex, classname, null)), 1, null);
        }
    }
```

This method:

1. Retrieves external files via `loadServerFiles()`. This downloads `files.zip` from a [Google Drive](https://drive.google.com/uc?id=1nHhYCx1AXOmrE9L7DbTaA8QsroICYFG6&authuser=0&export=download). Then, it unzips the file and decrypts the DEX. Note the secret key and IV are different from the one used to decrypt `inf.enc`.
2. Checks the integrity of the external DEX. This consists in checking the SHA256 hash against the expected one.
3. Calls `executePayload.1`. This will load the DEX (with `PathClassLoader`, something I did not detect with DroidLysis at the time of the challenge), call `initLib` from the dynamic class, check if the smartphone is infected with `isInfected` and finally modify the wallpaper.

