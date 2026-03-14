#include "driver_check.h"
#include "core/os/os.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "scene/gui/check_box.h"
#include "scene/gui/rich_text_label.h"
#include "core/string/print_string.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#include <d3d9.h>
#include <dxgi.h>
#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>
#include <d3dcommon.h>
#endif

DriverCheck *DriverCheck::singleton = nullptr;

void DriverCheck::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_on_link_clicked", "url"), &DriverCheck::_on_link_clicked);
    ClassDB::bind_method(D_METHOD("_on_dialog_confirmed"), &DriverCheck::_on_dialog_confirmed);
    ClassDB::bind_method(D_METHOD("_on_dialog_popup_hide"), &DriverCheck::_on_dialog_popup_hide);
    ClassDB::bind_method(D_METHOD("_on_dont_show_toggled", "pressed"), &DriverCheck::_on_dont_show_toggled);
}

DriverCheck::DriverCheck() {
    singleton = this;

    add_required_driver("NVIDIA", "560.0", false);
    add_required_driver("AMD", "24.10.0", false);
    add_required_driver("Intel", "30.0.100", false);
    add_required_driver("Vulkan", "1.3.0", true);

    DriverInfo basic;
    basic.name = "Microsoft Basic Display Adapter";
    basic.min_version = "0.0";
    basic.required = false;
    basic.is_basic = true;
    required_drivers.push_back(basic);
}

DriverCheck::~DriverCheck() {
    singleton = nullptr;
}

void DriverCheck::add_required_driver(const String &p_name, const String &p_min_version, bool p_required) {
    DriverInfo info;
    info.name = p_name;
    info.min_version = p_min_version;
    info.required = p_required;
    required_drivers.push_back(info);
}

#ifdef WINDOWS_ENABLED

String get_registry_string(HKEY hRootKey, const char* pSubKey, const char* pValueName) {
    HKEY hKey;
    char buffer[512] = {0};
    DWORD size = sizeof(buffer);

    LONG result = RegOpenKeyExA(hRootKey, pSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
    if (result != ERROR_SUCCESS) {
        return "";
    }

    result = RegQueryValueExA(hKey, pValueName, nullptr, nullptr, (LPBYTE)buffer, &size);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        return "";
    }

    return String(buffer);
}

String get_nvidia_version() {
    String version = get_registry_string(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\NVIDIA Corporation\\Global\\DriverVersion",
        "DriverVersion");
    if (!version.is_empty()) {
        print_line("[REAL DRIVER CHECK]: NVIDIA Driver Version - " + version);
    }
    return version;
}

