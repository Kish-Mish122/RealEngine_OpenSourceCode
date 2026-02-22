#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include "scene/gui/control.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/label.h"
#include "scene/gui/progress_bar.h"

class SplashScreen : public Control {
    GDCLASS(SplashScreen, Control);

private:
    TextureRect *logo = nullptr;
    Label *version_label = nullptr;
    Label *loading_label = nullptr;
    ProgressBar *progress = nullptr;
    float min_display_time = 2.0;
    float time_elapsed = 0.0;
    bool can_close_flag = false;

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    void set_logo(const Ref<Texture2D> &p_logo);
    void set_version(const String &p_version);
    void set_loading_text(const String &p_text);
    void set_progress(float p_progress);
    void close_splash();

    SplashScreen();
};

#endif
