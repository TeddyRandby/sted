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
i32 sted_fail(const char *file, const u32 line, const char *fmt);

#define sted_assert(expr, fmt) ((expr) ? 0 : sted_fail(__FILE__, __LINE__, fmt))

#define sted_cast(type, ptr) ((type *)ptr)

// Create an unsuccessful result
#define sted_err(code) ((sted_result_t){.status = code})

// Create a successful result with a value
#define sted_ok(value) ((sted_result_t){.status = STED_OK, .data = value})

// Crash if the result is not ok.
#define sted_try(result)                                                       \
  (sted_assert(result.status == STED_OK, "Status was not OK."))

// Crash if the result is not ok.
// Return the value, cast to the specified type.
#define sted_unwrap(type, result)                                              \
  ({                                                                           \
    sted_result_t __res__ = (result);                                          \
    assert(__res__.status == STED_OK);                                         \
    (type *)(__res__.data);                                                    \
  })

#define sted_size(self) (self->kind->item_size)

#define sted_alloc(kind, ptr, size)                                            \
  ((kind)->allocator((kind), ptr, size * (kind)->item_size))

#define sted_hash(kind, ptr) ((kind)->hasher((kind), ptr))

#define true 1
#define false 0

#define function extern inline

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

#define vec_eachtypesize(macro, ...)                                           \
  vec_eachtype(macro, 2, __VA_ARGS__);                                         \
  vec_eachtype(macro, 3, __VA_ARGS__);                                         \
  vec_eachtype(macro, 4, __VA_ARGS__)

#define vec_union_decl(type, size, args)                                       \
  typedef union sted_##v##size##_##type##_s vec_name(type, size)

#define vec_bin_decl(type, size, name)                                         \
  vec_name(type, size) sted_##v##size##_##type##_##name##v(                    \
      vec_name(type, size), vec_name(type, size))

#define vec_constant_decl(type, size, name)                                    \
  vec_name(type, size)                                                         \
      sted_##v##size##_##type##_##name(vec_name(type, size), type)

#define vec_dot_decl(type, size, args)                                         \
  type sted_##v##size##_##type##_##dot(vec_name(type, size),                   \
                                       vec_name(type, size))

vec_eachtypesize(vec_union_decl);

vec_eachtypesize(vec_bin_decl, add);
vec_eachtypesize(vec_bin_decl, mul);

vec_eachtypesize(vec_constant_decl, add);
vec_eachtypesize(vec_constant_decl, mul);

vec_eachtypesize(vec_dot_decl);

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

#define vec_union_4def(type)                                                   \
  union sted_v4_##type##_s {                                                   \
    struct {                                                                   \
      type w;                                                                  \
      type x;                                                                  \
      type y;                                                                  \
      type z;                                                                  \
    };                                                                         \
                                                                               \
    type type##s[4];                                                           \
  }

#define vec_union_defs(type, ...)                                              \
  vec_union_2def(type);                                                        \
  vec_union_3def(type);                                                        \
  vec_union_4def(type)

vec_eachtype(vec_union_defs);

#undef vec_union_defs
#undef vec_union_2def
#undef vec_union_3def
#undef vec_union_4def

#undef vec_name

#undef vec_eachtype
#undef vec_eachtypesize

#ifdef STED_IMPL

function i32 sted_fail(const char *file, const u32 line, const char *fmt) {
  fprintf(stderr, "Assertion failed in %s at line %u\nReason: %s\n", file, line,
          fmt);
  exit(1);
  return 1;
}

function sted_result_t sted_mem_default(const sted_kind_t *kind, void *ptr,
                                        u64 len) {

  if (len == 0) {
    free(ptr);
    return sted_ok(NULL);
  }

  void *result = realloc(ptr, kind->item_size * len);

  if (result == NULL)
    return sted_err(STED_MEMORY_ERR);

  return sted_ok(result);
}

function sted_result_t grow_slice(sted_slice_t *self) {

  self->cap *= 2;

  sted_result_t result = sted_alloc(self->kind, self->data, self->cap);

  self->data = sted_unwrap(u8, result);

  return sted_ok(NULL);
}

function sted_result_t grow_dict(sted_dict_t *self) {

  // Save a ref to the old array of data.
  u8 *old_data = self->data;
  u64 old_cap = self->cap;

  // Increase the capacity, reset the length, and reserve a new array.
  self->cap *= 2;
  self->len = 0;

  sted_result_t result = sted_alloc(&self->internal_kind, NULL, self->cap);

  self->data = sted_unwrap(u8, result);

  // For each kvp in the old array, insert it into the new dict.
  for (u64 i = 0; i < old_cap; i++) {
    u8 *bucket = old_data + i * self->internal_kind.item_size;
    void *key = bucket + 1;
    void *val = bucket + 1 + self->key_kind->item_size;

    if (bucket[0]) {
      sted_dict_set(self, key, val);
    }
  }

  sted_try(sted_alloc(&self->internal_kind, old_data, 0));

  return sted_ok(NULL);
}

