#include<Server/Server.h>
#include <Server/HTTPHandlerFactory.h>
#include<Common/ServerApplicationExt.h>
#include<Function/FunctionRegistry.h>
#include<AggregationFunction/AggregationFunctionRegistry.h>
#include<Ext/std_ext.h>
#include<Ext/scope_guard.h>
#include <CommonUtil/LoggerUtil.h>
#include <CommonUtil/SystemUtil.h>
#include<CommonUtil/FileUtil.h>
#include <CommonUtil/StringUtils.h>
#include<Poco/File.h>
#include<Poco/Timespan.h>
#include<Poco/DirectoryIterator.h>
#include<Poco/ThreadPool.h>
#include<Poco/Net/HTTPServerParams.h>
#include<Poco/Net/HTTPServer.h>
#include<Poco/Exception.h>
#include<Poco/Net/DNS.h>
#include <Poco/Net/NetException.h>
#include<memory>
#include<chrono>
#include<string>
#include<thread>
#include <sys/resource.h>

int DataBase::Server::main(const std::vector< std::string >& args)
{
    Poco::Logger* log = &logger();
    DataBase::registerFunctions();
    DataBase::registerAggregationFunctions();

    global_context = std_ext::make_unique<Context>(DataBase::Context::createGlobal());
    global_context->setGlobalContext(*global_context);

    std::string path = FileUtil::getCanonicalPath(config().getString("path"));
    std::string default_database = config().getString("default_database", "default");

    Poco::File(path + "data/" + default_database).createDirectories();
    Poco::File(path + "metadata/" + default_database).createDirectories();

    //加大文件句柄打开数量
    {
        rlimit rlim;
        if (getrlimit(RLIMIT_NOFILE, &rlim)) {
            throw Poco::Exception("Cannot getrlimit");
        }

        if (rlim.rlim_cur == rlim.rlim_max) {
            LOG_DEBUG(log, "rlimit on number of file descriptors is " << rlim.rlim_cur);
        } else {
            rlim_t old = rlim.rlim_cur;
            rlim.rlim_cur = config().getUInt("max_open_files", rlim.rlim_max);
            int rc = setrlimit(RLIMIT_NOFILE, &rlim);
            if (rc != 0)
                LOG_WARNING(log,
                            std::string("Cannot set max number of file descriptors to ") + std::to_string(rlim.rlim_cur) + ". Try to specify max_open_files according to your system limits. error: " + strerror( errno));
            else
                LOG_DEBUG(log, "Set max number of file descriptors to " << rlim.rlim_cur << " (was " << old << ").");
        }
    }

    //创建临时文件目录
    {
        std::string tmp_path = config().getString("tmp_path", path + "tmp/");
        Poco::File(tmp_path).createDirectories();

        Poco::DirectoryIterator dir_end;
        for (Poco::DirectoryIterator it(tmp_path); it != dir_end; ++it) {
            //启动时，如果发现临时目录当中有以tmp结尾的文件，那么就删除这个文件
            if (it->isFile() && StringUtils::startsWith(it.name(), "tmp")) {
                LOG_DEBUG(log, "Removing old temporary file " << it->path());
                it->remove();
            }
        }
    }

    Poco::File(path + "flags/").createDirectories();

    if (config().has("interserver_http_port")) {
        std::string this_host = config().getString("interserver_http_host", "");
        if (this_host.empty()) {
            //获取域名或主机名
            this_host = SystemUtil::getFQDNOrHostName();
            LOG_DEBUG(log,
                      "Configuration parameter 'interserver_http_host' doesn't exist or exists and empty. Will use '" + this_host + "' as replica host.");
        }
    }

	//退出主函数时析构，即退出时调用
    SCOPE_EXIT( {
        LOG_INFO(log, "Shutting down storages.");
        LOG_DEBUG(log, "Shutted down storages.");
        global_context.reset();
        LOG_DEBUG(log, "Destroyed global context.");
    });

    {
        Poco::Timespan keep_alive_timeout(config().getInt("keep_alive_timeout", 10), 0);
        Poco::ThreadPool server_pool(3, config().getInt("max_connections", 1024));
        Poco::Net::HTTPServerParams::Ptr http_params = new Poco::Net::HTTPServerParams;
        http_params->setTimeout(60000);
        http_params->setKeepAliveTimeout(keep_alive_timeout);
        std::vector<std::unique_ptr<Poco::Net::TCPServer>> servers;
        std::vector<std::string> listen_hosts;
        bool try_listen = false;
        if (listen_hosts.empty()) {
            listen_hosts.emplace_back("::1");
            listen_hosts.emplace_back("127.0.0.1");
            try_listen = true;
        }
        
        //构建socket监听地址函数
        auto make_socket_address = [&](const std::string & host, std::uint16_t port) {
            Poco::Net::SocketAddress socket_address;
            try
            {
                socket_address = Poco::Net::SocketAddress(host, port);
            }
            catch (const Poco::Net::DNSException & e)
            {
                if (e.code() == EAI_FAMILY
#if defined(EAI_ADDRFAMILY)
                        || e.code() == EAI_ADDRFAMILY
#endif
                   )
                {
                    LOG_ERROR(log,
                              "Cannot resolve listen_host (" << host << "), error: " << e.message() << ". "
                              "If it is an IPv6 address and your host has disabled IPv6, then consider to "
                              "specify IPv4 address to listen in <listen_host> element of configuration "
                              "file. Example: <listen_host>0.0.0.0</listen_host>");
                }
                throw;
            }
            return socket_address;
        };

        for (const auto & listen_host : listen_hosts) {
            /// For testing purposes, user may omit tcp_port or http_port or https_port in configuration file.
            try {
                /// HTTP
                if (config().has("http_port")) {
                    Poco::Net::SocketAddress http_socket_address = make_socket_address(listen_host,
                            config().getInt("http_port"));
                    Poco::Net::ServerSocket http_socket(http_socket_address);
                    http_socket.setReceiveTimeout(60000);
                    http_socket.setSendTimeout(60000);

                    servers.emplace_back(
                        new Poco::Net::HTTPServer(new DataBase::HTTPHandlerFactory(*this, "HTTPHandler-factory"), server_pool,
                                                  http_socket, http_params));

                    LOG_INFO(log, "Listening http://" + http_socket_address.toString());
                }

                /// TCP
//				if (config().has("tcp_port")) {
//					Poco::Net::SocketAddress tcp_address = make_socket_address(listen_host,
//							config().getInt("tcp_port"));
//					Poco::Net::ServerSocket tcp_socket(tcp_address);
//					tcp_socket.setReceiveTimeout(60000);
//					tcp_socket.setSendTimeout(60000);
//					servers.emplace_back(
//							new Poco::Net::TCPServer(new TCPHandlerFactory(*this), server_pool, tcp_socket,
//									new Poco::Net::TCPServerParams));
//
//					LOG_INFO(log, "Listening tcp: " + tcp_address.toString());
//				}
                //至少需要有一个TCP服务器或HTTP服务器
                if (servers.empty()) {
                    throw Poco::Exception("No 'tcp_port' and 'http_port' is specified in configuration file.");
                }
            } catch (const Poco::Net::NetException & e) {
                if (try_listen && e.code() == POCO_EPROTONOSUPPORT)
                    LOG_ERROR(log,
                              "Listen [" << listen_host << "]: " << e.what() << ": " << e.message() << "  If it is an IPv6 or IPv4 address and your host has disabled IPv6 or IPv4, then consider to " "specify not disabled IPv4 or IPv6 address to listen in <listen_host> element of configuration " "file. Example for disabled IPv6: <listen_host>0.0.0.0</listen_host> ." " Example for disabled IPv4: <listen_host>::</listen_host>");
                else
                    throw;
            }
        }

        //启动服务器
        for (auto & server : servers) {
            server->start();
        }
        LOG_INFO(log, "Ready for connections.");

        //退出作用域时会调用对象的析构函数，因此此处在离开{}作用域时就会调用以下代码块
        SCOPE_EXIT(
        {
            LOG_DEBUG(log, "Received termination signal.");
            LOG_DEBUG(log, "Waiting for current connections to close.");
            is_cancelled = true;
            int current_connections = 0;
            for (auto & server : servers) {
                server->stop();
                current_connections += server->currentConnections();
            }
            LOG_DEBUG(log, "Closed all listening sockets." << (current_connections ? " Waiting for " + std::to_string(current_connections) + " outstanding connections." : ""));
            if (current_connections)
            {
                const int sleep_max_ms = 1000 * config().getInt("shutdown_wait_unfinished", 5);
                const int sleep_one_ms = 100;
                int sleep_current_ms = 0;
                while (sleep_current_ms < sleep_max_ms) {
                    current_connections = 0;
                    for (auto & server : servers)
                        current_connections += server->currentConnections();
                    if (!current_connections) break;
                    sleep_current_ms += sleep_one_ms;
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_one_ms));
                }
            }
            LOG_DEBUG( log, "Closed connections." << (current_connections ? " But " + std::to_string(current_connections) + " remains." " Tip: To increase wait time add to config: <shutdown_wait_unfinished>60</shutdown_wait_unfinished>" : ""));
        });

        //等待外部终止应用
        waitForTerminationRequest();
    }

    return  Poco::Util::Application::EXIT_OK;
}

std::string DataBase::Server::getDefaultCorePath() const
{
    return FileUtil::getCanonicalPath(config().getString("path")) + "cores";
}


//这里注册主函数入口，该宏定义在Common/ServerApplicationExt.h
APP_SERVER_MAIN_FUNC(DataBase::Server,mainEntryDBServer) ;





