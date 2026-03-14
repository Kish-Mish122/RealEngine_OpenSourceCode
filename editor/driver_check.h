#ifndef DRIVER_CHECK_H
#define DRIVER_CHECK_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "scene/gui/dialogs.h"

// Vulkan version macros
#ifndef VK_MAKE_VERSION
#define VK_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))
#endif

#ifndef VK_VERSION_MAJOR
#define VK_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#endif
#ifndef VK_VERSION_MINOR
#define VK_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FF)
#endif
#ifndef VK_VERSION_PATCH
#define VK_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFF)
#endif

class DriverCheck : public RefCounted {
    GDCLASS(DriverCheck, RefCounted);

    struct DriverInfo {
        String name;
        String min_version;
        bool required = true;
        bool is_basic = false;
    };

    Vector<DriverInfo> required_drivers;
    Vector<String> missing_drivers;
    Vector<String> outdated_drivers;

    static DriverCheck *singleton;

protected:
    static void _bind_methods();
    void _on_link_clicked(const String &p_url);
    void _on_dialog_confirmed();
    void _on_dialog_popup_hide();
    void _on_dont_show_toggled(bool p_pressed);

private:
    static bool compare_versions(const String &p_version, const String &p_min_version);
    AcceptDialog *current_dialog = nullptr;

public:
    static DriverCheck *get_singleton() { return singleton; }

    void add_required_driver(const String &p_name, const String &p_min_version, bool p_required = true);
    void check_drivers();
    String get_system_driver_version(const String &p_driver_name);
    void show_driver_warning(Control *p_parent);

    DriverCheck();
    ~DriverCheck();
};


#endif // DRIVER_CHECK_H
