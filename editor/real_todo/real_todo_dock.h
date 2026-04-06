/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "editor/docks/editor_dock.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/popup.h"
#include "scene/gui/label.h"
#include "scene/gui/separator.h"
#include "scene/main/timer.h"
#include "scene/gui/box_container.h" 

class RealTodoItem;

class RealTodoDock : public EditorDock {
    GDCLASS(RealTodoDock, EditorDock);

private:
    void _on_task_completed(const String &p_id, bool p_completed);
    void _set_today_date();
    LineEdit *filter_edit;
    ScrollContainer *tasks_scroll;
    VBoxContainer *tasks_container;
    Button *add_button;
    
    Popup *add_popup;
    VBoxContainer *add_container;
    LineEdit *title_edit;
    LineEdit *date_edit;  // вместо Calendar
    Button *save_button;
    Button *cancel_button;
    
    Timer *check_timer;
    HashMap<String, RealTodoItem *> tasks;
    
    void _add_task_pressed();
    void _save_new_task();
    void _cancel_add();
    void _on_task_deleted(Node *p_node);
    void _filter_changed(const String &p_text);
    void _check_overdue();
    void _show_notification(const String &p_title, const String &p_message);
    String _generate_id();
    void _load_tasks();
    void _save_tasks();
    String _get_current_date();

protected:
    static void _bind_methods();
    virtual void _notification(int p_what);

public:
    RealTodoDock();
    ~RealTodoDock();
    
    void add_task(const String &p_title, const String &p_date);
    void remove_task(const String &p_id);
};
