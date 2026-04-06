/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "git_integration.h"
#include "core/string/print_string.h"
#include "core/os/os.h"
#include "editor/editor_node.h"
#include "core/io/stream_peer_tls.h"
#include "core/io/http_client_tcp.h"
#include "core/os/time.h"
#include "editor/settings/editor_settings.h"

GitIntegration *GitIntegration::singleton = nullptr;

void GitIntegration::_bind_methods() {
}

GitIntegration::GitIntegration() {
    singleton = this;

#ifdef WINDOWS_ENABLED
    git_path = "git.exe";
#else
    git_path = "git";
#endif

    if (ProjectSettings::get_singleton()) {
        project_path = ProjectSettings::get_singleton()->get_resource_path();
    }

    check_git_available();

    if (EditorSettings::get_singleton()) {
        if (EditorSettings::get_singleton()->has_setting("github/token")) {
            String saved_token = EditorSettings::get_singleton()->get("github/token");
            if (!saved_token.is_empty()) {
                print_line("[REAL GITHUB] Found saved token, attempting auto-login...");
                print_line("[REAL GITHUB] Token length: " + itos(saved_token.length()));
                github_login(saved_token);
            } else {
                print_line("[REAL GITHUB] Saved token is empty");
            }
        } else {
            print_line("[REAL GITHUB] No saved token found");
        }
    }
}

GitIntegration::~GitIntegration() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

bool GitIntegration::check_git_available() {
    String result = execute_git_command("--version");
    git_available = !result.is_empty() && result.contains("git version");
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
}

void GitIntegration::commit(const String &p_message) {
    if (!git_available || p_message.is_empty()) return;
    (void)execute_git_command("commit -m \"" + p_message + "\"");
}

void GitIntegration::add_all() {
    if (!git_available) return;
    (void)execute_git_command("add -A");
}

void GitIntegration::push() {
    if (!git_available) return;
    (void)execute_git_command("push");
}

void GitIntegration::pull() {
    if (!git_available) return;
    (void)execute_git_command("pull");
}

void GitIntegration::fetch() {
    if (!git_available) return;
    (void)execute_git_command("fetch");
}

String GitIntegration::get_current_branch() {
    if (!git_available) return "main";
    String branch = execute_git_command("rev-parse --abbrev-ref HEAD");
    return branch.strip_edges();
}

String GitIntegration::get_current_repo_name() {
    if (!git_available) return "";
    String remote = execute_git_command("config --get remote.origin.url");
    if (remote.is_empty()) return "";

    Vector<String> parts = remote.split("/");
    if (parts.size() > 0) {
        String last = parts[parts.size() - 1];
        return last.replace(".git", "");
    }
    return "";
}

// ==================== HTTP утилита ====================

