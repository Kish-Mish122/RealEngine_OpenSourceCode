#include "export_itch.h"
#include "editor/settings/editor_settings.h"
#include "editor/gui/editor_file_dialog.h"
#include "core/os/os.h"
#include "core/string/print_string.h"
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "scene/gui/box_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/progress_bar.h"   // <-- добавлено

ExportItchDialog::ExportItchDialog() {
    set_title(TTR("Export to itch.io"));
    set_min_size(Size2(500, 450));

    VBoxContainer *vbox = memnew(VBoxContainer);
    add_child(vbox);
    vbox->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

    // Butler path
    Label *butler_label = memnew(Label);
    butler_label->set_text(TTR("Butler executable path:"));
    vbox->add_child(butler_label);
    butler_path_edit = memnew(LineEdit);
    butler_path_edit->set_placeholder("C:/butler/butler.exe");
    vbox->add_child(butler_path_edit);

    // Username
    Label *user_label = memnew(Label);
    user_label->set_text(TTR("Itch.io username:"));
    vbox->add_child(user_label);
    username_edit = memnew(LineEdit);
    vbox->add_child(username_edit);

    // Game name
    Label *game_label = memnew(Label);
    game_label->set_text(TTR("Game name (as on Itch.io):"));
    vbox->add_child(game_label);
    game_name_edit = memnew(LineEdit);
    vbox->add_child(game_name_edit);

    // Channel
    Label *channel_label = memnew(Label);
    channel_label->set_text(TTR("Channel (e.g. windows-latest, html5):"));
    vbox->add_child(channel_label);
    channel_edit = memnew(LineEdit);
    channel_edit->set_text("windows-latest");
    vbox->add_child(channel_edit);

    // Export folder
    Label *export_label = memnew(Label);
    export_label->set_text(TTR("Exported build folder:"));
    vbox->add_child(export_label);
    HBoxContainer *export_hbox = memnew(HBoxContainer);
    vbox->add_child(export_hbox);
    export_path_edit = memnew(LineEdit);
    export_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    export_hbox->add_child(export_path_edit);
    Button *browse_btn = memnew(Button);
    browse_btn->set_text(TTR("Browse..."));
    browse_btn->connect(SceneStringName(pressed), callable_mp(this, &ExportItchDialog::_browse_export_folder));
    export_hbox->add_child(browse_btn);

    vbox->add_child(memnew(HSeparator));

    // Use standard OK/Cancel
    get_ok_button()->set_text(TTR("Upload"));
    get_cancel_button()->set_text(TTR("Cancel"));
    get_ok_button()->connect(SceneStringName(pressed), callable_mp(this, &ExportItchDialog::_upload));

    progress_dialog = nullptr;
    result_dialog = nullptr;
    upload_thread = nullptr;
    upload_finished = false;
}

void ExportItchDialog::show_dialog() {
    EditorSettings *es = EditorSettings::get_singleton();
    butler_path_edit->set_text(es->get_setting("export/itch/butler_path"));
    username_edit->set_text(es->get_setting("export/itch/username"));
    game_name_edit->set_text(es->get_setting("export/itch/game_name"));
    channel_edit->set_text(es->get_setting("export/itch/channel"));
    export_path_edit->set_text(es->get_setting("export/itch/export_path"));

    popup_centered();
}

void ExportItchDialog::_browse_export_folder() {
    EditorFileDialog *fd = memnew(EditorFileDialog);
    fd->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_DIR);
    fd->connect("dir_selected", callable_mp(this, &ExportItchDialog::_on_export_folder_selected));
    add_child(fd);
    fd->popup_centered();
}

void ExportItchDialog::_on_export_folder_selected(const String &p_path) {
    export_path_edit->set_text(p_path);
}

// Статическая функция-обёртка для потока
void ExportItchDialog::_upload_thread_func(void *p_userdata) {
    ExportItchDialog *self = (ExportItchDialog *)p_userdata;
    self->_upload_async();
}

void ExportItchDialog::_upload_async() {
    // Копируем значения, так как виджеты нельзя трогать из потока
    String butler = butler_path_edit->get_text().strip_edges();
    String username = username_edit->get_text().strip_edges();
    String game = game_name_edit->get_text().strip_edges();
    String channel = channel_edit->get_text().strip_edges();
    String export_path = export_path_edit->get_text().strip_edges();
    String target = username + "/" + game + ":" + channel;

    List<String> args;
    args.push_back("push");
    args.push_back(export_path);
    args.push_back(target);

    String output;
    int exit_code = 0;
    OS::get_singleton()->execute(butler, args, &output, &exit_code, true);

    upload_output = output;
    upload_exit_code = exit_code;
    upload_finished = true;

    // Передаём управление обратно в главный поток
    callable_mp(this, &ExportItchDialog::_upload_finished).call_deferred();
}

