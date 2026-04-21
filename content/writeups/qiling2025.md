---
layout: post
title: "Ph0wn Labs #3: Qiling Workshop"
author: cryptax
date: 2025-06-20
tags:
- qiling
- radare2
---

# Qiling Workshop

> I followed the [Qiling Workshop](https://github.com/blackb0x101/QilingWorkshop) of Ph0wn Labs #3, by Blackb0x_
> For environment setup, I used the Docker container provided on the GitHub link. Qiling is installed in `/opt`, and there are several root filesystems in `/rootfs`.
> There were 2 challenges: one Linux, one Windows. Both challenges can be solved in multiple ways, for example with static analysis, but the goal of this workshop was to **use Qiling to solve**. This is the writeup for the Windows challenge.

## Windows challenge

We have a `windows.exe` crackme. I'm on Linux, it's possible to run the binary with *wine*, but the goal of the workshop is to run it **with Qiling**.

### rootfs

I start a Python script to run Qiling:

```python
from qiling import *

# sandbox to emulate the EXE
def my_sandbox(path, rootfs):
    ql = Qiling(path, rootfs)
    ql.run()


if __name__ == "__main__":
    my_sandbox(["windows.exe"], "/rootfs/x86_windows")
```

This doesn't work: it doesn't find the executable. Even with a full path, it **complains the binary needs to be under the rootfs**. So I copy the binary in `/rootfs/x86_windows/bin/windows.exe` so that it accepts to launch it.

```python
if __name__ == "__main__":
    # execute Windows EXE under our rootfs
    my_sandbox(["/rootfs/x86_windows/bin/windows.exe"], "/rootfs/x86_windows")
```

### invalid memory read

Then, it compalins about an *invalid memory read*.


```
[x] 	Hexdump:
[x] 	ff 25 44 0f 88 6b cc cc
[x] 	Disassembly:
[=] 	000000006b823c20 [kernel32.dll         + 0x023c20]  ff 25 44 0f 88 6b    jmp                  qword ptr [rip 
+ 0x6b880f44]
...
unicorn.unicorn_py3.unicorn.UcError: Invalid memory read (UC_ERR_READ_UNMAPPED)
```

The error occurs at `jmp qword ptr [rip + 0x6b880f44]` at 0x000000006b823c20 in `kernel32.dll`. I open the dll in Radare2 and head to this address to understand what is happening:

```
root@1cb654fb8b1f:/rootfs/x86_windows/Windows/System32# r2 ./kernel32.dll 
0x6b81f8e0]> s 0x6b823c20
[0x6b823c20]> pd 10
            ;-- WriteConsoleA:
            0x6b823c20      ff25440f886b   jmp dword [sym.imp.api_ms_win_core_console_l1_1_0.dll_WriteConsoleA] ; 0x6b880f44 ; "\\x90\n"
```

THe issue is that **WriteConsoleA** is not mapped by Qiling.

### WriteConsoleA

We need to write a hook for `WriteConsoleA`.The [Microsoft documentation](https://learn.microsoft.com/en-us/windows/console/writeconsole) tells us the function "writes a character string to a console screen buffer". The prototype is the following:

```
BOOL WINAPI WriteConsole(
  _In_             HANDLE  hConsoleOutput,
  _In_       const VOID    *lpBuffer,
  _In_             DWORD   nNumberOfCharsToWrite,
  _Out_opt_        LPDWORD lpNumberOfCharsWritten,
  _Reserved_       LPVOID  lpReserved
);
```

To hook a Windows API, we need a special Python decorator which is documented in [Qiling / Hijack / Windows API](https://docs.qiling.io/en/latest/hijack/). We customize the decorator to match the parameters of WriteConsoleA.


```python
@winsdkapi(cc=CDECL, params={
    'hConsoleOutput'  : POINTER,
    'lpBuffer' : POINTER,
    'nNumberOfCharsToWrite': UINT,
    'lpNumberOfCharsWritten' : UINT,
    'lpReserved' : POINTER
})
```

We implement a basic write with a Python `print`. We read the content of the lpBuffer pointer with `ql.mem.read`

```python
@winsdkapi(cc=CDECL, params={
    'hConsoleOutput'  : POINTER,
    'lpBuffer' : POINTER,
    'nNumberOfCharsToWrite': UINT,
    'lpNumberOfCharsWritten' : UINT,
    'lpReserved' : POINTER
})
def hook_WriteConsoleA(ql : Qiling, address: int, params):
    lpBuffer = params['lpBuffer']
    towrite = params['nNumberOfCharsToWrite']
    written = params['lpNumberOfCharsWritten']
    
    data = ql.mem.read(lpBuffer, towrite)
    text = data.decode(errors='ignore')  # decode en ASCII/UTF-8 avec gestion d'erreur

    print(f"[WriteConsoleA] {text}")

    return towrite
```    

### ReadConsole

It's better but it's not finished: Qiling complains this time about not having ReadConsoleA.
Its [documentation is here](https://learn.microsoft.com/en-us/windows/console/readconsole).
We do the same.

```python
@winsdkapi(cc=CDECL, params={
    'hConsoleOutput'  : HANDLE,
    'lpBuffer' : LPVOID,
    'nNumberOfCharsToRead': DWORD,
    'lpNumberOfCharsRead' : LPDWORD,
    'pInputControl' : LPVOID
})
def hook_ReadConsole(ql : Qiling, address: int, params):
    lpBuffer = params['lpBuffer']
    toread = params['nNumberOfCharsToRead']
    lpNumberOfCharsRead = params['lpNumberOfCharsRead']
    
    user_input = input("ReadConsole input > ")[:toread]
    print(f"Read {user_input} ({len(user_input)} bytes)")
    ql.mem.write(lpBuffer, user_input.encode())

    if lpNumberOfCharsRead != 0:
        ql.mem.write(lpNumberOfCharsRead, len(user_input).to_bytes(4, 'little'))

    return 1
```

Now, the program runs successfully :) But we need to find the correct password.

### Reversing the CrackMe

With r2, we see the main functionality of the crackme lies in `main`. The program reads user input, applies some modification to it and compares it with `ph0wn`. If it is the same, we get a congratulations message, or it's a fail.

This is the part that modifies user input (first argument). The modification is applied 3 times with a different key (`g`, `!` and `E`) over the same length, 0x22.

```
            0x1400011d7      41b067         mov r8b, 0x67              ; 'g' ; 103
|           0x1400011da      ba22000000     mov edx, 0x22              ; '\"' ; 34 ; int64_t arg2
|           0x1400011df      488d4c2468     lea rcx, [var_68h]         ; int64_t arg1
|           0x1400011e4      e817feffff     call decode
|           0x1400011e9      41b021         mov r8b, 0x21              ; '!' ; 33
|           0x1400011ec      ba22000000     mov edx, 0x22              ; '\"' ; 34 ; int64_t arg2
|           0x1400011f1      488d4c2468     lea rcx, [var_68h]         ; int64_t arg1
|           0x1400011f6      e805feffff     call decode
|           0x1400011fb      41b045         mov r8b, 0x45              ; 'E' ; 69
|           0x1400011fe      ba22000000     mov edx, 0x22              ; '\"' ; 34 ; int64_t arg2
|           0x140001203      488d4c2468     lea rcx, [var_68h]         ; int64_t arg1
|           0x140001208      e8f3fdffff     call decode
```
I use [r2ai](https://github.com/radareorg/r2ai) to understand the decode function with Claude Sonnet 3.5

```
[0x140001000]> r2ai -a decompile this func
...
This function appears to be some sort of encryption/decryption routine that processes a buffer byte by byte. Here's what it does:

1. Takes three parameters:
   - A pointer to a buffer (LPVOID)
   - The length of the buffer (int64_t)
   - A key byte (passed in r8b register)

2. For each byte in the buffer:
   - Performs multiple XOR operations:
     * First with 0x34 (52)
     * Then with 0x24 (36)
     * Finally with the provided key byte
   - Does some additional arithmetic operations that might be meant to obscure the actual encryption process
   
3. The function includes what appears to be some anti-analysis code or additional transformations:
   - Bit manipulation with counter + 4
   - Complex arithmetic with constants 0x2C (44) and 0x21 (33)

Interesting observations:
1. Some of the operations appear to be redundant or might be meant to confuse analysis
2. The XOR operations with 0x34 and 0x24 are actually overwritten before being used
3. The only effective transformation on the buffer is the XOR with the key parameter
4. The final arithmetic operations with 0x2C and 0x21 don't affect the buffer contents

This might be a simple XOR encryption routine that has been obfuscated with additional meaningless operations to make analysis more difficult. The actual effective operation is just XORing each byte with the key parameter.
```

The AI tells us the program is somewhat obfuscated with junk code, but in reality it performs nothing more than an XOR.

```c
void simplified_process_buffer(uint8_t* buffer, int64_t length, uint8_t key) {
    for(int i = 0; i < length; i++) {
        buffer[i] ^= key;
    }
}
```

### Reversing the password

We know the result must be `ph0wn`. We can simply crack the password by reversing the XOR operations:

```c
#include <stdio.h>
#include <stdint.h>

void xor(uint8_t* buffer, int64_t length, uint8_t key) {
    for(int i = 0; i < length; i++) {
        buffer[i] ^= key;
    }
}

void main(void) {
  char buffer[0x23] = "ph0wn";
  xor(buffer, 0x22, 0x67);
  xor(buffer, 0x22, 0x21);
  xor(buffer, 0x22, 0x45);
  printf("buffer=%s\n", buffer);
}
```

The result is `sk3tm`

### Checking we get congrats

We run the binary with Qiling, and enter that password. We get the expected congratulations message.

```
[WriteConsoleA] Enter the password: 
[=] 	WriteConsoleA(hConsoleOutput = 0xfffffff5, lpBuffer = 0x14001a000, nNumberOfCharsToWrite = 0x14, lpNumberOfCharsWritten = 0x1cf4c, lpReserved = 0) = 0x14
ReadConsole input > sk3tm
Read sk3tm (5 bytes)
[=] 	ReadConsole(hConsoleOutput = 0xfffffff6, lpBuffer = 0x80000001cf80, nNumberOfCharsToRead = 0x21, lpNumberOfCharsRead = 0x80000001cf48, pInputControl = 0) = 0x1
[WriteConsoleA] Congratulation on stage 2 !
```

