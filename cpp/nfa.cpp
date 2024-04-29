#include "nfa.h"
#include <sstream>
#include "utils.h"

/**
 * 在自动机上执行指定的输入字符串。
 * @param text 输入字符串
 * @return 若拒绝，请 return Path::reject(); 。若接受，请手工构造一个Path的实例并返回。
 */

Path result;
std::vector<std::pair<int, std::string>> path; // 记录第x步的状态和剩余串
// 标记是否为/b/B
std::vector<bool> is_bB;
// 第一次实验要求支持的特殊字符有：\d \w \s \D \W \S \.
// 前六个的定义同一般正则表达式中的定义，最后一个\.则等同于一般正则表达式中的.，可匹配任何字符。
bool NFA::special_match(char by, char c){
    if(by == 'd'){
        if(c >= '0' && c <= '9'){
            return true;
        }
    }
    else if(by == 'w'){
        if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||(c == '_')){
            return true;
        }
    }
    else if(by == 's'){
        if(c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'){
            return true;
        }
    }
    else if(by == 'D'){
        if(special_match('d',c)==false){
            return true;
        }
    }
    else if(by == 'W'){
        if(special_match('w',c)==false){
            return true;
        }
    }
    else if(by == 'S'){
        if(special_match('s',c)==false){
            return true;
        }
    }
    else if(by == '.'){
        if (c != '\r' && c != '\n'){
            return true;
        }
        else if (S_flag == true){
            std::cout<<"S"<<std::endl;
            return true;
        }
    }
    return false;
}