String GitIntegration::http_request(const String &p_url, const HashMap<String, String> &p_headers,
                                    const String &p_method, const String &p_body) {
    print_line("[HTTP] Request to: " + p_url);

    Ref<HTTPClientTCP> client;
    client.instantiate();

    // Парсим URL
    String host = p_url;
    String path = "/";
    bool use_ssl = p_url.begins_with("https://");

    if (p_url.begins_with("https://")) {
        host = p_url.substr(8);
    } else if (p_url.begins_with("http://")) {
        host = p_url.substr(7);
    }

    int path_start = host.find_char('/');
    if (path_start != -1) {
        path = host.substr(path_start);
        host = host.substr(0, path_start);
    }

    print_line("[HTTP] Connecting to host: " + host + ", SSL: " + (use_ssl ? "yes" : "no"));

    // Добавляем TLS опции для HTTPS
    Ref<TLSOptions> tls_options;
    if (use_ssl) {
        tls_options = TLSOptions::client();
    }

    Error err = client->connect_to_host(host, use_ssl ? 443 : 80, tls_options);
    if (err != OK) {
        print_line("[HTTP] Failed to connect: " + itos(err));
        return "";
    }

    // Ждём подключения
    int attempts = 0;
    while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
           client->get_status() == HTTPClient::STATUS_RESOLVING) {
        client->poll();
        OS::get_singleton()->delay_usec(100000); // 100ms
        attempts++;
        if (attempts > 50) { // 5 секунд таймаут
            print_line("[HTTP] Connection timeout");
            return "";
        }
    }

    if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
        print_line("[HTTP] Not connected, status: " + itos(client->get_status()));
        return "";
    }

    print_line("[HTTP] Connected successfully");

    // Формируем заголовки
    Vector<String> headers;
    headers.push_back("Host: " + host);
    headers.push_back("User-Agent: Real-Engine/1.0");
    headers.push_back("Accept: application/json");

    for (const KeyValue<String, String> &E : p_headers) {
        headers.push_back(E.key + ": " + E.value);
    }

    if (!p_body.is_empty()) {
        headers.push_back("Content-Type: application/json");
        headers.push_back("Content-Length: " + itos(p_body.utf8().length()));
    }

    // Выбираем метод
    HTTPClient::Method method = HTTPClient::METHOD_GET;
    if (p_method == "POST") method = HTTPClient::METHOD_POST;
    else if (p_method == "PUT") method = HTTPClient::METHOD_PUT;
    else if (p_method == "DELETE") method = HTTPClient::METHOD_DELETE;
    else if (p_method == "PATCH") method = HTTPClient::METHOD_PATCH;

    // Отправляем запрос (всегда с 5 параметрами)
    if (p_body.is_empty()) {
        err = client->request(method, path, headers, nullptr, 0);
    } else {
        CharString body_utf8 = p_body.utf8();
        err = client->request(method, path, headers, (const uint8_t*)body_utf8.get_data(), body_utf8.length());
    }

    if (err != OK) {
        print_line("[HTTP] Request failed: " + itos(err));
        return "";
    }

    // Ждём ответа
    attempts = 0;
    while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
        client->poll();
        OS::get_singleton()->delay_usec(100000); // 100ms
        attempts++;
        if (attempts > 50) { // 5 секунд таймаут
            print_line("[HTTP] Request timeout");
            return "";
        }
    }

    // Проверяем код ответа
    int response_code = client->get_response_code();
    print_line("[HTTP] Response code: " + itos(response_code));

    if (response_code == 0) {
        print_line("[HTTP] No response code");
        return "";
    }

    // Читаем заголовки ответа (нужен список для заполнения)
    List<String> response_headers;
    client->get_response_headers(&response_headers);

    // Читаем тело ответа
    PackedByteArray response_data;
    while (client->get_status() == HTTPClient::STATUS_BODY) {
        client->poll();
        PackedByteArray chunk = client->read_response_body_chunk();
        if (chunk.size() > 0) {
            response_data.append_array(chunk);
        } else {
            OS::get_singleton()->delay_usec(100000); // 100ms
        }
    }

    if (response_data.size() == 0) {
        print_line("[HTTP] Empty response");
        return "";
    }

    String result = String::utf8((const char*)response_data.ptr(), response_data.size());
    print_line("[HTTP] Response received: " + itos(result.length()) + " bytes");

    // Для отладки выведем первые 200 символов
    String preview = result.substr(0, 200);
    print_line("[HTTP] Preview: " + preview);

    return result;
}

// ==================== GitHub функции ====================

bool GitIntegration::github_login(const String &p_token) {
    print_line("[REAL GITHUB] Attempting login with token length: " + itos(p_token.length()));

    github_token = p_token;

    HashMap<String, String> headers;
    headers["Authorization"] = "token " + github_token;
    headers["User-Agent"] = "Real-Engine/1.0";
    headers["Accept"] = "application/vnd.github.v3+json";

    String response = http_request("https://api.github.com/user", headers);

    if (response.is_empty()) {
        print_line("[GitHub] Empty response");
        return false;
    }

    JSON json;
    Error err = json.parse(response);
    if (err != OK) {
        print_line("[REAL GITHUB] Failed to parse JSON: " + json.get_error_message());
        print_line("[REAL GITHUB] Response: " + response);
        return false;
    }

    Dictionary data = json.get_data();

    if (data.has("login")) {
        github_logged_in = true;
        current_username = data["login"];

        // СОХРАНЯЕМ ТОКЕН В НАСТРОЙКИ
        if (EditorSettings::get_singleton()) {
            EditorSettings::get_singleton()->set("github/token", p_token);
            EditorSettings::get_singleton()->save();

            // Проверяем, что сохранилось
            String check = EditorSettings::get_singleton()->get("github/token");
            print_line("[REAL GITHUB] Token saved, check: " + String(check.is_empty() ? "FAILED" : "OK"));
        }

        print_line("[REAL GITHUB] ✓ Logged in as: " + current_username);
        return true;
    } else {
        print_line("[REAL GITHUB] Login failed - no 'login' field in response");
        if (data.has("message")) {
            print_line("[REAL GITHUB] Error message: " + (String)data["message"]);
        }
    }

    return false;
}

void GitIntegration::github_logout() {
    github_token = "";
    github_logged_in = false;
    current_username = "";
    print_line("[REAL GITHUB] Logged out");
}

