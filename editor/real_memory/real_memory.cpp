/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_memory.h"
#include "core/string/print_string.h"
#include "editor/editor_node.h"
#include "editor/docks/scene_tree_dock.h"

void RealMemory::_bind_methods() {
    ClassDB::bind_method(D_METHOD("record_action", "type", "target", "properties"), &RealMemory::record_action);
    ClassDB::bind_method(D_METHOD("analyze_patterns"), &RealMemory::analyze_patterns);
    ClassDB::bind_method(D_METHOD("apply_suggestion", "suggestion"), &RealMemory::apply_suggestion);
}

RealMemory::RealMemory() {
    print_line("[REAL MEMORY]: The memory system is initialized");
}

RealMemory::~RealMemory() {
}

void RealMemory::record_action(const String &p_type, const String &p_target, const Dictionary &p_props) {
    Action act;
    act.type = p_type;
    act.target = p_target;
    act.properties = p_props;
    act.timestamp = OS::get_singleton()->get_ticks_msec();
    act.frequency = 1;
    
    action_history.push_back(act);
    
    // Если накопилось достаточно действий - анализируем
    if (action_history.size() % 50 == 0) {
        analyze_patterns();
    }
}

void RealMemory::analyze_patterns() {
    patterns.clear();
    
    // Ищем повторяющиеся последовательности
    for (int i = 0; i < action_history.size(); i++) {
        for (int j = i + 1; j < action_history.size(); j++) {
            if (action_history[i].type == action_history[j].type &&
                action_history[i].target == action_history[j].target) {
                
                String key = action_history[i].type + ":" + action_history[i].target;
                patterns[key].push_back(action_history[i]);
            }
        }
    }
    
    // Анализируем найденные паттерны
    for (const KeyValue<String, Vector<Action>> &E : patterns) {
        if (E.value.size() > 5) { // Если действие повторялось больше 5 раз
            _show_suggestion(
                "I noticed you often create " + E.key + ". Create a template?", // Надо ли создать шаблон?
                E.value[0].properties
            );
        }
    }
}

void RealMemory::_show_suggestion(const String &p_message, const Dictionary &p_action) {
    print_line("[REAL MEMORY]: " + p_message);
    // Пока просто выводим в консоль
}

void RealMemory::apply_suggestion(const Dictionary &p_suggestion) {
    String action_type = p_suggestion["type"];
    print_line("[REAL MEMORY]: Applying suggestion: " + action_type);
    
    // TODO: добавить реализацию создания шаблона
}
