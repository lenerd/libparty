// #include <stddef>
#include <iostream>
#include <boost/program_options.hpp>
#include "network/tcp_connection.hpp"
#include "ot/ot_hl17.hpp"
#include "util/options.hpp"


namespace po = boost::program_options;

struct Options
{
    Role role;
    size_t number_ots;
    std::string address;
    uint16_t port;
    size_t threads;
    std::string input_file;
    std::string output_file;
};

Options parse_arguments(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Produce help message")
        ("role,r", po::value<Role>()->required(), "Role (0 for sender, 1 for receiver)")
        ("number,n", po::value<size_t>()->default_value(128), "Number of OTs")
        ("address,a", po::value<std::string>()->default_value("127.0.0.1"), "IP address of the sender")
        ("port,p", po::value<uint16_t>()->default_value(7766), "Port address of the sender")
        ("threads,t", po::value<size_t>()->default_value(1), "Number of threads")
        ("input,i", po::value<std::string>()->default_value("in.txt"), "Input text file (only for receiver)")
        ("output,o", po::value<std::string>()->default_value("out.txt"), "Output text file (for sender and receiver resp.)")
    ;
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (po::error &e)
    {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        std::cerr << "\n" << desc << "\n";
        exit(EXIT_FAILURE);
    }

    if (vm.count("help"))
    {
        std::cerr << desc << "\n";
        exit(EXIT_FAILURE);
    }
    Options options;
    options.role = vm["role"].as<Role>();
    options.number_ots = vm["number"].as<size_t>();
    options.address = vm["address"].as<std::string>();
    options.port = vm["port"].as<uint16_t>();
    options.threads = vm["threads"].as<size_t>();
    options.input_file = vm["input"].as<std::string>();
    options.output_file = vm["output"].as<std::string>();
    return options;
}


int main(int argc, char* argv[])
{
    auto options{parse_arguments(argc, argv)};

    boost::asio::io_context io_context;
    try
    {
        auto connection(TCPConnection::from_role(options.role,
                                                 io_context,
                                                 options.address,
                                                 options.port));
        OT_HL17 ot{*connection};
        std::string foo;
        std::cin >> foo;
    }
    catch (std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}
