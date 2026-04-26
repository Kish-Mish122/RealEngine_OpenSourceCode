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

    status_label = memnew(Label);
    status_label->set_text("");
    vbox->add_child(status_label);

    // Use standard buttons
    get_ok_button()->set_text(TTR("Upload"));
    get_cancel_button()->set_text(TTR("Cancel"));
    get_ok_button()->connect(SceneStringName(pressed), callable_mp(this, &ExportItchDialog::_upload));
}

void ExportItchDialog::show_dialog() {
    EditorSettings *es = EditorSettings::get_singleton();
    butler_path_edit->set_text(es->get_setting("export/itch/butler_path"));
    username_edit->set_text(es->get_setting("export/itch/username"));
    game_name_edit->set_text(es->get_setting("export/itch/game_name"));
    channel_edit->set_text(es->get_setting("export/itch/channel"));
    export_path_edit->set_text(es->get_setting("export/itch/export_path"));

    status_label->set_text("");
    popup_centered();
}

void ExportItchDialog::_update_status(const String &p_text, bool p_error) {
    status_label->set_text(p_text);
    Color color = p_error ? Color(1, 0.5, 0.5) : Color(1, 1, 1);
    status_label->add_theme_color_override("font_color", color);
}

void ExportItchDialog::_browse_export_folder() {
    EditorFileDialog *fd = memnew(EditorFileDialog);
    fd->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_DIR);
    fd->connect("dir_selected", callable_mp(this, &ExportItchDialog::_on_export_folder_selected));
    fd->popup_centered();
}

void ExportItchDialog::_on_export_folder_selected(const String &p_path) {
    export_path_edit->set_text(p_path);
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
        _update_status(TTR("Butler path is empty."), true);
        print_line("[REAL ITCH]: Butler path is empty");
        return;
    }
    if (!FileAccess::exists(butler)) {
        _update_status(TTR("Butler executable not found."), true);
        print_line("[REAL ITCH]: Butler executable not found");
        return;
    }

    String username = username_edit->get_text().strip_edges();
    String game = game_name_edit->get_text().strip_edges();
    String channel = channel_edit->get_text().strip_edges();
    if (username.is_empty() || game.is_empty()) {
        _update_status(TTR("Username or game name is empty."), true);
        print_line("[REAL ITCH]: Username or game name is empty");
        return;
    }

    String export_path = export_path_edit->get_text().strip_edges();
    if (export_path.is_empty()) {
        _update_status(TTR("Export folder not specified."), true);
        print_line("[REAL ITCH]: Export folder not specified");
        return;
    }
    if (!DirAccess::exists(export_path)) {
        _update_status(TTR("Export folder does not exist."), true);
        print_line("[REAL ITCH]: Export folder does not exist");
        return;
    }

    String target = username + "/" + game + ":" + channel;
    List<String> args;
    args.push_back("push");
    args.push_back(export_path);
    args.push_back(target);

    _update_status(TTR("Uploading... Please wait..."));

    String output;
    int exit_code = 0;
    Error err = OS::get_singleton()->execute(butler, args, &output, &exit_code, true);
    print_line("[REAL ITCH]: Exit code: " + itos(exit_code) + ", output: " + output);

    if (err == OK && exit_code == 0) {
        _update_status(TTR("Upload successful!"));
        print_line("[REAL ITCH]: Upload successful!");
        hide();
    } else {
        _update_status(TTR("Upload failed: ") + output, true);
        print_line("[REAL ITCH]: Upload is Failed! Output: " + output, true);
    }
}

void ExportItchDialog::_notification(int p_what) {
}

void ExportItchDialog::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_upload"), &ExportItchDialog::_upload);
    ClassDB::bind_method(D_METHOD("_browse_export_folder"), &ExportItchDialog::_browse_export_folder);
    ClassDB::bind_method(D_METHOD("_on_export_folder_selected", "path"), &ExportItchDialog::_on_export_folder_selected);
}

/* Зачем столько логов?
При попытке экспорта может крашнуться Real Engine, логи помогают найти в чём была ошибка
*/
