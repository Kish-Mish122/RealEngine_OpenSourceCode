#include "git_integration.h"
#include "editor/editor_node.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/os/os.h"
#include "core/string/print_string.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/separator.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#endif

GitIntegration *GitIntegration::singleton = nullptr;

void GitIntegration::_bind_methods() {
}

GitIntegration::GitIntegration() {
    singleton = this;

#ifdef WINDOWS_ENABLED
    git_path = "git.exe"; // Узнаём путь к исполняемому файлу GIT
#else
    git_path = "git";
#endif

    if (ProjectSettings::get_singleton()) {
        project_path = ProjectSettings::get_singleton()->get_resource_path();
    }

    // Проверим наличие GIT
    check_git_available();
}

GitIntegration::~GitIntegration() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

bool GitIntegration::check_git_available() {
    String result = execute_git_command("--version");
    git_available = !result.is_empty() && result.contains("git version");

    if (git_available) {
        print_line("[REAL GIT] Found: " + result.strip_edges());
    }

    return git_available;
}

String GitIntegration::execute_git_command(const String &p_command) {
    if (!git_available && p_command != "--version") {
        return "";
    }

    String full_cmd = "\"" + git_path + "\" " + p_command;

    List<String> args;
    args.push_back("/c");
    args.push_back(full_cmd);

    String output;
    int exitcode = 0;

    OS::get_singleton()->execute("cmd.exe", args, &output, &exitcode, true);

    return output;
}

void GitIntegration::init_repository() {
    if (!git_available) return;

    (void)execute_git_command("init");

    String gitignore = project_path + "/.gitignore";
    Ref<FileAccess> f = FileAccess::open(gitignore, FileAccess::WRITE);
    if (f.is_valid()) {
        f->store_line("# Real Engine");
        f->store_line(".realengine/");
        f->store_line(".godot/");
        f->store_line("*.import");
        f->store_line("export_presets.cfg");
        f->store_line("");
        f->store_line("# OS files");
        f->store_line(".DS_Store");
        f->store_line("Thumbs.db");
    }
}

String GitIntegration::get_current_branch() {
    if (!git_available) return "git not available";
    String branch = execute_git_command("rev-parse --abbrev-ref HEAD");
    return branch.strip_edges();
}

String GitIntegration::get_status() {
    if (!git_available) return "git not available";
    String status = execute_git_command("status -s");
    if (status.is_empty()) {
        return "No changes"; // Если изменений в проекте нет, значит возращаем "Нет изменений"
    }
    return status;
}

void GitIntegration::commit(const String &p_message) {
    if (!git_available || p_message.is_empty()) return;

    (void)execute_git_command("add -A");
    (void)execute_git_command("commit -m \"" + p_message + "\"");
}

void GitIntegration::show_git_panel(Control *p_parent) {
    if (!p_parent) return;

    if (!git_available) {
        AcceptDialog *dialog = memnew(AcceptDialog);
        dialog->set_title("Git Not Available");
        dialog->set_text("Git is not installed or not in PATH.\n\nPlease install Git from:\nhttps://git-scm.com/");
        p_parent->add_child(dialog);
        dialog->popup_centered();
        return;
    }

    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Git Integration");

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    // Ветка
    Label *branch_label = memnew(Label);
    branch_label->set_text("Branch: " + get_current_branch());
    vb->add_child(branch_label);

    // Путь (коротко)
    Label *path_label = memnew(Label);
    String project_name = project_path.get_file();
    path_label->set_text("Project: " + project_name);
    vb->add_child(path_label);

    vb->add_child(memnew(HSeparator));

    // Статус (только первые 3 строки) чтобы не растягивалось окно до конца экрана если много изменений
    String status = get_status();
    Vector<String> lines = status.split("\n");
    String short_status;
    for (int i = 0; i < lines.size() && i < 3; i++) {
        short_status += lines[i] + "\n";
    }
    if (lines.size() > 3) { // Если больше 3 строчек, то вместо длинного написания текста, просто пишем "..."
        short_status += "...";
    }

    Label *status_label = memnew(Label);
    status_label->set_text(short_status);
    vb->add_child(status_label);

    vb->add_child(memnew(HSeparator));

    // Кнопки в одну строку
    HBoxContainer *hb = memnew(HBoxContainer);

    Button *init_btn = memnew(Button);
    init_btn->set_text("Init");
    init_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_init_pressed).bind(dialog));
    hb->add_child(init_btn);

    Button *close_btn = memnew(Button);
    close_btn->set_text("Close");
    close_btn->connect("pressed", Callable(dialog, "hide"));
    hb->add_child(close_btn);

    vb->add_child(hb);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::_on_init_pressed(AcceptDialog *dialog) {
    init_repository();

    // Показываем сообщение
    AcceptDialog *msg = memnew(AcceptDialog);
    msg->set_title("Git");
    msg->set_text("Repository initialized successfully!\n.gitignore file created.");
    msg->set_min_size(Size2(350, 150));
    EditorNode::get_singleton()->get_gui_base()->add_child(msg);
    msg->popup_centered(Size2(350, 150));

    // Обновляем диалог
    dialog->hide();
    dialog->queue_free();
    show_git_panel(EditorNode::get_singleton()->get_gui_base());
}

void GitIntegration::_on_commit_pressed(AcceptDialog *dialog, LineEdit *msg_edit) {
    String msg = msg_edit->get_text();
    if (msg.is_empty()) {
        AcceptDialog *err = memnew(AcceptDialog);
        err->set_title("Git Error");
        err->set_text("Commit message cannot be empty!");
        err->set_min_size(Size2(300, 100));
        EditorNode::get_singleton()->get_gui_base()->add_child(err);
        err->popup_centered(Size2(300, 100));
        return;
    }

    commit(msg);

    // Показываем сообщение
    AcceptDialog *msg_dlg = memnew(AcceptDialog);
    msg_dlg->set_title("Git");
    msg_dlg->set_text("Changes committed successfully!");
    msg_dlg->set_min_size(Size2(300, 100));
    EditorNode::get_singleton()->get_gui_base()->add_child(msg_dlg);
    msg_dlg->popup_centered(Size2(300, 100));

    // Обновляем диалог
    dialog->hide();
    dialog->queue_free();
    show_git_panel(EditorNode::get_singleton()->get_gui_base());
}
