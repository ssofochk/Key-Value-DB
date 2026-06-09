#include "commands.hpp"
#include "data_base.hpp"
#include "result_types.hpp"
#include "string_element.hpp"

#include <string>
#include <variant>
#include <vector>

Result Set(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    return data_base.PutElement(arguments[1], StringElement(arguments[2]));
}

Result Get(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        return NilResult{};
    }

    auto* element = std::get_if<StringElement>(value);

    if (!element) {
        return WrongType();
    }

    return StringResult(element->Get());
}

Result StrLen(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        return IntegerResult(0);
    }

    auto* element = std::get_if<StringElement>(value);

    if (!element) {
        return WrongType();
    }

    return IntegerResult(element->Size());
}

Result Append(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }
    
    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        Result result = data_base.PutElement(key, StringElement(arguments[2]));

        if (IsError(result)) {
            return result;
        }

        return IntegerResult(arguments[2].size());
    }

    return data_base.UpdateElement(key, [&](DataBase::Variant& value) -> Result {
        auto* element = std::get_if<StringElement>(&value);

        if (!element) {
            return WrongType();
        }

        element->Append(arguments[2]);

        return IntegerResult(element->Size());
    });
}