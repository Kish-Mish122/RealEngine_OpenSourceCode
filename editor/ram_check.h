#ifndef RAM_CHECK_H
#define RAM_CHECK_H

#include "core/object/ref_counted.h"
#include "scene/gui/dialogs.h"

class RAMCheck : public RefCounted {
    GDCLASS(RAMCheck, RefCounted);

    static RAMCheck *singleton;
    uint64_t min_ram_mb = 2048; // Самый минимум - 2GB. Это пипец вообще...
    uint64_t warning_ram_mb = 4096; // Предупреждение - 4GB. Стандарт, но мало...

protected:
    static void _bind_methods();

public:
    static RAMCheck *get_singleton() { return singleton; }

    uint64_t get_total_ram_mb();
    uint64_t get_available_ram_mb();
    void show_ram_warning(Control *p_parent);
    void check_ram_at_startup(Control *p_parent);

    RAMCheck();
    ~RAMCheck();
};

#endif // RAM_CHECK_H
