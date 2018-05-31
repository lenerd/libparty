// MIT License
//
// Copyright (c) 2018 Lennart Braun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstddef>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include "network/tcp_connection.hpp"
#include "ot/ot_co15.hpp"
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
    OT_Protocol ot_protocol;
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
        ("ot", po::value<OT_Protocol>()->default_value(OT_Protocol::HL17), "OT Protocol to use")
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
    options.ot_protocol = vm["ot"].as<OT_Protocol>();
    return options;
}


std::vector<bool> parse_inputfile(const Options& options)
{
    std::vector<bool> choices(options.number_ots);
    std::ifstream f(options.input_file);
    bool b;
    for (size_t i = 0; i < options.number_ots; ++i)
    {
        f >> b;
        choices[i] = b;
    }
    return choices;
}

void write_outputfile_receiver(const Options& options,
                               const std::vector<bytes_t> output)
{
    std::ofstream f(options.output_file);
    for (auto& o : output)
    {
        f << hexlify(o, true) << "\n";
    }
}

void write_outputfile_sender(const Options& options,
                             const std::vector<std::pair<bytes_t, bytes_t>> output)
{
    std::ofstream f(options.output_file);
    for (const auto& [o0, o1] : output)
    {
        f << hexlify(o0, true) << "," << hexlify(o1, true) << "\n";
    }
}


int main(int argc, char* argv[])
{
    auto options{parse_arguments(argc, argv)};

    boost::asio::io_context io_context;
    std::thread io_thread;

    try
    {
        auto work = boost::asio::make_work_guard(io_context);
        io_thread = std::thread([&io_context]{io_context.run();});

        auto connection(TCPConnection::from_role(options.role,
                                                 io_context,
                                                 options.address,
                                                 options.port));
        std::unique_ptr<RandomOT> ot;
        switch (options.ot_protocol)
        {
            case OT_Protocol::CO15:
                ot = std::make_unique<OT_CO15>(*connection);
            case OT_Protocol::HL17:
                ot = std::make_unique<OT_HL17>(*connection);
        };

        if (options.role == Role::server)
        {
            std::vector<std::pair<bytes_t, bytes_t>> output;
            if (options.threads == 1)
                output = ot->send(options.number_ots);
            else
                output = ot->parallel_send(options.number_ots, options.threads);
            write_outputfile_sender(options, output);
        }
        else  // Role::client
        {
            auto choices = parse_inputfile(options);
            std::vector<bytes_t> output;
            if (options.threads == 1)
                output = ot->recv(choices);
            else
                output = ot->parallel_recv(choices, options.threads);
            write_outputfile_receiver(options, output);
        }

    }
    catch (std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << "\n";
    }

    io_context.stop();
    io_thread.join();

    return EXIT_SUCCESS;
}
