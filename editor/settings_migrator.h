#ifndef SETTINGS_MIGRATOR_H
#define SETTINGS_MIGRATOR_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/io/file_access.h"
#include "core/io/dir_access.h"
#include "core/io/json.h"
#include "core/version.h"

class SettingsMigrator : public RefCounted {
    GDCLASS(SettingsMigrator, RefCounted);

    struct OldSettingsFile {
        String path;
        String version;
        int version_major;
        int version_minor;
        uint64_t modified_time;
    };

    static SettingsMigrator *singleton;

protected:
    static void _bind_methods();

public:
    static SettingsMigrator *get_singleton() { return singleton; }

    SettingsMigrator();
    ~SettingsMigrator();

    // Поиск старых настроек
    Vector<OldSettingsFile> find_old_settings();
    void migrate_from_version(const String &p_old_path);
    void migrate_all();  // ТОЛЬКО ОДИН РАЗ!

    // Перенос конкретных настроек
    void migrate_editor_settings(const String &p_old_path);
    void migrate_project_times();

    // Версионирование
    int get_current_version_major() const;
    int get_current_version_minor() const;

    // Очистка (опционально)
    void cleanup_old_settings(bool p_keep_latest = true);
    void set_auto_cleanup(bool p_enabled);
};

#endif // SETTINGS_MIGRATOR_H
