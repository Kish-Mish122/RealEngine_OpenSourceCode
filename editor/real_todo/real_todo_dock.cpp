/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_todo_dock.h"
#include "real_todo_item.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "core/string/print_string.h"
#include "core/io/config_file.h"
#include "core/os/time.h"

void RealTodoDock::_bind_methods() {
}

String RealTodoDock::_get_current_date() {
    Time *time = Time::get_singleton();
    Dictionary dt = time->get_datetime_dict_from_system();
    String date = String::num(dt["year"]) + "-" +
                  String::num(dt["month"]).pad_zeros(2) + "-" +
                  String::num(dt["day"]).pad_zeros(2);
    return date;
}

RealTodoDock::RealTodoDock() {
    set_name("To Do");
    set_v_size_flags(SIZE_EXPAND_FILL);
    
    VBoxContainer *main_vbox = memnew(VBoxContainer);
    main_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(main_vbox);
    
    // Фильтр с иконкой
    filter_edit = memnew(LineEdit);
    filter_edit->set_placeholder("Filter tasks...");
    filter_edit->set_clear_button_enabled(true);
    filter_edit->connect("text_changed", callable_mp(this, &RealTodoDock::_filter_changed));
    main_vbox->add_child(filter_edit);
    
    // Список задач
    tasks_scroll = memnew(ScrollContainer);
    tasks_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    main_vbox->add_child(tasks_scroll);
    
    tasks_container = memnew(VBoxContainer);
    tasks_container->set_h_size_flags(SIZE_EXPAND_FILL);
    tasks_scroll->add_child(tasks_container);
    
    // Кнопка добавления
    add_button = memnew(Button);
    add_button->set_text("Add Task");
    add_button->connect("pressed", callable_mp(this, &RealTodoDock::_add_task_pressed));
    main_vbox->add_child(add_button);
    
    // Попап - делаем его красивым и центрированным
    add_popup = memnew(Popup);
    add_popup->set_title("New Task");
    add_popup->set_exclusive(true);
    add_child(add_popup);
    
    // Контейнер с отступами
    MarginContainer *margin = memnew(MarginContainer);
    margin->add_theme_constant_override("margin_left", 20 * EDSCALE);
    margin->add_theme_constant_override("margin_right", 20 * EDSCALE);
    margin->add_theme_constant_override("margin_top", 20 * EDSCALE);
    margin->add_theme_constant_override("margin_bottom", 20 * EDSCALE);
    add_popup->add_child(margin);
    
    add_container = memnew(VBoxContainer);
    add_container->add_theme_constant_override("separation", 12 * EDSCALE);
    margin->add_child(add_container);
    
    // Заголовок
    Label *title_label = memnew(Label);
    title_label->set_text("New Task");
    title_label->add_theme_font_size_override("font_size", 18);
    title_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    add_container->add_child(title_label);
    
    add_container->add_child(memnew(HSeparator));
    
    // Поле названия
    Label *name_label = memnew(Label);
    name_label->set_text("Task:");
    add_container->add_child(name_label);
    
    title_edit = memnew(LineEdit);
    title_edit->set_placeholder("What needs to be done?");
    title_edit->set_custom_minimum_size(Size2(250, 35) * EDSCALE);
    add_container->add_child(title_edit);
    
    // Поле даты
    Label *date_label = memnew(Label);
    date_label->set_text("Due Date:");
    add_container->add_child(date_label);
    
    date_edit = memnew(LineEdit);
    date_edit->set_placeholder("YYYY-MM-DD");
    date_edit->set_custom_minimum_size(Size2(250, 35) * EDSCALE);
    add_container->add_child(date_edit);
    
    // Кнопка "Today"
    Button *today_btn = memnew(Button);
    today_btn->set_text("Today");
    today_btn->set_flat(true);
    today_btn->connect("pressed", callable_mp(this, &RealTodoDock::_set_today_date));
    add_container->add_child(today_btn);
    
    add_container->add_child(memnew(HSeparator));
    
    // Кнопки
    HBoxContainer *btn_box = memnew(HBoxContainer);
    btn_box->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    btn_box->add_theme_constant_override("separation", 15 * EDSCALE);
    add_container->add_child(btn_box);
    
    save_button = memnew(Button);
    save_button->set_text("Create");
    save_button->set_custom_minimum_size(Size2(100, 35) * EDSCALE);
    save_button->connect("pressed", callable_mp(this, &RealTodoDock::_save_new_task));
    btn_box->add_child(save_button);
    
    cancel_button = memnew(Button);
    cancel_button->set_text("Cancel");
    cancel_button->set_custom_minimum_size(Size2(100, 35) * EDSCALE);
    cancel_button->connect("pressed", callable_mp(this, &RealTodoDock::_cancel_add));
    btn_box->add_child(cancel_button);
    
    // Таймер
    check_timer = memnew(Timer);
    check_timer->set_wait_time(60);
    check_timer->connect("timeout", callable_mp(this, &RealTodoDock::_check_overdue));
    add_child(check_timer);
    check_timer->start();
    
    _load_tasks();
}

