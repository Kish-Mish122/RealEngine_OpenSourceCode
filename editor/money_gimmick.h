#pragma once

#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/box_container.h"
#include "scene/gui/label.h"

class MoneyGimmick : public AcceptDialog {
    GDCLASS(MoneyGimmick, AcceptDialog);

private:
    LineEdit *card_number_edit;
    LineEdit *cvv_edit;
    LineEdit *expiry_edit;
    Label *card_type_label;
    Button *cashout_button;

    void _on_card_number_changed(const String &p_text);
    void _on_cashout_pressed();
    void _validate_and_cashout();
    String _detect_card_type(const String &p_number);
    bool _is_luhn_valid(const String &p_number);

protected:
    static void _bind_methods();

public:
    MoneyGimmick();
    void show_dialog();
};
