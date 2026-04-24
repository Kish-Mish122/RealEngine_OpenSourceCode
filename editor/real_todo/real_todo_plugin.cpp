/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_todo_plugin.h"
#include "real_todo_dock.h"
#include "core/string/print_string.h"

void RealTodoPlugin::_bind_methods() {
}

RealTodoPlugin::RealTodoPlugin() {
}

RealTodoPlugin::~RealTodoPlugin() {
}

void RealTodoPlugin::_enter_tree() {
    RealTodoDock *todo_dock = memnew(RealTodoDock);
    add_control_to_dock(EditorPlugin::DOCK_SLOT_RIGHT_BR, todo_dock);
}

void RealTodoPlugin::_exit_tree() {
}

String RealTodoPlugin::get_plugin_name() const {
    return "RealTodo";
}
