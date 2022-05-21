#define STED_IMPL
#include "src/sted.h"

u64 hash_int(const kind_t *k, void *i) { return *cast(i32, i); }
u64 hash_char(const kind_t *k, void *i) { return *cast(char, i); }

i32 main() {
  kind_t int_kind = {
      .item_size = sizeof(i32),
      .allocator = mem_default,
      .hasher = hash_int,
  };

  kind_t char_kind = {
      .item_size = sizeof(char),
      .allocator = mem_default,
      .hasher = hash_char,
  };

  array_t *ints = unwrap(array_t, array_create(&int_kind));

  i32 a = 69;
  i32 b = -420;

  for (i32 i = 0; i < 6; i++) {
    try(array_emplace(ints, &i));
  }

  printf("[");
  array_each_as(ints, i, printf(" %i", *cast(i32, i)));
  printf(" ]\n");

  try(array_emplace(ints, &b));

  printf("[");
  array_each_as(ints, i, printf(" %i", *cast(i32, i)));
  printf(" ]\n");

  i32 *c = unwrap(i32, array_append(ints));

  *c = 6969;

  printf("[");
  array_each_as(ints, i, printf(" %i", *cast(i32, i)));
  printf(" ]\n");

  view_t *v = unwrap(view_t, array_view(ints, 2, 4));

  printf("[");
  view_each_as(v, i, printf(" %i", *cast(i32, i)));
  printf(" ]\n");

  view_destroy(v);

  array_destroy(ints);

  dict_t *int_dict = unwrap(dict_t, dict_create(&int_kind, &int_kind));

  for (i32 i = 0; i < 16; i++) {
    try(dict_set(int_dict, &i, &i));

    c = unwrap(i32, dict_get(int_dict, &i));
    printf("got: %i\n", *c);
  }

  dict_each_as(int_dict, k, v,
               printf("k(%i), v(%i)\n", *cast(i32, k), *cast(i32, v)));

  dict_destroy(int_dict);

  v2_f64_t fs = {.f64s = {69, .420}};

  fs = v2_f64_addv(fs, fs);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  fs = v2_f64_add(fs, 69);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  fs = v2_f64_mul(fs, 0.1);

  printf("{ %lf, %lf }\n", fs.x, fs.y);

  printf("%lf\n", v2_f64_dot(fs, fs));

  char *path_name = "../README.md";

  view_t *path =
      unwrap(view_t, view_create(&char_kind, path_name, strlen(path_name)));

  printf("%lu\n", path->kind->item_size);

  array_t *readme = unwrap(array_t, io_readfile(path));

  array_each_as(readme, i, printf("%c", *cast(char, i)));
}
