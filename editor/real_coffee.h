/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "scene/main/node.h"
#include "scene/main/timer.h"

class RealCoffee : public Node {
    GDCLASS(RealCoffee, Node);

private:
    Timer *coffee_timer = nullptr;
    bool enabled = true;
    int interval_minutes = 60;
    bool show_notifications = true;

    void _show_notification();
    void _on_timer_timeout();
    void _load_settings();

protected:
    static void _bind_methods();

public:
    RealCoffee();
    ~RealCoffee();

    void start_timer();
    void stop_timer();
    void restart_timer();

    void set_enabled(bool p_enabled);
    bool is_enabled() const { return enabled; }

    void set_interval(int p_minutes);
    int get_interval() const { return interval_minutes; }

    // Убираем override
    virtual void _notification(int p_what);
};
