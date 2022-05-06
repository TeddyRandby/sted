#ifndef STED_H
#define STED_H
/*
  This is a base layer of useful things for all of my c projects.

  Basic included utilities:
    - Buffers
    - Vectors
    - Slices
    - Typedefs
*/

/* ------------ TYPES ------------ */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u8 boolean;

typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

/* ------------ HELPERS ------------ */
#define sted_cast(type, ptr) ((type *)ptr)

// Create an unsuccessful result
#define sted_err(code) ((sted_result_t){.status = code})

// Create a successful result with a value
#define sted_ok(value) ((sted_result_t){.status = STED_OK, .data = value})

// Crash if the result is not ok.
#define sted_try(result) (assert(result.status == STED_OK))

// Crash if the result is not ok.
// Return the value, cast to the specified type.
#define sted_unwrap(type, result)                                              \
  ({                                                                           \
    sted_result_t __res__ = (result);                                          \
    assert(__res__.status == STED_OK);                                         \
    (type *)(__res__.data);                                                    \
  })

#define sted_size(self) (self->kind->item_size)

#define sted_mem(kind, ptr, size)                                              \
  ((kind)->allocator((kind), ptr, size * (kind)->item_size))

#define sted_hash(kind, ptr) ((kind)->hasher((kind), ptr))

#define true 1
#define false 0

/* ------------ STATUS CODES ------------ */
typedef enum sted_code_e {
  STED_OK = 0,
  STED_MEMORY_ERR,
  STED_BOUNDS_ERR,
  STED_CAST_ERR,
  STED_WRAP_ERR,
} sted_code_k;

typedef struct sted_result_s sted_result_t;

/* ------------ KINDS ------------ */
typedef struct sted_kind_s sted_kind_t;

typedef sted_result_t (*sted_mem_fn)(const sted_kind_t *, void *, u64);

sted_result_t sted_mem_default(const sted_kind_t *kind, void *, u64 len);

typedef u64 (*sted_hash_fn)(const sted_kind_t *, void *);

/* ------------ VECTORS ------------ */
// Macro for concatenating the vector type names easier.
#define vec_name(type, size) sted_v##size##_##type##_t

// Util macro that calls the given macro for each vector type
// passing the rest of the args along.
#define vec_eachtype(macro, ...)                                               \
  macro(i32, __VA_ARGS__);                                                     \
  macro(i64, __VA_ARGS__);                                                     \
  macro(u32, __VA_ARGS__);                                                     \
  macro(u64, __VA_ARGS__);                                                     \
  macro(f32, __VA_ARGS__);                                                     \
  macro(f64, __VA_ARGS__)

// Macro to pass to eachtype for declaring unions
#define vec_union_decl(type, size)                                             \
  typedef union sted_##v##size##_##type##_s vec_name(type, size)

// Macro to pass declare functions
#define vec_bin_decl(type, size, name)                                         \
  vec_name(type, size) sted_##v##size##_##type##_##name(vec_name(type, size),  \
                                                        vec_name(type, size))

#define vec_scale_decl(type, size)                                             \
  vec_name(type, size)                                                         \
      sted_##v##size##_##type##_##scale(vec_name(type, size), type)

#define vec_translate_decl(type, size)                                         \
  vec_name(type, size)                                                         \
      sted_##v##size##_##type##_##translate(vec_name(type, size), type)

#define vec_dot_decl(type, size)                                               \
  type sted_##v##size##_##type##_##dot(vec_name(type, size),                   \
                                       vec_name(type, size))

vec_eachtype(vec_union_decl, 2);
vec_eachtype(vec_union_decl, 3);

vec_eachtype(vec_bin_decl, 2, add);
vec_eachtype(vec_bin_decl, 3, add);

vec_eachtype(vec_bin_decl, 2, sub);
vec_eachtype(vec_bin_decl, 3, sub);

vec_eachtype(vec_bin_decl, 2, mul);
vec_eachtype(vec_bin_decl, 3, mul);

vec_eachtype(vec_bin_decl, 2, div);
vec_eachtype(vec_bin_decl, 3, div);

vec_eachtype(vec_scale_decl, 2);
vec_eachtype(vec_scale_decl, 3);

