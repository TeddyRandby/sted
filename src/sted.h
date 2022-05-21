#ifndef STED_H
#define STED_H
/*
  This is a base layer of useful things for all of my c projects.

  Basic included utilities:
    - Buffers
    - Vectors
    - arrays
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

//////////////////////////////
//                          //
//          HELPERS         //
//                          //
//////////////////////////////

#define cast(type, ptr) ((type *)ptr)

#define true 1

#define false 0

#define function extern inline

#define loop while (true)

/* ------------ RUNTIME ASSERTION ------------ */
i32 fail(const char *file, const u32 line, const char *fmt);

#define check(expr, fmt) ((expr) ? 0 : fail(__FILE__, __LINE__, fmt))

//////////////////////////////
//                          //
//          RESULTS         //
//                          //
//////////////////////////////

// The struct result type.
typedef struct result_s result_t;

// Create an unsuccessful result
#define err(code) ((result_t){.status = code})

// Create a successful result with a value
#define ok(value) ((result_t){.status = OK, .data = value})

// Crash if the result is not ok.
#define try(result) (check(result.status == OK, "Status was not OK."))

// Crash if the result is not ok.
// Return the value, cast to the specified type.
#define unwrap(type, result)                                                   \
  ({                                                                           \
    result_t __res__ = (result);                                               \
    check(__res__.status == OK, "Status was not OK.");                         \
    (type *)(__res__.data);                                                    \
  })

// Possible error codes that can be returned by sted functions.
typedef enum code_e {
  OK = 0,
  MEMORY_ERR,
  BOUNDS_ERR,
  CAST_ERR,
  WRAP_ERR,
} code_k;

//////////////////////////////
//                          //
//      DATA STRUCTURES     //
//                          //
//////////////////////////////

/* ------------ KINDS ------------ */
typedef struct kind_s kind_t;

typedef result_t (*mem_fn)(const kind_t *, void *, u64);

result_t mem_default(const kind_t *kind, void *, u64 len);

typedef u64 (*hash_fn)(const kind_t *, void *);

#define size(self) (self->kind->item_size)

#define alloc(kind, ptr, size)                                                 \
  ((kind)->allocator((kind), ptr, size * (kind)->item_size))

#define copy(kind, dest, src, size)                                            \
  (memcpy(dest, src, size * (kind)->item_size))

#define hash(kind, ptr) ((kind)->hasher((kind), ptr))

/* ------------ VECTORS ------------ */
// Macro for concatenating the vector type names easier.
#define vec_name(type, size) v##size##_##type##_t

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
  typedef union v##size##_##type##_s vec_name(type, size)

#define vec_bin_decl(type, size, name)                                         \
  vec_name(type, size)                                                         \
      v##size##_##type##_##name##v(vec_name(type, size), vec_name(type, size))

#define vec_constant_decl(type, size, name)                                    \
  vec_name(type, size) v##size##_##type##_##name(vec_name(type, size), type)

#define vec_dot_decl(type, size, args)                                         \
  type v##size##_##type##_##dot(vec_name(type, size), vec_name(type, size))

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

/* ------------ ARRAYS ------------ */
typedef struct array_s array_t;

result_t array_create(kind_t *);

result_t array_destroy(array_t *);

result_t array_emplace(array_t *, void *);

result_t array_append(array_t *);

result_t array_pop(array_t *);

result_t array_set(array_t *, u64, void *);

result_t array_get(array_t *, u64);

result_t array_view(array_t *, u64, u64);

#define array_each_as(s, var, body)                                            \
  for (u64 __i = 0; __i < s->len; __i++) {                                     \
    void *var = s->data + __i * size(s);                                       \
    body;                                                                      \
  }
/* ------------ SLICES ------------ */

typedef struct slice_s slice_t;

result_t slice_create(kind_t *, const u8 *, u64);

result_t slice_destroy(slice_t *);

result_t slice_get(slice_t *, u64);

/* ------------ VIEWS ------------ */
typedef struct view_s view_t;

result_t view_create(kind_t *, void *, u64);

result_t view_destroy(view_t *);

result_t view_get(view_t *, u64);