void RealTodoDock::_set_today_date() {
    date_edit->set_text(_get_current_date());
}

RealTodoDock::~RealTodoDock() {
    _save_tasks();
}

void RealTodoDock::_notification(int p_what) {
    if (p_what == NOTIFICATION_PREDELETE) {
        _save_tasks();
    }
}

String RealTodoDock::_generate_id() {
    Time *time = Time::get_singleton();
    return "task_" + String::num(time->get_unix_time_from_system()) + "_" + String::num(rand() % 10000);
}

void RealTodoDock::add_task(const String &p_title, const String &p_date) {
    RealTodoItem *item = memnew(RealTodoItem);
    String id = _generate_id();
    item->set_task(id, p_title, p_date);
    
    // Подключаем сигналы
    item->connect("task_deleted", callable_mp(this, &RealTodoDock::remove_task));
    item->connect("task_completed", callable_mp(this, &RealTodoDock::_on_task_completed));
    item->connect("tree_exited", callable_mp(this, &RealTodoDock::_on_task_deleted).bind(item));
    
    tasks_container->add_child(item);
    tasks[id] = item;
    _save_tasks();
}

void RealTodoDock::_on_task_completed(const String &p_id, bool p_completed) {
    print_line("[REAL TODO]: Task " + p_id + " completed: " + String(p_completed ? "yes" : "no"));
    _save_tasks();
}

void RealTodoDock::remove_task(const String &p_id) {
    if (tasks.has(p_id)) {
        tasks[p_id]->queue_free();
        tasks.erase(p_id);
        _save_tasks();
    }
}

void RealTodoDock::_add_task_pressed() {
    title_edit->clear();
    date_edit->set_text(_get_current_date());
    
    Size2 popup_size = Size2(350, 400) * EDSCALE;
    add_popup->set_size(popup_size);
    
    // Переименовал window в main_window, чтобы не конфликтовало
    Window *main_window = get_window();
    if (main_window) {
        Size2 window_size = main_window->get_size();
        Vector2 center = (window_size - popup_size) / 2;
        add_popup->set_position(center);
    }
    
    add_popup->popup();
}

void RealTodoDock::_save_new_task() {
    String task_title = title_edit->get_text().strip_edges();  // переименовал
    if (task_title.is_empty()) {
        _show_notification("Error", "Please enter a task description.");
        return;
    }
    
    String date = date_edit->get_text().strip_edges();
    add_task(task_title, date);
    add_popup->hide();
}
void RealTodoDock::_cancel_add() {
    add_popup->hide();
}

void RealTodoDock::_on_task_deleted(Node *p_node) {
    RealTodoItem *item = Object::cast_to<RealTodoItem>(p_node);
    if (item) {
        tasks.erase(item->get_task_id());
        _save_tasks();
    }
}

void RealTodoDock::_filter_changed(const String &p_text) {
    for (int i = 0; i < tasks_container->get_child_count(); i++) {
        RealTodoItem *item = Object::cast_to<RealTodoItem>(tasks_container->get_child(i));
        if (item) {
            if (p_text.is_empty() || item->get_task_title().findn(p_text) >= 0) {
                item->show();
            } else {
                item->hide();
            }
        }
    }
}

void RealTodoDock::_check_overdue() {
    String current_date = _get_current_date();
    
    for (int i = 0; i < tasks_container->get_child_count(); i++) {
        RealTodoItem *item = Object::cast_to<RealTodoItem>(tasks_container->get_child(i));
        if (item && !item->is_completed()) {
            bool overdue = item->get_task_date() < current_date;
            item->set_overdue(overdue);
            
            if (overdue) {
                _show_notification("Task Overdue!", 
                    "Task \"" + item->get_task_title() + "\" is overdue!");
            }
        }
    }
}

void RealTodoDock::_show_notification(const String &p_title, const String &p_message) {
    EditorNode::get_singleton()->show_warning(p_message, p_title);
}

void RealTodoDock::_load_tasks() {
    Ref<ConfigFile> config;
    config.instantiate();
    
    String path = "user://todo.cfg";
    if (config->load(path) == OK) {
        Array tasks_data = config->get_value("todo", "tasks", Array());
        for (int i = 0; i < tasks_data.size(); i++) {
            Dictionary task_data = tasks_data[i];
            RealTodoItem *item = memnew(RealTodoItem);
            item->deserialize(task_data);
            item->connect("tree_exited", callable_mp(this, &RealTodoDock::_on_task_deleted).bind(item));
            tasks_container->add_child(item);
            tasks[task_data["id"]] = item;
        }
    }
}

void RealTodoDock::_save_tasks() {
    Ref<ConfigFile> config;
    config.instantiate();
    
    Array tasks_data;
    for (int i = 0; i < tasks_container->get_child_count(); i++) {
        RealTodoItem *item = Object::cast_to<RealTodoItem>(tasks_container->get_child(i));
        if (item) {
            tasks_data.append(item->serialize());
        }
    }
    
    config->set_value("todo", "tasks", tasks_data);
    
    String path = "user://todo.cfg";
    config->save(path);
}