Vector<GitIntegration::RepoData> GitIntegration::github_list_repos() {
    Vector<RepoData> repos;
    if (!github_logged_in) return repos;

    HashMap<String, String> headers;
    headers["Authorization"] = "token " + github_token;
    headers["User-Agent"] = "Real-Engine/1.0";

    String response = http_request("https://api.github.com/user/repos?per_page=100&sort=updated", headers);

    JSON json;
    json.parse(response);
    Array data = json.get_data();

    for (int i = 0; i < data.size(); i++) {
        Dictionary repo = data[i];
        RepoData rd;
        rd.name = repo["name"];
        rd.full_name = repo["full_name"];
        rd.description = repo.get("description", "");
        rd.url = repo["html_url"];
        rd.is_private = repo["private"];
        repos.push_back(rd);
    }

    return repos;
}

bool GitIntegration::github_create_repo(const String &p_name, const String &p_description, bool p_private) {
    if (!github_logged_in) return false;

    HashMap<String, String> headers;
    headers["Authorization"] = "token " + github_token;
    headers["User-Agent"] = "Real-Engine/1.0";
    headers["Content-Type"] = "application/json";

    Dictionary body;
    body["name"] = p_name;
    body["description"] = p_description;
    body["private"] = p_private;
    body["auto_init"] = true;

    String response = http_request("https://api.github.com/user/repos", headers, "POST", JSON::stringify(body));

    JSON json;
    json.parse(response);
    Dictionary data = json.get_data();

    if (data.has("id")) {
        print_line("[REAL GITHUB] Repository created: " + p_name);
        return true;
    } else {
        print_line("[REAL GITHUB] Failed to create repository");
        return false;
    }
}

bool GitIntegration::backup_project_to_github(const String &p_repo_name, const String &p_commit_message) {
    if (!github_logged_in) {
        print_line("[REAL GITHUB] Not logged in");
        return false;
    }

    if (!git_available) {
        print_line("[REAL GIT] Git not available");
        return false;
    }

    print_line("[REAL GITHUB] Creating backup repository: " + p_repo_name);

    if (!github_create_repo(p_repo_name, "Backup of " + project_path.get_file(), true)) {
        print_line("[REAL GITHUB] Failed to create repository");
        return false;
    }

    if (!FileAccess::exists(project_path + "/.git")) {
        init_repository();
    }

    add_all();

    commit(p_commit_message);

    String remote_url = "https://github.com/" + current_username + "/" + p_repo_name + ".git";

    // Удаляем старый remote если есть
    String remotes = execute_git_command("remote -v");
    if (remotes.contains("origin")) {
        (void)execute_git_command("remote remove origin"); // Игнорируем результат
    }

    (void)execute_git_command("remote add origin " + remote_url); // Игнорируем результат

    String branch = get_current_branch();
    String push_result = execute_git_command("push -u origin " + branch);

    if (push_result.contains("error")) {
        print_line("[GitHub] Push failed: " + push_result);
        return false;
    }

    print_line("[REAL GITHUB] Project backed up to: " + remote_url);
    return true;
}

