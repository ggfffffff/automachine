#include "regex.h"

/**
 * 注：如果你愿意，你可以自由的using namespace。
 */

/**
 * 编译给定的正则表达式。
 * 具体包括两个过程：解析正则表达式得到语法分析树（这步已经为你写好，即parse方法），
 * 和在语法分析树上进行分析（遍历），构造出NFA（需要你完成的部分）。
 * 在语法分析树上进行分析的方法，可以是直接自行访问该树，也可以是使用antlr的Visitor机制，详见作业文档。
 * 你编译产生的结果，NFA应保存在当前对象的nfa成员变量中，其他内容也建议保存在当前对象下（你可以自由地在本类中声明新的成员）。
 * @param pattern 正则表达式的字符串
 * @param flags 正则表达式的修饰符
 */
// childnode当前节点，now_state当前状态

// 记录要捕获的分组数量
int capture_num = 0;
// 在此函数中填充rules
int Regex::dfs(antlr4::tree::ParseTree *childNode ,int now_state)
{
    // printf("dfs");
    //std::cout<<"dfs"<<std::endl;
    int final_state;

    // 如果是非叶子节点（规则节点）
    if(childNode->getTreeType() == antlr4::tree::ParseTreeType::RULE){ // 如果当前节点是规则节点(非叶子节点)
        auto ruleNode = (antlr4::RuleContext*)childNode; 
        // Regex类型的节点
        if(ruleNode->getRuleIndex() == regexParser::RuleRegex){
            // auto regexNode = (regexParser::RegexContext*)ruleNode;
            // 构造终态
            nfa.num_states++;
            //std::cout<<"nfa.num_states="<<nfa.num_states<<std::endl;
            final_state = nfa.num_states;
            //nfa.group_begin.push_back(now_state);
            //nfa.group_end.push_back(final_state);

            std::vector<Rule> newRules0;
            nfa.rules.push_back(newRules0);
        
            for(auto child : ruleNode->children){
                if(child->getText() != "|"){
                    // 构造小自动机的初态
                    nfa.num_states++;
                    //std::cout<<"nfa.num_states="<<nfa.num_states<<std::endl;
                    std::vector<Rule> newRules;
                    nfa.rules.push_back(newRules);

                    // 把当前state和child的start_state之间相连epsilon转移
                    Rule rule;
                    rule.type = EPSILON;
                    rule.dst = nfa.num_states;
                    rule.reverse = false;
                    rule.greedy = true;
                    nfa.rules[now_state].push_back(rule);

                    // 把每个小自动机的终态和final_state之间相连epsilon转移
                    int end_state = dfs(child,nfa.num_states);
                    Rule finalRule;
                    finalRule.type = EPSILON;
                    finalRule.dst = final_state;
                    finalRule.reverse = false;
                    finalRule.greedy = true;
                    nfa.rules[end_state].push_back(finalRule);
                }
            }
        }

        // Expression类型的节点
        else if(ruleNode->getRuleIndex() == regexParser::RuleExpression){
            //auto expressionNode = (regexParser::ExpressionContext*)ruleNode;
            final_state = now_state;
            for(auto child : ruleNode->children){
                final_state = dfs(child, final_state);
            }
        }

        // ExpressionItem类型的节点
        else if(ruleNode->getRuleIndex() == regexParser::RuleExpressionItem){
            //std::cout<<"expressionItem"<<std::endl;
            auto itemNode = (regexParser::ExpressionItemContext*)ruleNode;
            
            auto *anch = itemNode->anchor();
            // 如果有anchor
            if(anch != nullptr){
                //std::cout<<"anchor"<<std::endl;
                Rule rule;
                rule.type = EPSILON;
                rule.dst = now_state + 1;
                nfa.num_states++;
                std::vector<Rule> newRules;
                nfa.rules.push_back(newRules);

                if(anch->AnchorStartOfString()){
                    rule.by = "^";
                    nfa.rules[now_state].push_back(rule);
                    //std::cout<<rule.dst<<std::endl;
                }
                else if(anch->AnchorWordBoundary()){
                    rule.by = "b";
                    nfa.rules[now_state].push_back(rule);
                }
                else if(anch->AnchorEndOfString()){
                    //std::cout<<"now_state"<<now_state<<std::endl;
                    rule.by = "$";
                    rule.dst = 1;
                    nfa.rules[now_state].insert(nfa.rules[now_state].begin(),rule);
                }
                else if(anch->AnchorNonWordBoundary()){
                    rule.by = "B";
                    nfa.rules[now_state].push_back(rule);
                }
                //std::cout<<"anchorend"<<std::endl;
                //std::cout<<"anchorend"<<std::endl;
                final_state =  rule.dst;
            }
            //std::cout<<"anchorend"<<std::endl;


            regexParser::NormalItemContext *normalItem = itemNode->normalItem();
            // 如果是normalItem
            if(normalItem != nullptr){
                //std::cout<<"normalItem="<<normalItem->getText()<<std::endl;
                // single
                if(normalItem->single() != nullptr){
                    //std::cout<<"single"<<std::endl;
                    auto single = normalItem->single();
                    // 单个字符
                    if(single->char_() != nullptr){
                        //std::cout<<"char"<<std::endl;
                        Rule rule;
                        nfa.num_states++;
                        std::vector<Rule> newRules;
                        nfa.rules.push_back(newRules);
                        rule.dst = nfa.num_states;
                        rule.type = NORMAL;
                        rule.greedy = true;
                        // 转义字符，忽略"/"
                        if(single->char_()->EscapedChar() != nullptr){
                            rule.by = single->char_()->EscapedChar()->getText()[1];
                        }
                        // 普通字符
                        else{
                            rule.by = single->char_()->getText();
                            //std::cout<<"single->char_()->getText():"<<single->char_()->getText()<<std::endl;
                        }
                        //std::cout<<rule.by<<std::endl;
                        rule.to = "";
                        nfa.rules[now_state].push_back(rule);
                        final_state = rule.dst;
                    }      
                    // 表示一类字符的元字符
                    else if(single->characterClass() != nullptr){
                        Rule rule;
                        nfa.num_states++;
                        std::vector<Rule> newRules;
                        nfa.rules.push_back(newRules);
                        rule.dst = nfa.num_states;
                        rule.type = SPECIAL;
                        rule.greedy = true;
                        rule.by = single->characterClass()->getText()[1];
                        rule.to = "";
                        nfa.rules[now_state].push_back(rule);
                        final_state = rule.dst;
                    }
                    // .匹配任意字符
                    else if(single->AnyCharacter() != nullptr){
                        Rule rule;
                        nfa.num_states++;
                        std::vector<Rule> newRules;
                        nfa.rules.push_back(newRules);
                        rule.dst = nfa.num_states;
                        rule.type = SPECIAL;
                        rule.greedy = true;
                        rule.by = ".";
                        rule.to = "";
                        nfa.rules[now_state].push_back(rule);
                        final_state = rule.dst;
                    }
                    // 中括号字符组
                    else if(single->characterGroup() != nullptr){
                        nfa.num_states++;
                        final_state = nfa.num_states;
                        std::vector<Rule> newRules;
                        nfa.rules.push_back(newRules);
                    
                        auto negModifier = single->characterGroup()->characterGroupNegativeModifier();
                        auto groupItems = single->characterGroup()->characterGroupItem();
                        for(auto item : groupItems){
                            Rule rule;
                            rule.dst = final_state;
                            rule.greedy = true;
                            if(negModifier != nullptr){
                                rule.reverse = true;
                            }
                            else{
                                rule.reverse = false;
                            }
                            // 单个字符，除了-
                            if(item->charInGroup() != nullptr){
                                rule.type = NORMAL;
                                // 转义字符
                                if(item->charInGroup()->EscapedChar() != nullptr){
                                    rule.by = item->charInGroup()->EscapedChar()->getText()[1];
                                }
                                // 普通字符
                                else{
                                    rule.by = item->charInGroup()->getText();
                                }
                                rule.to = "";
                            }
                            // 表示一类字符的元字符
                            else if(item->characterClass() != nullptr){
                                rule.type = SPECIAL;
                                rule.by = item->characterClass()->getText()[1];
                                rule.to = "";
                            }
                            // 字符区间
                            else if(item->characterRange() != nullptr){
                                rule.type = RANGE;
                                rule.by = item->characterRange()->charInGroup()[0]->getText();
                                rule.to = item->characterRange()->charInGroup()[1]->getText();
                            }  
                            nfa.rules[now_state].push_back(rule); 
                        }
                    }
                }

                // 如果有group
                else if(normalItem->group() != nullptr){
                    auto group = normalItem->group();
                    final_state = dfs(group->regex(),now_state);
                    // 如果分组要捕获
                    if(group->groupNonCapturingModifier()==nullptr){
                        //std::cout<<"capture"<<std::endl;
                        // 如果没有出现相同的则加入组
                        bool same = false;
                        for(int i = 0; i < nfa.group_begin.size(); i++){
                            //std::cout<<"nfa.group_begin["<<i<<"]"<<nfa.group_begin[i]<<std::endl;
                            //std::cout<<"now_state"<<now_state<<std::endl;
                            if(now_state == nfa.group_begin[i]){
                                if(final_state == nfa.group_end[i]){
                                    same = true;
                                }
                            }
                        }
                        if(same == false){
                            //std::cout<<"capture"<<std::endl;
                            capture_num++;
                            nfa.group_begin.push_back(now_state);
                            nfa.group_end.push_back(final_state);
                            //std::cout<<now_state<<"->"<<final_state<<std::endl;
                        }
                    }
                }
            }

            regexParser::QuantifierContext *quantifier = itemNode->quantifier();
            // 如果有quantifier
            if(quantifier != nullptr){
                //auto quantifiertype = normalItem->quantifierType();
                std::string qf = quantifier->quantifierType()->getText();
            
                if(qf == "*"){
                    // 建立从final_state到now_state的epsilon转移
                    Rule rule;
                    rule.type = EPSILON;
                    rule.dst = now_state;
                    nfa.rules[final_state].push_back(rule);

                    // 建立从now_state到final_state的epsilon转移
                    Rule rule1;
                    rule1.type = EPSILON;
                    rule1.dst = final_state;
                    // 非贪婪匹配把直接从state到final_state的epsilon转换放前面，先访问
                    // 插在前面的不用再处理
                    if(quantifier->lazyModifier() != nullptr){
                        //rule.greedy = false;
                        nfa.rules[now_state].insert(nfa.rules[now_state].begin(),rule1);
                    }
                    else{
                        //rule.greedy = true;
                        nfa.rules[now_state].push_back(rule1);
                    }
                }
                else if(qf == "+"){
                    // 建立从final_state到now_state的epsilon转移
                    Rule rule;
                    rule.type = EPSILON;
                    rule.dst = now_state;
                    // 非贪婪匹配把直接从final_state到state的epsilon转换放后面，后访问
                    // 但是这个放后面只是放在了当前的后面，所以应当在全部插入完成之后再处理
                    // 所以依然是在非贪婪的时候插在最前面，放在match里遍历rules的时候将这个vector逆序
                    if(quantifier->lazyModifier() != nullptr){
                        //std::cout<<"greedy = false"<<std::endl;
                        rule.greedy = false;
                        nfa.rules[final_state].insert(nfa.rules[final_state].begin(),rule);
                    }
                    else{
                        rule.greedy = true;
                        nfa.rules[final_state].push_back(rule);
                    }
                }
                else if(qf == "?"){
                    // 建立从now_state到final_state的epsilon转移
                    Rule rule;
                    rule.type = EPSILON;
                    rule.dst = final_state;
                    if(quantifier->lazyModifier() != nullptr){
                        //rule.greedy = false;
                        nfa.rules[now_state].insert(nfa.rules[now_state].begin(),rule);
                    }
                    else{
                        //rule.greedy = true;
                        nfa.rules[now_state].push_back(rule);
                    }   
                }
                // 匹配若干次数范围
                else{
                    // 构造有条件的转移
                    auto range = quantifier->quantifierType()->rangeQuantifier();
                    Rule rule;
                    rule.dst = now_state;
                    rule.range_from = stoi(range->rangeQuantifierLowerBound()->getText());  
                    if (range->rangeQuantifierUpperBound() != nullptr){
                        rule.range_to = stoi(range->rangeQuantifierUpperBound()->getText());
                    }
                    else if (range->rangeDelimiter() != nullptr){
                        rule.range_to = 999999999;
                    }
                    else{
                        rule.range_to = rule.range_from;
                    }
                    nfa.rules[final_state].push_back(rule);
                    nfa.rangestate[final_state] = nfa.rules[final_state].size() - 1;

                    // 可以不走，相当于?s
                    if (rule.range_from == 0) 
                    {
                        rule.type = EPSILON;
                        rule.dst = final_state;
                        rule.reverse = false;
                        if(quantifier->lazyModifier() != nullptr){
                            //rule.greedy = false;
                            nfa.rules[now_state].insert(nfa.rules[now_state].begin(),rule);
                        }
                        else{
                            //rule.greedy = true;
                            nfa.rules[now_state].push_back(rule);
                        }   
                    }
                }
            }
        }
        return final_state;
    }
}

