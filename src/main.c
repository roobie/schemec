/*

  main.c -- Template main()

  This program is free software you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include <libguile.h>

static SCM
env_hostname (void) {
  char *s = getenv("HOSTNAME");
  if (s == NULL) {
    return SCM_BOOL_F;
  }
  else {
    return scm_from_locale_string(s);
  }
}

typedef struct {
  char *name;
  void *data;
} component_t;

#define INITIAL_ENTITY_COUNT 1024
#define INITIAL_COMPONENT_COUNT 8
#define DEFAULT_POOL_SIZE 1024

int total_entity_count = 0;
int entities[INITIAL_ENTITY_COUNT] = { 0 };
int next_entity_id = 1;
component_t *ent2comp_map[INITIAL_ENTITY_COUNT][INITIAL_COMPONENT_COUNT] = { 0 };
component_t *comp_recycle_pool[DEFAULT_POOL_SIZE] = { 0 };

static bool
component_t_recycle(component_t *component) {
  if (component == NULL) {
    return false;
  }

  return true;
}

static int
_ecs_create_entity() {
  int entity_id = next_entity_id;
  next_entity_id += 1;
  total_entity_count += 1;
  component_t *ptr = malloc(INITIAL_COMPONENT_COUNT * sizeof(component_t));
  if (ptr == NULL) {
    return -1;
  }
  *ent2comp_map[entity_id] = ptr;

  return entity_id;
}

static SCM
ecs_create_entity() {
  return scm_from_int(_ecs_create_entity());
}

static bool
_ecs_destroy_entity(int id) {
  for (int i = 0; i < total_entity_count; ++i) {
    if (entities[i] == id) {
      entities[i] = 0;
      total_entity_count -= 1;
      component_t *entity_components = *ent2comp_map[id];
      int j = 0;
      do {
        ++j;
      } while(component_t_recycle(&entity_components[j]));
      return true;
    }
  }

  return false;
}

static SCM
ecs_destroy_entity(SCM scm_id) {
  int id = scm_to_int(scm_id);
  if (_ecs_destroy_entity(id)) {
    return SCM_BOOL_T;
  }

  return SCM_BOOL_F;
}

static void
init_ecs_module(void *data) {
  // scm_c_define_gsubr("env-hostname", 0, 0, 0, env_hostname);

  // arguments are: name, required-, optional-, rest-params, c_definition
  scm_c_define_gsubr("ecs/create-entity", 0, 0, 0, ecs_create_entity);
  scm_c_define_gsubr("ecs/destroy-entity", 1, 0, 0, ecs_destroy_entity);
  scm_c_export("ecs/create-entity",
               "ecs/destroy-entity",
               NULL);
}

static SCM
modify_main_module(void *data) {
  scm_c_use_module("omni ecs");
  return SCM_BOOL_T;
}

static SCM
modify_guile_user_module(void *data) {
  scm_c_use_module("omni ecs");
  scm_c_use_module("omni main");
  scm_c_eval_string("(omni/init)");
  return SCM_BOOL_T;
}

static void
inner_main(void *closure, int argc, char **argv) {
  // prep

  // SCM ecs_module =
  scm_c_define_module("omni ecs",
                      &init_ecs_module,
                      NULL);

  // LOAD the scheme code
  scm_c_primitive_load("./scheme/main.scm");

  // update defined modules with stuff that they want
  SCM main_module = scm_c_resolve_module("omni main");
  scm_c_call_with_current_module(main_module,
                                 &modify_main_module,
                                 NULL);

  // update the default module with good stuff
  SCM guile_user_module = scm_c_resolve_module("guile-user");
  scm_c_call_with_current_module(guile_user_module,
                                 &modify_guile_user_module,
                                 NULL);

  // Load nice readline module, and activate it, for more pleasure.
  scm_c_eval_string("(use-modules (ice-9 readline))");
  scm_c_eval_string("(activate-readline)");

  // start shell
  scm_shell(argc, argv);
  // shell exited
}

int
main(int argc, char **argv) {
  scm_boot_guile(argc, argv, inner_main, 0);
  return 0; // never reached, see inner_main
}