void GitIntegration::show_backup_dialog(Control *p_parent) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Backup Project to GitHub");
    dialog->set_min_size(Size2(550, 400));
    dialog->set_size(Size2(550, 400));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    // Информация о проекте
    Label *info = memnew(Label);
    info->set_text("Project: " + project_path.get_file());
    info->set_theme_type_variation("HeaderSmall");
    vb->add_child(info);

    vb->add_child(memnew(HSeparator));

    // Получаем список репозиториев
    Vector<RepoData> repos = github_list_repos();

    // Выбор существующего репозитория
    Label *existing_label = memnew(Label);
    existing_label->set_text("Select existing repository:");
    vb->add_child(existing_label);

    OptionButton *repo_select = memnew(OptionButton);
    repo_select->add_item("-- Create new repository --", -1);
    for (int i = 0; i < repos.size(); i++) {
        String name = repos[i].name;
        if (repos[i].is_private) {
            name += " [PRIVATE]";
        }
        repo_select->add_item(name, i);
    }
    repo_select->select(0);
    vb->add_child(repo_select);

    vb->add_child(memnew(HSeparator));

    // Или создание нового
    Label *new_label = memnew(Label);
    new_label->set_text("Or create new:");
    vb->add_child(new_label);

    HBoxContainer *name_hb = memnew(HBoxContainer);
    name_hb->add_child(memnew(Label("Repo name:")));
    LineEdit *repo_edit = memnew(LineEdit);
    repo_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    repo_edit->set_text(project_path.get_file().to_lower().replace(" ", "-") + "-backup");
    name_hb->add_child(repo_edit);
    vb->add_child(name_hb);

    // Сообщение коммита
    HBoxContainer *msg_hb = memnew(HBoxContainer);
    msg_hb->add_child(memnew(Label("Commit message:")));
    LineEdit *msg_edit = memnew(LineEdit);
    msg_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);

    // Получаем текущее время
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    msg_edit->set_text("Backup " + String(buffer));
    msg_hb->add_child(msg_edit);
    vb->add_child(msg_hb);

    // Приватность
    CheckBox *private_cb = memnew(CheckBox);
    private_cb->set_text("Private repository (for new repos)");
    private_cb->set_pressed(true);
    vb->add_child(private_cb);

    // Информация
    Label *info_label = memnew(Label);
    info_label->set_text("Select an existing repo or enter a name for a new one.");
    info_label->set_modulate(Color(0.7, 0.7, 0.7));
    vb->add_child(info_label);

    // Кнопки
    HBoxContainer *btn_hb = memnew(HBoxContainer);
    btn_hb->set_alignment(BoxContainer::ALIGNMENT_CENTER);

    Button *backup_btn = memnew(Button);
    backup_btn->set_text("Start Backup");
    backup_btn->set_custom_minimum_size(Size2(150, 30));
    backup_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_enhanced_backup).bind(
        repo_select, repo_edit, msg_edit, private_cb, dialog));
    btn_hb->add_child(backup_btn);

    Button *cancel_btn = memnew(Button);
    cancel_btn->set_text("Cancel");
    cancel_btn->set_custom_minimum_size(Size2(100, 30));
    cancel_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    btn_hb->add_child(cancel_btn);

    vb->add_child(btn_hb);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::_on_enhanced_backup(OptionButton *repo_select, LineEdit *repo_edit,
                                          LineEdit *msg_edit, CheckBox *private_cb,
                                          AcceptDialog *dialog) {
    dialog->hide();

    int selected_idx = repo_select->get_selected_id();
    String commit_msg = msg_edit->get_text();

    // Показываем прогресс
    EditorNode::get_singleton()->show_accept("Starting backup...", "Please wait");

    if (selected_idx >= 0) {
        // Используем существующий репозиторий
        Vector<RepoData> repos = github_list_repos();
        if (selected_idx < repos.size()) {
            String repo_name = repos[selected_idx].name;
            bool success = backup_to_existing_repo(repo_name, commit_msg);

            if (success) {
                String message = "Backup completed successfully!\n\n";
                message += "Repository: " + current_username + "/" + repo_name + "\n";
                message += "Commit: " + commit_msg;
                EditorNode::get_singleton()->show_accept(message, "Success");
            } else {
                EditorNode::get_singleton()->show_accept("Backup failed!\nCheck console for details.", "Error");
            }
        }
    } else {
        String new_repo = repo_edit->get_text();
        bool is_private = private_cb->is_pressed();

        if (github_create_repo(new_repo, "Backup of " + project_path.get_file(), is_private)) {
            bool success = backup_to_existing_repo(new_repo, commit_msg);

            if (success) {
                String message = "New repository created and backed up!\n\n";
                message += "Repository: " + current_username + "/" + new_repo + "\n";
                message += "Commit: " + commit_msg;
                EditorNode::get_singleton()->show_accept(message, "Success");
            }
        } else {
            EditorNode::get_singleton()->show_accept("Failed to create repository!", "Error");
        }
    }
}

bool GitIntegration::backup_to_existing_repo(const String &p_repo_name, const String &p_commit_message) {
    if (!github_logged_in) {
        print_line("[REAL GITHUB] Not logged in");
        return false;
    }

    if (!git_available) {
        print_line("[REAL GIT] Git not available");
        return false;
    }

    print_line("[REAL GITHUB] Backing up to: " + p_repo_name);

    if (!FileAccess::exists(project_path + "/.git")) {
        init_repository();
    }

    add_all();

    commit(p_commit_message);

    String remote_url = "https://github.com/" + current_username + "/" + p_repo_name + ".git";

    // Удаляем старый remote если есть
    String remotes = execute_git_command("remote -v");
    if (remotes.contains("origin")) {
        (void)execute_git_command("remote remove origin");
    }

    (void)execute_git_command("remote add origin " + remote_url);

    String branch = get_current_branch();
    print_line("[GitHub] Pushing to " + remote_url + " branch " + branch);

    String push_result = execute_git_command("push -u origin " + branch);

    if (push_result.contains("error") || push_result.contains("failed")) {
        print_line("[GitHub] Push failed: " + push_result);
        return false;
    }

    print_line("[GitHub] Backup complete!");
    return true;
}

