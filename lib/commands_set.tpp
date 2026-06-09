#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <class Storage>
Result SAdd(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        SetElement set;
        int added = 0;

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            added += set.Add(arguments[i]);
        }

        Result result = data_base.PutElement(key, std::move(set));

        if (IsError(result)) {
            return result;
        }

        return IntegerResult(added);
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* set_value = std::get_if<SetElement>(&value);
        if (!set_value) {
            return WrongType();
        }

        int added = 0;

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            added += set_value->Add(arguments[i]);
        }

        return IntegerResult(added);
    });
}

template <class Storage>
Result SRem(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        return IntegerResult(0);
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* set_value = std::get_if<SetElement>(&value);

        if (!set_value) {
            return WrongType();
        }

        int removed = 0;

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            removed += set_value->Remove(arguments[i]);
        }

        return IntegerResult(removed);
    });
}

template <class Storage>
Result SIsMember(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];
    const std::string& member = arguments[2];

    auto* value = data_base.GetElement(key);
    if (!value) {
        return IntegerResult(0);
    }

    auto* set_value = std::get_if<SetElement>(value);
    if (!set_value) {
        return WrongType();
    }

    return IntegerResult(set_value->Contains(member) ? 1 : 0);
}

template <class Storage>
Result SMembers(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }
    const std::string& key = arguments[1];

    auto* value = data_base.GetElement(key);
    if (!value) {
        return ListResult{};
    }

    auto* set_value = std::get_if<SetElement>(value);
    if (!set_value) {
        return WrongType();
    }

    return ListResult(set_value->GetAll());
}

template <class Storage>
Result SCard(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }
    const std::string& key = arguments[1];

    auto* value = data_base.GetElement(key);
    if (!value) {
        return IntegerResult(0);
    }

    auto* set_value = std::get_if<SetElement>(value);
    if (!set_value) {
        return WrongType();
    }

    return IntegerResult(set_value->Size());
}

template <class Storage>
Result SUnion(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    SetElement result;

    for (std::size_t i = 1; i < arguments.size(); ++i) {
        const std::string& key = arguments[i];

        auto* value = data_base.GetElement(key);
        if (!value) {
            continue;
        }

        auto* set_value = std::get_if<SetElement>(value);
        if (!set_value) {
            return WrongType();
        }

        result.Unite(*set_value);
    }

    return ListResult(result.GetAll());
}

template <class Storage>
Result SInter(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    if (arguments.size() <= 1) {
        return ListResult{};
    }

    auto* value = data_base.GetElement(arguments[1]);
    if (!value) {
        return ListResult{};
    }

    auto* res_set = std::get_if<SetElement>(value);
    if (!res_set) {
        return WrongType();
    }

    SetElement result = *res_set;

    for (std::size_t i = 2; i < arguments.size(); ++i) {
        auto* value = data_base.GetElement(arguments[i]);
        if (!value) {
            return ListResult{};
        }

        auto* set_value = std::get_if<SetElement>(value);
        if (!set_value) {
            return WrongType();
        }

        result.Intersect(*set_value);
    }

    return ListResult(result.GetAll());
}

template <class Storage>
Result SDiff(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    if (arguments.size() <= 1) {
        return ListResult{};
    }

    auto* value = data_base.GetElement(arguments[1]);
    if (!value) {
        return ListResult{};
    }

    auto* first_set = std::get_if<SetElement>(value);
    if (!first_set) {
        return WrongType();
    }

    SetElement result = *first_set;

    for (std::size_t i = 2; i < arguments.size(); ++i) {
        auto* value = data_base.GetElement(arguments[i]);
        if (!value) {
            continue;
        }

        auto* set_value = std::get_if<SetElement>(value);
        if (!set_value) {
            return WrongType();
        }

        result.Diff(*set_value);
    }

    return ListResult(result.GetAll());
}

template <class Storage>
Result SMove(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 4)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& source = arguments[1];
    const std::string& destination = arguments[2];
    const std::string& member = arguments[3];

    auto* source_value = data_base.GetElement(source);
    if (!source_value) {
        return IntegerResult(0);
    }

    auto* source_set = std::get_if<SetElement>(source_value);
    if (!source_set) {
        return WrongType();
    }

    if (!source_set->Contains(member)) {
        return IntegerResult(0);
    }

    if (source == destination) {
        return IntegerResult(1);
    }

    Result result = OkResult{};

    if (!data_base.GetElement(destination)) {
        SetElement destination_set;
        destination_set.Add(member);

        result = data_base.PutElement(destination, std::move(destination_set));
    } else {
        result = data_base.UpdateElement(destination, [&](typename DataBase<Storage>::Variant& value) -> Result {
            auto* destination_set = std::get_if<SetElement>(&value);

            if (!destination_set) {
                return WrongType();
            }

            destination_set->Add(member);

            return OkResult{};
        });
    }

    if (IsError(result)) {
        return result;
    }

    result = data_base.UpdateElement(source, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* source_set = std::get_if<SetElement>(&value);

        if (!source_set) {
            return WrongType();
        }

        source_set->Remove(member);

        return OkResult{};
    });

    if (IsError(result)) {
        return result;
    }

    return IntegerResult(1);
}