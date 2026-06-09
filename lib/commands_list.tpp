#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <class Storage>
Result LPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        ListElement list;

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            list.PushFront(arguments[i]);
        }

        std::size_t size = list.Size();

        Result result = data_base.PutElement(key, std::move(list));

        if (IsError(result)) {
            return result;
        }

        return IntegerResult(size);
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* list_value = std::get_if<ListElement>(&value);

        if (!list_value) {
            return WrongType();
        }

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            list_value->PushFront(arguments[i]);
        }

        return IntegerResult(list_value->Size());
    });
}

template <class Storage>
Result RPush(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        ListElement list;

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            list.PushBack(arguments[i]);
        }

        std::size_t size = list.Size();

        Result result = data_base.PutElement(key, std::move(list));

        if (IsError(result)) {
            return result;
        }

        return IntegerResult(size);
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* list_value = std::get_if<ListElement>(&value);

        if (!list_value) {
            return WrongType();
        }

        for (std::size_t i = 2; i < arguments.size(); ++i) {
            list_value->PushBack(arguments[i]);
        }

        return IntegerResult(list_value->Size());
    });
}

template <class Storage>
Result LPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!(CheckEqualArguments(arguments, 3) || CheckEqualArguments(arguments, 2))) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        return NilResult{};
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* list_value = std::get_if<ListElement>(&value);

        if (!list_value) {
            return WrongType();
        }

        if (list_value->Empty()) {
            return NilResult{};
        }

        if (CheckEqualArguments(arguments, 2)) {
            return StringResult(list_value->PopFront());
        }

        int count;

        try {
            count = std::stoi(arguments[2]);
        } catch (const std::invalid_argument&) {
            return ErrorResult("not an integer");
        } catch (const std::out_of_range&) {
            return ErrorResult("index out of range");
        }

        if (count < 0) {
            return ErrorResult("value is out of range");
        }

        if (count == 0) {
            return ListResult{};
        }

        std::vector<std::string> result;

        while (count > 0 && !list_value->Empty()) {
            result.push_back(list_value->PopFront());
            --count;
        }

        return ListResult{result};
    });
}

template <class Storage>
Result RPop(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!(CheckEqualArguments(arguments, 3) || CheckEqualArguments(arguments, 2))) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    if (!data_base.IsContain(key)) {
        return NilResult{};
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* list_value = std::get_if<ListElement>(&value);

        if (!list_value) {
            return WrongType();
        }

        if (list_value->Empty()) {
            return NilResult{};
        }

        if (arguments.size() == 2) {
            return StringResult(list_value->PopBack());
        }

        int count;

        try {
            count = std::stoi(arguments[2]);
        } catch (const std::invalid_argument&) {
            return ErrorResult("not an integer");
        } catch (const std::out_of_range&) {
            return ErrorResult("index out of range");
        }

        if (count < 0) {
            return ErrorResult("value is out of range");
        }

        if (count == 0) {
            return ListResult{};
        }

        std::vector<std::string> result;

        while (count > 0 && !list_value->Empty()) {
            result.push_back(list_value->PopBack());
            --count;
        }

        return ListResult{result};
    });
}

template <class Storage>
Result LLen(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        return IntegerResult(0);
    }

    auto* list_value = std::get_if<ListElement>(value);

    if (!list_value) {
        return WrongType();
    }

    return IntegerResult(list_value->Size());
}

void NormalizeIndex(int& index, const std::size_t size) {
    if (index < 0) {
        index += size;
    }
}

template <class Storage>
Result LRange(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 4)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    int start, stop;

    try {
        start = std::stoi(arguments[2]);
        stop = std::stoi(arguments[3]);
    } catch (const std::invalid_argument&) {
        return ErrorResult("not an integer");
    } catch (const std::out_of_range&) {
        return ErrorResult("index out of range");
    }

    auto* value = data_base.GetElement(key);

    if (!value) {
        return ListResult{};
    }

    auto* list_value = std::get_if<ListElement>(value);

    if (!list_value) {
        return WrongType();
    }

    NormalizeIndex(start, list_value->Size());
    NormalizeIndex(stop, list_value->Size());

    std::size_t size = list_value->Size();

    if (size == 0) {
        return ListResult{};
    }

    if (start < 0) {
        start = 0;
    }

    if (stop >= size) {
        stop = size - 1;
    }

    if (start >= size || stop < 0 || start > stop) {
        return ListResult{};
    }

    std::vector<std::string> ans;

    for (int i = start; i <= stop; ++i) {
        ans.push_back((*list_value)[i]);
    }

    return ListResult(ans);
}

template <class Storage>
Result LIndex(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    int index;

    try {
        index = std::stoi(arguments[2]);
    } catch (const std::invalid_argument&) {
        return ErrorResult("not an integer");
    } catch (const std::out_of_range&) {
        return ErrorResult("index out of range");
    }

    auto* value = data_base.GetElement(key);

    if (!value) {
        return NilResult{};
    }

    auto* list_value = std::get_if<ListElement>(value);

    if (!list_value) {
        return WrongType();
    }

    NormalizeIndex(index, list_value->Size());

    if (index < 0 || index >= list_value->Size()) {
        return NilResult{};
    }

    return StringResult((*list_value)[index]);
}

template <class Storage>
Result LSet(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 4)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    int index;

    try {
        index = std::stoi(arguments[2]);
    } catch (const std::invalid_argument&) {
        return ErrorResult("not an integer");
    } catch (const std::out_of_range&) {
        return ErrorResult("index out of range");
    }

    if (!data_base.IsContain(key)) {
        return ErrorResult("no such key");
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& value) -> Result {
        auto* list_value = std::get_if<ListElement>(&value);

        if (!list_value) {
            return WrongType();
        }

        NormalizeIndex(index, list_value->Size());

        if (index < 0 || index >= list_value->Size()) {
            return ErrorResult("index out of range");
        }

        (*list_value)[index] = arguments[3];

        return OkResult{};
    });
}

template <class Storage>
Result LInsert(DataBase<Storage>& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 5)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];
    std::string pos = Normalize(arguments[2]);
    const std::string& pivot = arguments[3];
    const std::string& value = arguments[4];

    if (pos != "BEFORE" && pos != "AFTER") {
        return ErrorResult("syntax error");
    }

    if (!data_base.IsContain(key)) {
        return IntegerResult(0);
    }

    return data_base.UpdateElement(key, [&](typename DataBase<Storage>::Variant& element) -> Result {
        auto* list_value = std::get_if<ListElement>(&element);

        if (!list_value) {
            return WrongType();
        }

        return IntegerResult(list_value->Insert(pos, pivot, value));
    });
}