void GitIntegration::_on_backup_confirm(LineEdit *repo_edit, LineEdit *msg_edit, CheckBox *private_cb, AcceptDialog *dialog) {
    String repo_name = repo_edit->get_text();
    String commit_msg = msg_edit->get_text();
    bool is_private = private_cb->is_pressed();

    dialog->hide();

    if (backup_project_to_github(repo_name, commit_msg)) {
        String message = "Project backed up successfully!\n\n";
        message += "Repository: " + current_username + "/" + repo_name + "\n";
        message += "Commit: " + commit_msg;
        EditorNode::get_singleton()->show_accept(message, "Success");
    } else {
        String message = "Backup failed!\n\n";
        message += "Check console for details.";
        EditorNode::get_singleton()->show_accept(message, "Error");
    }
}

// ==================== UI Диалоги ====================

void GitIntegration::show_github_dialog(Control *p_parent) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("GitHub Integration");
    dialog->set_min_size(Size2(500, 300));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    if (!github_logged_in) {
        Label *info = memnew(Label);
        info->set_text("Enter your GitHub Personal Access Token\n"
                      "Create one at: https://github.com/settings/tokens\n"
                      "Required scopes: repo, read:user");
        vb->add_child(info);

        HBoxContainer *token_hb = memnew(HBoxContainer);
        token_hb->add_child(memnew(Label("Token:")));
        LineEdit *token_edit = memnew(LineEdit);
        token_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        token_edit->set_secret(true);
        token_hb->add_child(token_edit);
        vb->add_child(token_hb);

        Button *login_btn = memnew(Button);
        login_btn->set_text("Login");
        login_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_github_login).bind(token_edit, dialog));
        vb->add_child(login_btn);
    } else {
        Label *welcome = memnew(Label);
        welcome->set_text("Logged in as: " + current_username);
        vb->add_child(welcome);

        Button *logout_btn = memnew(Button);
        logout_btn->set_text("Logout");
        logout_btn->connect("pressed", callable_mp(this, &GitIntegration::github_logout));
        vb->add_child(logout_btn);
    }

    Button *close_btn = memnew(Button);
    close_btn->set_text("Close");
    close_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    vb->add_child(close_btn);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::_on_github_login(LineEdit *token_edit, AcceptDialog *dialog) {
    String token = token_edit->get_text();
    if (github_login(token)) {
        dialog->hide();
        // Используем EditorNode для получения parent
        show_github_dialog(EditorNode::get_singleton()->get_gui_base());
    }
}

// ==================== GitLab функции ====================

bool GitIntegration::gitlab_login(const String &p_token, const String &p_url) {
    gitlab_token = p_token;
    gitlab_url = p_url;

    HashMap<String, String> headers;
    headers["Authorization"] = "Bearer " + gitlab_token;
    headers["User-Agent"] = "Real-Engine/1.0";

    String response = http_request(gitlab_url + "/api/v4/user", headers);

    JSON json;
    json.parse(response);
    Dictionary data = json.get_data();

    if (data.has("username")) {
        gitlab_logged_in = true;
        current_username = data["username"];
        print_line("[REAL GITLAB] Logged in as: " + current_username);
        return true;
    }
    return false;
}

void GitIntegration::gitlab_logout() {
    gitlab_token = "";
    gitlab_url = "";
    gitlab_logged_in = false;
    current_username = "";
}

Vector<GitIntegration::RepoData> GitIntegration::gitlab_list_repos() {
    Vector<RepoData> repos;
    if (!gitlab_logged_in) return repos;

    HashMap<String, String> headers;
    headers["Authorization"] = "Bearer " + gitlab_token;
    headers["User-Agent"] = "Real-Engine/1.0";

    String response = http_request(gitlab_url + "/api/v4/projects?membership=true&per_page=100", headers);

    JSON json;
    json.parse(response);
    Array data = json.get_data();

    for (int i = 0; i < data.size(); i++) {
        Dictionary repo = data[i];
        RepoData rd;
        rd.name = repo["name"];
        rd.full_name = repo["path_with_namespace"];
        rd.description = repo.get("description", "");
        rd.url = repo["web_url"];
        rd.is_private = repo["visibility"] == "private";
        repos.push_back(rd);
    }

    return repos;
}

