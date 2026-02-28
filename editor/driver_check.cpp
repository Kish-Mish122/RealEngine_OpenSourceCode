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

void DriverCheck::_bind_methods() {}

DriverCheck::DriverCheck() {
    singleton = this;

    add_required_driver("NVIDIA", "560.0", false);
    add_required_driver("AMD", "24.10.0", false);
    add_required_driver("Intel", "30.0.100", false);
    add_required_driver("Vulkan", "1.3.0", true);

    // Проверка на базовый драйвер Windows
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
String get_nvidia_version() {
    HKEY hKey;
    char version[256] = "";
    DWORD size = sizeof(version);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\NVIDIA Corporation\\Global\\DriverVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueExA(hKey, "DriverVersion", nullptr, nullptr, (LPBYTE)version, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            print_line("[REAL DRIVER CHECK]: get drivers version (nvidia) - " + String(version));
            return String(version);
        }
        RegCloseKey(hKey);
    }
    return "";
}

String get_amd_version() {
    HKEY hKey;
    char version[256] = "";
    DWORD size = sizeof(version);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\AMD\\Driver", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueExA(hKey, "DriverVersion", nullptr, nullptr, (LPBYTE)version, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            print_line("[REAL DRIVER CHECK]: get drivers version (amd) - " + String(version));
            return String(version);
        }
        RegCloseKey(hKey);
    }
    return "";
}

String get_intel_version() {
    HKEY hKey;
    char version[256] = "";
    DWORD size = sizeof(version);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Intel\\Display", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueExA(hKey, "DriverVersion", nullptr, nullptr, (LPBYTE)version, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            print_line("[REAL DRIVER CHECK]: get drivers version (intel) - " + String(version));
            return String(version);
        }
        RegCloseKey(hKey);
    }
    return "";
}

String get_vulkan_version() {
    HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
    if (!vulkan) return "";

    typedef uint32_t (*PFN_vkEnumerateInstanceVersion)(uint32_t*);
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)GetProcAddress(vulkan, "vkEnumerateInstanceVersion");

    uint32_t version = 0;
    if (vkEnumerateInstanceVersion && vkEnumerateInstanceVersion(&version) == 0) {
        FreeLibrary(vulkan);
        uint32_t major = version >> 22;
        uint32_t minor = (version >> 12) & 0x3FF;
        uint32_t patch = version & 0xFFF;
        String ver = vformat("%d.%d.%d", major, minor, patch);
        print_line("[REAL DRIVER CHECK]: get drivers version (vulkan) - " + ver);
        return ver;
    }

    FreeLibrary(vulkan);
    return "";
}

#ifdef WINDOWS_ENABLED
String get_gpu_name() {
    HKEY hKey;
    char gpu_name[256] = "";
    DWORD size = sizeof(gpu_name);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueExA(hKey, "DriverDesc", nullptr, nullptr, (LPBYTE)gpu_name, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return String(gpu_name);
        }
        RegCloseKey(hKey);
    }
    return "";
}
#endif // WINDOWS_ENABLED

String DriverCheck::get_system_driver_version(const String &p_driver_name) {
#ifdef WINDOWS_ENABLED
    if (p_driver_name == "NVIDIA") return get_nvidia_version();
    if (p_driver_name == "AMD") return get_amd_version();
    if (p_driver_name == "Intel") return get_intel_version();
    if (p_driver_name == "Vulkan") return get_vulkan_version();
#endif
    return "";
}

bool compare_versions(const String &p_version, const String &p_min_version) {
    Vector<String> ver_parts = p_version.split(".");
    Vector<String> min_parts = p_min_version.split(".");

    for (int i = 0; i < min_parts.size() && i < ver_parts.size(); i++) {
        float v = ver_parts[i].to_float();
        float m = min_parts[i].to_float();
        if (v < m) return false;
        if (v > m) return true;
    }
    return ver_parts.size() >= min_parts.size();
}

bool is_basic_display_driver() {
    String gpu_name = get_gpu_name().to_lower();
    print_line("[REAL DRIVER CHECK]: GPU name - " + gpu_name);

    return gpu_name.contains("microsoft basic display") ||
           gpu_name.contains("standard vga") ||
           gpu_name.contains("basic display");
}
#endif