/* ------------ SLICES ------------ */

function sted_result_t sted_slice_create(sted_kind_t *kind) {
  assert(kind->allocator != NULL);

  sted_slice_t *self =
      sted_unwrap(sted_slice_t, sted_alloc(kind, NULL, sizeof(sted_slice_t)));

  // If the allocation fails, the try will crash the program.

  self->kind = kind;
  self->len = 0;
  self->cap = 2;

  self->data =
      sted_unwrap(u8, self->kind->allocator(self->kind, NULL, self->cap));

  return sted_ok(self);
}

function sted_result_t sted_slice_destroy(sted_slice_t *self) {

  sted_alloc(self->kind, self->data, 0);
  sted_alloc(self->kind, self, 0);

  return sted_ok(NULL);
}

function sted_result_t sted_slice_emplace(sted_slice_t *self, void *data) {
  if (self->len == self->cap) {
    sted_try(grow_slice(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  memcpy(new_item, data, self->kind->item_size);

  return sted_ok(new_item);
}

function sted_result_t sted_slice_append(sted_slice_t *self) {
  if (self->len == self->cap) {
    sted_try(grow_slice(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  return sted_ok(new_item);
}

function sted_result_t sted_slice_pop(sted_slice_t *self) {
  if (self->len == 0) {
    return sted_err(STED_BOUNDS_ERR);
  }

  self->len--;
  return sted_ok(self->data + self->kind->item_size * self->len);
}

function sted_result_t sted_slice_get(sted_slice_t *self, u64 offset) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  return sted_ok(self->data + self->kind->item_size * offset);
}

function sted_result_t sted_slice_set(sted_slice_t *self, u64 offset,
                                      void *data) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  void *dest = memcpy(self->data + self->kind->item_size * offset, data,
                      self->kind->item_size);

  return sted_ok(dest);
}

function sted_result_t sted_slice_view(sted_slice_t *self, u64 offset,
                                       u64 len) {

  if (len + offset > self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  sted_view_t *view = sted_unwrap(
      sted_view_t, sted_alloc(self->kind, NULL, sizeof(sted_view_t)));

  view->kind = self->kind;
  view->data = self->data + self->kind->item_size * offset;
  view->len = len;

  return sted_ok(view);
}

/* ------------ VIEWS ------------ */

function sted_result_t sted_view_create(sted_kind_t *kind, void *data,
                                        u64 len) {
  assert(kind->allocator != NULL);

  sted_view_t *self =
      sted_unwrap(sted_view_t, sted_alloc(kind, NULL, sizeof(sted_view_t)));

  self->data = (u8 *)data;
  self->len = len;

  return sted_ok(self);
};

function sted_result_t sted_view_destroy(sted_view_t *self) {
  sted_try(self->kind->allocator(self->kind, self, 0));

  return sted_ok(NULL);
};

function sted_result_t sted_view_get(sted_view_t *self, u64 offset) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  return sted_ok(self->data + self->kind->item_size * offset);
};

/* ------------ DICTIONARIES------------ */
function sted_result_t sted_dict_create(sted_kind_t *val_kind,
                                        sted_kind_t *key_kind) {

  sted_kind_t internal_kind = {
      .item_size = val_kind->item_size + key_kind->item_size + 1,
      .allocator = key_kind->allocator,
      .user_name = "__internal_kind__",
  };

  sted_dict_t *self = sted_unwrap(
      sted_dict_t, sted_alloc(&internal_kind, NULL, sizeof(sted_dict_t)));

  self->internal_kind = internal_kind;
  self->key_kind = key_kind;
  self->val_kind = val_kind;
  self->len = 0;
  self->cap = 8;

  self->data =
      sted_unwrap(u8, sted_alloc(&self->internal_kind, NULL, self->cap));

  return sted_ok(self);
};

function boolean dict_has_key(sted_dict_t *self, u64 index) {
  return *(self->data + self->internal_kind.item_size * index);
}

function boolean sted_dict_has_key(sted_dict_t *self, void *key) {
  u64 index = sted_hash(self->key_kind, key) % self->cap;
  return dict_has_key(self, index);
};

function sted_result_t sted_dict_destroy(sted_dict_t *self) {
  self->internal_kind.allocator(&self->internal_kind, self->data, 0);
  free(self);
  return sted_ok(NULL);
}

function sted_result_t sted_dict_set(sted_dict_t *self, void *key, void *val) {

  if (self->len > self->cap * STED_DICT_LOAD) {
    grow_dict(self);
  }

  size_t index = sted_hash(self->key_kind, key) % self->cap;
  u8 *bucket = self->data + self->internal_kind.item_size * index;

  if (bucket[0]) {
    // Chain through until a key matches
    while (memcmp(key, bucket + 1, self->key_kind->item_size) != 0) {
      // If the key doesnt exist
      if (!bucket[0]) {
        // Insert a new kvp
        break;
      }
      // Check next bucket
      bucket = self->data + self->internal_kind.item_size * ++index;

      if (bucket >= self->data + self->internal_kind.item_size * self->cap) {
        return sted_err(STED_WRAP_ERR);
      }
    }
  }

  // insert new key and value.
  self->len++;

  // The bucket's key has been seen.
  bucket[0] = true;

  memcpy(bucket + 1, key, self->key_kind->item_size);

  memcpy(bucket + 1 + self->key_kind->item_size, val,
         self->val_kind->item_size);

  // Return a pointer to the set value.
  return sted_ok(bucket + 1 + self->key_kind->item_size);
}

function sted_result_t sted_dict_get(sted_dict_t *self, void *key) {
  size_t index = sted_hash(self->key_kind, key) % self->cap;
  u8 *bucket = self->data + self->internal_kind.item_size * index;

  if (!dict_has_key(self, index)) {
    // The key does not exist
    return sted_ok(NULL);
  }

  // memcmp the given key and the key we have to make sure its a match
  while (memcmp(bucket + 1, key, self->key_kind->item_size) != 0) {
    // return a pointer into the bucket at the value's position.
    if (bucket >= self->data + self->internal_kind.item_size * self->cap) {
      return sted_err(STED_WRAP_ERR);
    }

    // Chain the buckets
    bucket = self->data + self->internal_kind.item_size * ++index;
  }

  return sted_ok(bucket + self->key_kind->item_size + 1);
}

/* ------------ VECTORS ------------ */
#define vec_name(type, size) sted_v##size##_##type##_t

#define vec_eachtype(macro, ...)                                               \
  macro(i32, __VA_ARGS__);                                                     \
  macro(i64, __VA_ARGS__);                                                     \
  macro(u32, __VA_ARGS__);                                                     \
  macro(u64, __VA_ARGS__);                                                     \
  macro(f32, __VA_ARGS__);                                                     \
  macro(f64, __VA_ARGS__)

#define vec_eachtypesize(macro, ...)                                           \
  vec_eachtype(macro, 2, __VA_ARGS__);                                         \
  vec_eachtype(macro, 3, __VA_ARGS__);                                         \
  vec_eachtype(macro, 4, __VA_ARGS__)

#define vec_bin_def(type, name, op)                                            \
  function vec_name(type, 2) sted_##v2##_##type##_##name##v(                   \
      vec_name(type, 2) a, vec_name(type, 2) b) {                              \
    return (vec_name(type, 2)){.x = a.x op b.x, .y = a.y op b.y};              \
  }                                                                            \
  function vec_name(type, 3) sted_##v3##_##type##_##name##v(                   \
      vec_name(type, 3) a, vec_name(type, 3) b) {                              \
    return (vec_name(type, 3)){                                                \
        .x = a.x op b.x, .y = a.y op b.y, .z = a.z op b.z};                    \
  }                                                                            \
  function vec_name(type, 4) sted_##v4##_##type##_##name##v(                   \
      vec_name(type, 4) a, vec_name(type, 4) b) {                              \
    return (vec_name(type, 4)){                                                \
        .w = a.w op b.w, .x = a.x op b.x, .y = a.y op b.y, .z = a.z op b.z};   \
  }

#define vec_constant_def(type, name, op)                                       \
  function vec_name(type, 2)                                                   \
      sted_##v2##_##type##_##name(vec_name(type, 2) a, type b) {               \
    return (vec_name(type, 2)){.x = a.x op b, .y = a.y op b};                  \
  }                                                                            \
  function vec_name(type, 3)                                                   \
      sted_##v3##_##type##_##name(vec_name(type, 3) a, type b) {               \
    return (vec_name(type, 3)){.x = a.x op b, .y = a.y op b, .z = a.z op b};   \
  }                                                                            \
  function vec_name(type, 4)                                                   \
      sted_##v4##_##type##_##name(vec_name(type, 4) a, type b) {               \
    return (vec_name(type, 4)){                                                \
        .w = a.w op b, .x = a.x op b, .y = a.y op b, .z = a.z op b};           \
  }

#define vec_dot_def(type, size, args)                                          \
  function type sted_##v##size##_##type##_dot(vec_name(type, size) a,          \
                                              vec_name(type, size) b) {        \
    type result = 0;                                                           \
    for (i32 i = 0; i < size; i++)                                             \
      result += a.type##s[i] * b.type##s[i];                                   \
    return result;                                                             \
  }

vec_eachtype(vec_bin_def, add, +);
vec_eachtype(vec_bin_def, mul, *);

vec_eachtype(vec_constant_def, add, +);
vec_eachtype(vec_constant_def, mul, *);

vec_eachtypesize(vec_dot_def);

#undef vec_scale_def
#undef vec_translate_def
#undef vec_dot_def
#undef vec_bin_def

#undef vec_name
#undef vec_eachtype
#undef vec_eachtypesize

#endif

#endif