void ExportItchDialog::_upload() {
    EditorSettings *es = EditorSettings::get_singleton();
    es->set_setting("export/itch/butler_path", butler_path_edit->get_text());
    es->set_setting("export/itch/username", username_edit->get_text());
    es->set_setting("export/itch/game_name", game_name_edit->get_text());
    es->set_setting("export/itch/channel", channel_edit->get_text());
    es->set_setting("export/itch/export_path", export_path_edit->get_text());
    es->save();

    String butler = butler_path_edit->get_text().strip_edges();
    if (butler.is_empty()) {
        OS::get_singleton()->alert(TTR("Butler path is empty."), TTR("Export Error"));
        print_line("[REAL ITCH]: Butler path is empty");
        return;
    }
    if (!FileAccess::exists(butler)) {
        OS::get_singleton()->alert(TTR("Butler executable not found."), TTR("Export Error"));
        print_line("[REAL ITCH]: Butler.exe not found!");
        return;
    }

    String username = username_edit->get_text().strip_edges();
    String game = game_name_edit->get_text().strip_edges();
    String channel = channel_edit->get_text().strip_edges();
    if (username.is_empty() || game.is_empty()) {
        OS::get_singleton()->alert(TTR("Username or game name is empty."), TTR("Export Error"));
        print_line("[REAL ITCH]: Username or game name is empty!");
        return;
    }

    String export_path = export_path_edit->get_text().strip_edges();
    if (export_path.is_empty()) {
        OS::get_singleton()->alert(TTR("Export folder not specified."), TTR("Export Error"));
        print_line("[REAL ITCH]: Export folder not specified!");
        return;
    }
    if (!DirAccess::exists(export_path)) {
        OS::get_singleton()->alert(TTR("Export folder does not exist."), TTR("Export Error"));
        print_line("[REAL ITCH]: Export folder does not exist!");
        return;
    }

    // Закрываем окно ввода
    hide();

    // Создаём окно прогресса (без кнопок)
    progress_dialog = memnew(AcceptDialog);
    add_child(progress_dialog);
    progress_dialog->set_title(TTR("Export Progress"));
    progress_dialog->set_min_size(Size2(350, 120));
    progress_dialog->get_ok_button()->hide();

    VBoxContainer *vb = memnew(VBoxContainer);
    progress_dialog->add_child(vb);
    Label *info_label = memnew(Label);
    info_label->set_text(TTR("Uploading, please wait..."));
    vb->add_child(info_label);
    ProgressBar *pb = memnew(ProgressBar);
    pb->set_indeterminate(true);
    vb->add_child(pb);

    progress_dialog->popup_centered();

    // Запускаем поток
    upload_finished = false;
    upload_thread = memnew(Thread);
    upload_thread->start(_upload_thread_func, this);
}

void ExportItchDialog::_upload_finished() {
    if (progress_dialog) {
        progress_dialog->hide();
        progress_dialog->queue_free();
        progress_dialog = nullptr;
    }

    String message;
    if (upload_exit_code == 0) {
        message = TTR("Upload successful! The game is now on itch.io.");
        result_dialog = memnew(AcceptDialog);
        add_child(result_dialog);
        result_dialog->set_title(TTR("Upload Successful"));
        result_dialog->get_ok_button()->set_text(TTR("OK"));
        print_line("[REAL ITCH]: Upload successful! The game now on itch.io!");
    } else {
        message = TTR("Upload failed!");
        result_dialog = memnew(AcceptDialog);
        add_child(result_dialog);
        result_dialog->set_title(TTR("Upload Failed"));
        result_dialog->get_ok_button()->set_text(TTR("OK"));
        print_line("[REAL ITCH]: Upload failed! Error details: " + upload_output);
    }

    VBoxContainer *vb = memnew(VBoxContainer);
    result_dialog->add_child(vb);
    Label *msg_label = memnew(Label);
    msg_label->set_text(message);
    msg_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
    vb->add_child(msg_label);

    result_dialog->connect(SceneStringName(confirmed), callable_mp(this, &ExportItchDialog::_on_result_closed));
    result_dialog->popup_centered();

    if (upload_thread) {
        upload_thread->wait_to_finish();
        memdelete(upload_thread);
        upload_thread = nullptr;
    }
    upload_finished = false;
}

void ExportItchDialog::_on_result_closed() {
    if (result_dialog) {
        result_dialog->queue_free();
        result_dialog = nullptr;
    }
}

void ExportItchDialog::_notification(int p_what) {
    // не используется
}

void ExportItchDialog::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_upload"), &ExportItchDialog::_upload);
    ClassDB::bind_method(D_METHOD("_browse_export_folder"), &ExportItchDialog::_browse_export_folder);
    ClassDB::bind_method(D_METHOD("_on_export_folder_selected", "path"), &ExportItchDialog::_on_export_folder_selected);
    ClassDB::bind_method(D_METHOD("_on_result_closed"), &ExportItchDialog::_on_result_closed);
}
