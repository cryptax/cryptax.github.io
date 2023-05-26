# Good old friend - THCon 2021



# Good old friend

The challenge provides an Android APK.

## Reversing the APK

The main activity of this APK is `party.thcon.y2021.level1.MainActivity`.
Its `onCreate()` method does the following:

1. **Anti-debug**. If the app is being debugged, display an alert dialog saying "Find another way" and quit.

```java
 if((this.getApplicationInfo().flags & 2) != 0) {
            this.findanotherway("App is debuggable");
        }
```

2. **Anti-root**. Search for `su` in the `PATH` of the system. If it is not found, search for rooting apps or binaries such as `Superuser.apk`, `/system/xbin/daemonsu` etc. If either are found, display the alert window and quit.

```java
 if(thereissu == 0 && (Build.TAGS == null || !Build.TAGS.contains("test-keys"))) {
            String[] v4 = {"/system/app/Superuser.apk", "/system/xbin/daemonsu", "/system/etc/init.d/99SuperSUDaemon", "/system/bin/.ext/.su", "/system/etc/.has_su_daemon", "/system/etc/.installed_su_daemon", "/dev/com.koushikdutta.superuser.daemon/"};
            int v1 = 0;
            while(v1 < 7) {
                if(new File(v4[v1]).exists()) {

...
 if(v2 != 0) {
            this.findanotherway("Device is rooted");
        }
```

3. Ask for a password and when the main check button is pressed, call a listener `b()`:

```java
this.checkbtn.setOnClickListener(new b(this));
```

The listener calls `checkInput()` and displays "Correct" or "Wrong":

```java
public final void onClick(View arg4) {
        String v1;
        String v0;
        MainActivity mainactivity = this.main;
        if(mainactivity.checkInput(mainactivity.password.getText().toString()) == 0) {
            v0 = "Correct!";
            v1 = "You can use this password to validate the challenge";
        }
        else {
            v0 = "Wrong!";
            v1 = "Try again.";
        }

        new AlertDialog.Builder(mainactivity).setTitle(v0).setMessage(v1).setPositiveButton("OK", null).create().show();
    }
```

The method `checkInput()` is native, and loaded from the library `native-lib`:

```java
 public static {
        System.loadLibrary("native-lib");
    }
    
public native int checkInput(String arg1) {}
```

The argument to `checkInput` is the password the end-user supplies for verification.

## Reversing the native library

So, we need to reverse the library.  If we unpack the APK, it is located in the `./lib` subdirectory, and we find a library for ARM and x86 architectures. I usually prefer ARM, but x86 is often decompiled better by decompilers.

With JEB, I try to decompile the `armeabi-v7a` library. I focus on the exported function `Java_party_thcon_y2021_level1_MainActivity_checkInput`, which corresponds to the JNI for our native method `checkInput`.

The decompilation is not perfect, but I understand the check routine performs AES CBC encryption with a fixed IV and key on the password, and checks that the encrypted result matches the expected one.

```c
 AES_init_ctx_iv((int)&ctx, (int)&iv, (int)&v1, len);
    AES_CBC_encrypt_buffer((int)&ctx, (int)&v2, v15, len);
    size_t __n = (size_t)min(v15, 32);
    int v16 = memcmp((void*)0x26FC0, &v2, __n);
    if(v16 != 0) {
        android_log_print(3, "MyLib", "FAILURE!\n");
        result = 1;
    }
    else {
        android_log_print(3, "MyLib", "SUCCESS!\n");
        result = 0;
    }
```

So, we just need to located the IV, the key and the expected result.
The expected result is easily located with JEB at 0x26FC0:

![](/images/thcon21-goodold-result.png)

It is `e47bc2dffaa645cb89a87780bb1619ef5daa2aadf4cda3ebd1884e64a2b43b68`.

However, the key and IV are obscure with JEB...

```c
  VLD1.64((unsigned long long)v10, (unsigned long long)(v10 >>> 0x40X), 159648);
    VST1.64((unsigned long long)v10, (unsigned long long)(v10 >>> 0x40X), &iv);
    VLD1.64((unsigned long long)v10, (unsigned long long)(v10 >>> 0x40X), 159664);
    VST1.64((unsigned long long)v10, (unsigned long long)(v10 >>> 0x40X), &v1);
    aeabi_memclr8(&v2, 1000);
    int v12 = (unsigned int)v8;
    int v13 = 0;
    while(1) {
        int v14 = min(v12 * 0x80000000 == 0 ? (unsigned int)(unsigned char)v12 >>> 1: v3, 1000);
        v12 = (unsigned int)v8;
        if(v14 <= v13) {
            break;
        }
        else {
            *(char*)(v13 + (int)&v2) = *(char*)((int)((v12 & 0x1) == 0 ? &v5: ptr0) + v13);
            ++v13;
        }
    }
```

So, I head to Ghidra. As decompilers often process x86 better, I use the x86 library this time. In `AES_init_ctx_inv`, the first argument is a context structure, the second is the key (`local_28`), and the third is the IV (`local_38`).

```c
  AES_init_ctx_iv(aAStack1248,(uchar *)&local_28,(uchar *)&local_38);
  AES_CBC_encrypt_buffer(aAStack1248,auStack1056,uVar4);
```

With Ghidra, the values for the key and IV are easily to find. They are hard-coded:

```c
  local_1c = 0x3c4fcf09;
  local_20 = 0x8815f7ab;
  local_24 = 0xa6d2ae28;
  local_28 = 0x16157e2b;
  local_2c = 0xf0e0d0c;
  local_30 = 0xb0a0908;
  local_34 = 0x7060504;
  local_38 = 0x3020100;
```

The IV is `000102030405060708090a0b0c0d0e0f`.
The key is `2b7e151628aed2a6abf7158809cf4f3c`.

## Decrypting

```python
from Crypto.Cipher import AES

key = bytes.fromhex('2b7e151628aed2a6abf7158809cf4f3c')
iv =    bytes.fromhex('000102030405060708090a0b0c0d0e0f')
ciphertext = bytes.fromhex('e47bc2dffaa645cb89a87780bb1619ef5daa2aadf4cda3ebd1884e64a2b43b68')

cipher = AES.new(key, AES.MODE_CBC, iv)
plaintext = cipher.decrypt(ciphertext)
print(plaintext)
````

This displays: `b'THCon21{C_1$_n3v3r_2_f@r}\x00\x00\x00\x00\x00\x00\x00'`. The flag is **THCon21{C_1$_n3v3r_2_f@r}**.


Sadly, I did not flag this challenge on time, because of a typo in the key I never saw...

