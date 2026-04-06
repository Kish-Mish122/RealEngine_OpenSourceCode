/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_todo_item.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"

void RealTodoItem::_bind_methods() {
    ADD_SIGNAL(MethodInfo("task_deleted", PropertyInfo(Variant::STRING, "task_id")));
    ADD_SIGNAL(MethodInfo("task_completed", PropertyInfo(Variant::STRING, "task_id"), PropertyInfo(Variant::BOOL, "completed")));
}

RealTodoItem::RealTodoItem() {
    set_custom_minimum_size(Size2(0, 45) * EDSCALE);
    
    HBoxContainer *hbox = memnew(HBoxContainer);
    hbox->set_h_size_flags(SIZE_EXPAND_FILL);
    add_child(hbox);
    
    checkbox = memnew(CheckBox);
    checkbox->connect("toggled", callable_mp(this, &RealTodoItem::_on_checkbox_toggled));
    hbox->add_child(checkbox);
    
    VBoxContainer *vbox = memnew(VBoxContainer);
    vbox->set_h_size_flags(SIZE_EXPAND_FILL);
    hbox->add_child(vbox);
    
    title_label = memnew(Label);
    title_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
    vbox->add_child(title_label);
    
    date_label = memnew(Label);
    date_label->add_theme_font_size_override("font_size", 10);
    vbox->add_child(date_label);
    
    delete_button = memnew(Button);
    delete_button->set_flat(true);
    delete_button->set_button_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon("Remove", "EditorIcons"));
    delete_button->connect("pressed", callable_mp(this, &RealTodoItem::_on_delete_pressed));
    hbox->add_child(delete_button);
}

void RealTodoItem::set_task(const String &p_id, const String &p_title, const String &p_date, bool p_completed) {
    task_id = p_id;
    task_title = p_title;
    task_date = p_date;
    completed = p_completed;
    
    title_label->set_text(p_title);
    date_label->set_text("Due: " + p_date);
    checkbox->set_pressed(p_completed);
    
    _update_style();
}

void RealTodoItem::set_overdue(bool p_overdue) {
    overdue = p_overdue;
    _update_style();
}

void RealTodoItem::_update_style() {
    if (completed) {
        title_label->add_theme_color_override("font_color", Color(0.5, 0.5, 0.5));
        set_modulate(Color(1, 1, 1, 0.6));
    } else if (overdue) {
        title_label->add_theme_color_override("font_color", Color(1, 0.3, 0.3));
        date_label->add_theme_color_override("font_color", Color(1, 0.3, 0.3));
    } else {
        title_label->add_theme_color_override("font_color", Color(1, 1, 1));
        date_label->add_theme_color_override("font_color", Color(0.7, 0.7, 0.7));
    }
}

void RealTodoItem::_on_checkbox_toggled(bool p_pressed) {
    completed = p_pressed;
    _update_style();
    emit_signal("task_completed", task_id, completed);
}

void RealTodoItem::_on_delete_pressed() {
    print_line("[REAL TODO]: Deleting task: " + task_title);
    emit_signal("task_deleted", task_id);
    queue_free();
}

Dictionary RealTodoItem::serialize() const {
    Dictionary dict;
    dict["id"] = task_id;
    dict["title"] = task_title;
    dict["date"] = task_date;
    dict["completed"] = completed;
    return dict;
}

void RealTodoItem::deserialize(const Dictionary &p_data) {
    task_id = p_data["id"];
    task_title = p_data["title"];
    task_date = p_data["date"];
    completed = p_data.get("completed", false);
    
    title_label->set_text(task_title);
    date_label->set_text("Due: " + task_date);
    checkbox->set_pressed(completed);
    _update_style();
}
