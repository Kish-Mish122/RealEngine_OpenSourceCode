#pragma once

#include "editor/themes/editor_theme.h"

class RealEngineTheme : public EditorTheme {
    GDCLASS(RealEngineTheme, EditorTheme);

protected:
    static void _bind_methods();

public:
    RealEngineTheme();
    void apply_real_engine_theme();
};