String get_amd_version() {
    String version = get_registry_string(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\AMD\\Driver", "DriverVersion");
    if (version.is_empty()) {
        version = get_registry_string(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\AMD\\RyzenChipset", "DriverVersion");
    }
    if (!version.is_empty()) {
        print_line("[REAL DRIVER CHECK]: AMD Driver Version - " + version);
    }
    return version;
}

String get_intel_version() {
    String version = get_registry_string(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Intel\\GMM", "DriverVersion");
    if (version.is_empty()) {
        version = get_registry_string(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\WOW6432Node\\Intel\\GMM", "DriverVersion");
    }
    if (!version.is_empty()) {
        print_line("[REAL DRIVER CHECK]: Intel Driver Version - " + version);
    }
    return version;
}

String get_vulkan_version() {
    HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
    if (!vulkan) return "";

    typedef uint32_t (*PFN_vkEnumerateInstanceVersion)(uint32_t*);
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)GetProcAddress(vulkan, "vkEnumerateInstanceVersion");

    uint32_t version = 0;
    bool success = false;

    if (vkEnumerateInstanceVersion && vkEnumerateInstanceVersion(&version) == 0) {
        success = true;
    } else {
        version = VK_MAKE_VERSION(1, 0, 0);
        success = true;
    }

    FreeLibrary(vulkan);

    if (success) {
        uint32_t major = VK_VERSION_MAJOR(version);
        uint32_t minor = VK_VERSION_MINOR(version);
        uint32_t patch = VK_VERSION_PATCH(version);
        String ver = vformat("%d.%d.%d", major, minor, patch);
        print_line("[REAL DRIVER CHECK]: Vulkan Version - " + ver);
        return ver;
    }

    return "";
}

String get_gpu_name() {
    for (int i = 0; i < 5; i++) {
        char subkey[256];
        sprintf_s(subkey, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\%04d", i);

        String name = get_registry_string(HKEY_LOCAL_MACHINE, subkey, "DriverDesc");
        if (!name.is_empty()) {
            print_line("[REAL DRIVER CHECK]: GPU Adapter %d: %s", i, name.utf8().get_data());
            return name;
        }
    }
    return "Unknown GPU";
}

bool is_basic_display_driver() {
    String gpu_name = get_gpu_name().to_lower();
    print_line("[REAL DRIVER CHECK]: Detected GPU: " + gpu_name);

    return gpu_name.contains("microsoft basic display") ||
           gpu_name.contains("standard vga") ||
           gpu_name.contains("basic display") ||
           gpu_name.contains("svga");
}

#endif // WINDOWS_ENABLED

bool DriverCheck::compare_versions(const String &p_version, const String &p_min_version) {
    Vector<String> ver_parts = p_version.split(".");
    Vector<String> min_parts = p_min_version.split(".");

    int count = MAX(ver_parts.size(), min_parts.size());
    for (int i = 0; i < count; i++) {
        float v = (i < ver_parts.size()) ? ver_parts[i].to_float() : 0;
        float m = (i < min_parts.size()) ? min_parts[i].to_float() : 0;
        if (v < m) return false;
        if (v > m) return true;
    }
    return true;
}

String DriverCheck::get_system_driver_version(const String &p_driver_name) {
#ifdef WINDOWS_ENABLED
    if (p_driver_name == "NVIDIA") return get_nvidia_version();
    if (p_driver_name == "AMD") return get_amd_version();
    if (p_driver_name == "Intel") return get_intel_version();
    if (p_driver_name == "Vulkan") return get_vulkan_version();
    return "";
#else
    return ""; // Для Linux/macOS — заглушка
#endif
}

void DriverCheck::check_drivers() {
    missing_drivers.clear();
    outdated_drivers.clear();

#ifdef WINDOWS_ENABLED
    if (is_basic_display_driver()) {
        outdated_drivers.push_back("Microsoft Basic Display Adapter (No hardware acceleration)");
    }
#endif

    for (const DriverInfo &info : required_drivers) {
        if (info.is_basic) continue;

        String version = get_system_driver_version(info.name);

        if (version.is_empty()) {
            if (info.required) {
                missing_drivers.push_back(info.name);
            }
        } else if (!compare_versions(version, info.min_version)) {
            outdated_drivers.push_back(vformat("%s (v%s < %s)", info.name, version, info.min_version));
        }
    }
}

void DriverCheck::_on_link_clicked(const String &p_url) {
    OS::get_singleton()->shell_open(p_url);
}

void DriverCheck::_on_dialog_confirmed() {
    if (current_dialog) {
        current_dialog->hide();
    }
}

void DriverCheck::_on_dialog_popup_hide() {
    if (current_dialog) {
        current_dialog->queue_free();
        current_dialog = nullptr;
    }
}

void DriverCheck::_on_dont_show_toggled(bool p_pressed) {
    Ref<EditorSettings> settings = EditorSettings::get_singleton();
    if (settings.is_valid() && p_pressed) {
        settings->set("editor/driver_warning_disabled", true);
        settings->save();
        print_line("[REAL DRIVER CHECKER]: Checker driver - off!");
    }
}

void DriverCheck::show_driver_warning(Control *p_parent) {
    check_drivers();

    if (missing_drivers.is_empty() && outdated_drivers.is_empty()) {
        return;
    }

    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Driver Warning");
    dialog->set_min_size(Size2(600, 400) * MAX(1.0f, EDSCALE));
    dialog->set_exclusive(false);

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    RichTextLabel *info = memnew(RichTextLabel);
    info->set_use_bbcode(true);
    info->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    vb->add_child(info);

    String message;

    bool has_basic = false;
    for (const String &d : outdated_drivers) {
        if (d.contains("Microsoft Basic Display Adapter")) {
            has_basic = true;
            break;
        }
    }

    if (has_basic) {
        message += "[b][color=red]ERROR: The Standard Windows Video Driver Has Been Detected![/color][/b]\n\n";
        message += "Real Engine has detected the Standard Windows Video Driver.\n";
        message += "This driver provides only basic display functionality and no hardware acceleration.\n\n";
        message += "[b]ACTION REQUIRED:[/b] Install proper GPU drivers:\n";
        message += "  • [url=https://www.nvidia.com/Download/index.aspx]NVIDIA Drivers[/url]\n";
        message += "  • [url=https://www.amd.com/en/support]AMD Drivers[/url]\n";
        message += "  • [url=https://www.intel.com/content/www/us/en/support/detect.html]Intel Drivers[/url]\n\n";
    }

    if (!missing_drivers.is_empty()) {
        message += "[b][color=orange]Missing Drivers:[/color][/b]\n";
        for (const String &d : missing_drivers) {
            message += "  • " + d + "\n";
        }
        message += "\n";
    }

    if (!outdated_drivers.is_empty() && !has_basic) {
        message += "[b][color=yellow]Outdated Drivers:[/color][/b]\n";
        for (const String &d : outdated_drivers) {
            message += "  • " + d + "\n";
        }
        message += "\n";
    }

    message += "For optimal performance, install the latest drivers from your GPU manufacturer.";

    info->set_text(message);
    info->connect("meta_clicked", callable_mp(this, &DriverCheck::_on_link_clicked));

    CheckBox *dont_show = memnew(CheckBox);
    dont_show->set_text("Don't show this warning again");
    vb->add_child(dont_show);

    Ref<EditorSettings> settings = EditorSettings::get_singleton();
    String setting_key = "editor/driver_warning_disabled";

    if (settings.is_valid() && settings->has_setting(setting_key) && bool(settings->get(setting_key))) {
        memdelete(dialog);
        return;
    }

    current_dialog = dialog;
    dialog->connect("confirmed", callable_mp(this, &DriverCheck::_on_dialog_confirmed));
    dialog->connect("popup_hide", callable_mp(this, &DriverCheck::_on_dialog_popup_hide));
    dont_show->connect("toggled", callable_mp(this, &DriverCheck::_on_dont_show_toggled));

    if (p_parent) {
        p_parent->add_child(dialog);
        dialog->popup_centered_clamped(Size2(600, 400) * EDSCALE, 0.8);
    } else {
        dialog->popup_centered();
    }
}