void Regex::compile(const std::string &pattern, const std::string &flags) {
    //printf("compile");
    // std::cout<<"compile"<<std::endl;
    regexParser::RegexContext *tree = Regex::parse(pattern); // 这是语法分析树
    // TODO 请你将在上次实验的内容粘贴过来，在其基础上进行修改。
    // 你应该根据tree中的内容，恰当地构造NFA
    // 构造好的NFA，和其他数据变量（如有），均建议保存在当前对象中。
    // 例如 (this->)nfa.num_states = 100;
    
    //处理flag
    for(auto ch:flags)
    {
        if(ch == 'm'){
            nfa.M_flag = true;
        }
        if(ch == 's'){
            nfa.S_flag = true;
            //std::cout<<"true";
        }
    }
    //nfa.num_states = 1;
    std::vector<Rule> newRules;
    nfa.rules.push_back(newRules);
    auto newTree = (antlr4::tree::ParseTree*)tree;
    if(newTree == nullptr){
        //std::cout<<"null"<<std::endl;
        return;
    }
    int end_state = dfs(newTree,0);
    //std::cout<<"compile"<<std::endl;
    //std::cout<<"end_state="<<end_state<<std::endl;
    for(int i = 0; i <= nfa.num_states; i++){
        nfa.is_final.push_back(false);
    }
    nfa.is_final[1] = true;
    //std::cout<<"cole"<<std::endl;
}

