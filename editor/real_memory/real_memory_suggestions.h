/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "scene/gui/panel.h"
#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/texture_rect.h"

class RealMemorySuggestion : public Panel {
    GDCLASS(RealMemorySuggestion, Panel);

private:
    Label *message_label;
    Button *apply_button;
    Button *remind_later_button;
    Button *dont_show_again_button;
    TextureRect *icon;
    
    Dictionary suggestion_data;

    void _apply_pressed();
    void _later_pressed();
    void _never_pressed();

protected:
    static void _bind_methods();
    virtual void _notification(int p_what);

public:
    RealMemorySuggestion();
    void set_suggestion(const String &p_message, const Dictionary &p_data);
};
