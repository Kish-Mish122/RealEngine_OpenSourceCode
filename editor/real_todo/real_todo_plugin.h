/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "editor/plugins/editor_plugin.h"

class RealTodoPlugin : public EditorPlugin {
    GDCLASS(RealTodoPlugin, EditorPlugin);

protected:
    static void _bind_methods();

public:
    RealTodoPlugin();
    ~RealTodoPlugin();

    void _enter_tree();  // без override
    void _exit_tree();   // без override
    virtual String get_plugin_name() const override;
};
