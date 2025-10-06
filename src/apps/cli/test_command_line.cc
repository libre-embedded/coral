#ifdef NDEBUG
#undef NDEBUG
#endif

/* internal */
#include "cli/text.h"
#include "common.h"

bool value;

void test_first_arg_bool(const char **argc, std::size_t argv)
{
    CommandLine cli(argc, argv, &logger);

    assert(cli.get_command().compare("cmd") == 0);

    assert(cli.as_bool<0>(value));

    assert(not cli.as_bool<1>(value));
}

void test_first_arg_not_bool(const char **argc, std::size_t argv)
{
    CommandLine cli(argc, argv, &logger);

    assert(not cli.as_bool<0>(value));
}

void test_bools_basic(Buffer &buffer)
{
    Processor proc(buffer, test_first_arg_bool, true /* auto_poll */);

    std::stringstream("cmd true\n") >> buffer;
    assert(value);

    std::stringstream("cmd false\n") >> buffer;
    assert(not value);

    buffer.set_data_available();
    Processor new_proc(buffer, test_first_arg_not_bool, true /* auto_poll */);

    std::stringstream("cmd ASDF\n") >> buffer;
    assert(not value);
}

void test_first_arg_int_second_not_int(const char **argc, std::size_t argv)
{
    CommandLine cli(argc, argv, &logger);

    long int_value;
    assert(cli.as_long<0>(int_value));
    assert(int_value == 123);

    assert(not cli.as_long<1>(int_value));

    value = true;
}

void test_ints_basic(Buffer &buffer)
{
    buffer.set_data_available();
    Processor proc(buffer, test_first_arg_int_second_not_int,
                   true /* auto_poll */);

    value = false;
    std::stringstream("cmd 123 asdf\n") >> buffer;
    assert(value);
}

void test_first_arg_float_second_not_float(const char **argc, std::size_t argv)
{
    CommandLine cli(argc, argv, &logger);

    double double_value;
    assert(cli.as_double<0>(double_value));
    assert(double_value == 123.0);

    assert(not cli.as_double<1>(double_value));

    value = true;
}

void test_doubles_basic(Buffer &buffer)
{
    buffer.set_data_available();
    Processor proc(buffer, test_first_arg_float_second_not_float,
                   true /* auto_poll */);

    value = false;
    std::stringstream("cmd 123.0 asdf\n") >> buffer;
    assert(value);
}

int main(void)
{
    Buffer buffer;

    test_bools_basic(buffer);
    test_ints_basic(buffer);
    test_doubles_basic(buffer);

    get_stdout(true);
    get_stdout(false);
    print_verb_name_condition("name", "verb", true, false, false);
    print_verb_name_condition("name", "verb", false, false, false);
    print_verb_name_condition("name", "verb", false, true, false);

    return 0;
}
