---
layout: default
title: "Lab Microarchitectural Timing Attacks"
nav_exclude: false
search_exclude: false
has_children: false
has_toc: false
---

## Table of contents
{: .no_toc .text-delta }

* TOC
{:toc toc_levels="3..3"}

## Setup
First, download the archive [code.zip](./code.zip), which contains the programs for this exercise session.

Each exercise has a "victim" program that you will try to attack. To mount your attack, you need to modify the corresponding "exploit" program. In the exploit program, you can use the exported functions at the beginning of the exploit file, the macros defined in `common.h`, and the helper functions in `cacheutils.h`. You cannot modify the victim program nor directly access buffers or functions that are not exported in the exploit code.

You can compile and run your attack by running `make run_[exploit_program]`.
For instance, to run the first exercise, use `make run_0_exploit_memcmp`.

> **Requirements**: The lab should run on any x86 architecture. However, microarchitectural attacks are microarchitecture-dependent so making it work on your own machine might require a bit of tweaking.
I made sure that the lab works on the lab machines of EURECOM so if you're in doubt use them.
It won't work on non-x86 architectures.
{: .warning}

> **Solutions**: I will publish the solutions on this website at the end of the week.
{: .info}

#### Credits and external sources
- Exercises 0 & 1 are adapted from **KU Leuven's course on Computer Architecture**: [https://cass-kul.github.io/exercises/7-cache/](https://cass-kul.github.io/exercises/7-cache/)
- Exercise 4 is taken from **MIT course on Secure Hardware Design**, available at [https://shd.mit.edu/home/](https://shd.mit.edu/home/). They have many more super cool open-source lab sessions on microarchitectural security so if you're interested, I strongly recommend to try it out!


## Exercise 0: Timing attack on memcmp
The file `0_victim_memcmp.c` contains the code of a password checker.
In this exercise, you have to infer the value of the password using a timing attack.
The attacker code is in `0_exploit_memcmp.c`.

1. Examine the source code in `0_victim_memcmp.c`. Can you identify a timing side-channel in the code?
2. Modify `0_exploit_memcmp.c` to measure and print the execution time of the victim. You can use the functions `rdtsc_begin` and `rdtsc_end` defined in `cacheutils.h`.
3. Explain why the execution timings being printed are not always exactly the same when repeatedly providing the exact same input. Modify your code to repeatedly measure execution times and print the median. You can use the function `compute_median` in `cacheutils.h`. Why is it a good idea to print the median instead of the average?
4. Use your timing measurements to infer the password. First infer the length before finally inferring all the secret bytes. You can assume the secret PIN code uses only numeric digits (0-9).

{% if site.show_solutions %}
#### Solution

The function `check_pwd` contains two early exits that reveal information about the secret password. We need to time this function to guess the password.

We can measure the execution time repeatedly and print the median with the following code:

```c
    /* collect execution timing samples */
    for (j = 0; j < NUM_SAMPLES; j++) {
        tsc1 = rdtsc_begin();
        allowed = check_pwd(pwd, user_len);
        tsc2 = rdtsc_end();
        times[j] = tsc2 - tsc1;
    }

    /* compute median over all samples (avg may be affected by outliers) */
    compute_median(times, NUM_SAMPLES, &med);
    printf("time (med clock cycles): %lu\n", med);
```

Even with this median, we might notice different time values for the same inputs. This happens because the timing of one execution depends on many factors. Modern processors include a wide range of microarchitectural optimizations, such as instruction and data caches, pipelining, branch prediction, dynamic frequency scaling, out-of-order execution, etc. The execution can also be delayed by external events, such as an interrupt that is handled by the operating system. Taking the median instead of the average reduces the effect of such outliers.

It's time to guess the password. First, we can notice that the program exits immediately if we provide a password with an incorrect length.

``` c
if (user_len != secret_len)
    return 0;
```

If we guess the length incorrectly, the program exits. If we guess it correctly, it continues to compare the individual characters in the password.

This means that based on the execution time, we can tell whether we guessed the length correctly: the program will take longer to execute in that case.

``` bash
Enter super secret password ('q' to exit): 0
time (med clock cycles): 68
Enter super secret password ('q' to exit): 00
time (med clock cycles): 70
Enter super secret password ('q' to exit): 000
time (med clock cycles): 372
```

Now we know that the password consists of 3 characters.

Let us examine how the program compares the individual characters:

``` c
for (i = 0; i < user_len; i++)
{
    if (user[i] != secret_pwd[i])
        return 0;
}
```

The same principle applies here: the program quits at the first character that does not match. This means that the longer the execution takes, the more characters at the start of the password we managed to guess correctly.

This way, we can guess the characters one-by-one, starting from the start of the
string. Assuming that the password is a pin made of `N` digits, this means that
to guess the entire password, we need a total of `N * 10` guesses. Compare this
with `10^N` guesses if we could not guess one-by-one, but would have to iterate
over every possible combination!

``` bash
Enter super secret password ('q' to exit): 200
time (med clock cycles): 372
Enter super secret password ('q' to exit): 300
time (med clock cycles): 372
Enter super secret password ('q' to exit): 400
time (med clock cycles): 372
Enter super secret password ('q' to exit): 500
time (med clock cycles): 782
```

``` bash
Enter super secret password ('q' to exit): 510
time (med clock cycles): 784
Enter super secret password ('q' to exit): 520
time (med clock cycles): 1072
```

``` bash
Enter super secret password ('q' to exit): 522
time (med clock cycles): 1070
Enter super secret password ('q' to exit): 523
time (med clock cycles): 1070
Enter super secret password ('q' to exit): 524
--> You entered: '524'
ACCESS ALLOWED
________$$$$
_______$$__$
_______$___$$
_______$___$$
_______$$___$$
________$____$$
________$$____$$$
_________$$_____$$
_________$$______$$
__________$_______$$
____$$$$$$$________$$
__$$$_______________$$$$$$
_$$____$$$$____________$$$
_$___$$$__$$$____________$$
_$$________$$$____________$
__$$____$$$$$$____________$
__$$$$$$$____$$___________$
__$$_______$$$$___________$
___$$$$$$$$$__$$_________$$
____$________$$$$_____$$$$
____$$____$$$$$$____$$$$$$
_____$$$$$$____$$__$$
_______$_____$$$_$$$
________$$$$$$$$$$
```

{% endif %}

## Exercise 1: Simple Flush+Reload attack

The file `1_victim_vote.c` contains the following function:

``` c
void cast_vote(uint8_t candidate)
{
    if (candidate == 'a')
        (*votes_a)++;
    else
        (*votes_b)++;
}
```

We want to detect which of the candidates the user voted for. To increase the
vote count for candidate `a` or for candidate `b`, the program first has to load
the corresponding current number of votes from memory (i.e. `votes_a` or
`votes_b`). This is the access we will try to detect. Based on whether `votes_a`
or `votes_b` has been accessed, we know which candidate got the vote.

Edit `1_exploit_vote.c` to implement the missing *flush* and *reload* attacker phases.
You can use the provided `void flush(void *adrs)` and `int reload(void *adrs)` functions defined in `cacheutils.h`.
The latter returns the number of CPU cycles needed for the reload.
If necessary, compensate for timing noise from modern processor optimizations by repeating the
experiment a sufficient number of times and taking the median.

Finally, modify the victim to check that your solution works for both `secret_candidate = 'a'` and `secret_candidate = 'a'`.

{% if site.show_solutions %}
#### Solution

Using the Flush+Reload technique, we (as the attacker) need to take the following steps:
1. Flush `votes_a` from the cache;
2. Let the `secret_vote` function execute with a secret input `candidate`. This
   will load the value of either `votes_a` or `votes_b` from memory, and place
   it in the cache;
3. After the execution is done, reload `votes_a`. If the access time is low,
   this signals a cache hit. A cache hit in turn indicates that the victim
   process has accessed `votes_a`, which means a vote for *candidate A*. On the
   other hand, if the access time is high, this means the value has not been
   cached, this is not the candidate the user voted for.

Of course, the above process could be executed with `votes_b` in place of `votes_a`. Since there are only two candidates, it suffices to check whether one of the two variables has been accessed.

In order to draw a conclusion in the above example, we need to have an idea about what a "low" and "high" access time is. To make our job easier, we can just measure the access time of both the `votes_a` and `votes_b` variables and see which one takes considerably less time: that one is the one the user voted for.

Using the provided functions, this is simply:

```c
// 1) Flush both probe slots
flush(votes_a);
flush(votes_b);
// 2) Victim runs once, touching exactly one slot
run_victim();
// 3) Reload both and record timings
time_a = reload(votes_a);
time_b = reload(votes_b);
```

Similarly to the previous example, we will compensate for variations in the timings by averaging the measurements over multiple executions. The final code is thus:


``` c
    uint64_t times[2][REPEAT];

    for (int r = 0; r < REPEAT; r++) {
        // 1) Flush both probe slots
        flush(votes_a);
        flush(votes_b);

        // 2) Victim runs once, touching exactly one slot
        run_victim();

        // 3) Reload both and record timings
        times[0][r] = reload(votes_a);
        times[1][r] = reload(votes_b);
    }

    // Compute medians
    uint64_t meda, medb ;
    compute_median(times[0], REPEAT, &meda);
    compute_median(times[1], REPEAT, &medb);

    printf("Median times: candidate a=%lu cycles, candidate b=%lu cycles\n", meda, medb);

    char guessed = (meda < medb) ? 'a' : 'b';
    printf("Guessed secret_candidate = %c\n", guessed);
```

This will provide us with an output from which it is clear to see which candidate has been voted for:

``` bash
make run_1_exploit_vote
taskset -c 6 ./1_exploit_vote
Median times: candidate a=483 cycles, candidate b=138 cycles
Guessed secret_candidate = b
```

{% endif %}

## Exercise 2: Extract pin code with Flush+Reload
The last exercise extracted a simple boolean using Flush+Reload.
In this exercise, we increase the difficulaty and try to extract a 4 digit pin code from the victim.

Take a look at the code in `2_victim_pin.c`.
The function `run_victim` contains an out-of-bound memory access that we can exploit to access the secret.
The secret will then be encoded in the second memory access to `shared_array`.

``` c
void run_victim(size_t idx) {
    __asm__ volatile ("" ::: "memory");
    // Notice the out of bound access here
    // It can be used to encode secret in the cache
    uint8_t v = public_arr[idx];
    shared_array[v * PROBE_SCALE]++;
}
```

1. Use `readelf` or `objdump` to find suitable indexes to pass to the function in order to access the secret. Hint: look for the symbols `public_array` and `secret`.

2. Fill the code in `2_exploit_pin.c` to extract the secret using Flush+Reload.
   Hint: first start with the first digit of the pin.
   Identify which memory locations are accessed by the victim in `shared_array`.
   Use this information to infer the secret.
   Like in the other exercises, repeat the measurements multiple times and take the median.

3. When you have managed to extract the first digit, you can generalize to extracting the 3 others!

**Troubleshouting:**
- Sometimes the prefetcher gets in the way of the attack and starts prefetching the whole array, making individual numbers indistinguishable. In that case, try to change the `PROBE_SCALE` parameter in `common.h` to something that is bigger and that will look more irregular (definitely not a power of 2!).
- You can also reduce the number of iterations for the measurements (on machines I tested, 1 iteration was actually accurate enough!).

{% if site.show_solutions %}
#### Solution

We first need to check the location of `public_arr` and `secret` in the binary to calculate the correct index:

```
objdump -t 2_exploit_pin | grep -nE "(secret|public)"
24:0000000000004000 g     O .data       0000000000000010              public_arr
[...]
45:0000000000004010 g     O .data       0000000000000005              secret
```

We can see that `secret` is located 16 bytes after `public_arr` in the binary: they are contiguous.
Therefore, to access `secret[0] .. secret[3]` we can use `public_arr[16] .. public_arr[19]` 


TODO: Finish writing the solution

{% endif %}

## Exercice 3: Extract vote code with Spectre-v1
After realizing that his code was insecure, our developer from Exercise 1 came up with a different design.
Now the victim first authenticates, cast his vote, and deauthenticate. While de-authenticating, the victim fluses all relevant entries so an attacker monitoring the cache bofore/after `run_victim` cannot learn anything about `SECRET_CANDIDATE`. Additionally, our attacker cannot just run `cast_vote` to trigger the memory access because she will not be authenticated.

Unfortunately (or fortunately for us), our developer was not aware of Spectre attacks. Your goal in this exercise is to adapt your solution from Exercise 1 to bypass the authentication and retrieve `SECRET_CANDIDATE`.

``` c
int cast_vote()
{
    uint8_t auth = is_authenticated();
    // Only cast vote if the victim is authenticated
    // But can we bypass this check using Spectre?
    if (auth) {
        probe_buffer[IDX(SECRET_CANDIDATE)]++;
        return 0;
    }
    return -1;
}

void run_victim(void)
{
    authenticate(); // sets auth to 1
    cast_vote();
    deauthenticate(); // flush all cache entries so nothing can be learned
}
```

1. Try your solution from exercise 1 that monitors accesses to `run_victim`. Does it work in this case?
2. Now try to monitor access to `cast_vote` instead. Does it work?
3. Normally, the two previous attemps should not work. You first need to train the branch predictor to trigger mispredictions in `cast_vote`. Implement this successful attacks and make sure it works with both `CANDIDATE_A` and `CANDIDATE_B`.

**Troubleshouting:**

- Beware that branch predictors are complex, can be susceptible to noise, and are also very good at predicting patterns. Therefore do NOT run experiments in a loop to get an average like in the previous exercises: doing that can make it harder to trigger misprediction because the predictor will quickly learn the pattern. Instead, just run your code several times to see the pattern (on my machine, even 1 iteration was very accurate!). However, I encourage you to try it yourself, and if you get it working in a loop, send me your solution :D! 
- If the attack does not work, you can modify the victim for debugging. For instance, you can add a `dummy` shared variable that's accessed in the branch to see if you successfully trigger misspeculation.
- Of course, don't expect to be able to print things during your speculation window!
- If you think that you cannot trigger mispredictions, modify the victim to make the branch resolution slower with the following code. You can export `flush_auth` in your attack before calling the victim to make sure the branch will be very slow and increase your chances of misspeculation!

```
volatile uint8_t authenticated = 0;
volatile uint8_t *authenticated_ptr = &authenticated;
volatile uint8_t **authenticated_ptr_ptr = &authenticated_ptr;
volatile uint8_t ***authenticated_ptr_ptr_ptr = &authenticated_ptr_ptr;

void flush_auth(void) {
    flush((void*)&authenticated);
    flush((void*)&authenticated_ptr);
    flush((void*)&authenticated_ptr_ptr);
    flush((void*)&authenticated_ptr_ptr_ptr);
}

static inline uint8_t is_authenticated() {
    return ***authenticated_ptr_ptr_ptr;
}
```

## Exercise 4: Gather information about your target
**Original source:** [https://shd.mit.edu/2025/labs/cache.html](https://shd.mit.edu/2025/labs/cache.html)

For the previous attacks, we had shared memory and could use Flush+Reload so getting detailed information about the cache was not so critical. However, for more complex attacks like Prime+Probe that we saw during the lecture, it is important to gather more knowledge about your microarchitecture. In this part of the lab, you will see a few practical approaches people use to gain detailed microarchitecture information of commodity hardware.

{% if site.exercise_4_1 %}
### 4.1: Determining Machine Architecture
{% endif %}

The simplest way to gather hardware information is to use existing public system interfaces and documentation. Here is a list of commands that can be used to determine machine architecture information on Linux.

- `lscpu`: Provides information on the type of processor and some summary information about the architecture in the machine.
- `less /proc/cpuinfo`: Provides detailed information about each logical processor in the machine. (Type q to exit.)
- `getconf -a | grep CACHE`: Displays the system configuration related to the cache. This will provide detailed information about how the cache is structured. The numbers that are reported using this command use Bytes (B) as the unit.

In addition, [https://en.wikichip.org/wiki/WikiChip](WikiChip) is a good source of information, as it provides information specific to each processor and architecture. You can find a detailed architecture description of many machines here, which additionally provides the raw latency value for accessing different levels of caches.

> Fill in the blanks in the following table using the information you gathered about the cache configuration of your machine. You should be able to directly obtain the information for the first 3 blank columns using commands above. You will need to derive the number of sets from information in the previous columns. For the raw latency, try to find it on [https://en.wikichip.org/wiki/WikiChip](WikiChip).
{: .info}

| Cache     | Cache Line Size | Total Size | Number of Ways (Associativity) | Number of Sets | Raw Latency |
|-----------|-----------------|------------|--------------------------------|----------------|-------------|
| L1-Data   |                 |            |                                |                |             |
| L2        |                 |            |                                |                |             |
| L3        |                 |            |                                |                |             |


{% if site.show_solutions %}
##### Solution
1. Fill the first three columns from `getconf -a | grep CACHE`
2. Number of sets = Total Size / (Cache Line Size × Associativity)

TODO: Example for lab machine?
{% endif %}

{% if site.exercise_4_1 %}

### 4.2: Timing a Memory Access

The information you can get from public sources can be limited, as hardware companies would not like to disclose all of their proprietary design details to general users and potential competitors. An alternative way to gather information is to reverse engineer the processor by running some very carefully designed instruction sequences on the processor and observing their behaviors.

In this part, you will try to reverse engineer the latencies for accessing the cache hierarchy. Specifically, we would like to know how long it takes to access cache lines that are located in the (a) L1 data cache, (b) L2 cache, (c) L3 cache, and (d) the DRAM.

#### The Reverse Engineering Plan

To measure the L1 latency, we can perform a load operation on a target address to bring the corresponding cache line into the L1 data cache. Then, we measure the access latency by counting the cycles it takes to re-access the same target address using measure_one_block_access_time. We have provided this code for you, and you can compile the starter code using the command make, and then run it with make run.

TODO: use cacheutils

Your task is to complete the `main` function in `TODO` to populate the three arrays dram_latency, l2_latency, and l3_latency. 

1. Start with measuring DRAM latency, since measuring DRAM latencies is the easiest. You can leverage the instruction `clflush` to place the target address to DRAM.

2. Measuring L2 and L3 latencies is slightly more complex. To measure the L2 latency, we need to place the target address in the L2 cache. However, simply accessing the target address will make the address reside in the L1 cache. Therefore, need to access other addresses to evict the target address from the L1 cache. Thus, you first need to access the line to bring it into L1, then create cache conflicts to evict it into L2. When it comes to measuring the L3 latency, you need to similarly create cache conflicts to evict the cache line from both the L1 cache and the L2 cache.

> Fill in the code in main.c to populate the arrays dram_latency, l2_latency, and l3_latency.
{: .info}

#### Helper Functions

Before you start, read the code in utility.h and understand the following functions.

- `rdtscp` and `rdtscp64`: Read the current timestamp counter of the processor and return a 32-bit or 64-bit integer.
- `lfence`: Perform a serializing operation. Ask the processor to first complete the memory loads before the lfence instruction, then start issuing memory loads after the lfence instruction. Other variants of fences exist, such as sfence and mfence.
- `measure_one_block_access_time`: Measure the latency of performing one memory access to a given address.
- `clflush`: Flush a given address from the cache, evict the line from the whole cache hierarchy so that later accesses to the address will load from DRAM.
- `print_results_plaintext` and `print_results_for_visualization`: Print the collected latency data in different formats. The default Makefile compiles two binaries: `main` uses `print_results_plaintext`, while main-visual uses `print_results_for_visualization`.

#### Pointer Arithmetic
Pointer arithemetic operations, such as `new_ptr = old_ptr + 1`, means moving the pointer forward by one element. For different types of pointers whose element size is different, the actual bytes being moved can be very different. For example, given a `uint8_t` pointer, since each element is 1 byte, +1 means moving the pointer foward by 1 byte. However, +1 of a `uint64_t` pointer means moving the pointer forward by 8 bytes. We highly suggest to use `uint8_t` pointers to make your address calculation easier and avoid introducing addressing mistakes.

#### Visualization Support

Microarchitectural side channels are notoriously noisy, and it is common to get inconsistent latency results from run to run. To combat noise, the most commonly used methodology is to repeat the experiments and plot the distribution of the observed latencies. We have provided two Python scripts to help you launch multiple measurements and visualize these measurements. To install python packages used in these two scripts, please run:

`python3 -m pip install matplotlib tqdm`

- `run.py`: A python script that will generate 100 runs from the main-visual binary. It will create a folder (if one doesn’t already exist) called data, and it will store all the generated samples there in json format. The script will overwrite the folder if it already exists.
- `graph.py`: A python script that will plot the histogram of the samples collected from run.py. It will read the JSON files from the folder data and generate a pdf file of the histogram in a folder called graph.

#### Expected Outcome

If you want to check whether you are on the right track, you should look for the following patterns in your visualized plot. We also include an example plot below.

- There are distinct peaks for DRAM, L3, and L2 latency.
- The L1 and L2 latency do not need to be distinguishable.

TODO: Reference distribution plot

#### Practical tips
DO NOT take latency measurements while also printing. Instead, measure then print.

When debugging your code, it is tempting to write code like this, which we call “measuring while printing”.

```c
    for i in l1_cache:
        # Observe some aspect of the cache state
        val = time_access(cache line i)
        # In the same measurement loop, print the observed value out!
        printf("The cache took %d cycles", val)
        # Now we go to the next interation and measure again
```

Do not do this! We are no longer in the regular world, we are in the microarchitectural world, where each assembly instruction counts!

What do we mean by this? Under the hood, a “simple” call to printf involves executing a huge number of instructions. When you call printf, you are going to go to the libc library, doing some string processing, and eventually making a system call into the kernel (so, the entire CPU performs a context switch, and does who knows what else). Think about how many cache lines this call to printf will need to read/write – printing anything is a destructive action to the state of the cache.

Instead, you should measure then print. We suggest structuring your code like this:

```c
uint64_t measurements[NUM_THINGS_TO_MEASURE]
# Measure
for i in l1_cache:
    measurements[i] = time_access(cache line i)
# Then, print :)
print(measurements)
```

The following tips may help you if you get stuck when you could not observe differences between the L2 and L3 cache latency. A common pitfall is not properly evicting the target address from the L1/L2 cache due to various reasons.

- Cache Line Size != Integer Size: To begin with, you should be careful with the the mismatch of access granularities. The smallest operational unit in cache is the cache line, which is larger than the size of an integer. Accessing two integers that fall into the same line (more precisely, that fall within the same cache line size aligned region of memory) will result in a cache hit, and won’t cause an eviction. So make sure to use eviction addresses that do not map to the same cache line when attempting to evict.

- Advanced Cache Replacement Policy: The cache replacement policy in modern processors is more advanced than the simple policies that we learned in class, and is often not completely known to us. It may intelligently decide to keep a target address in the cache, rather than evicting it. To combat the advanced replacement policy, we suggest accessing the eviction buffer multiple times.

- Virtual to physical address translation: Intuitively, we would imagine that given a cache, if we have a buffer whose size matches the cache size, then accessing each element in the buffer allows us to fully occupy every slot in the cache. However, this may not always be the case, due to virtual to physical address translation. Note that on our machine, the cache mapping is a function of physical address, while the software uses virtual address.

Let’s consider a toy example where a 8KB directly-mapped cache which can hold two 4K pages. If we have a two-page-size buffer, after virtual address translation, we can end up with three posibilities: 1) the buffer covers the whole cache; 2) both pages map to the top half of the cache; and 3) both pages map to the bottom half of the cache.

In this case, how can we reliably evict data from a certain cache level without the control of the address translation procedure? The common trick is to just use a buffer that is bigger than the cache itself – between 1.5x or even 4x of the cache size. Even though the eviction might still not be guaranteed, its likelihood is high enough.
{% endif %}

## Bonus Spectre exercise
If you want more, look at the exercise in the folder `bonus`. It's similar to Exercise 2 but with a Spectre twist.
Now, the victim has architectural protection against buffer overlows (i.e., it checks that the index is in bounds) and the goal is to use Spectre to bypass it:

``` c
void run_victim(size_t idx) {
    if (idx < BOUND) {
        uint8_t v = public_arr[idx];
        shared_array[v * PROBE_SCALE]++;
    }
}
```

I couldn't manage to make it work on my machine but I invite you to try; and if you do manage to make it work, send me your solution :D!

The difficulty is to find an access pattern that is fast enough to fit in your misspeculation window (the multiplication we used in the previous exercise seems too slow), but still complex enough that it cannot be learned by the prefetcher (otherwise the prefetcher prefetches everything and we cannot observe secret-dependent patterns in our `shared_array`).
You'll probably have to modify the victim to make it work (while still keeping the essence of the exercise of course).

Some possible leads:
- You can try to change how `shared_array` is accessed (don't forget to also change in the attacker code)!
- You can try to increase the length of the speculation window in the victim.
- Start with only one character and then try to get the next ones.
