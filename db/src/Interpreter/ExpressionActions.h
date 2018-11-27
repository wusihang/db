#pragma once
#include<memory>
#include<Streams/Block.h>

namespace DataBase {
class ExpressionActions {
public:
    void execute(IO::Block & block) const;
};

using ExpressionActionsPtr = std::shared_ptr<ExpressionActions>;

}
