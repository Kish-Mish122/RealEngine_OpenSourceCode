#pragma once

#include "scene/gui/panel.h"
#include "scene/gui/label.h"
#include "scene/gui/box_container.h"
#include "scene/main/timer.h"

class SystemStatusBar : public Panel {
    GDCLASS(SystemStatusBar, Panel);

private:
    Timer *update_timer;
    Label *fps_label;
    Label *ram_label;

    void _update_stats();

protected:
    static void _bind_methods();
    virtual void _notification(int p_what);

public:
    SystemStatusBar();
};
