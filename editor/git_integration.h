#ifndef GIT_INTEGRATION_H
#define GIT_INTEGRATION_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "scene/gui/dialogs.h"

class GitIntegration : public RefCounted {
    GDCLASS(GitIntegration, RefCounted);

    String git_path;
    bool git_available = false;

    static GitIntegration *singleton;

protected:
    static void _bind_methods();

public:
    String project_path;

    static GitIntegration *get_singleton() { return singleton; }

    GitIntegration();
    ~GitIntegration();

    bool check_git_available();
    String execute_git_command(const String &p_command);
    void init_repository();
    String get_current_branch();
    String get_status();
    void commit(const String &p_message);

    void show_git_panel(Control *p_parent);

    void _on_init_pressed(AcceptDialog *dialog);
    void _on_commit_pressed(AcceptDialog *dialog, LineEdit *msg_edit);
};

#endif // GIT_INTEGRATION_H