bool GitIntegration::gitlab_create_repo(const String &p_name, const String &p_description, bool p_private) {
    if (!gitlab_logged_in) return false;

    HashMap<String, String> headers;
    headers["Authorization"] = "Bearer " + gitlab_token;
    headers["User-Agent"] = "Real-Engine/1.0";
    headers["Content-Type"] = "application/json";

    Dictionary body;
    body["name"] = p_name;
    body["description"] = p_description;
    body["visibility"] = p_private ? "private" : "public";
    body["initialize_with_readme"] = true;

    String response = http_request(gitlab_url + "/api/v4/projects", headers, "POST", JSON::stringify(body));

    JSON json;
    json.parse(response);
    Dictionary data = json.get_data();

    if (data.has("id")) {
        print_line("[GitLab] Repository created: " + p_name);
        return true;
    }
    return false;
}

bool GitIntegration::backup_project_to_gitlab(const String &p_repo_name, const String &p_commit_message) {
    if (!gitlab_logged_in) {
        print_line("[REAL GITLAB] Not logged in");
        return false;
    }

    if (!git_available) {
        print_line("[REAL GIT] Git not available");
        return false;
    }

    print_line("[REAL GITLAB] Creating backup repository: " + p_repo_name);

    if (!gitlab_create_repo(p_repo_name, "Backup of " + project_path.get_file(), true)) {
        print_line("[REAL GITLAB] Failed to create repository");
        return false;
    }

    if (!FileAccess::exists(project_path + "/.git")) {
        init_repository();
    }

    add_all();

    commit(p_commit_message);

    String remote_url;
    if (gitlab_url == "https://gitlab.com") {
        remote_url = "https://gitlab.com/" + current_username + "/" + p_repo_name + ".git";
    } else {
        // Для self-hosted GitLab
        remote_url = gitlab_url + "/" + current_username + "/" + p_repo_name + ".git";
    }

    // Удаляем старый remote если есть
    String remotes = execute_git_command("remote -v");
    if (remotes.contains("origin")) {
        String result = execute_git_command("remote remove origin");
        // Игнорируем результат, но переменная нужна чтобы убрать warning
        if (result.is_empty()) {}
    }

    String result = execute_git_command("remote add origin " + remote_url);
    if (result.is_empty()) {}

    String branch = get_current_branch();
    String push_result = execute_git_command("push -u origin " + branch);

    if (push_result.contains("error")) {
        print_line("[REAL GITLAB] Push failed: " + push_result);
        return false;
    }

    print_line("[REAL GITLAB] Project backed up to: " + remote_url);
    return true;
}

void GitIntegration::show_gitlab_backup_dialog(Control *p_parent) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Backup Project to GitLab");
    dialog->set_min_size(Size2(500, 350));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    // Информация о проекте
    Label *info = memnew(Label);
    info->set_text("Project: " + project_path.get_file());
    vb->add_child(info);

    // Название репозитория
    HBoxContainer *name_hb = memnew(HBoxContainer);
    name_hb->add_child(memnew(Label("Repo name:")));
    LineEdit *repo_edit = memnew(LineEdit);
    repo_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    repo_edit->set_text(project_path.get_file().to_lower().replace(" ", "-"));
    name_hb->add_child(repo_edit);
    vb->add_child(name_hb);

    // Сообщение коммита
    HBoxContainer *msg_hb = memnew(HBoxContainer);
    msg_hb->add_child(memnew(Label("Commit message:")));
    LineEdit *msg_edit = memnew(LineEdit);
    msg_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    msg_edit->set_text("Backup " + Time::get_singleton()->get_datetime_string_from_system());
    msg_hb->add_child(msg_edit);
    vb->add_child(msg_hb);

    // Приватность
    CheckBox *private_cb = memnew(CheckBox);
    private_cb->set_text("Private repository");
    private_cb->set_pressed(true);
    vb->add_child(private_cb);

    // Информация о пользователе
    if (gitlab_logged_in) {
        Label *user_info = memnew(Label);
        user_info->set_text("Logged in as: " + current_username);
        user_info->set_modulate(Color(0.5, 1, 0.5));
        vb->add_child(user_info);
    }

    // Кнопки
    HBoxContainer *btn_hb = memnew(HBoxContainer);

    Button *backup_btn = memnew(Button);
    backup_btn->set_text("Create Backup");
    backup_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_gitlab_backup_confirm).bind(repo_edit, msg_edit, private_cb, dialog));
    btn_hb->add_child(backup_btn);

    Button *cancel_btn = memnew(Button);
    cancel_btn->set_text("Cancel");
    cancel_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    btn_hb->add_child(cancel_btn);

    vb->add_child(btn_hb);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::_on_gitlab_backup_confirm(LineEdit *repo_edit, LineEdit *msg_edit, CheckBox *private_cb, AcceptDialog *dialog) {
    String repo_name = repo_edit->get_text();
    String commit_msg = msg_edit->get_text();
    bool is_private = private_cb->is_pressed();

    dialog->hide();

    if (backup_project_to_gitlab(repo_name, commit_msg)) {
        String message = "Project backed up to GitLab successfully!\n\n";
        message += "Repository: " + current_username + "/" + repo_name + "\n";
        message += "Commit: " + commit_msg;
        EditorNode::get_singleton()->show_accept(message, "Success");
    } else {
        String message = "GitLab backup failed!\n\n";
        message += "Check console for details.";
        EditorNode::get_singleton()->show_accept(message, "Error");
    }
}

