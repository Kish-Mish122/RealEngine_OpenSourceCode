/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_todo_plugin.h"
#include "real_todo_dock.h"
#include "core/string/print_string.h"

void RealTodoPlugin::_bind_methods() {
    print_line("[REAL TODO]: _bind_methods called");
}

RealTodoPlugin::RealTodoPlugin() {
    print_line("[REAL TODO]: PLUGIN CREATED !!!");
}

RealTodoPlugin::~RealTodoPlugin() {
    print_line("[REAL TODO]: PLUGIN DESTROYED !!!");
}

void RealTodoPlugin::_enter_tree() {
    print_line("[REAL TODO]: ENTER TREE !!!");
    RealTodoDock *todo_dock = memnew(RealTodoDock);
    add_control_to_dock(EditorPlugin::DOCK_SLOT_RIGHT_BR, todo_dock);
    print_line("[REAL TODO]: Dock added !!!");
}

void RealTodoPlugin::_exit_tree() {
    print_line("[REAL TODO]: EXIT TREE !!!");
}

String RealTodoPlugin::get_plugin_name() const {
    return "RealTodo";
}
