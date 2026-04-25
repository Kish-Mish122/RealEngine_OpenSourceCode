#ifndef EXPORT_ITCH_H
#define EXPORT_ITCH_H

#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"

class ExportItchDialog : public ConfirmationDialog {
    GDCLASS(ExportItchDialog, ConfirmationDialog);

private:
    LineEdit *butler_path_edit;
    LineEdit *username_edit;
    LineEdit *game_name_edit;
    LineEdit *channel_edit;
    LineEdit *export_path_edit;
    Label *status_label;

    void _upload();
    void _update_status(const String &p_text, bool p_error = false);
    void _browse_export_folder();
    void _on_export_folder_selected(const String &p_path);

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    ExportItchDialog();
    void show_dialog();
};

#endif // EXPORT_ITCH_H
