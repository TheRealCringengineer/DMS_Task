# DMS_Task

## Project
Simple project for `DMS (Driver Monitoring System`

The idea is to parse events from some "input" and detected specific events in it

Examples (from `CustomConditions.h`):
1) Driver is looking away from the road/camera for more then 2 seconds
2) Driver is sleeping. Both eyes are closed for more then 2 seconds

Basic path of data is:
```
                              ┌─────────────┐
                              │CONDITION    │
                              │             │
                              └─────────────┘
                                     ▲       
                                     │CHECKS 
┌─────────┐                          │       
│SOME     │         ┌─────────┐ ┌────┼────┐  
│INPUT    │         │INPUT    │ │DECODER  │  
│SOURCE   ┼────────►│DATA     ┼─►         │  
│         │         └─────────┘ └────┬────┘  
│         │                          │       
└─────────┘                          │SENDS  
                                     │       
                                ┌────▼────┐  
                                │EVENT    │  
                                │BUS      │  
                                │         │  
                                └────┬────┘  
                                     │       
                                     │       
                              ┌──────▼─────┐ 
                              │ │ │HANDLERS│ 
                              │ │ │        │ 
                              │ │ │        │ 
                              └────────────┘ 
```

## Building
Should work fine both on windows and/or linux
Tested on MSVC 19.41.34123 x64
Tested on gcc version 12.2.0 x64

### How to build
```
cd DMS_Task
cmake -S . -B build_dir ...additional params
cmake --build build_dir
```
That's all

## Usage
Very simple example on how different components should be used is in `main.cpp`
Assuming you have nothing and want to implement everything from scratch

1) Implement `InputData` interface for your own method of data input
2) Implement `Decoder` interface to process data from `InputData` and check conditions
3) Implement one or multiple `Condition` to check specific values in `InputData`
4) Use `EventBus` to process different events detected in `Decoder`

## Memory leaks check

```
==9328== Memcheck, a memory error detector
==9328== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==9328== Using Valgrind-3.19.0 and LibVEX; rerun with -h for copyright info
==9328== Command: ./DMS_Task
==9328==
Event : Looking away started at 4.9 ended at 7.8
Event : Looking away started at 8.4 ended at 10.75
Event : Looking away started at 13.55 ended at 15.85
Event : Eyes closed started at 17.15 ended at 19.25
Event : Looking away started at 19.4 ended at 23
Event : Looking away started at 26.2 ended at 29.75
Event : Looking away started at 37.2 ended at 45.05
==9328==
==9328== HEAP SUMMARY:
==9328==     in use at exit: 0 bytes in 0 blocks
==9328==   total heap usage: 21,987 allocs, 21,987 frees, 582,542 bytes allocated
==9328==
==9328== All heap blocks were freed -- no leaks are possible
==9328==
==9328== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
