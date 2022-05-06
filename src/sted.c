#include "sted.h"
#include <string.h>

sted_result_t sted_mem_default(const sted_kind_t *kind, void *ptr, u64 len) {

  if (len == 0) {
    free(ptr);
    return sted_ok(NULL);
  }

  void *result = realloc(ptr, kind->item_size * len);

  if (result == NULL)
    return sted_err(STED_MEMORY_ERR);

  return sted_ok(result);
}

static inline sted_result_t grow_slice(sted_slice_t *self) {

  self->cap *= 2;

  sted_result_t result = sted_mem(self->kind, self->data, self->cap);

  self->data = sted_unwrap(u8, result);

  return sted_ok(NULL);
}

static inline sted_result_t grow_dict(sted_dict_t *self) {

  // Save a ref to the old array of data.
  u8 *old_data = self->data;
  u64 old_cap = self->cap;

  // Increase the capacity, reset the length, and reserve a new array.
  self->cap *= 2;
  self->len = 0;

  sted_result_t result = sted_mem(&self->internal_kind, NULL, self->cap);

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

  sted_try(sted_mem(&self->internal_kind, old_data, 0));

  return sted_ok(NULL);
}

/* ------------ SLICES ------------ */

inline sted_result_t sted_slice_create(sted_kind_t *kind) {
  assert(kind->allocator != NULL);

  sted_slice_t *self =
      sted_unwrap(sted_slice_t, sted_mem(kind, NULL, sizeof(sted_slice_t)));

  // If the allocation fails, the try will crash the program.

  self->kind = kind;
  self->len = 0;
  self->cap = 2;

  self->data =
      sted_unwrap(u8, self->kind->allocator(self->kind, NULL, self->cap));

  return sted_ok(self);
}

inline sted_result_t sted_slice_destroy(sted_slice_t *self) {

  sted_mem(self->kind, self->data, 0);
  sted_mem(self->kind, self, 0);

  return sted_ok(NULL);
}