void DriverCheck::check_drivers() {
    missing_drivers.clear();
    outdated_drivers.clear();

    // Проверка базового драйвера Windows
    if (is_basic_display_driver()) {
        outdated_drivers.push_back("Microsoft Basic Display Adapter (No hardware acceleration)");
    }

    for (const DriverInfo &info : required_drivers) {
        if (info.is_basic) continue;

        String version = get_system_driver_version(info.name);

        if (version.is_empty()) {
            if (info.required) {
                missing_drivers.push_back(info.name);
            }
        } else if (!compare_versions(version, info.min_version)) {
            outdated_drivers.push_back(info.name + " (v" + version + " < " + info.min_version + ")");
        }
    }
}

void DriverCheck::show_driver_warning(Control *p_parent) {
    check_drivers();

    if (missing_drivers.is_empty() && outdated_drivers.is_empty()) {
        return;
    }

    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("Driver Warning");
    dialog->set_min_size(Size2(600, 400) * EDSCALE);
    dialog->set_exclusive(false);

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    RichTextLabel *info = memnew(RichTextLabel);
    info->set_use_bbcode(true);
    info->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    vb->add_child(info);

    String message;

    // Проверяем наличие базового драйвера
    bool has_basic = false;
    for (const String &d : outdated_drivers) {
        if (d.contains("Microsoft Basic Display Adapter")) {
            has_basic = true;
            break;
        }
    }

    // Опа-па... Стандартный драйвер обнаружен! Показываем ошибку!
    if (has_basic) {
        message += "[b][color=red]ERROR: The Standard Windows Video Driver Has Been Detected![/color][/b]\n\n";
        message += "Real Engine has discovered the Standard Windows Video Driver! You need it to display images, even if you don't have a driver for the graphics card!\n\n";
        message += "But it gets in the way a lot!\n\n";
        message += "Please download the driver to your graphics card for the full functionality of the Real Engine!\n\n";
        message += "[b]REQUIRED ACTION:[/b] You can download it here:\n";
        message += "  • [url=https://www.nvidia.com/Download/index.aspx]NVIDIA Drivers[/url]\n";
        message += "  • [url=https://www.amd.com/en/support]AMD Drivers[/url]\n";
        message += "  • [url=https://www.intel.com/content/www/us/en/support/detect.html]Intel Drivers[/url]\n\n";
    }

    // Сообщение - "Драйвера - нет"
    if (!missing_drivers.is_empty()) {
        message += "[b][color=orange]Missing Drivers:[/color][/b]\n";
        for (const String &d : missing_drivers) {
            message += "  • " + d + "\n";
        }
        message += "\n";
    }

    // Сообщение "Драйвер(а) - устарел(и)"
    if (!outdated_drivers.is_empty() && !has_basic) {
        message += "[b][color=yellow]Outdated Drivers:[/color][/b]\n";
        for (const String &d : outdated_drivers) {
            message += "  • " + d + "\n";
        }
        message += "\n";
    }

    message += "[color=white]For optimal performance, please install the latest drivers from your GPU manufacturer's website.[/color]";

    info->append_text(message);

    // Добавляем ссылки как кликабельные
    info->connect("meta_clicked", callable_mp(this, &DriverCheck::_on_link_clicked));

    CheckBox *dont_show = memnew(CheckBox);
    dont_show->set_text("Don't show this warning again");
    vb->add_child(dont_show);

    dialog->connect(SceneStringName(confirmed), Callable(dialog, "queue_free"));

    if (p_parent) {
        p_parent->add_child(dialog);
    }

    dialog->call_deferred("popup_centered");
}

// Добавьте обработчик кликов по ссылкам
void DriverCheck::_on_link_clicked(const String &p_url) {
    OS::get_singleton()->shell_open(p_url);
}

// OpenGL я решил не делать, так как слишком сложно. Лучше всего этого не делать, вместо того что перелопатить весь проект, сломать и снова переделывать.
