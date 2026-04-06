/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "scene/gui/panel_container.h"
#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/box_container.h"

class RealTodoItem : public PanelContainer {
    GDCLASS(RealTodoItem, PanelContainer);

private:
    CheckBox *checkbox;
    Label *title_label;
    Label *date_label;
    Button *delete_button;
    
    String task_id;
    String task_title;
    String task_date;
    bool completed;
    bool overdue;

    void _update_style();
    void _on_checkbox_toggled(bool p_pressed);
    void _on_delete_pressed();

protected:
    static void _bind_methods();  // только объявление, без реализации!

public:
    RealTodoItem();
    
    void set_task(const String &p_id, const String &p_title, const String &p_date, bool p_completed = false);
    String get_task_id() const { return task_id; }
    String get_task_title() const { return task_title; }
    String get_task_date() const { return task_date; }
    bool is_completed() const { return completed; }
    void set_overdue(bool p_overdue);
    
    Dictionary serialize() const;
    void deserialize(const Dictionary &p_data);
};
