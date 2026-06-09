#include <string>
#include <variant>
#include <vector>

template <class Storage>
Result Set(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    return data_base.PutElement(arguments[1], StringElement(arguments[2]));
}

template <class Storage>
Result Get(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
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

template <class Storage>
Result StrLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
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

template <class Storage>
Result Append(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
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

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* element = std::get_if<StringElement>(&value);

        if (!element) {
            return WrongType();
        }

        element->Append(arguments[2]);

        return IntegerResult(element->Size());
    });
}