/**
 * 在给定的输入文本上，进行正则表达式匹配，返回匹配到的第一个结果。
 * 匹配不成功时，返回空vector( return std::vector<std::string>(); ，或使用返回初始化列表的语法 return {}; )；
 * 匹配成功时，返回一个std::vector<std::string>，其中下标为0的元素是匹配到的字符串，
 * 下标为i(i>=1)的元素是匹配结果中的第i个分组。
 * @param text 输入的文本
 * @return 如上所述
 */
std::vector<std::string> Regex::match(std::string text) {
    // TODO 请你将在上次实验的内容粘贴过来，在其基础上进行修改。
    //std::cout<<"match"<<std::endl;
    //std::cout<<"nfa.num_states="<<nfa.num_states<<std::endl;
    for(int i = 0; i < nfa.rules.size(); i++){
        //std::cout<<"is_final["<<i<<"]="<<nfa.is_final[i]<<std::endl;
        // +非贪婪的额外处理
        if(nfa.rules[i].size() != 0 && nfa.rules[i][0].greedy == false){
            //std::cout<<"greedy"<<std::endl;
            reverse(nfa.rules[i].begin(),nfa.rules[i].end());
        }
        // for(int j = 0; j < nfa.rules[i].size(); j++){
        //     std::cout<<i<<"->"<<nfa.rules[i][j].dst<<std::endl;
        //     std::cout<<"by="<<nfa.rules[i][j].by<<std::endl;
        //     std::cout<<"type="<<nfa.rules[i][j].type<<std::endl;
        //     // std::cout<<"greedy="<<nfa.rules[i][j].greedy<<std::endl;
        //     // std::cout<<"reverse="<<nfa.rules[i][j].reverse<<std::endl;
        //     std::cout<<std::endl;
        // }
    }
    //std::cout<<"----------------------------------------------"<<std::endl;
    int length = text.length();
    bool is_start = false;
    std::vector<std::string> result;
    for(int i = 0; i < length; i++){
        //std::cout<<"i="<<i<<std::endl;
        //std::cout<<"inexec"<<std::endl;
    
        if((nfa.M_flag == true && (text[i] == '\r' || text[i] == '\n')) || (i == 0)){
            is_start = true;
        }
        else{
            is_start = false;
        }
        std::string newText = text.substr(i,length);
        Path path = nfa.exec(newText,is_start);
        //std::cout<<path.consumes.size()<<std::endl;
        //std::cout<<"----------------------------------------------"<<std::endl;
        //text.erase(text.begin());
        // 完整匹配
        //std::cout<<path.consumes.size()<<std::endl;
        if(path.consumes.size() != 0){
            if(capture_num == 0){
                //std::cout<<"all"<<std::endl;
                std::string str = "";
                for(auto strin : path.consumes){
                    str.append(strin);
                }
                result.push_back(str);
                return result;
            }

            else if(capture_num != 0){
                // for(int i = 0; i < nfa.group_begin.size(); i++){
                //     std::cout<<nfa.group_begin[i]<<" ";
                //     std::cout<<nfa.group_end[i]<<" ";
                // }
                // 在开头插入以输出完整匹配
                nfa.group_begin.insert(nfa.group_begin.begin(),0);
                nfa.group_end.insert(nfa.group_end.begin(),1);
                // 删去重复元素
                for(int i = 0; i < nfa.group_begin.size(); i++){
                    for(int j = i + 1; j < nfa.group_begin.size(); j++){
                        if(nfa.group_begin[j] == nfa.group_begin[i]){
                            if(nfa.group_end[j] == nfa.group_end[i]){
                                nfa.group_begin.erase(nfa.group_begin.begin()+j);
                                nfa.group_end.erase(nfa.group_end.begin()+j);
                            }
                        }
                    }
                }
                for(int i = 0; i < nfa.group_begin.size(); i++){  
                    // 对每一个区间找最长state序列
                    //std::vector<int> group_match;
                    std::string str = "";
                    for(int j = 0; j < path.states.size(); j++){
                        if(path.states[j] == nfa.group_begin[i]){
                            // std::cout<<"nfa.group_begin["<<i<<"]="<<nfa.group_begin[i]<<std::endl;
                            // std::cout<<"nfa.group_end["<<i<<"]="<<nfa.group_end[i]<<std::endl;
                            for(int k = path.states.size() - 1; k >= j; k--){
                                //std::cout<<"path.states["<<k<<"]="<<path.states[k]<<std::endl;
                                if(path.states[k] == nfa.group_end[i]){
                                    //std::cout<<"find it"<<std::endl;
                                    for(int it = j; it < k; it++){
                                        //group_match.push_back(path.states[it]);
                                        //std::cout<<path.states[it]<<" ";
                                        str = str + path.consumes[it];
                                    }
                                }
                            }
                        }
                    }
                    //std::cout<<std::endl;
                    result.push_back(str);
                }   
                return result;
            }
        }

        // std::cout<<"-------------------------------------"<<std::endl;
        // for(int i = 0; i < nfa.group_begin.size(); i++){
        //     std::cout<<nfa.group_begin[i]<<"->"<<nfa.group_end[i]<<std::endl;
        // }
    }
    return {};
}



