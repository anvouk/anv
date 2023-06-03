# anv

anv is a collection of one-header-only C99 libraries.

All libraries prefixed with `anv_` are made by me and under MIT license.

The libs inside the `repackages` folder are existing libs which have been converted
into single header-only and/or with changes. These should be 1:1 drop-in replacement
to the originals without any changes in existing public interfaces code.

List of changes applied:

- Converted to single header only lib
- Reformatted/cleaned-up
- Made conformant with stricter build options (e.g. `-Wall -Wextra -Werror -Wpedantic`)
- Other improvements (see comments in header files for more info)

All repackaged libs are under their original license.

Needless to say, this collection has been heavily inspired by [stb](https://github.com/nothings/stb).
Hope this small collection may be of help.

## Usage

Every header has its own `[HEADER_NAME]_IMPLEMENTATION` macro.
Define this macro before including the source file in exactly one place in your
program.

After this is done, the header can be included as normal wherever is needed.

## ANV libs list

|      Headers      |  OS   | Description                                |
|:-----------------:|:-----:|--------------------------------------------|
|    anv_hhoh.h     |  Win  | File handles handler (thxs to windows...)  |
| anv_testsuite_2.h | Cross | Simple, self-contained unit test library   |
|  anv_metalloc.h   | Cross | Store metadata for allocated memory blocks |
|     anv_arr.h     | Cross | Dynamic general purpose heap array in C    |

## Repackaged libs

|   Headers   |     OS      |      LICENSE      | SOURCE                               | Description                                                                       |
|:-----------:|:-----------:|:-----------------:|--------------------------------------|-----------------------------------------------------------------------------------|
|  halloc.h   |    Cross    |       BSD-2       | https://github.com/apankrat/halloc   | Hierarchical memory allocator                                                     |
| stb_alloc.h |    Cross    | MIT/PUBLIC DOMAIN | https://github.com/nothings/stb      | Hierarchical memory allocator from stb.h + fixes                                  |
|  stb_ds.h   |    Cross    | MIT/PUBLIC DOMAIN | https://github.com/nothings/stb      | Dynamic arrays/maps but with stb_alloc.h integration (allows memory 'auto-free'). |
| coroutine.h | MACOS/LINUX |        MIT        | https://github.com/cloudwu/coroutine | Asymmetric coroutine library.                                                     |

## Deprecated libs

Old and deprecated stuff can be found under the `deprecated` folder.

## More examples

Additional examples/demos can be found inside the `examples` folder.

## Run tests

1. `cd tests`
2. `make`

## Benchmarks

Benchmarks against other single header C libs can be found [here](https://github.com/anvouk/anv_benchmarks)
