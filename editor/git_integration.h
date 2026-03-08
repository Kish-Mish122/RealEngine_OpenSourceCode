#ifndef GIT_INTEGRATION_H
#define GIT_INTEGRATION_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/io/http_client.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"
#include "core/io/http_client_tcp.h"

// GUI includes
#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/check_box.h"
#include "scene/gui/option_button.h"
#include "scene/gui/tree.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/box_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/scroll_container.h"

class GitIntegration : public RefCounted {
    GDCLASS(GitIntegration, RefCounted);

    struct RepoData {
        String name;
        String full_name;
        String description;
        String url;
        bool is_private;
        String default_branch;
    };

    struct IssueData {
        int number;
        String title;
        String body;
        String state;
        String created_at;
        String updated_at;
    };

    String project_path;
    String git_path;
    bool git_available = false;

    // GitHub данные
    String github_token;
    String gitlab_token;
    String gitlab_url;
    bool github_logged_in = false;
    bool gitlab_logged_in = false;
    String current_username;
    Vector<RepoData> user_repos;

    static GitIntegration *singleton;

    // HTTP утилита - объявляем в private
    String http_request(const String &p_url, const HashMap<String, String> &p_headers,
                        const String &p_method = "GET", const String &p_body = "");

protected:
    static void _bind_methods();

public:
    bool backup_project_to_github(const String &p_repo_name, const String &p_commit_message = "Backup");
    void show_backup_dialog(Control *p_parent);
    void _on_backup_confirm(LineEdit *repo_edit, LineEdit *msg_edit, CheckBox *private_cb, AcceptDialog *dialog);

    bool backup_project_to_gitlab(const String &p_repo_name, const String &p_commit_message = "Backup");
    void show_gitlab_backup_dialog(Control *p_parent);
    void _on_gitlab_backup_confirm(LineEdit *repo_edit, LineEdit *msg_edit, CheckBox *private_cb, AcceptDialog *dialog);

    void _on_enhanced_backup(OptionButton *repo_select, LineEdit *repo_edit, LineEdit *msg_edit, CheckBox *private_cb, AcceptDialog *dialog);
    bool backup_to_existing_repo(const String &p_repo_name, const String &p_commit_message);
    void show_enhanced_backup_dialog(Control *p_parent);

    static GitIntegration *get_singleton() { return singleton; }

    GitIntegration();
    ~GitIntegration();

    // Основные Git функции
    bool check_git_available();
    String execute_git_command(const String &p_command);
    void init_repository();
    void commit(const String &p_message);
    void add_all();
    void push();
    void pull();
    void fetch();
    String get_current_branch();
    String get_current_repo_name();

    // GitHub функции
    bool github_login(const String &p_token);
    void github_logout();
    bool is_github_logged_in() const { return github_logged_in; }
    Vector<RepoData> github_list_repos();
    bool github_create_repo(const String &p_name, const String &p_description, bool p_private);

    // GitLab функции
    bool gitlab_login(const String &p_token, const String &p_url = "https://gitlab.com");
    void gitlab_logout();
    bool is_gitlab_logged_in() const { return gitlab_logged_in; }
    Vector<RepoData> gitlab_list_repos();
    bool gitlab_create_repo(const String &p_name, const String &p_description, bool p_private);

    // Issues
    Vector<IssueData> list_issues(const String &p_repo);
    bool create_issue(const String &p_repo, const String &p_title, const String &p_body);

    // UI
    void show_github_dialog(Control *p_parent);
    void show_gitlab_dialog(Control *p_parent);
    void show_create_repo_dialog(Control *p_parent);
    void show_issues_dialog(Control *p_parent, const String &p_repo);

    // Обработчики - объявляем как public
    void _on_github_login(LineEdit *token_edit, AcceptDialog *dialog);
    void _on_gitlab_login(LineEdit *token_edit, LineEdit *url_edit, AcceptDialog *dialog);
    void _on_create_repo(LineEdit *name_edit, TextEdit *desc_edit, CheckBox *private_cb, OptionButton *platform_cb, AcceptDialog *dialog);
    void _on_create_issue(LineEdit *title_edit, TextEdit *body_edit, const String &p_repo, AcceptDialog *dialog);
};

#endif // GIT_INTEGRATION_H