// ==================== Issues функции ====================

Vector<GitIntegration::IssueData> GitIntegration::list_issues(const String &p_repo) {
    Vector<IssueData> issues;

    if (!github_logged_in && !gitlab_logged_in) return issues;

    HashMap<String, String> headers;
    String url;

    if (github_logged_in) {
        headers["Authorization"] = "token " + github_token;
        url = "https://api.github.com/repos/" + current_username + "/" + p_repo + "/issues?state=all&per_page=100";
    } else if (gitlab_logged_in) {
        headers["Authorization"] = "Bearer " + gitlab_token;
        url = gitlab_url + "/api/v4/projects/" + current_username + "%2F" + p_repo + "/issues?per_page=100";
    }

    headers["User-Agent"] = "Real-Engine/1.0";

    String response = http_request(url, headers);

    JSON json;
    json.parse(response);
    Array data = json.get_data();

    for (int i = 0; i < data.size(); i++) {
        Dictionary issue = data[i];
        IssueData id;
        id.number = issue["number"];
        id.title = issue["title"];
        id.body = issue.get("body", "");
        id.state = issue["state"];
        issues.push_back(id);
    }

    return issues;
}

bool GitIntegration::create_issue(const String &p_repo, const String &p_title, const String &p_body) {
    if (!github_logged_in && !gitlab_logged_in) return false;

    HashMap<String, String> headers;
    String url;
    Dictionary body;
    body["title"] = p_title;
    body["body"] = p_body;

    if (github_logged_in) {
        headers["Authorization"] = "token " + github_token;
        url = "https://api.github.com/repos/" + current_username + "/" + p_repo + "/issues";
    } else if (gitlab_logged_in) {
        headers["Authorization"] = "Bearer " + gitlab_token;
        url = gitlab_url + "/api/v4/projects/" + current_username + "%2F" + p_repo + "/issues";
    }

    headers["User-Agent"] = "Real-Engine/1.0";
    headers["Content-Type"] = "application/json";

    String response = http_request(url, headers, "POST", JSON::stringify(body));

    JSON json;
    json.parse(response);
    Dictionary data = json.get_data();

    if (data.has("id") || data.has("number")) {
        print_line("[REAL GIT] Issue created: " + p_title);
        return true;
    }
    return false;
}

// ==================== UI Диалоги ====================

void GitIntegration::show_gitlab_dialog(Control *p_parent) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("GitLab Integration");
    dialog->set_min_size(Size2(500, 350));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    if (!gitlab_logged_in) {
        Label *info = memnew(Label);
        info->set_text("Enter your GitLab Personal Access Token\n"
                      "Create one at: https://gitlab.com/-/profile/personal_access_tokens\n"
                      "Required scopes: read_user, api");
        vb->add_child(info);

        HBoxContainer *token_hb = memnew(HBoxContainer);
        token_hb->add_child(memnew(Label("Token:")));
        LineEdit *token_edit = memnew(LineEdit);
        token_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        token_edit->set_secret(true);
        token_hb->add_child(token_edit);
        vb->add_child(token_hb);

        HBoxContainer *url_hb = memnew(HBoxContainer);
        url_hb->add_child(memnew(Label("URL (optional):")));
        LineEdit *url_edit = memnew(LineEdit);
        url_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        url_edit->set_text("https://gitlab.com");
        url_hb->add_child(url_edit);
        vb->add_child(url_hb);

        Button *login_btn = memnew(Button);
        login_btn->set_text("Login");
        login_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_gitlab_login).bind(token_edit, url_edit, dialog));
        vb->add_child(login_btn);
    } else {
        Label *welcome = memnew(Label);
        welcome->set_text("Logged in as: " + current_username);
        vb->add_child(welcome);

        Button *logout_btn = memnew(Button);
        logout_btn->set_text("Logout");
        logout_btn->connect("pressed", callable_mp(this, &GitIntegration::gitlab_logout));
        vb->add_child(logout_btn);
    }

    Button *close_btn = memnew(Button);
    close_btn->set_text("Close");
    close_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    vb->add_child(close_btn);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::show_create_repo_dialog(Control *p_parent) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Create Repository");
    dialog->set_min_size(Size2(500, 400));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    Label *platform_label = memnew(Label);
    platform_label->set_text("Select platform:");
    vb->add_child(platform_label);

    OptionButton *platform_cb = memnew(OptionButton);
    platform_cb->add_item("GitHub");
    platform_cb->add_item("GitLab");
    platform_cb->select(0);
    vb->add_child(platform_cb);

    HBoxContainer *name_hb = memnew(HBoxContainer);
    name_hb->add_child(memnew(Label("Name:")));
    LineEdit *name_edit = memnew(LineEdit);
    name_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    name_hb->add_child(name_edit);
    vb->add_child(name_hb);

    Label *desc_label = memnew(Label("Description:"));
    vb->add_child(desc_label);

    TextEdit *desc_edit = memnew(TextEdit);
    desc_edit->set_custom_minimum_size(Size2(0, 80));
    vb->add_child(desc_edit);

    CheckBox *private_cb = memnew(CheckBox);
    private_cb->set_text("Private repository");
    vb->add_child(private_cb);

    Button *create_btn = memnew(Button);
    create_btn->set_text("Create");
    create_btn->connect("pressed", callable_mp(this, &GitIntegration::_on_create_repo).bind(name_edit, desc_edit, private_cb, platform_cb, dialog));
    vb->add_child(create_btn);

    Button *close_btn = memnew(Button);
    close_btn->set_text("Cancel");
    close_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    vb->add_child(close_btn);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