/**
 * 在给定的输入文本上，进行正则表达式匹配，返回匹配到的**所有**结果。
 * 匹配不成功时，返回空vector( return std::vector<std::string>(); ，或使用返回初始化列表的语法 return {}; )；
 * 匹配成功时，返回一个std::vector<std::vector<std::string>>，其中每个元素是每一个带分组的匹配结果，其格式同match函数的返回值（详见上面）。
 * @param text 输入的文本
 * @return 如上所述
 */
std::vector<std::vector<std::string>> Regex::matchAll(std::string text) {
    // TODO 请你完成这个函数
    //std::cout<<"matchall"<<std::endl;
    // 记录是否已匹配上，若是，则直接跳过以保证不覆盖
    bool is_matched = false;

    for(int i = 0; i < nfa.rules.size(); i++){
        //std::cout<<"?"<<std::endl;
        // 非贪婪的额外处理
        if(nfa.rules[i].size() != 0 && nfa.rules[i][0].greedy == false){
            std::cout<<"greedy"<<std::endl;
            reverse(nfa.rules[i].begin(),nfa.rules[i].end());
        }
        // for(int j = 0; j < nfa.rules[i].size(); j++){
        //     std::cout<<i<<"->"<<nfa.rules[i][j].dst<<std::endl;
        //     std::cout<<"by="<<nfa.rules[i][j].by<<std::endl;
        //     std::cout<<"type="<<nfa.rules[i][j].type<<std::endl;
        //     // std::cout<<"greedy="<<nfa.rules[i][j].greedy<<std::endl;
        //     // std::cout<<"reverse="<<nfa.rules[i][j].reverse<<std::endl;
        //     std::cout<<std::endl;
        // }
    }
    //std::cout<<"----------------------------------------------"<<std::endl;
    int length = text.length();
    std::vector<std::vector<std::string>> result;
    int shift = 0;
    for(int i = 0; i < length; i++){
        //std::cout<<"i="<<i<<std::endl;
        //std::cout<<"inexec"<<std::endl;
        bool is_start = false;
        if(i == 0){
            is_start = true;
        }
        else{
            //std::cout<<"1"<<std::endl;
            if((nfa.M_flag == true && (text[i - 1] == '\r' || text[i - 1] == '\n')) || (i == 0)){
                is_start = true;
                //std::cout<<"2"<<std::endl;
            }
            else{
                is_start = false;
            }
        }
        //std::cout<<"is_start"<<is_start<<std::endl;
        std::string newText;
        //newText = text.substr(i,length);
        if(is_matched == false){
            newText = text.substr(i,length);
        }
        else if(is_matched == true){
            //std::cout<<"matched"<<std::endl;
            newText = text.substr(i+shift-1,length);
            i = i + shift - 1;
            is_matched = false;
        }
        // std::cout<<newText<<std::endl;
        // std::cout<<std::endl;
        Path path;
        path = nfa.exec(newText,is_start);
        // for(int i = 0; i < path.states.size(); i++){
        //     std::cout<<path.states[i]<<" ";
        // }
        //text.erase(text.begin());
        if(path.consumes.size() != 0){
            is_matched = true;
            shift = path.consumes.size();
            if(capture_num == 0){
                //std::cout<<"all"<<std::endl;
                std::string str = "";
                for(auto strin : path.consumes){
                    str.append(strin);
                }
                std::vector<std::string> temp_res;
                temp_res.push_back(str);
                //std::cout<<str<<std::endl;
                result.push_back(temp_res);
            }

            else if(capture_num != 0){
                // 在开头插入以输出完整匹配
                //std::cout<<"capture!=0"<<std::endl;
                nfa.group_begin.insert(nfa.group_begin.begin(),0);
                nfa.group_end.insert(nfa.group_end.begin(),1);
                // 删去重复元素
                for(int i = 0; i < nfa.group_begin.size(); i++){
                    for(int j = i + 1; j < nfa.group_begin.size(); j++){
                        if(nfa.group_begin[j] == nfa.group_begin[i]){
                            if(nfa.group_end[j] == nfa.group_end[i]){
                                nfa.group_begin.erase(nfa.group_begin.begin()+j);
                                nfa.group_end.erase(nfa.group_end.begin()+j);
                            }
                        }
                    }
                }
                // for(int i = 0; i < nfa.group_begin.size(); i++){
                //     std::cout<<nfa.group_begin[i]<<" ";
                //     std::cout<<nfa.group_end[i]<<std::endl;
                // }
                std::vector<std::string> temp_res;
                // std::cout<<"----------"<<std::endl;
                for(int i = 0; i < nfa.group_begin.size(); i++){  
                    // 对每一个区间找最长state序列
                    //std::vector<int> group_match;
                    std::string str = "";
                    for(int j = 0; j < path.states.size(); j++){
                        if(path.states[j] == nfa.group_begin[i]){
                            // std::cout<<"nfa.group_begin["<<i<<"]="<<nfa.group_begin[i]<<std::endl;
                            // std::cout<<"nfa.group_end["<<i<<"]="<<nfa.group_end[i]<<std::endl;
                            for(int k = path.states.size() - 1; k >= j; k--){
                                //std::cout<<"path.states["<<k<<"]="<<path.states[k]<<std::endl;
                                if(path.states[k] == nfa.group_end[i]){
                                    //std::cout<<"find it"<<std::endl;
                                    for(int it = j; it < k; it++){
                                        //group_match.push_back(path.states[it]);
                                        //std::cout<<path.states[it]<<" ";
                                        //std::cout<<"in"<<std::endl;
                                        str = str + path.consumes[it];
                                        // std::cout<<"out"<<std::endl;
                                    }
                                }
                            }
                        }
                    }
                    //std::cout<<str<<std::endl;
                    temp_res.push_back(str);
                }
                result.push_back(temp_res);   
            }
        }   
        // std::cout<<"-------------------------------------"<<std::endl;
        // for(int i = 0; i < nfa.group_begin.size(); i++){
        //     std::cout<<nfa.group_begin[i]<<"->"<<nfa.group_end[i]<<std::endl;
        // }
    }
    return result;
}