#define view_each_as(view, var, body)                                          \
  for (u64 __i = 0; __i < view->len; __i++) {                                  \
    void *var = view->data + __i * size(view);                                 \
    body;                                                                      \
  }

/* ------------ DICTIONARIES------------ */
typedef struct dict_s dict_t;

result_t dict_create(kind_t *, kind_t *);

result_t dict_destroy(dict_t *);

boolean dict_has_key(dict_t *, void *);

result_t dict_set(dict_t *, void *, void *);

result_t dict_get(dict_t *, void *);

#define DICT_LOAD 0.7

#define dict_each_as(d, key, val, body)                                        \
  for (u64 __i = 0; __i < d->cap; __i++) {                                     \
    u8 *__bucket = d->data + __i * d->internal_kind.item_size;                 \
    void *key = __bucket + 1;                                                  \
    void *val = __bucket + 1 + d->key_kind->item_size;                         \
    if (__bucket[0])                                                           \
      body;                                                                    \
  }

//////////////////////////////
//                          //
//        FILE I/O          //
//                          //
//////////////////////////////

result_t io_readfile(view_t *);

//////////////////////////////
//                          //
//      IMPLEMENTATION      //
//                          //
//////////////////////////////

#define STED_IMPL
#ifdef STED_IMPL

/* ------------ STRUCT DEFINITIONS ------------ */
struct kind_s {
  u64 item_size;
  mem_fn allocator;
  hash_fn hasher;

  void *user_data;
  const char *user_name;
};

struct result_s {
  code_k status;
  void *data;
};

struct slice_s {
  kind_t *kind;

  u64 len;
  u8 data[];
};

struct view_s {
  kind_t *kind;

  u8 *data;
  u64 len;
};

struct array_s {
  kind_t *kind;

  u8 *data;
  u64 len;
  u64 cap;
};

struct dict_s {
  u8 *data;
  u64 cap;
  u64 len;
  kind_t *key_kind;
  kind_t *val_kind;
  kind_t internal_kind;
};

#define vec_union_2def(type)                                                   \
  union v2_##type##_s {                                                        \
    struct {                                                                   \
      type x;                                                                  \
      type y;                                                                  \
    };                                                                         \
                                                                               \
    type type##s[2];                                                           \
  }

#define vec_union_3def(type)                                                   \
  union v3_##type##_s {                                                        \
    struct {                                                                   \
      type x;                                                                  \
      type y;                                                                  \
      type z;                                                                  \
    };                                                                         \
                                                                               \
    type type##s[3];                                                           \
  }

#define vec_union_4def(type)                                                   \
  union v4_##type##_s {                                                        \
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

function i32 fail(const char *file, const u32 line, const char *fmt) {
  fprintf(stderr, "Panic in %s at line %u\nReason: %s\n", file, line, fmt);
  exit(1);
  return 1;
}

function result_t mem_default(const kind_t *kind, void *ptr, u64 len) {

  if (len == 0) {
    free(ptr);
    return ok(NULL);
  }

  void *result = realloc(ptr, kind->item_size * len);

  if (result == NULL)
    return err(MEMORY_ERR);

  return ok(result);
}

function result_t grow_array(array_t *self) {

  self->cap *= 2;

  result_t result = alloc(self->kind, self->data, self->cap);

  self->data = unwrap(u8, result);

  return ok(NULL);
}

function result_t grow_dict(dict_t *self) {

  // Save a ref to the old array of data.
  u8 *old_data = self->data;
  u64 old_cap = self->cap;

  // Increase the capacity, reset the length, and reserve a new array.
  self->cap *= 2;
  self->len = 0;

  result_t result = alloc(&self->internal_kind, NULL, self->cap);

  self->data = unwrap(u8, result);

  // For each kvp in the old array, insert it into the new dict.
  for (u64 i = 0; i < old_cap; i++) {
    u8 *bucket = old_data + i * self->internal_kind.item_size;
    void *key = bucket + 1;
    void *val = bucket + 1 + self->key_kind->item_size;

    if (bucket[0]) {
      dict_set(self, key, val);
    }
  }

  try(alloc(&self->internal_kind, old_data, 0));

  return ok(NULL);
}

