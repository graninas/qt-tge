// translation_types.h
#pragma once
#include <string>
#include <variant>
#include <optional>

namespace tge::formula::translation {


struct Value {
    enum class Type { Int, Float, Bool, Error };
    Type type;
    int intValue = 0;
    double floatValue = 0.0;
    bool boolValue = false;
    // For now, only one value is valid depending on type
};


} // namespace tge::formula::translation
