# anv

anv is a collection of one-header-only C99 libraries made by me.

Needless to say, this collection has been heavily inspired by [stb](https://github.com/nothings/stb).
Hope this small collection may be of help.

## Usage

Every header has its own `[HEADER_NAME]_IMPLEMENTATION` macro that must be
defined one and only one time in the program before including it.

Once this is done, simply include the header like any other wherever is needed.

## Libs list

|      Headers      |  OS   | Description                                |
|:-----------------:|:-----:|--------------------------------------------|
|    anv_hhoh.h     |  Win  | File handles handler (thxs to windows...)  |
| anv_testsuite_2.h | Cross | Simple, self-contained unit test library   |
|  anv_metalloc.h   | Cross | Store metadata for allocated memory blocks |
|     anv_arr.h     | Cross | Dynamic general purpose heap array in C    |

## Deprecated libs

Old and deprecated stuff can be found under the `deprecated` folder.

## More examples

Additionale examples/demos can be found inside the `examples` folder.

## Run tests

1. `cd tests`
2. `make`