/* ------------ arrayS ------------ */

function result_t array_create(kind_t *kind) {
  assert(kind->allocator != NULL);

  array_t *self = unwrap(array_t, alloc(kind, NULL, sizeof(array_t)));

  // If the allocation fails, the try will crash the program.

  self->kind = kind;
  self->len = 0;
  self->cap = 2;

  self->data = unwrap(u8, self->kind->allocator(self->kind, NULL, self->cap));

  return ok(self);
}

function result_t array_destroy(array_t *self) {

  alloc(self->kind, self->data, 0);
  alloc(self->kind, self, 0);

  return ok(NULL);
}

function result_t array_emplace(array_t *self, void *data) {
  if (self->len == self->cap) {
    try(grow_array(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  memcpy(new_item, data, self->kind->item_size);

  return ok(new_item);
}

function result_t array_append(array_t *self) {
  if (self->len == self->cap) {
    try(grow_array(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  return ok(new_item);
}

function result_t array_pop(array_t *self) {
  if (self->len == 0) {
    return err(BOUNDS_ERR);
  }

  self->len--;
  return ok(self->data + self->kind->item_size * self->len);
}

function result_t array_get(array_t *self, u64 offset) {
  if (offset >= self->len) {
    return err(BOUNDS_ERR);
  }

  return ok(self->data + self->kind->item_size * offset);
}

function result_t array_set(array_t *self, u64 offset, void *data) {
  if (offset >= self->len) {
    return err(BOUNDS_ERR);
  }

  void *dest = memcpy(self->data + self->kind->item_size * offset, data,
                      self->kind->item_size);

  return ok(dest);
}

function result_t array_view(array_t *self, u64 offset, u64 len) {

  if (len + offset > self->len) {
    return err(BOUNDS_ERR);
  }

  view_t *view = unwrap(view_t, alloc(self->kind, NULL, sizeof(view_t)));

  view->kind = self->kind;
  view->data = self->data + self->kind->item_size * offset;
  view->len = len;

  return ok(view);
}

/* ------------ VIEWS ------------ */

function result_t view_create(kind_t *kind, void *data, u64 len) {
  assert(kind->allocator != NULL);

  view_t *self = unwrap(view_t, alloc(kind, NULL, sizeof(view_t)));

  self->kind = kind;
  self->data = (u8 *)data;
  self->len = len;

  return ok(self);
};

function result_t view_destroy(view_t *self) {
  try(self->kind->allocator(self->kind, self, 0));

  return ok(NULL);
};

function result_t view_get(view_t *self, u64 offset) {
  if (offset >= self->len) {
    return err(BOUNDS_ERR);
  }

  return ok(self->data + self->kind->item_size * offset);
};

/* ------------ SLICES ------------ */

function result_t slice_create(kind_t *kind, const u8 *data, u64 len) {

  result_t res =
      kind->allocator(kind, NULL, sizeof(array_t) + (len * kind->item_size));

  array_t *self = unwrap(array_t, res);

  self->kind = kind;
  self->len = len;

  copy(kind, self->data, data, len);

  return ok(self);
}

function result_t slice_destroy(slice_t *self) {
  try(alloc(self->kind, self, 0));

  return ok(NULL);
}

function result_t slice_get(slice_t *self, u64 offset) {
  if (offset >= self->len) {
    return err(BOUNDS_ERR);
  }

  return ok(self->data + self->kind->item_size * offset);
}

/* ------------ DICTIONARIES------------ */
function result_t dict_create(kind_t *val_kind, kind_t *key_kind) {

  kind_t internal_kind = {
      .item_size = val_kind->item_size + key_kind->item_size + 1,
      .allocator = key_kind->allocator,
      .user_name = "__internal_kind__",
  };

  dict_t *self = unwrap(dict_t, alloc(&internal_kind, NULL, sizeof(dict_t)));

  self->internal_kind = internal_kind;
  self->key_kind = key_kind;
  self->val_kind = val_kind;
  self->len = 0;
  self->cap = 8;

  self->data = unwrap(u8, alloc(&self->internal_kind, NULL, self->cap));

  return ok(self);
};

function boolean dict_has_key(dict_t *self, void *key) {
  u64 index = hash(self->key_kind, key) % self->cap;
  return *(self->data + self->internal_kind.item_size * index);
};

function result_t dict_destroy(dict_t *self) {
  self->internal_kind.allocator(&self->internal_kind, self->data, 0);
  free(self);
  return ok(NULL);
}

function result_t dict_set(dict_t *self, void *key, void *val) {

  if (self->len > self->cap * DICT_LOAD) {
    grow_dict(self);
  }

  size_t index = hash(self->key_kind, key) % self->cap;
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
        return err(WRAP_ERR);
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
  return ok(bucket + 1 + self->key_kind->item_size);
}

function result_t dict_get(dict_t *self, void *key) {
  size_t index = hash(self->key_kind, key) % self->cap;
  u8 *bucket = self->data + self->internal_kind.item_size * index;

  boolean has_key = *(self->data + self->internal_kind.item_size * index);

  if (!has_key) {
    // The key does not exist
    return ok(NULL);
  }

  // memcmp the given key and the key we have to make sure its a match
  while (memcmp(bucket + 1, key, self->key_kind->item_size) != 0) {
    // return a pointer into the bucket at the value's position.
    if (bucket >= self->data + self->internal_kind.item_size * self->cap) {
      return err(WRAP_ERR);
    }

    // Chain the buckets
    bucket = self->data + self->internal_kind.item_size * ++index;
  }

  return ok(bucket + self->key_kind->item_size + 1);
}

/* ------------ VECTORS ------------ */
#define vec_name(type, size) v##size##_##type##_t

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
  function vec_name(type, 2)                                                   \
      v2##_##type##_##name##v(vec_name(type, 2) a, vec_name(type, 2) b) {      \
    return (vec_name(type, 2)){.x = a.x op b.x, .y = a.y op b.y};              \
  }                                                                            \
  function vec_name(type, 3)                                                   \
      v3##_##type##_##name##v(vec_name(type, 3) a, vec_name(type, 3) b) {      \
    return (vec_name(type, 3)){                                                \
        .x = a.x op b.x, .y = a.y op b.y, .z = a.z op b.z};                    \
  }                                                                            \
  function vec_name(type, 4)                                                   \
      v4##_##type##_##name##v(vec_name(type, 4) a, vec_name(type, 4) b) {      \
    return (vec_name(type, 4)){                                                \
        .w = a.w op b.w, .x = a.x op b.x, .y = a.y op b.y, .z = a.z op b.z};   \
  }

#define vec_constant_def(type, name, op)                                       \
  function vec_name(type, 2)                                                   \
      v2##_##type##_##name(vec_name(type, 2) a, type b) {                      \
    return (vec_name(type, 2)){.x = a.x op b, .y = a.y op b};                  \
  }                                                                            \
  function vec_name(type, 3)                                                   \
      v3##_##type##_##name(vec_name(type, 3) a, type b) {                      \
    return (vec_name(type, 3)){.x = a.x op b, .y = a.y op b, .z = a.z op b};   \
  }                                                                            \
  function vec_name(type, 4)                                                   \
      v4##_##type##_##name(vec_name(type, 4) a, type b) {                      \
    return (vec_name(type, 4)){                                                \
        .w = a.w op b, .x = a.x op b, .y = a.y op b, .z = a.z op b};           \
  }

#define vec_dot_def(type, size, args)                                          \
  function type v##size##_##type##_dot(vec_name(type, size) a,                 \
                                       vec_name(type, size) b) {               \
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

/* ------------ FILE IO ------------ */

function result_t io_readfile(view_t *path) {
  check(path->kind->item_size == 1, "Path should have a kind with size 1");

  FILE *file = fopen(cast(char, path->data), "r");

  check(file != NULL, "File could not be opened");

  char ch;

  array_t *result = unwrap(array_t, array_create(path->kind));

  loop {
    ch = fgetc(file);

    array_emplace(result, &ch);

    if (ch == EOF)
      break;
  }

  fclose(file);

  return ok(result);
}

#endif

#endif
