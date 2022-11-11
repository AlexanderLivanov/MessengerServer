#include "Listener/Listener.hpp"


chat_room room;

awaitable<void> listener(tcp::acceptor acceptor) {
    for (;;) {
        std::make_shared<chat_session>(
            co_await acceptor.async_accept(use_awaitable),
            room
            )->start();
    }
}


awaitable<void> connector(tcp::socket socket, const std::string& ip) {
    try {
        co_await socket.async_connect({ boost::asio::ip::address::from_string(ip),1337 }, use_awaitable);
        if (socket.is_open()) {
            std::make_shared<chat_session>(std::move(socket), room)->start();
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

auto main(int argc, char* argv[], char* envp[]) -> int{
    try{
        boost::asio::io_context ctx;
        std::vector<std::thread> threads;
        
        std::ifstream file("node.txt");
        std::vector<std::string> ips;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line == std::string("127.0.0.1") || line == std::string("0.0.0.0") || line == std::string("localhost") || line == std::string("::1")) {
                    continue;
                }
                ips.push_back(line);
            }
            file.close();
        }
        
        for (auto&& ip : ips) {
            co_spawn(ctx, connector(tcp::socket(ctx), ip), detached);
        }

        co_spawn(ctx, listener(tcp::acceptor(ctx, { tcp::v4(),1337 })), detached);
        

        boost::asio::signal_set signals(ctx,SIGINT,SIGTERM);
        signals.async_wait([&](auto,auto){ctx.stop();});

        auto threadCount = std::thread::hardware_concurrency() * 2;
        for (int n = 0; n < threadCount; n++) {
            threads.emplace_back([&] {ctx.run(); });
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }    
    return 0;
}