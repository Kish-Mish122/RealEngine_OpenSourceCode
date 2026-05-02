#ifndef EXPORT_ITCH_H
#define EXPORT_ITCH_H

#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "core/os/thread.h"

class ExportItchDialog : public ConfirmationDialog {
    GDCLASS(ExportItchDialog, ConfirmationDialog);

private:
    LineEdit *butler_path_edit;
    LineEdit *username_edit;
    LineEdit *game_name_edit;
    LineEdit *channel_edit;
    LineEdit *export_path_edit;

    AcceptDialog *progress_dialog;
    AcceptDialog *result_dialog;

    Thread *upload_thread;
    String upload_output;
    int upload_exit_code;
    volatile bool upload_finished;

    void _upload();
    void _browse_export_folder();
    void _on_export_folder_selected(const String &p_path);
    static void _upload_thread_func(void *p_userdata);
    void _upload_async();
    void _upload_finished();
    void _on_result_closed();

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    ExportItchDialog();
    void show_dialog();
};

#endif // EXPORT_ITCH_H
