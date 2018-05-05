#pragma once

namespace DataBase {

class Context {
public:
    ~Context();
    static Context createGlobal();

    void setGlobalContext(Context & context_) {
        global_context = &context_;
    }

    enum ApplicationType {
        SERVER,         /// The program is run as wsdb-server daemon (default behavior)
        CLIENT,         /// wsdb-client
        LOCAL           /// wsdb-local
    };
private:
    Context(); // use createGlobal or copy construct instead
    Context * global_context = nullptr;
};

}
