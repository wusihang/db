#include<Parser/ParserUseQuery.h>
#include<Parser/IdentifierParser.h>
#include<Parser/KeywordParser.h>
#include<Parser/ASTIdentifier.h>
#include<Parser/ASTUseQuery.h>
#include<Ext/typeid_cast.h>

namespace DataBase {

bool ParserUseQuery::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
	//保存起始位置
    TokenIterator begin = pos;
	//初始化关键字Parser, Use Query的关键字是USE
    KeywordParser s_use("USE");
	//标识符解析
    IdentifierParser name_p;
    std::shared_ptr<IAST> database;

    if (!s_use.ignore(pos, expected))
        return false;

    if (!name_p.parse(pos, database, expected))
        return false;

    auto query = std::make_shared<ASTUseQuery>(StringRange(begin, pos));
    query->database = typeid_cast<ASTIdentifier &>(*database).name;
    node = query;
    return true;
}

}
