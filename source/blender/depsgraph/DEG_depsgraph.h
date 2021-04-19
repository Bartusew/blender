/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2013 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup depsgraph
 *
 * Public API for Depsgraph
 *
 * Dependency Graph
 * ================
 *
 * The dependency graph tracks relations between various pieces of data in
 * a Blender file, but mainly just those which make up scene data. It is used
 * to determine the set of operations need to ensure that all data has been
 * correctly evaluated in response to changes, based on dependencies and visibility
 * of affected data.
 * Evaluation Engine
 * =================
 *
 * The evaluation takes the operation-nodes the Depsgraph has tagged for updating,
 * and schedules them up for being evaluated/executed such that the all dependency
 * relationship constraints are satisfied.
 */

/* ************************************************* */
/* Forward-defined typedefs for core types
 * - These are used in all depsgraph code and by all callers of Depsgraph API...
 */

#pragma once

#include "DNA_ID.h"

/* Dependency Graph */
typedef struct Depsgraph Depsgraph;

/* ------------------------------------------------ */

struct Main;

struct Scene;
struct ViewLayer;

typedef enum eEvaluationMode {
  DAG_EVAL_VIEWPORT = 0, /* evaluate for OpenGL viewport */
  DAG_EVAL_RENDER = 1,   /* evaluate for render purposes */
} eEvaluationMode;

/* DagNode->eval_flags */
enum {
  /* Regardless to curve->path animation flag path is to be evaluated anyway,
   * to meet dependencies with such a things as curve modifier and other guys
   * who're using curve deform, where_on_path and so. */
  DAG_EVAL_NEED_CURVE_PATH = (1 << 0),
  /* A shrinkwrap modifier or constraint targeting this mesh needs information
   * about non-manifold boundary edges for the Target Normal Project mode. */
  DAG_EVAL_NEED_SHRINKWRAP_BOUNDARY = (1 << 1),
};

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************ */
/* Depsgraph API */

/* CRUD ------------------------------------------- */

// Get main depsgraph instance from context!

/* Create new Depsgraph instance */
// TODO: what args are needed here? What's the building-graph entry point?
Depsgraph *DEG_graph_new(struct Main *bmain,
                         struct Scene *scene,
                         struct ViewLayer *view_layer,
                         eEvaluationMode mode);

void DEG_graph_replace_owners(struct Depsgraph *depsgraph,
                              struct Main *bmain,
                              struct Scene *scene,
                              struct ViewLayer *view_layer);

/* Free Depsgraph itself and all its data */
void DEG_graph_free(Depsgraph *graph);

/* Node Types Registry ---------------------------- */

/* Register all node types */
void DEG_register_node_types(void);

/* Free node type registry on exit */
void DEG_free_node_types(void);

/* Update Tagging -------------------------------- */

/* Update dependency graph when visible scenes/layers changes. */
void DEG_graph_on_visible_update(struct Main *bmain, Depsgraph *depsgraph, const bool do_time);

/* Update all dependency graphs when visible scenes/layers changes. */
void DEG_on_visible_update(struct Main *bmain, const bool do_time);

/* NOTE: Will return NULL if the flag is not known, allowing to gracefully handle situations
 * when recalc flag has been removed. */
const char *DEG_update_tag_as_string(IDRecalcFlag flag);

void DEG_id_tag_update(struct ID *id, int flag);
void DEG_id_tag_update_ex(struct Main *bmain, struct ID *id, int flag);

void DEG_graph_id_tag_update(struct Main *bmain,
                             struct Depsgraph *depsgraph,
                             struct ID *id,
                             int flag);

/* Tag all dependency graphs when time has changed. */
void DEG_time_tag_update(struct Main *bmain);

/* Tag a dependency graph when time has changed. */
void DEG_graph_time_tag_update(struct Depsgraph *depsgraph);

/* Mark a particular datablock type as having changing. This does
 * not cause any updates but is used by external render engines to detect if for
 * example a datablock was removed. */
void DEG_graph_id_type_tag(struct Depsgraph *depsgraph, short id_type);
void DEG_id_type_tag(struct Main *bmain, short id_type);

/* Set a depsgraph to flush updates to editors. This would be done
 * for viewport depsgraphs, but not render or export depsgraph for example. */
void DEG_enable_editors_update(struct Depsgraph *depsgraph);

/* Check if something was changed in the database and inform editors about this,
 * then clear recalc flags. */
void DEG_editors_update(struct Depsgraph *depsgraph, bool time);

/* Clear recalc flags after editors or renderers have handled updates. */
void DEG_ids_clear_recalc(Depsgraph *depsgraph, const bool backup);

/* Restore recalc flags, backed up by a previous call to DEG_ids_clear_recalc.
 * This also clears the backup. */
void DEG_ids_restore_recalc(Depsgraph *depsgraph);

/* ************************************************ */
/* Evaluation Engine API */

/* Graph Evaluation  ----------------------------- */

/* Frame changed recalculation entry point. */
void DEG_evaluate_on_framechange(Depsgraph *graph, float ctime);

/* Data changed recalculation entry point. */
void DEG_evaluate_on_refresh(Depsgraph *graph);

/* Editors Integration  -------------------------- */

/* Mechanism to allow editors to be informed of depsgraph updates,
 * to do their own updates based on changes.
 */

typedef struct DEGEditorUpdateContext {
  struct Main *bmain;
  struct Depsgraph *depsgraph;
  struct Scene *scene;
  struct ViewLayer *view_layer;
} DEGEditorUpdateContext;

typedef void (*DEG_EditorUpdateIDCb)(const DEGEditorUpdateContext *update_ctx, struct ID *id);
typedef void (*DEG_EditorUpdateSceneCb)(const DEGEditorUpdateContext *update_ctx,
                                        const bool updated);

/* Set callbacks which are being called when depsgraph changes. */
void DEG_editors_set_update_cb(DEG_EditorUpdateIDCb id_func, DEG_EditorUpdateSceneCb scene_func);

/* Evaluation  ----------------------------------- */

bool DEG_is_evaluating(const struct Depsgraph *depsgraph);

bool DEG_is_active(const struct Depsgraph *depsgraph);
void DEG_make_active(struct Depsgraph *depsgraph);
void DEG_make_inactive(struct Depsgraph *depsgraph);

/* Evaluation Debug ------------------------------ */

void DEG_debug_print_begin(struct Depsgraph *depsgraph);

void DEG_debug_print_eval(struct Depsgraph *depsgraph,
                          const char *function_name,
                          const char *object_name,
                          const void *object_address);

void DEG_debug_print_eval_subdata(struct Depsgraph *depsgraph,
                                  const char *function_name,
                                  const char *object_name,
                                  const void *object_address,
                                  const char *subdata_comment,
                                  const char *subdata_name,
                                  const void *subdata_address);

void DEG_debug_print_eval_subdata_index(struct Depsgraph *depsgraph,
                                        const char *function_name,
                                        const char *object_name,
                                        const void *object_address,
                                        const char *subdata_comment,
                                        const char *subdata_name,
                                        const void *subdata_address,
                                        const int subdata_index);

void DEG_debug_print_eval_parent_typed(struct Depsgraph *depsgraph,
                                       const char *function_name,
                                       const char *object_name,
                                       const void *object_address,
                                       const char *parent_comment,
                                       const char *parent_name,
                                       const void *parent_address);

void DEG_debug_print_eval_time(struct Depsgraph *depsgraph,
                               const char *function_name,
                               const char *object_name,
                               const void *object_address,
                               float time);

#ifdef __cplusplus
} /* extern "C" */
#endif