inline sted_result_t sted_slice_emplace(sted_slice_t *self, void *data) {
  if (self->len == self->cap) {
    sted_try(grow_slice(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  memcpy(new_item, data, self->kind->item_size);

  return sted_ok(new_item);
}

inline sted_result_t sted_slice_append(sted_slice_t *self) {
  if (self->len == self->cap) {
    sted_try(grow_slice(self));
  }

  u8 *new_item = self->data + self->kind->item_size * self->len++;

  return sted_ok(new_item);
}

inline sted_result_t sted_slice_pop(sted_slice_t *self) {
  if (self->len == 0) {
    return sted_err(STED_BOUNDS_ERR);
  }

  self->len--;
  return sted_ok(self->data + self->kind->item_size * self->len);
}

inline sted_result_t sted_slice_get(sted_slice_t *self, u64 offset) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  return sted_ok(self->data + self->kind->item_size * offset);
}

inline sted_result_t sted_slice_set(sted_slice_t *self, u64 offset,
                                    void *data) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  void *dest = memcpy(self->data + self->kind->item_size * offset, data,
                      self->kind->item_size);

  return sted_ok(dest);
}

inline sted_result_t sted_slice_view(sted_slice_t *self, u64 offset, u64 len) {

  if (len + offset > self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  sted_view_t *view =
      sted_unwrap(sted_view_t, sted_mem(self->kind, NULL, sizeof(sted_view_t)));

  view->kind = self->kind;
  view->data = self->data + self->kind->item_size * offset;
  view->len = len;

  return sted_ok(view);
}

/* ------------ VIEWS ------------ */

sted_result_t sted_view_create(sted_kind_t *kind, void *data, u64 len) {
  assert(kind->allocator != NULL);

  sted_view_t *self =
      sted_unwrap(sted_view_t, sted_mem(kind, NULL, sizeof(sted_view_t)));

  self->data = data;
  self->len = len;

  return sted_ok(self);
};

sted_result_t sted_view_destroy(sted_view_t *self) {
  sted_try(self->kind->allocator(self->kind, self, 0));

  return sted_ok(NULL);
};

sted_result_t sted_view_get(sted_view_t *self, u64 offset) {
  if (offset >= self->len) {
    return sted_err(STED_BOUNDS_ERR);
  }

  return sted_ok(self->data + self->kind->item_size * offset);
};

/* ------------ DICTIONARIES------------ */
sted_result_t sted_dict_create(sted_kind_t *val_kind, sted_kind_t *key_kind) {

  sted_kind_t internal_kind = {
      .allocator = key_kind->allocator,
      .item_size = val_kind->item_size + key_kind->item_size + 1,
      .user_name = "__internal_kind__",
  };

  sted_dict_t *self = sted_unwrap(
      sted_dict_t, sted_mem(&internal_kind, NULL, sizeof(sted_dict_t)));

  self->internal_kind = internal_kind;
  self->key_kind = key_kind;
  self->val_kind = val_kind;
  self->len = 0;
  self->cap = 8;

  self->data = sted_unwrap(u8, sted_mem(&self->internal_kind, NULL, self->cap));

  return sted_ok(self);
};

static inline boolean dict_has_key(sted_dict_t *self, u64 index) {
  return *(self->data + self->internal_kind.item_size * index);
}

boolean sted_dict_has_key(sted_dict_t *self, void *key) {
  u64 index = sted_hash(self->key_kind, key) % self->cap;
  return dict_has_key(self, index);
};

sted_result_t sted_dict_destroy(sted_dict_t *self) {
  self->internal_kind.allocator(&self->internal_kind, self->data, 0);
  free(self);
  return sted_ok(NULL);
}

sted_result_t sted_dict_set(sted_dict_t *self, void *key, void *val) {

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

sted_result_t sted_dict_get(sted_dict_t *self, void *key) {
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

#define vec_bin_def(type, name, op)                                            \
  vec_name(type, 2)                                                            \
      sted_##v2##_##type##_##name(vec_name(type, 2) a, vec_name(type, 2) b) {  \
    return (vec_name(type, 2)){.x = a.x op b.x, .y = a.y op b.y};              \
  }                                                                            \
  vec_name(type, 3)                                                            \
      sted_##v3##_##type##_##name(vec_name(type, 3) a, vec_name(type, 3) b) {  \
    return (vec_name(type, 3)){                                                \
        .x = a.x op b.x, .y = a.y op b.y, .z = a.z op b.z};                    \
  }

#define vec_scale_def(type, size)                                              \
  vec_name(type, size)                                                         \
      sted_##v##size##_##type##_scale(vec_name(type, size) a, type b) {        \
    return (vec_name(type, size)){.x = a.x * b, .y = a.y * b};                 \
  }

#define vec_translate_def(type, size)                                          \
  vec_name(type, size)                                                         \
      sted_##v##size##_##type##_translate(vec_name(type, size) a, type b) {    \
    return (vec_name(type, size)){.x = a.x + b, .y = a.y + b};                 \
  }

#define vec_dot_def(type, size)                                                \
  type sted_##v##size##_##type##_dot(vec_name(type, size) a,                   \
                                     vec_name(type, size) b) {                 \
    type result = 0;                                                           \
    for (i32 i = 0; i < size; i++)                                             \
      result += a.type##s[i] * b.type##s[i];                                   \
    return result;                                                             \
  }

vec_eachtype(vec_bin_def, add, +);
vec_eachtype(vec_bin_def, sub, -);
vec_eachtype(vec_bin_def, mul, *);
vec_eachtype(vec_bin_def, div, /);

vec_eachtype(vec_scale_def, 2);
vec_eachtype(vec_scale_def, 3);

vec_eachtype(vec_translate_def, 2);
vec_eachtype(vec_translate_def, 3);

vec_eachtype(vec_dot_def, 2);
vec_eachtype(vec_dot_def, 3);

#undef vec_scale_def
#undef vec_translate_def
#undef vec_dot_def
#undef vec_bin_def

#undef vec_name
#undef vec_eachtype
