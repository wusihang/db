#pragma once
#include<set>
#include<Parser/TokenIterator.h>
#include<Parser/IAST.h>

namespace DataBase {

class Expected {
public:
    const char * max_parsed_pos = nullptr;
    std::set<const char *> variants;

    /// 'description' should be statically allocated string.
    void add(const char * current_pos, const char * description)
    {
        if (!max_parsed_pos || current_pos > max_parsed_pos)
        {
            variants.clear();
            max_parsed_pos = current_pos;
        }

        if (!max_parsed_pos || current_pos >= max_parsed_pos)
            variants.insert(description);
    }

    void add(TokenIterator it, const char * description)
    {
        add(it->begin, description);
    }
};


class IParser {
public:
    //获取这个parser的名称,用于打印日志等
    virtual const char * getName() const = 0;

	//解析接口, 返回值表示解析是否成功, node用于存储解析后的语法树, expected用于保存相关描述
    virtual bool parse(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) = 0;

	//从指定位置开始,忽略相关token处理, 相关描述信息保存到expected
    bool ignore(TokenIterator & pos, Expected & expected)
    {
        std::shared_ptr<IAST>  ignore_node;
        return parse(pos, ignore_node, expected);
    }

    //从指定位置完全忽略token处理,不保留expected
    bool ignore(TokenIterator & pos)
    {
        Expected expected;
        return ignore(pos, expected);
    }

    //检查token解析是否有效, 并将相关描述放在expected, 如果无效,那么pos重置为开始 , 忽略处理后的语法树
    bool check(TokenIterator & pos, Expected & expected)
    {
        TokenIterator begin = pos;
        std::shared_ptr<IAST> node;
        if (!parse(pos, node, expected))
        {
            pos = begin;
            return false;
        }
        else
            return true;
    }

    virtual ~IParser() {}

};

}
