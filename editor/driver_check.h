#ifndef DRIVER_CHECK_H
#define DRIVER_CHECK_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "scene/gui/dialogs.h"

class DriverCheck : public RefCounted {
    GDCLASS(DriverCheck, RefCounted);

    struct DriverInfo {
        String name; // <-- Это название драйвера. String нужен для того, чтобы в сообщении было название драйвера
        String min_version; // <-- Это минимальная версия драйвера. String нужен для указание минимальной версии драйвера
        bool required = true;
        bool is_basic = false; // <-- Это классический драйвер Windows. Он нужен для того, чтобы была картинка, даже если нет драйвера на видеокарту
    };

    Vector<DriverInfo> required_drivers;
    Vector<String> missing_drivers;
    Vector<String> outdated_drivers;

    static DriverCheck *singleton;

protected:
    static void _bind_methods();

public:
    static DriverCheck *get_singleton() { return singleton; }

    void add_required_driver(const String &p_name, const String &p_min_version, bool p_required = true);
    void check_drivers();
    void _on_link_clicked(const String &p_url);
    String get_system_driver_version(const String &p_driver_name);
    void show_driver_warning(Control *p_parent);

    DriverCheck();
    ~DriverCheck();
};

#endif // DRIVER_CHECK_H