// 记录上一个被消耗的字符
char nowchar = NULL;
bool NFA::fun(int q, std::string str, int step, std::vector<int> e_state, bool is_start){
    result.consumes.clear();
    result.states.clear();
    //std::cout<<"fun1"<<std::endl;
    for(int i = 0; i < e_state.size(); i++){
        //std::cout<<"e_state:"<<e_state[i]<<std::endl;
        if(path[step].first == e_state[i]){
            return false;
        }
    }
    //std::cout<<path[step].first<<std::endl;
    //if(is_final[path[step].first] == true && path[step].second == ""){
    if(is_final[path[step].first] == true){
        //std::cout<<"is_final["<<path[step].first<<"]=true"<<std::endl;
        for(int i = 0; i < step; i++){
            //std::cout<<"i="<<i<<std::endl;
            result.states.push_back(path[i].first);
            if(path[i].second.length() - path[i+1].second.length() == 1 && is_bB[i] == false){
                result.consumes.push_back(std::string(1,path[i].second[0]));
            }
            else if(path[i+1].second.length() == path[i].second.length() || is_bB[i] == true){
                //std::cout<<"epsilon"<<i<<std::endl;
                result.consumes.push_back("");
            }
            //std::cout<<result.consumes[i]<<" ";
        }
        //std::cout<<std::endl;
        result.states.push_back(path[step].first);
        return true;
    }

    bool doreverse = false;

    for(int i = 0; i < rules[path[step].first].size(); i++){
        Rule rule = rules[path[step].first][i];
        //std::cout<<"rule="<<rule.by[0]<<" str[0]="<<str[0]<<std::endl;
        if(rule.reverse == false){
            //std::cout<<"reverse false"<<std::endl;
            //std::cout<<"reverse = false"<<std::endl;
            // epsilon转移
            if(rule.type == EPSILON){
                // 一般epsilon转移
                if(rule.by == ""){
                    std::string new_str = str;
                    e_state.push_back(path[step].first);
                    // std::cout<<"EPSILON ";
                    //std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                    // std::cout<<"str:"<<str<<std::endl;
                    // std::cout<<std::endl;
                    path.push_back({rule.dst,new_str});
                    is_bB.push_back(false);
                    if(fun(rule.dst, new_str, step+1, e_state,is_start)){
                        return true;
                    };
                    is_bB.pop_back();
                    path.pop_back();
                    e_state.pop_back();
                }

                // anchor
                // 正常情况下，匹配整个字符串
                else if(rule.by == "^"){
                    // 是开头，包括是m情况下的单行开头
                    if(is_start == true){
                        is_start = false;
                        std::string new_str = str;
                        e_state.push_back(path[step].first);
                        // std::cout<<"^"<<std::endl;
                        //std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                        //std::cout<<"str:"<<str<<std::endl;
                        //std::cout<<std::endl;
                        path.push_back({rule.dst,new_str});
                        is_bB.push_back(false);
                        if(fun(rule.dst, new_str, step+1, e_state,is_start)){
                            return true;
                        };
                        is_bB.pop_back();
                        path.pop_back();
                        e_state.pop_back();
                    }
                    else{
                        return false;
                    }
                }
                else if(rule.by == "$"){
                    // 是结尾
                    //std::cout<<"$$"<<std::endl;
                    if((str == "") ||(M_flag == true && (str[0] == '\n' || str[0] == '\r'))){
                        // std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                        // std::cout<<"$"<<std::endl;
                        // path[step].first = 1;
                        std::string new_str = str;
                        e_state.push_back(path[step].first);
                        //std::cout<<"$"<<std::endl;
                        //std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                        // std::cout<<"str:"<<str<<std::endl;
                        // std::cout<<std::endl;
                        path.push_back({rule.dst,new_str});
                        is_bB.push_back(false);
                        // if(fun(rule.dst, new_str, step+1, e_state, is_start)){
                        //     //std::cout<<"true"<<std::endl;
                        //     return true;
                        // };
                        // is_bB.pop_back();
                        // path.pop_back();
                        // e_state.pop_back();
                        // return true;
                        for(int i = 0; i < step-1; i++){
                            result.states.push_back(path[i].first);
                            if(path[i].second.length() - path[i+1].second.length() == 1 && is_bB[i] == false){
                                    result.consumes.push_back(std::string(1,path[i].second[0]));
                            }
                            else if(path[i+1].second.length() == path[i].second.length() || is_bB[i] == true){
                                //std::cout<<"epsilon"<<i<<std::endl;
                                result.consumes.push_back("");
                            }
                            //std::cout<<result.consumes[i]<<" ";
                        }
                        //std::cout<<path[step+1].first;
                        result.states.push_back(1);
                        //std::cout<<"out ";
                        return true;
                    }
                    else{
                        if(i == rules[path[step].first].size() - 1){
                            return false;
                        }
                    }
                }
                else if(rule.by == "b"){
                    // 是单词边界
                    // std::cout<<"b"<<std::endl;
                    // std::cout<<"nowchar="<<nowchar<<" "<<special_match('W',nowchar)<<std::endl;
                    // std::cout<<"str[0]="<<str[0]<<" "<<special_match('W',str[0])<<std::endl;

                    if((special_match('W',str[0]) == true && special_match('w',nowchar) == true) || (special_match('w',str[0]) == true && special_match('W',nowchar) == true) || is_start == true || str == ""){
                        //std::cout<<"bin"<<std::endl;
                        nowchar = str[0];
                        std::string new_str = str.substr(1,str.length());
                        e_state.push_back(path[step].first);
                        
                        // std::cout<<"$"<<std::endl;
                        // std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                        // std::cout<<"str:"<<str<<std::endl;
                        // std::cout<<std::endl;
                        path.push_back({rule.dst,new_str});
                        is_bB.push_back(true);
                        //path[path.size()-1].second.length()++;
                        if(fun(rule.dst, new_str, step+1, e_state, is_start)){
                            return true;
                        };
                        is_bB.pop_back();
                        path.pop_back();
                        e_state.pop_back();
                    }
                    else{
                        return false;
                    }
                }
                else if(rule.by == "B"){
                    // 不是单词边界
                    if((special_match('W',str[0]) == true && special_match('W',nowchar) == true) || (special_match('w',str[0]) == true && special_match('w',nowchar) == true)){
                        nowchar = str[0];
                        std::string new_str = str.substr(1,str.length());
                        e_state.push_back(path[step].first);
                        //std::cout<<"$"<<std::endl;
                        // std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                        // std::cout<<"str:"<<str<<std::endl;
                        // std::cout<<std::endl;
                        path.push_back({rule.dst,new_str});
                        is_bB.push_back(true);
                        if(fun(rule.dst, new_str, step+1, e_state, is_start)){
                            return true;
                        };
                        is_bB.pop_back();
                        path.pop_back();
                        e_state.pop_back();
                    }
                    else{
                        return false;
                    }
                }

                // 有区间限定
                if(rule.range_from != -1 || rule.range_to != 0){
                    rule.visit_num++;
                    std::string new_str = str;
                    e_state.push_back(path[step].first);
                    //std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                    //std::cout<<"str:"<<str<<std::endl;
                    //std::cout<<std::endl;
                    path.push_back({rule.dst,new_str});
                    is_bB.push_back(false);
                    if(fun(rule.dst, new_str, step+1, e_state,is_start)){
                        return true;
                    };
                    is_bB.pop_back();
                    path.pop_back();
                    e_state.pop_back();
                    rule.visit_num--;
                }
            }
            // 一般字符转移
            else if(rule.type == NORMAL && str != "" && rule.by[0] == str[0]){
                std::vector<int> new_state;
                nowchar = str[0];
                std::string new_str = str.substr(1,str.length());
                //std::cout<<"NORMAL ";
                //std::cout<<path[step].first<<"->"<<rule.dst<<" ";
                //std::cout<<"str:"<<str[0]<<std::endl;
                // std::cout<<std::endl;
                path.push_back({rule.dst,new_str});
                is_bB.push_back(false);
                if(fun(rule.dst, new_str, step+1, new_state, is_start)){
                    //std::cout<<"true"<<std::endl;
                    return true;
                };
                is_bB.pop_back();
                path.pop_back();
            }
            // 字符区间转移
            else if(rule.type == RANGE && str != "" && str[0] >= rule.by[0] && str[0] <= rule.to[0]){
                std::vector<int> new_state;
                nowchar = str[0];
                std::string new_str = str.substr(1,str.length());
                // std::cout<<"RANGE ";
                // std::cout<<path[step].first<<"->"<<rule.dst<<std::endl;
                // std::cout<<"str:"<<str<<std::endl;
                // std::cout<<std::endl;
                path.push_back({rule.dst,new_str});
                is_bB.push_back(false);
                if(fun(rule.dst, new_str, step+1, new_state, is_start)){
                    return true;
                };
                is_bB.pop_back();
                path.pop_back();
            }
            // 特殊字符转移
            else if(rule.type == SPECIAL && str != "" && special_match(rule.by[0],str[0]) == true){
                //std::cout<<"S="<<S_flag<<std::endl;
                std::vector<int> new_state;
                nowchar = str[0];
                std::string new_str = str.substr(1,str.length());
                // std::cout<<"SPECIAL ";
                // std::cout<<path[step].first<<"->"<<rule.dst<<" ";
                // std::cout<<"str:"<<str[0]<<std::endl;
                // std::cout<<std::endl;
                path.push_back({rule.dst,new_str});
                is_bB.push_back(false);
                if(fun(rule.dst, new_str, step+1, new_state, is_start)){
                    //std::cout<<"return true"<<std::endl;
                    return true;
                };
                is_bB.pop_back();
                path.pop_back();
            }
        }

        else if (rule.reverse == true){
            //std::cout<<"true"<<std::endl;
            doreverse = true;
        }
    }
    
    if(doreverse == true){
        // 对于取反的情况，要求每一条特殊转移都为false
        //std::cout<<"reverse true"<<std::endl;
        // 创建一个二维数组，dst相同的在同一行
        std::vector<std::vector<Rule>> same_dst;
        //int dst_index = 0;
        for(int i = 0; i < rules[path[step].first].size(); i++){
            Rule rule = rules[path[step].first][i];
            bool same_flag = false;
            //intial.push_back(rules[path[step].first][0]);
        //std::cout<<"same_dst.size()="<<same_dst.size()<<std::endl;
            if(same_dst.size() != 0){
                for(int j = 0; j < same_dst.size(); j++){
                    //std::cout<<"same_dst[j].size()="<<same_dst[j].size()<<std::endl;
                    if(same_dst[j].size() != 0 && rule.dst == same_dst[j][0].dst){
                        same_dst[j].push_back(rule);
                        same_flag = true;
                    }
                }
            }
            if(same_flag == false){
                std::vector<Rule> temp;
                temp.push_back(rule);
                same_dst.push_back(temp);
            }
        }
            bool flag = true;
        for(int i = 0; i < same_dst.size(); i++){
            for(int j = 0; j < same_dst[i].size(); j++){
                Rule rule = same_dst[i][j];
                if(rule.reverse == true){
                    // std::cout<<"str[0]="<<str[0]<<std::endl;
                    // std::cout<<"by[0]="<<rule.by[0]<<std::endl;
                    // std::cout<<"special_match="<<special_match(rule.by[0],str[0])<<std::endl;
                    // 一般字符转移
                    if(rule.type == NORMAL && str != "" && rule.by[0] == str[0]){
                        nowchar = str[0];
                        //std::cout<<"NORMAL"<<std::endl;
                        flag = false;
                    }
                    // 字符区间转移
                    else if(rule.type == RANGE && str != "" && str[0] >= rule.by[0] && str[0] <= rule.to[0]){
                        //std::cout<<"RANGE"<<std::endl;
                        nowchar = str[0];
                        flag = false;
                    }
                    // 特殊字符转移
                    else if(rule.type == SPECIAL && str != "" && special_match(rule.by[0],str[0]) == true){
                        //std::cout<<"SPECIAL"<<std::endl;
                        nowchar = str[0];
                        flag = false;
                    }  
                }
            }   
            if(flag == true){
                std::vector<int> new_state;
                std::string new_str = str.substr(1,str.length());
                path.push_back({same_dst[i][0].dst,new_str});
                is_bB.push_back(false);
                if(fun(same_dst[i][0].dst, new_str, step+1, new_state,is_start)){
                    return true;
                };
                is_bB.pop_back();
                path.pop_back();
            }
        }
    }
    
    return false;
}

