---
layout: post
title: "LabubaRAT anti-analysis features"
author: cryptax
date: 2026-07-31
draft: false
tags:
- LabubaRAT
- 2026
- malware
- RAT
- Rust
- reverse
- antidebug
- VM
- sandbox
- CUDA
---

LabubaRAT is a Remote Access Tool (RAT) implemented in Rust. It was discovered and analyzed by [two researchers at Blackpoint Cyber’s Adversary Pursuit Group](https://blackpointcyber.com/blog/labubarat-a-rust-based-remote-access-tool-masquerading-as-nvidia-software/).

Usually, when there's one technical analysis, people tend to think it's "enough". In reality, in complex malware such as this one, there are always several different angles to look into and that can't be covered in a single blog post.

In this blog post, I am going to cover techniques the malware author used to deceive the analyst.

## No malloc / free

In the main function of the RAT - and mostly everywhere in the binary - we see numerous calls to `GetProcessHeap()`, or `HeapFree()` etc.

```c
if (puVar20[-1] != 0) {
   pvVar2 = (LPVOID)*puVar20;
   pvVar10 = GetProcessHeap();
   HeapFree(pvVar10,0,pvVar2);
}
```

This is because this Rust binary was compiled *without linking to msvcrt* (Microsfot Visual C++ Runtime). Consequently, the binary does *not* have the standard `malloc` / `free` allocators, and Rust *uses the OS's heap API directly*.

In this case, the binary was compiled for *Windows* MSVC. Rust uses the Windows Heap API (`GetProcessHeap`, `HeapAlloc`, `HeapFree`...). Those calls appear any time Rust code uses things like `Box<T>`, `Vec<T>` or String which all allocate on the heap.

Ok. But why? Actually, this is a **deliberate anti-analysis technique** because it's impossible to hook `malloc` or `free` - which is what typical EDR or security products would attempt to do to track allocations. Any tool hooking `malloc` or `free` won't see anything special. 

## Confusing Anti-VM feature

One of the first calls the main does appear to be an anti-VM function. Precisely, it calls first what looks like a wrapper (0x14067ed60), which in turn calls the anti-VM function (0x14067e530), that I named `init_sandbox_regex`.

From assembly, the re-constructed code (written in Rust) looks like this:

```rust

fn check_sandbox_probe() -> bool {
    let patterns = [
        r"(?i)nvidia|nvml|nvenc",              
        r"(?i)gpu|cuda|cudnn",                 
        r"(?i)monitor|sensor|temperature",     
        r"(?i)fan|cooler|thermal",             
        r"(?i)display|adapter|framebuffer",    
        r"(?i)container|docker|kubernetes",    
        r"\bUUID\b|\bGUID\b",                  
        r"(?:[\d]{1,3}\.){3}[\d]{1,3}",        
        r"[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}",
    ];

    let regex_set = RegexSet::new(patterns)
        .expect("failed to compile sandbox detection regex patterns");

    regex_set.is_match("probe_init")
}
```

You will certainly notice the regex pattern, like `container|docker|kubernertes` which are perfect to detect if the binary is running in a virtual environment or not.

**Except many things are wrong and don't make sense**. Have you noticed them?

A regex such as `gpu|cuda|cudnn` detects the host has a GPU or CUDA. If this is the case, this is probably *not* a virtual environment, but a real hardware. Same for the `nvidia` or the `fan` patterns. Reciprocally, the IP address pattern can be used to detect test/virtual environments (127.0.0.1 etc). So, we have regex that match positively VM while other match it negatively. Yet, all patterns are grouped in a single set and matched together with `is_match`. With `is_match`, the result is a boolean: there is no way to say "our string matches only those patterns". Conclusion: the pattern set is weird.

Next, the line `regex_set.is_match("probe_init")` does not make sense either: we will always be matching the same hard coded string, `probe_init`, against our list of patterns. The result is fixed and will always be `false`.

So, we inspect the caller of `init_sandbox_regex`. The decompiled code is very short... too short! That's because Ghidra (12.1.2) messes it up. A memcpy helper function (0x140677e40) had been marked as no-return. When Ghidra followed the call, it assumed execution never continued past it, and consequently never decompiled the code after the call.

![](/images/labuba_init_anti_analysis.png)

The solution is to clear the no-return flag for the memcpy helper function, then delete and re-create the function.

![](/images/labuba_init_anti_analysis_full.png)

We see that `init_sandbox_regex()` returns a boolean in `sandbox_ret`. This value is then stored in `db_path_len` (does not make sense) and finally overwritten later. Indeed, `std::env::temp_dir` returns a temporary directory, and to that, the code appends `nvctr_sys.db`.

So, the return value of `init_sandbox_regex()` is never used. It's **dead code**, either left there by some bad copy/paste, or intentionally left there to confuse the analyst (I confirm it confused me...).

**There is absolutely no VM/sandbox detection at all**, the entire point of the function is to create the absolute path for the config database `nvctr_sys.db`.

## Time check

Later, the LabubaRAT main function shows the following code:

```
  GetSystemTimePreciseAsFileTime(&local_468);
  if (local_468 < 0x19db1ded53e8000) {
    lVar16 = 0xeec9d9346455996;
  }
  else {
    lVar16 = (((local_468 + 0xfe624e212ac18000U) / 10000000) * -0x61c8864680b583eb ^
             ((local_468 + 0xfe624e212ac18000U) % 10000000) * 100) * -0x61c8864680b583eb;
  }
  DVar8 = GetCurrentProcessId();
  DAT_1408940e8 =
       (longlong)&local_468 * 0x517cc1b727220a95 + lVar16 + (ulonglong)DVar8 * 0x6c62272e07bb0142;
```



`GetSystemTimePreciseAsFileTime` get the current date. This is compared with a constant, `0x19db1ded53e8000` which corresponds to Nov 20, 2031.

If the current date is before that date - this is the expected situation - we go in the `if` and use a constant `lVar16`. If the date is greater (the malware author probably wants to detect this abnormal situation), another constant is computed.

From the current process id, the `lVar16` value and the current date, a seed is computed. Using a different constant when the date is abnormal ensures the seed is of a different nature in that case, and consequently for example, that subsequent decryption will fail.

This is a **time check** function. 

## Anti-Debug

After generating the seed, the code calls `IsDebuggerPresent()`.

```
  BVar9 = IsDebuggerPresent();
  if (BVar9 != 0) {
    infinite_loop();
    if ((unaff_R13 & 1) == 0) {
                    /* WARNING: Subroutine does not return */
      FUN_1403148b0(0);
    }
                    /* WARNING: Subroutine does not return */
    FUN_1403148b0(0,0x18);
  }
```

If a debugger is indeed present, the code will go into an infinite loop (causing the analyst difficulty to debug and stop the process).

This is basic but effective debugger detection code.

## Conclusion

The sample has exhibited:

1. Anti-analysis with no malloc/free
2. Dead code looking like anti-VM, potentially left to confuse the analyst, or just by error
3. Time check
4. Anti-debug

If you like reverse engineering, there is (nearly) *always* more to say on a sample which has already been analyzed. Look into it. You'll learn lots of assembly tricks, and also in this case, Rust specificities.

-- Cryptax