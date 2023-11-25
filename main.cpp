#include <iostream>
#include <expected>
#include <vector>
#include <array>
#include <utility>

enum class search_error {
    is_empty
};

template<std::size_t Size, class Errors> requires std::is_enum_v<Errors>
class error_handler {
    std::array<std::string, Size> messages_;

public:
    explicit error_handler(std::array<std::string, Size> messages)
            :messages_{std::move(messages)} { }

    [[nodiscard]] const std::string& message(const Errors& error) const
    {
        return messages_[std::to_underlying(error)];
    }
};

template<class Value, std::size_t Size, class Errors>
struct exception_handler : public error_handler<Size, Errors> {
    using return_type = Value;

    explicit exception_handler(const std::array<std::string, Size>& messages)
            :error_handler<Size, Errors>(messages) { }

    return_type operator()(const Errors& error) const
    {
        throw std::logic_error{this->message(error)};
    }
};

template<class Value, std::size_t Size, class Errors>
struct expected_handler : public error_handler<Size, Errors> {
    using return_type = std::expected<Value, Errors>;

    explicit expected_handler(const std::array<std::string, Size>& messages)
            :error_handler<Size, Errors>(messages) { }

    return_type operator()(const Errors& error) const
    {
        return std::unexpected{error};
    }
};

template<class ErrorHandler>
typename ErrorHandler::return_type find_max(const std::vector<int>& nums, const ErrorHandler& error_handler)
{
    if (nums.empty()) {
        return error_handler(search_error::is_empty);
    }
    int max{nums[0]};
    for (std::size_t i{1}; i<nums.size(); i++) {
        max = std::max(nums[i], max);
    }
    return max;
}

void print_max(std::size_t index)
{
    std::cout << "max is: " << index << "\n";
}

void print_error_message(const std::string& msg)
{
    std::cerr << "could not find max, because " << msg << "\n";
}

template<class ExpectedHandler>
void use_expected(const std::vector<int>& nums, const ExpectedHandler& handler)
{
    if (auto max = find_max(nums, handler); max.has_value()) {
        print_max(max.value());
    }
    else {
        print_error_message(handler.message(max.error()));
    }
}

template<class ExceptionHandler>
void use_exceptions(const std::vector<int>& nums, const ExceptionHandler& handler)
{
    try {
        print_max(find_max(nums, handler));
    }
    catch (const std::exception& e) {
        print_error_message(e.what());
    }
}

void compare_error_handling()
{
    // error messages
    constexpr std::size_t error_count{1};
    std::array<std::string, error_count>messages{"is empty"};

    // samples
    using value_t = int;
    constexpr std::size_t sample_count{2};
    std::array<std::vector<value_t>, sample_count>samples{{{}, {-1, 2, 0}}};

    // handlers
    expected_handler<value_t, error_count, search_error> expected_handler{messages};
    exception_handler<value_t, error_count, search_error> exception_handler{messages};

    // use handlers
    for (const auto& sample: samples) {
        use_expected(sample, expected_handler);
        use_exceptions(sample, exception_handler);
    }
}

int main()
{
    compare_error_handling();
}