/**
 * 在给定的输入文本上，进行基于正则表达式的替换，返回替换完成的结果。
 * 需要支持分组替换，如$1表示将此处填入第一个分组匹配到的内容。具体的规则和例子详见文档。
 * @param text 输入的文本
 * @param replacement 要将每一处正则表达式的匹配结果替换为什么内容
 * @return 替换后的文本
 */
std::string Regex::replaceAll(std::string text, std::string replacement) {
    // TODO 请你完成这个函数
    std::vector<std::vector<std::string>> matched = matchAll(text);
    // 被替换的位置
    std::vector<int> replace_pos; 
    // 要替换分组               
    std::vector<int> replace_num;                           
    for (int i = 0; i < replacement.length() - 1; i++) 
    {
        if (replacement[i] == '$')
        {
            int num = 0;
            if (replacement[i + 1] >= '0' && replacement[i + 1] <= '9')
            {
                num = num + replacement[i + 1] - '0';
                replacement.erase(replacement.begin() + i + 1); 
                if (i + 2 < replacement.length() && replacement[i + 2] >= '0' && replacement[i + 2] <= '9')
                {
                    num = num * 10;
                    num = num + replacement[i + 2] - '0';
                    replacement.erase(replacement.begin() + i + 2);
                }
                replace_pos.push_back(i);
                replace_num.push_back(num);
            }
            else if (replacement[i + 1] == '$')
                replacement.erase(replacement.begin() + i + 1);
        }
    }
    int start = 0;
    for (int i = 0; i < matched.size(); i++) 
    {
        std::string replace = replacement;
        // 前面替换造成的shift
        int shift = 0; 
        for (int j = 0; j < replace_pos.size(); j++)
        {
            replace.replace(shift + replace_pos[j], 1, matched[i][replace_num[j]]); 
            shift = shift + matched[i][replace_num[j]].size() - 1;
        }
        std::string str = matched[i][0];        
        int pos = text.find(str, start);       
        text.replace(pos, str.length(), replace);
        start = start + pos - start + replace.length(); 
    }
    return text;
}


