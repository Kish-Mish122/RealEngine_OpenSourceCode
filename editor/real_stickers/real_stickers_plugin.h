#pragma once

#include "editor/plugins/editor_plugin.h"

class RealStickersPlugin : public EditorPlugin {
    GDCLASS(RealStickersPlugin, EditorPlugin);

protected:
    static void _bind_methods();

public:
    RealStickersPlugin();
    ~RealStickersPlugin();

    void _enter_tree();      // без override
    void _exit_tree();       // без override
    virtual String get_plugin_name() const override;

private:
    class RealStickersDock *dock = nullptr;
};