vec_eachtype(vec_translate_decl, 2);
vec_eachtype(vec_translate_decl, 3);

vec_eachtype(vec_dot_decl, 2);
vec_eachtype(vec_dot_decl, 3);

#undef vec_bin_decl
#undef vec_union_decl
#undef vec_dot_decl
#undef vec_translate_decl
#undef vec_scale_decl

/* ------------ SLICES ------------ */
typedef struct sted_slice_s sted_slice_t;

sted_result_t sted_slice_create(sted_kind_t *);

sted_result_t sted_slice_destroy(sted_slice_t *);

sted_result_t sted_slice_emplace(sted_slice_t *, void *);

sted_result_t sted_slice_append(sted_slice_t *);

sted_result_t sted_slice_pop(sted_slice_t *);

sted_result_t sted_slice_set(sted_slice_t *, u64, void *);

sted_result_t sted_slice_get(sted_slice_t *, u64);

sted_result_t sted_slice_view(sted_slice_t *, u64, u64);

#define sted_slice_each_as(s, var, body)                                       \
  for (u64 __i = 0; __i < s->len; __i++) {                                     \
    void *var = s->data + __i * sted_size(s);                                  \
    body;                                                                      \
  }

/* ------------ VIEWS ------------ */
typedef struct sted_view_s sted_view_t;

sted_result_t sted_view_create(sted_kind_t *, void *, u64);

sted_result_t sted_view_destroy(sted_view_t *);

sted_result_t sted_view_get(sted_view_t *, u64);

#define sted_view_each_as(view, var, body)                                     \
  for (u64 __i = 0; __i < view->len; __i++) {                                  \
    void *var = view->data + __i * sted_size(view);                            \
    body;                                                                      \
  }

/* ------------ DICTIONARIES------------ */
typedef struct sted_dict_s sted_dict_t;

sted_result_t sted_dict_create(sted_kind_t *, sted_kind_t *);

sted_result_t sted_dict_destroy(sted_dict_t *);

boolean sted_dict_has_key(sted_dict_t *, void *);

sted_result_t sted_dict_set(sted_dict_t *, void *, void *);

sted_result_t sted_dict_get(sted_dict_t *, void *);

#define STED_DICT_LOAD 0.7

#define sted_dict_each_as(d, key, val, body)                                   \
  for (u64 __i = 0; __i < d->cap; __i++) {                                     \
    u8 *__bucket = d->data + __i * d->internal_kind.item_size;                 \
    void *key = __bucket + 1;                                                  \
    void *val = __bucket + 1 + d->key_kind->item_size;                         \
    if (__bucket[0])                                                           \
      body;                                                                    \
  }

/* ------------ STRUCT DEFINITIONS ------------ */
struct sted_kind_s {
  u64 item_size;
  sted_mem_fn allocator;
  sted_hash_fn hasher;

  void *user_data;
  const char *user_name;
};

struct sted_result_s {
  sted_code_k status;
  void *data;
};

struct sted_view_s {
  sted_kind_t *kind;

  u8 *data;
  u64 len;
};

struct sted_slice_s {
  sted_kind_t *kind;

  u8 *data;
  u64 len;
  u64 cap;
};

struct sted_dict_s {
  u8 *data;
  u64 cap;
  u64 len;
  sted_kind_t *key_kind;
  sted_kind_t *val_kind;
  sted_kind_t internal_kind;
};

#define vec_union_2def(type)                                                   \
  union sted_v2_##type##_s {                                                   \
    struct {                                                                   \
      type x;                                                                  \
      type y;                                                                  \
    };                                                                         \
                                                                               \
    type type##s[2];                                                           \
  }

#define vec_union_3def(type)                                                   \
  union sted_v3_##type##_s {                                                   \
    struct {                                                                   \
      type x;                                                                  \
      type y;                                                                  \
      type z;                                                                  \
    };                                                                         \
                                                                               \
    type type##s[3];                                                           \
  }

#define vec_union_defs(type, ...)                                              \
  vec_union_2def(type);                                                        \
  vec_union_3def(type)

vec_eachtype(vec_union_defs);

#undef vec_union_defs
#undef vec_union_2def
#undef vec_union_3def

#undef vec_name
#undef vec_eachtype

#endif