/**
 * 解析正则表达式的字符串，生成语法分析树。
 * 你应该在compile函数中调用一次本函数，以得到语法分析树。
 * 通常，你不需要改动此函数，也不需要理解此函数实现每一行的具体含义。
 * 但是，你应当对语法分析树的数据结构(RegexContext)有一定的理解，作业文档中有相关的教程可供参考。
 * @param pattern 要解析的正则表达式的字符串
 * @return RegexContext类的对象的指针。保证不为空指针。
 */
regexParser::RegexContext *Regex::parse(const std::string &pattern) {
    if (antlrInputStream) throw std::runtime_error("此Regex对象已被调用过一次parse函数，不可以再次调用！");
    antlrInputStream = new antlr4::ANTLRInputStream(pattern);
    antlrLexer = new regexLexer(antlrInputStream);
    antlrTokenStream = new antlr4::CommonTokenStream(antlrLexer);
    antlrParser = new regexParser(antlrTokenStream);
    regexParser::RegexContext *tree = antlrParser->regex();
    if (!tree) throw std::runtime_error("parser解析失败(函数返回了nullptr)");
    auto errCount = antlrParser->getNumberOfSyntaxErrors();
    if (errCount > 0) throw std::runtime_error("parser解析失败，表达式中有" + std::to_string(errCount) + "个语法错误！");
    if (antlrTokenStream->LA(1) != antlr4::Token::EOF)
        throw std::runtime_error("parser解析失败，解析过程未能到达字符串结尾，可能是由于表达式中间有无法解析的内容！已解析的部分："
                                 + antlrTokenStream->getText(antlrTokenStream->get(0),
                                                             antlrTokenStream->get(antlrTokenStream->index() - 1)));
    return tree;
}

// 此析构函数是为了管理ANTLR语法分析树所使用的内存的。你不需要阅读和理解它。
Regex::~Regex() {
    delete antlrInputStream;
    delete antlrLexer;
    delete antlrTokenStream;
    delete antlrParser;
}
