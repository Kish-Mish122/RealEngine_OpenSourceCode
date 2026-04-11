#include "money_gimmick.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "core/string/print_string.h"
#include "scene/gui/box_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"

void MoneyGimmick::_bind_methods() {
}

MoneyGimmick::MoneyGimmick() {
    print_line("MoneyGimmick: constructor");
    set_title("Cash out the money");
    set_min_size(Size2(350, 300) * EDSCALE);

    VBoxContainer *vbox = memnew(VBoxContainer);
    vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    add_child(vbox);

    // Номер карты
    Label *card_label = memnew(Label);
    card_label->set_text("Card number:");
    vbox->add_child(card_label);

    card_number_edit = memnew(LineEdit);
    card_number_edit->set_placeholder("1234 5678 9012 3456");
    card_number_edit->connect("text_changed", callable_mp(this, &MoneyGimmick::_on_card_number_changed));
    vbox->add_child(card_number_edit);

    // Тип карты
    card_type_label = memnew(Label);
    card_type_label->set_text("");
    vbox->add_child(card_type_label);

    // CVV
    Label *cvv_label = memnew(Label);
    cvv_label->set_text("CVV:");
    vbox->add_child(cvv_label);

    cvv_edit = memnew(LineEdit);
    cvv_edit->set_placeholder("123");
    cvv_edit->set_max_length(3);
    vbox->add_child(cvv_edit);

    // Срок годности
    Label *expiry_label = memnew(Label);
    expiry_label->set_text("Expiration date (MM/YY):");
    vbox->add_child(expiry_label);

    expiry_edit = memnew(LineEdit);
    expiry_edit->set_placeholder("12/26");
    expiry_edit->set_max_length(5);
    vbox->add_child(expiry_edit);

    vbox->add_child(memnew(HSeparator));

    cashout_button = memnew(Button);
    cashout_button->set_text("Cash out");
    cashout_button->connect("pressed", callable_mp(this, &MoneyGimmick::_on_cashout_pressed));
    vbox->add_child(cashout_button);

    // Кнопка OK (унаследована от AcceptDialog) скрыта, мы используем свою.
    get_ok_button()->hide();
}

void MoneyGimmick::show_dialog() {
    if (!EditorNode::get_singleton()->get_gui_base()) {
        callable_mp(this, &MoneyGimmick::show_dialog).call_deferred();
        return;
    }
    card_number_edit->clear();
    cvv_edit->clear();
    expiry_edit->clear();
    card_type_label->set_text("");
    popup_centered();
}

void MoneyGimmick::_on_card_number_changed(const String &p_text) {
    String cleaned = p_text.replace(" ", "");
    String card_type = _detect_card_type(cleaned);
    if (!card_type.is_empty()) {
        card_type_label->set_text("Card type: " + card_type);
    } else {
        card_type_label->set_text("");
    }
}

String MoneyGimmick::_detect_card_type(const String &p_number) {
    if (p_number.is_empty()) return "";
    if (p_number.begins_with("4")) return "Visa";
    if (p_number.begins_with("5")) return "Mastercard";
    if (p_number.begins_with("2")) return "MIR";
    return "";
}

bool MoneyGimmick::_is_luhn_valid(const String &p_number) {
    if (p_number.length() < 13 || p_number.length() > 19) return false;
    return true;
}

void MoneyGimmick::_on_cashout_pressed() {
    _validate_and_cashout();
}

void MoneyGimmick::_validate_and_cashout() {
    String card_number = card_number_edit->get_text().replace(" ", "");
    String cvv = cvv_edit->get_text();
    String expiry = expiry_edit->get_text();

    // Простые проверки
    if (card_number.is_empty() || cvv.is_empty() || expiry.is_empty()) {
        EditorNode::get_singleton()->show_warning("Fill in all the fields!");
        return;
    }
    if (cvv.length() != 3) {
        EditorNode::get_singleton()->show_warning("CVV must consist of 3 digits.");
        return;
    }
    if (expiry.length() != 5 || expiry[2] != '/') {
        EditorNode::get_singleton()->show_warning("Incorrect date format (MM/YY).");
        return;
    }
    if (!_is_luhn_valid(card_number)) {
        EditorNode::get_singleton()->show_warning("The card number was entered incorrectly.");
        return;
    }
    if (_detect_card_type(card_number).is_empty()) {
        EditorNode::get_singleton()->show_warning("The card type is not recognized.");
        return;
    }

    // Если всё "правильно", показываем предупреждение
    AcceptDialog *warning = memnew(AcceptDialog);
    warning->set_title("Attention!");
    warning->set_text("Article: Article 174 of the Criminal Code of the Russian Federation. Legalization (laundering) of funds or other property acquired by other persons through criminal means.");
    warning->set_ok_button_text("I got it all figured out");
    EditorNode::get_singleton()->get_gui_base()->add_child(warning);
    warning->popup_centered();
}