Path NFA::exec(std::string text, bool is_start) {
    // TODO 请你完成这个函数
    //std::cout<<"exec"<<std::endl;
    path.clear();
    path.push_back({0,text});
    bool flag;
    std::vector<int> e_state;
    flag = fun(0,text,0,e_state,is_start);
    if(flag == true){
        //std::cout<<"consume="<<result.states.size();
        return result;
    }
    return Path::reject();
}

/**
 * 将Path转为（序列化为）文本的表达格式（以便于通过stdout输出）
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */
std::ostream &operator<<(std::ostream &os, Path &path) {
    if (!path.states.empty()) {
        if (path.consumes.size() != path.states.size() - 1)
            throw std::runtime_error("Path的len(consumes)不等于len(states)-1！");
        for (int i = 0; i < path.consumes.size(); ++i) {
            os << path.states[i] << " " << path.consumes[i] << " ";
        }
        os << path.states[path.states.size() - 1];
    } else os << std::string("Reject");
    return os;
}

/**
 * 从自动机的文本表示构造自动机
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */
NFA NFA::from_text(const std::string &text) {
    NFA nfa = NFA();
    bool reading_rules = false;
    std::istringstream ss(text);
    std::string line, type;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.find("type:") == 0) {
            type = strip(line.substr(5));
            continue;
        }
        if (type != "nfa") throw std::runtime_error("输入文件的类型不是nfa！");
        if (line.find("states:") == 0) {
            nfa.num_states = std::stoi(line.substr(7));
            for (int i = 0; i < nfa.num_states; ++i) {
                nfa.rules.emplace_back();
                nfa.is_final.push_back(false);
            }
            continue;
        } else if (line.find("final:") == 0) {
            if (nfa.num_states == 0) throw std::runtime_error("states必须出现在final和rules之前!");
            std::istringstream ss2(line.substr(6));
            int t;
            while (true) {
                ss2 >> t;
                if (!ss2.fail()) nfa.is_final[t] = true;
                else break;
            }
            reading_rules = false;
            if (ss2.eof()) continue;
        } else if (line.find("rules:") == 0) {
            if (nfa.num_states == 0) throw std::runtime_error("states必须出现在final和rules之前!");
            reading_rules = true;
            continue;
        } else if (line.find("input:") == 0) {
            reading_rules = false;
            continue;
        } else if (reading_rules) {
            auto arrow_pos = line.find("->"), space_pos = line.find(' ');
            if (arrow_pos != std::string::npos && space_pos != std::string::npos && arrow_pos < space_pos) {
                int src = std::stoi(line.substr(0, arrow_pos));
                int dst = std::stoi(line.substr(arrow_pos + 2, space_pos - (arrow_pos + 2)));
                auto content = line.substr(space_pos + 1);
                bool success = true;
                while (success && !content.empty()) {
                    auto p = content.find(' ');
                    if (p == std::string::npos) p = content.size();
                    else if (p == 0) p = 1; // 当第一个字母是空格时，说明转移的字符就是空格。于是假定第二个字母也是空格（如果不是，会在后面直接报错）
                    Rule rule{dst};
                    if (p == 3 && content[1] == '-') {
                        rule.type = RANGE;
                        rule.by = content[0];
                        rule.to = content[2];
                    } else if (p == 2 && content[0] == '\\') {
                        if (content[1] == 'e') rule.type = EPSILON;
                        else {
                            rule.type = SPECIAL;
                            rule.by = content[1];
                        }
                    } else if (p == 1 && (p >= content.length() || content[p] == ' ')) {
                        rule.type = NORMAL;
                        rule.by = content[0];
                    } else success = false;
                    nfa.rules[src].push_back(rule);
                    content = content.substr(std::min(p + 1, content.size()));
                }
                if (success) continue;
            }
        }
        throw std::runtime_error("无法parse输入文件！失败的行： " + line);
    }
    if (!ss.eof()) throw std::runtime_error("无法parse输入文件！(stringstream在getline的过程中发生错误)");;
    return nfa;
}
