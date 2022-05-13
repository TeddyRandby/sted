#define STED_IMPL

#include "src/sted.h"

u64 hash_int(const sted_kind_t *k, void *i) { return *sted_cast(i32, i); }

i32 main() {
  sted_kind_t int_kind = {
      .item_size = sizeof(i32),
      .allocator = sted_mem_default,
      .hasher = hash_int,
  };

  sted_slice_t *ints = sted_unwrap(sted_slice_t, sted_slice_create(&int_kind));

  i32 a = 69;
  i32 b = -420;

  for (i32 i = 0; i < 6; i++) {
    sted_try(sted_slice_emplace(ints, &i));
  }

  printf("[");
  sted_slice_each_as(ints, i, printf(" %i", *sted_cast(i32, i)));
  printf(" ]\n");

  sted_try(sted_slice_emplace(ints, &b));

  printf("[");
  sted_slice_each_as(ints, i, printf(" %i", *sted_cast(i32, i)));
  printf(" ]\n");

  i32 *c = sted_unwrap(i32, sted_slice_append(ints));

  *c = 6969;

  printf("[");
  sted_slice_each_as(ints, i, printf(" %i", *sted_cast(i32, i)));
  printf(" ]\n");

  sted_view_t *v = sted_unwrap(sted_view_t, sted_slice_view(ints, 2, 4));

  printf("[");
  sted_view_each_as(v, i, printf(" %i", *sted_cast(i32, i)));
  printf(" ]\n");

  sted_view_destroy(v);

  sted_slice_destroy(ints);

  sted_dict_t *int_dict =
      sted_unwrap(sted_dict_t, sted_dict_create(&int_kind, &int_kind));

  for (i32 i = 0; i < 16; i++) {
    sted_try(sted_dict_set(int_dict, &i, &i));

    c = sted_unwrap(i32, sted_dict_get(int_dict, &i));
    printf("got: %i\n", *c);
  }

  sted_dict_each_as(
      int_dict, k, v,
      printf("k(%i), v(%i)\n", *sted_cast(i32, k), *sted_cast(i32, v)));

  sted_dict_destroy(int_dict);

  sted_v2_f64_t fs = {.f64s = {69, .420}};

  fs = sted_v2_f64_addv(fs, fs);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  fs = sted_v2_f64_add(fs, 69);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  fs = sted_v2_f64_mul(fs, 0.1);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  printf("%lf\n", sted_v2_f64_dot(fs, fs));
}
