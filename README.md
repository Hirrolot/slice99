# Slice99
[![CI](https://github.com/Hirrolot/slice99/workflows/C/C++%20CI/badge.svg)](https://github.com/Hirrolot/slice99/actions)
[![docs](https://img.shields.io/badge/docs-github.io-blue)](https://hirrolot.github.io/slice99/slice99_8h.html)

This library provides [array slicing] facilities for pure C99.

[array slicing]: https://en.wikipedia.org/wiki/Array_slicing

[[`examples/demo.c`](examples/demo.c)] [ [Playground >>](https://godbolt.org/z/ef3j66) ]
```c
#include <slice99.h>

#include <assert.h>

int main(void) {
    Slice99 str = Slice99_from_str("hello world");

    // Accessors {
    assert(*(char *)Slice99_first(str) == 'h');
    assert(*(char *)Slice99_last(str) == 'd');
    assert(*(char *)Slice99_get(str, 4) == 'o');
    // }

    // Subslicing {
    assert(Slice99_primitive_eq(Slice99_sub(str, 3, 9), Slice99_from_str("lo wor")));
    // }

    // Predicates {
    assert(!Slice99_is_empty(str));
    assert(Slice99_primitive_starts_with(str, Slice99_from_str("hello")));
    assert(Slice99_primitive_ends_with(str, Slice99_from_str("world")));
    // }

    // Mutators {
    Slice99 data = Slice99_from_array((int[]){5, 8, 1, 9});
    int backup;

    Slice99_swap(data, 1, 3, &backup);
    assert(*(int *)Slice99_get(data, 1) == 9);
    assert(*(int *)Slice99_get(data, 3) == 8);

    data = Slice99_from_array((int[]){1, 2, 3});
    Slice99_reverse(data, &backup);
    assert(Slice99_primitive_eq(data, Slice99_from_array((int[]){3, 2, 1})));
    // }

    // ... and more!
}
```

The main type is [`Slice99`]. It represents a pointer to some array accompanied with an array item size and its length. The utilities `Slice99_*` are used to manipulate this type.

To install Slice99, just copy [`slice99.h`](slice99.h) to your code base (or use [Git submodules]) and then `#include <slice99.h>` where necessary.

Enjoy!

[`Slice99`]: https://hirrolot.github.io/slice99/structSlice99.html
[Git submodules]: https://git-scm.com/book/en/v2/Git-Tools-Submodules

## Motivation

Normally, we pass a data pointer with its length as separate arguments:

```c
void foo(size_t len, uint8_t buffer[static len]) { /* ... */ }
```

However, this interface is notoriously easy to misuse by passing an invalid length or a null pointer for `buffer`. Moreover, programmers frequently tend to perform specific operations on `buffer` and `len`, leading to even more bugs and code clutter:

```c
// Advance the buffer by HEADER_SIZE.
buffer += HEADER_SIZE;
len -= HEADER_SIZE;
```

Slice99 reifies such patterns into functions (`Slice99_advance` below), thus assisting in code safety and clarity:

```c
void foo(Slice99 buffer) {
    // ...
    buffer = Slice99_advance(buffer, HEADER_SIZE);
    // ...
}
```

Another use case of Slice99 is zero-copy parsers: you can return slices from your parser pointing to actual data, without `malloc`ing and `memcpy`ing just to append `'\0'` each time.

## Typed slices

You can define a strongly typed slice with `SLICE99_DEF_TYPED`:

```c
#include <slice99.h>

typedef struct {
    double x, y;
} Point;

SLICE99_DEF_TYPED(MyPoints, Point);

int main(void) {
    MyPoints points =
        Slice99_typed_from_array(MyPoints, (Point[]){{1.5, 32.5}, {12.0, 314.01}, {-134.10, -9.3}});

    MyPoints first_two = MyPoints_sub(points, 0, 2);
    Point *first = MyPoints_first(points);
    bool is_empty = MyPoints_is_empty(points);
}
```

`SLICE99_DEF_TYPED(MyPoints, Point)` generates a slice named `MyPoints` of the following structure:

```c
typedef struct {
    Point *ptr;
    size_t len;
} MyPoints;
```

It also generates `inline static` functions like `MyPoints_sub` that type-check their arguments. These functions merely desugar to their untyped `Slice99_*` counterparts.

### Fundamental types

This library automatically defines typed slices of several fundamental types. I recommend using them instead of untyped `Slice99`, if possible. For more information, see the docs.

## Projects using Slice99

 - [Hirrolot/smolrtsp](https://github.com/Hirrolot/smolrtsp) --  A small, portable, extensible RTSP 1.0 implementation in C99.

## FAQ

### Q: Can I use this library to develop bare-metal software?

A: Yes, see the [docs](https://hirrolot.github.io/slice99/slice99_8h.html#details).
