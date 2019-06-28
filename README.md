# anv

anv is a collection of one-header-only C libraries made by me.

Needless to say, this collection has been heavily inspired by [stb](https://github.com/nothings/stb).
Hope this small collection may be of help.

## Usage

Every header has its own `[HEADER_NAME]_IMPLEMENTATION` macro that must be 
defined one and only one time in the program before including it.

Once this is done, simply include the header like any other wherever is needed.

## Libs list

|   Headers   |  OS   | Description                                                   |
|:-----------:|:-----:|---------------------------------------------------------------|
| anv_bench.h | Cross | Quick and dirty benchmarking                                  |
| anv_hhoh.h  | Win   | File handles handler (thxs to windows...)                     |
| anv_leaks.h | Cross | Yet another memory leaks detector (Checks for bad usages too) |
| anv_trace.h | Cross | Yet another tracing library                                   |