void GitIntegration::show_issues_dialog(Control *p_parent, const String &p_repo) {
    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Issues - " + p_repo);
    dialog->set_min_size(Size2(600, 400));

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    // Получаем список issues
    Vector<IssueData> issues = list_issues(p_repo);

    if (issues.size() == 0) { // Если нет никаких проблем, то пишем, что их нет, что логично!
        Label *no_issues = memnew(Label);
        no_issues->set_text("No issues found");
        vb->add_child(no_issues);
    } else {
        Tree *tree = memnew(Tree);
        tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        tree->set_columns(3);
        tree->set_column_titles_visible(true);
        tree->set_column_title(0, "#");
        tree->set_column_title(1, "Title");
        tree->set_column_title(2, "State");

        TreeItem *root = tree->create_item();
        for (int i = 0; i < issues.size(); i++) {
            TreeItem *item = tree->create_item(root);
            item->set_text(0, itos(issues[i].number));
            item->set_text(1, issues[i].title);
            item->set_text(2, issues[i].state);
            if (issues[i].state == "open") {
                item->set_custom_color(2, Color(0, 1, 0));
            } else {
                item->set_custom_color(2, Color(1, 0, 0));
            }
        }
        vb->add_child(tree);
    }

    Button *close_btn = memnew(Button);
    close_btn->set_text("Close");
    close_btn->connect("pressed", callable_mp((Window*)dialog, &Window::hide));
    vb->add_child(close_btn);

    p_parent->add_child(dialog);
    dialog->popup_centered();
}

// Обработчики
void GitIntegration::_on_gitlab_login(LineEdit *token_edit, LineEdit *url_edit, AcceptDialog *dialog) {
    String token = token_edit->get_text();
    String url = url_edit->get_text();
    if (gitlab_login(token, url)) {
        dialog->hide();
        show_gitlab_dialog(EditorNode::get_singleton()->get_gui_base());
    }
}

void GitIntegration::_on_create_repo(LineEdit *name_edit, TextEdit *desc_edit, CheckBox *private_cb, OptionButton *platform_cb, AcceptDialog *dialog) {
    String name = name_edit->get_text();
    String desc = desc_edit->get_text();
    bool is_private = private_cb->is_pressed();
    int platform = platform_cb->get_selected();

    bool success = false;
    if (platform == 0 && github_logged_in) {
        success = github_create_repo(name, desc, is_private);
    } else if (platform == 1 && gitlab_logged_in) {
        success = gitlab_create_repo(name, desc, is_private);
    }

    if (success) {
        dialog->hide();
        EditorNode::get_singleton()->show_accept("Repository created successfully!", "Success");
        print_line("[REAL GIT]: Repo Created");
    }
}

void GitIntegration::_on_create_issue(LineEdit *title_edit, TextEdit *body_edit, const String &p_repo, AcceptDialog *dialog) {
    String title = title_edit->get_text();
    String body = body_edit->get_text();

    if (create_issue(p_repo, title, body)) {
        dialog->hide();
        show_issues_dialog(EditorNode::get_singleton()->get_gui_base(), p_repo);
    }
}
