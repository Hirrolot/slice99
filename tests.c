/*
 * MIT License
 *
 * Copyright (c) 2020 Temirkhan Myrzamadi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define SLICE99_INCLUDE_SORT
#define SLICE99_INCLUDE_BSEARCH

#include <slice99.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert_algebraic.h>

#define TEST(name) static void test_##name(void)

// A small eDSL for asserting a slice {
#define ASSERT_SLICE(slice, expected_ptr, expected_item_size, expected_len)                        \
    do {                                                                                           \
        Slice99 slice_ = (slice);                                                                  \
        assert(slice_.ptr == (expected_ptr));                                                      \
        assert(slice_.item_size == (expected_item_size));                                          \
        assert(slice_.len == (expected_len));                                                      \
                                                                                                   \
    } while (0)

#define PTR
#define ITEM_SIZE
#define LEN
// }

// Property-based testing {
static Slice99 gen_int_slice(void) {
    const size_t len = rand() % (10);
    Slice99 data = Slice99_new(malloc(len * sizeof(int)), sizeof(int), len);

    for (size_t i = 0; i < len; i++) {
        *(int *)Slice99_get(data, i) = rand() % INT_MAX;
    }

    return data;
}
// }

// Auxiliary functions {
static int int_comparator(const void *lhs, const void *rhs) {
    return *(const int *)lhs - *(const int *)rhs;
}

static bool slice_int_eq(Slice99 lhs, Slice99 rhs) {
    return Slice99_eq(lhs, rhs, int_comparator);
}
// }

TEST(from_str) {
    // clang-format off
    const char *str = "";
    ASSERT_SLICE(
        Slice99_from_str(""),
        PTR str,
        ITEM_SIZE sizeof(char),
        LEN strlen(str)
    );

    str = "abc";
    ASSERT_SLICE(
        Slice99_from_str("abc"),
        PTR str,
        ITEM_SIZE sizeof(char),
        LEN strlen(str)
    );
    // clang-format on
}

TEST(from_ptrdiff) {
    int data[] = {1, 2, 3, 4, 5};

    // clang-format off
    ASSERT_SLICE(
        Slice99_from_ptrdiff(data, data, sizeof(int)),
        PTR data,
        ITEM_SIZE sizeof(int),
        LEN 0
    );

    ASSERT_SLICE(
        Slice99_from_ptrdiff(data, data + Slice99_array_len(data),
        sizeof(int)),
        PTR data,
        ITEM_SIZE sizeof(int),
        LEN Slice99_array_len(data)
    );

    ASSERT_SLICE(
        Slice99_from_ptrdiff(data + 1, data + 4, sizeof(int)),
        PTR data + 1,
        ITEM_SIZE sizeof(int),
        LEN 3
    );
    // clang-format on
}

TEST(is_empty) {
    assert(!Slice99_is_empty(Slice99_from_array((int[]){1, 2, 3})));
    assert(Slice99_is_empty(Slice99_new("abc", 1, 0)));
}

TEST(size) {
    assert(Slice99_size(Slice99_from_array((int[]){1, 2, 3})) == sizeof(int) * 3);
    assert(Slice99_size(Slice99_new("abc", 1, 0)) == 0);
}

TEST(get) {
    int data[] = {1, 2, 3};
    Slice99 slice = Slice99_from_array(data);

    assert(Slice99_get(slice, 0) == &data[0]);
    assert(Slice99_get(slice, 1) == &data[1]);
    assert(Slice99_get(slice, 2) == &data[2]);

    assert((int *)Slice99_get(slice, 3) - 1 == &data[2]);
    assert((int *)Slice99_get_cast_type(slice, -1, int) + 1 == &data[0]);
}

TEST(first) {
    int data[] = {1, 2, 3};
    assert(Slice99_first(Slice99_from_array(data)) == &data[0]);
}

TEST(last) {
    int data[] = {1, 2, 3};
    assert(Slice99_last(Slice99_from_array(data)) == &data[2]);
}

TEST(sub) {
    int data[] = {1, 2, 3, 4, 5};

    // clang-format off
    ASSERT_SLICE(
        Slice99_sub(Slice99_from_array(data), 0, 0),
        PTR data,
        ITEM_SIZE sizeof(int),
        LEN 0
    );

    ASSERT_SLICE(
        Slice99_sub(Slice99_from_array(data), 0, 3),
        PTR data,
        ITEM_SIZE sizeof(int),
        LEN 3
    );

    ASSERT_SLICE(
        Slice99_sub(Slice99_from_array(data), 2, 4),
        PTR data + 2,
        ITEM_SIZE sizeof(int),
        LEN 2
    );

    ASSERT_SLICE(
        Slice99_sub_cast_type(Slice99_new(data + 2, sizeof(int), 3), -2, 1, int),
        PTR data,
        ITEM_SIZE sizeof(int),
        LEN 3
    );

    ASSERT_SLICE(
        Slice99_sub_cast_type(Slice99_new(data + 3, sizeof(int), 2), -2, -1, int),
        PTR data + 1,
        ITEM_SIZE sizeof(int),
        LEN 1
    );
    // clang-format on
}

// Slice99_primitive_eq {
TEST(primitive_eq_basic) {
    char str[] = "12345";

    Slice99 slice_1 = Slice99_from_array((int[]){1, 2, 3, 4, 5}),
            slice_2 = Slice99_from_array((int[]){1, 2, 3, 4, 5}),
            slice_3 = Slice99_from_array((int[]){6, 7, 8});

    assert(Slice99_primitive_eq(slice_1, slice_2));

    assert(!Slice99_primitive_eq(slice_1, slice_3));
    assert(!Slice99_primitive_eq(Slice99_from_str(str), slice_1));
}

TEST(primitive_eq_is_equivalence) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

        ASSERT_EQUIVALENCE(Slice99_primitive_eq, slices[0], slices[1], slices[2]);

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(primitive_eq) {
    test_primitive_eq_basic();
    test_primitive_eq_is_equivalence();
}
// } (Slice99_primitive_eq)

// Slice99_eq {
TEST(eq_basic) {
    Slice99 slice_1 = Slice99_from_array((int[]){1, 2, 3, 4, 5}),
            slice_2 = Slice99_from_array((int[]){1, 2, 3, 4, 5}),
            slice_3 = Slice99_from_array((int[]){6, 7, 8});

    assert(Slice99_primitive_eq(slice_1, slice_2));
    assert(!Slice99_primitive_eq(slice_1, slice_3));
}

TEST(eq_is_equivalence) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

        ASSERT_EQUIVALENCE(slice_int_eq, slices[0], slices[1], slices[2]);

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(eq) {
    test_eq_basic();
    test_eq_is_equivalence();
}
// } (Slice99_eq)

// Slice99_primitive_starts_with {
TEST(primitive_starts_with_basic) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});

    assert(Slice99_primitive_starts_with(slice, Slice99_sub(slice, 0, 0)));
    assert(Slice99_primitive_starts_with(slice, Slice99_sub(slice, 0, 3)));

    assert(!Slice99_primitive_starts_with(slice, Slice99_sub(slice, 1, 2)));
    assert(!Slice99_primitive_starts_with(slice, Slice99_sub(slice, 3, 5)));
}

TEST(primitive_starts_with_partial_order) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

        ASSERT_PARTIAL_ORDER(
            Slice99_primitive_starts_with, Slice99_primitive_eq, slices[0], slices[1], slices[2]);

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(primitive_starts_with_empty_slice_is_min) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        assert(Slice99_primitive_starts_with(slice, Slice99_empty(sizeof(int))));

        free(slice.ptr);
    }
}

TEST(primitive_starts_with_reflexive) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        ASSERT_REFLEXIVE(Slice99_primitive_starts_with, slice);

        free(slice.ptr);
    }
}

TEST(primitive_starts_with) {
    test_primitive_starts_with_basic();
    test_primitive_starts_with_partial_order();
    test_primitive_starts_with_empty_slice_is_min();
    test_primitive_starts_with_reflexive();
}
// } (Slice99_primitive_starts_with)

// Slice99_starts_with {
#define STARTS_WITH(slice, prefix) Slice99_starts_with(slice, prefix, int_comparator)

TEST(starts_with_basic) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});

    assert(STARTS_WITH(slice, Slice99_sub(slice, 0, 0)));
    assert(STARTS_WITH(slice, Slice99_sub(slice, 0, 3)));

    assert(!STARTS_WITH(slice, Slice99_sub(slice, 1, 2)));
    assert(!STARTS_WITH(slice, Slice99_sub(slice, 3, 5)));
}

TEST(starts_with_partial_order) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

#define EQ(slice, other) Slice99_eq(slice, other, int_comparator)

        ASSERT_PARTIAL_ORDER(STARTS_WITH, EQ, slices[0], slices[1], slices[2]);

#undef EQ

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(starts_with_empty_slice_is_min) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        assert(STARTS_WITH(slice, Slice99_empty(sizeof(int))));

        free(slice.ptr);
    }
}

TEST(starts_with_reflexive) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        ASSERT_REFLEXIVE(STARTS_WITH, slice);

        free(slice.ptr);
    }
}

TEST(starts_with) {
    test_starts_with_basic();
    test_starts_with_partial_order();
    test_starts_with_empty_slice_is_min();
    test_starts_with_reflexive();
}

#undef STARTS_WITH
// } (Slice99_starts_with)

// Slice99_primitive_ends_with {
TEST(primitive_ends_with_basic) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});

    assert(Slice99_primitive_ends_with(slice, Slice99_sub(slice, 0, 0)));
    assert(Slice99_primitive_ends_with(slice, Slice99_sub(slice, 3, slice.len)));

    assert(!Slice99_primitive_ends_with(slice, Slice99_sub(slice, 1, 4)));
    assert(!Slice99_primitive_ends_with(slice, Slice99_sub(slice, 0, 3)));
}

TEST(primitive_ends_with_partial_order) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

        ASSERT_PARTIAL_ORDER(
            Slice99_primitive_ends_with, Slice99_primitive_eq, slices[0], slices[1], slices[2]);

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(primitive_ends_with_empty_slice_is_max) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        assert(Slice99_primitive_ends_with(slice, Slice99_empty(sizeof(int))));

        free(slice.ptr);
    }
}

TEST(primitive_ends_with_reflexive) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        ASSERT_REFLEXIVE(Slice99_primitive_ends_with, slice);

        free(slice.ptr);
    }
}

TEST(primitive_ends_with) {
    test_primitive_ends_with_basic();
    test_primitive_ends_with_partial_order();
    test_primitive_ends_with_empty_slice_is_max();
    test_primitive_ends_with_reflexive();
}
// } (Slice99_primitive_ends_with)

// Slice99_ends_with {
#define ENDS_WITH(slice, postfix) Slice99_ends_with(slice, postfix, int_comparator)

TEST(ends_with_basic) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});

    assert(ENDS_WITH(slice, Slice99_sub(slice, 0, 0)));
    assert(ENDS_WITH(slice, Slice99_sub(slice, 3, slice.len)));

    assert(!ENDS_WITH(slice, Slice99_sub(slice, 1, 4)));
    assert(!ENDS_WITH(slice, Slice99_sub(slice, 0, 3)));
}

TEST(ends_with_partial_order) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slices[] = {
            gen_int_slice(),
            gen_int_slice(),
            gen_int_slice(),
        };

#define EQ(slice, other) Slice99_eq(slice, other, int_comparator)

        ASSERT_PARTIAL_ORDER(ENDS_WITH, EQ, slices[0], slices[1], slices[2]);

#undef EQ

        free(slices[0].ptr);
        free(slices[1].ptr);
        free(slices[2].ptr);
    }
}

TEST(ends_with_empty_slice_is_max) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        assert(ENDS_WITH(slice, Slice99_empty(sizeof(int))));

        free(slice.ptr);
    }
}

TEST(ends_with_reflexive) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        ASSERT_REFLEXIVE(ENDS_WITH, slice);

        free(slice.ptr);
    }
}

TEST(ends_with) {
    test_ends_with_basic();
    test_ends_with_partial_order();
    test_ends_with_empty_slice_is_max();
    test_ends_with_reflexive();
}

#undef ENDS_WITH
// } (Slice99_ends_with)

TEST(copy) {
    int data[] = {1, 2, 3, 4, 5};
    int copied[Slice99_array_len(data)];

    Slice99_copy(Slice99_from_array(copied), Slice99_from_array(data));
    assert(memcmp(data, copied, sizeof(data)) == 0);
    memset(copied, 0, sizeof(copied));

    Slice99_copy(Slice99_new(copied, 1, 0), Slice99_from_array(data));
    assert(memcmp(data, copied, sizeof(data)) == 0);
    memset(copied, 0, sizeof(copied));
}

TEST(sort) {
    int data[] = {123};
    Slice99 slice = Slice99_new(data, sizeof(char), 0);
    Slice99_sort(slice, int_comparator);
    assert(memcmp(data, (const int[]){123}, sizeof(data)) == 0);

    slice = Slice99_from_array((int[]){62, -15, 60, 0, -19019, 145});
    Slice99_sort(slice, int_comparator);
    assert(memcmp(slice.ptr, (const int[]){-19019, -15, 0, 60, 62, 145}, Slice99_size(slice)) == 0);
}

TEST(bsearch) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5, 6, 7});

    int key = 5;
    assert(*(int *)Slice99_bsearch(slice, &key, int_comparator) == key);

    key = 101;
    assert(Slice99_bsearch(slice, &key, int_comparator) == NULL);
}

TEST(swap) {
    int temp;

    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});

    Slice99_swap(slice, 1, 3, &temp);
    assert(*(int *)Slice99_get(slice, 1) == 4);
    assert(*(int *)Slice99_get(slice, 3) == 2);
}

TEST(swap_with_slice) {
    int temp;

    Slice99 lhs = Slice99_from_array((int[]){1, 2, 3, 4, 5}),
            rhs = Slice99_from_array((int[]){6, 7, 8, 9, 0});

    Slice99_swap_with_slice(lhs, rhs, &temp);

    assert(memcmp(lhs.ptr, (const int[]){6, 7, 8, 9, 0}, Slice99_size(lhs)) == 0);
    assert(memcmp(rhs.ptr, (const int[]){1, 2, 3, 4, 5}, Slice99_size(rhs)) == 0);
}

// Slice99_reverse {
TEST(reverse_basic) {
    int temp;

    char empty_str[] = "";
    Slice99 slice = Slice99_new(empty_str, 1, 0);
    Slice99_reverse(slice, &temp);
    assert(memcmp(slice.ptr, empty_str, Slice99_size(slice)) == 0);

    slice = Slice99_from_array((int[]){1, 2, 3, 4, 5});
    Slice99_reverse(slice, &temp);
    assert(memcmp(slice.ptr, (const int[]){5, 4, 3, 2, 1}, Slice99_size(slice)) == 0);
}

static Slice99 slice_rev_aux(Slice99 slice) {
    int temp;
    Slice99_reverse(slice, &temp);
    return slice;
}

TEST(reverse_involutive) {
    for (size_t i = 0; i < 100; i++) {
        Slice99 slice = gen_int_slice();

        int *saved_array;
        if ((saved_array = malloc(Slice99_size(slice))) == NULL) {
            abort();
        }

        memcpy(saved_array, slice.ptr, sizeof(Slice99_size(slice)));
        Slice99 saved_slice = Slice99_from_array(saved_array);

        ASSERT_INVOLUTIVE(slice_rev_aux, Slice99_primitive_eq, saved_slice);
        free(saved_array);
    }
}

TEST(reverse) {
    test_reverse_basic();
    test_reverse_involutive();
}
// } (Slice99_reverse)

// Slice99_split_at {
TEST(split_at_empty_slice) {
    Slice99 slice = Slice99_empty(1), lhs, rhs;
    Slice99_split_at(slice, 0, &lhs, &rhs);

    // clang-format off
    ASSERT_SLICE(
        lhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN slice.len
    );
    ASSERT_SLICE(
        rhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN slice.len
    );
    // clang-format on
}

TEST(split_at_non_empty_slice) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5}), lhs, rhs;
    Slice99_split_at(slice, 2, &lhs, &rhs);

    // clang-format off
    ASSERT_SLICE(
        lhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN 2
    );
    ASSERT_SLICE(
        rhs,
        PTR (int *)slice.ptr + 2,
        ITEM_SIZE slice.item_size,
        LEN 3
    );
    // clang-format on
}

TEST(split_at_beginning) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5}), lhs, rhs;
    Slice99_split_at(slice, 0, &lhs, &rhs);

    // clang-format off
    ASSERT_SLICE(
        lhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN 0
    );
    ASSERT_SLICE(
        rhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN slice.len
    );
    // clang-format on
}

TEST(split_at_end) {
    Slice99 slice = Slice99_from_array((int[]){1, 2, 3, 4, 5}), lhs, rhs;
    Slice99_split_at(slice, 5, &lhs, &rhs);

    // clang-format off
    ASSERT_SLICE(
        lhs,
        PTR slice.ptr,
        ITEM_SIZE slice.item_size,
        LEN slice.len
    );
    ASSERT_SLICE(
        rhs,
        PTR (int *)slice.ptr + 5,
        ITEM_SIZE slice.item_size,
        LEN 0
    );
    // clang-format on
}

TEST(split_at) {
    test_split_at_empty_slice();
    test_split_at_non_empty_slice();
    test_split_at_beginning();
    test_split_at_end();
}
// } (Slice99_split_at)

int main(void) {
    srand((unsigned)time(NULL));

    test_from_str();
    test_from_ptrdiff();
    test_is_empty();
    test_size();
    test_get();
    test_first();
    test_last();
    test_sub();
    test_primitive_eq();
    test_eq();
    test_primitive_starts_with();
    test_starts_with();
    test_primitive_ends_with();
    test_ends_with();
    test_copy();
    test_sort();
    test_bsearch();
    test_swap();
    test_swap_with_slice();
    test_reverse();
    test_split_at();

    puts("All the tests have passed!");
}
