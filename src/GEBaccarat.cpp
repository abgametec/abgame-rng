#include "GEBaccarat.h"
#include <spdlog/spdlog.h>
#include "GEUtils.h"

namespace GE{
    static std::string getCardStr(int id, int no){
        int a = (id-1)%13+1;
        int c = (id-1)/13;
        const char* ccode = "SHCD";
        int cn = no;
        return Utils::format("%d%d%c%d%d%d%s", a/10, a%10, ccode[c], cn/100, (cn/10)%10, cn%10, Utils::randomString(10).c_str());
    }

    static int getCardNo(int n){
        if(n==0){ return 0; }
        return (n-1)%13+1;
    }

    static char getCardPoint(char n){
        if(n==0){ return 0; }
        char p = getCardNo(n);
        return p<10?p:0;
    }

    static std::string getCardStr(char index){
        static const char* flowers[] = {"♠", "♥", "♣", "♦"};
        static const char* nums[] = {"1","2","3","4","5","6","7","8","9","10","J","Q","K"};
        return index==0 ? "/" : (std::string(flowers[(index-1)/13]) + std::string(nums[(index-1)%13]));
    }

    static char getCardsPoint(char a, char b, char c=0){
        return (getCardPoint(a) + getCardPoint(b) + getCardPoint(c))%10;
    }

    Baccarat::Result* Baccarat::getResult(long tableId, int round, int turn, ERRORCODE& errorCode){
        auto table_it = _tables.find(tableId);
        if(table_it == _tables.end()){
            spdlog::warn("找不到所请求的桌号结果tableId:{}, round:{}, turn:{}", tableId, round, turn);
            errorCode = ERRORCODE::CHECK_PARAM_ERROR;
            return nullptr;
        }

        auto& table = table_it->second;
        if(table.round != round || table.turn != turn-1){
            spdlog::warn("结果请求校验错误tableId:{}, round:{}, turn:{}", tableId, round, turn);
            errorCode = ERRORCODE::RESULT_NOT_FOUND;
            return nullptr;
        }

        if(table.results.size() < turn){
            spdlog::warn("超出开奖结果范围tableId:{}, round:{}, turn:{}", tableId, round, turn);
            errorCode = ERRORCODE::EXTEND_RANGE;
            return nullptr;
        }

        Result& result = table.results[table.turn++];

        return &result;
    }

    bool Baccarat::autoCheckEnd(long tableId){
        auto table_it = _tables.find(tableId);
        if(table_it == _tables.end()){
            return true;
        }
        auto& table = table_it->second;
        if(table.results.size() == table.turn){
            _tables.erase(table_it);
            return true;
        }

        return false;
    }

    Baccarat::TableInfo* Baccarat::getTableInfo(long tableId){
        auto table_it = _tables.find(tableId);
        if(table_it != _tables.end()){
            return &table_it->second;
        }

        return nullptr;
    }

    Baccarat::GoodRoadType detectGoodRoad(const std::vector<int>& roadList, int& outParam1, int& outParam2){
        // 长龙
        int len = roadList.size();
        if(len >= 1 && roadList[len-1] >= 5){
            outParam1 = roadList[len-1];
            return Baccarat::GoodRoadType::Dragon;
        }

        // 单跳
        if(len >=5
            && roadList[len-1]==1
            && roadList[len-2]==1
            && roadList[len-3]==1
            && roadList[len-4]==1
            && roadList[len-5]==1){
                outParam1 = 5;
                while(len > outParam1 && roadList[len-outParam1-1] == 1){
                    ++outParam1;
                }
                return Baccarat::GoodRoadType::Jump;
        }

        // 排排坐
        if(len >= 4
            && roadList[len-4] > 1
            && roadList[len-4]==roadList[len-3]
            && roadList[len-3]==roadList[len-2]
            && roadList[len-1] <= roadList[len-2]){
                outParam1 = roadList[len-1] + roadList[len-2] + roadList[len-3] + roadList[len-4];
                int index = 4;
                while(len > index && roadList[len-index-1] == roadList[len-index]){
                    outParam1 += roadList[len-index-1];
                    ++index;
                }
                outParam2 = roadList[len-2];
                return Baccarat::GoodRoadType::Sit;
        }

        // A房B厅
        if(len >= 4 && (roadList[len-1] != 1 || roadList[len-2] != 1)){
            int b = roadList[len-1];
            int a = roadList[len-2];
            int index = 3;
            int count = b + a;
            while(len >= index && roadList[len-index] == b){
                count += b;
                std::swap(a, b);
                ++index;
            }
            if(index > 4){
                outParam1 = count;
                outParam2 = (a << 16) | b;
                return Baccarat::GoodRoadType::Room;
            }
        }

        // A房B厅
        if(len >= 5 && roadList[len-1] < roadList[len-3]){
            int b = roadList[len-2];
            int a = roadList[len-3];
            int index = 4;
            int count = b + a + roadList[len-1];
            while(len >= index && roadList[len-index] == b){
                count += b;
                std::swap(a, b);
                ++index;
            }
            if(index > 5){
                outParam1 = count;
                outParam2 = (a << 16) | b;
                return Baccarat::GoodRoadType::Room;
            }
        }

        // 三角
        if(len >= 4){
            if(roadList[len - 1] == roadList[len - 2]-1
                && roadList[len - 2] == roadList[len - 3]-1
                && roadList[len - 3] == roadList[len - 4]-1){
                    int index = 5;
                    outParam1 = roadList[len - 1] + roadList[len - 2] + roadList[len - 3] + roadList[len - 4];
                    outParam2 = roadList[len - 4];
                    while(len >= index && roadList[len-index+1] == roadList[len-index]-1){
                        outParam2 = roadList[len-index];
                        outParam1 += roadList[len-index];
                        ++index;
                    }
                    return Baccarat::GoodRoadType::Triangle;
                }
            if(roadList[len - 1] == roadList[len - 2]+1
                && roadList[len - 2] == roadList[len - 3]+1
                && roadList[len - 3] == roadList[len - 4]+1){
                    int index = 5;
                    outParam1 = roadList[len - 1] + roadList[len - 2] + roadList[len - 3] + roadList[len - 4];
                    outParam2 = roadList[len - 1];
                    while(len >= index && roadList[len-index+1] == roadList[len-index]+1){
                        outParam1 += roadList[len-index];
                        ++index;
                    }
                    return Baccarat::GoodRoadType::Triangle;
                }
        }

        // 三角
        if(len >= 5){
            if(roadList[len - 2] == roadList[len - 3]-1
                && roadList[len - 3] == roadList[len - 4]-1
                && roadList[len - 4] == roadList[len - 5]-1
                && roadList[len - 1] < roadList[len -2]){
                int index = 6;
                outParam1 = roadList[len - 1] + roadList[len - 2] + roadList[len - 3] + roadList[len - 4] + roadList[len - 5];
                outParam2 = roadList[len - 5];
                while(len >= index && roadList[len-index+1] == roadList[len-index]-1){
                    outParam1 += roadList[len-index];
                    outParam2 = roadList[len-index];
                    ++index;
                }
                return Baccarat::GoodRoadType::Triangle;
            }
            if(roadList[len - 2] == roadList[len - 3]+1
                && roadList[len - 3] == roadList[len - 4]+1
                && roadList[len - 4] == roadList[len - 5]+1){
                int index = 6;
                outParam1 = roadList[len - 1] + roadList[len - 2] + roadList[len - 3] + roadList[len - 4] + roadList[len - 5];
                outParam2 = roadList[len - 2];
                while(len >= index && roadList[len-index+1] == roadList[len-index]+1){
                    outParam1 += roadList[len-index];
                    ++index;
                }
                return Baccarat::GoodRoadType::Triangle;
            }
        }

        return Baccarat::GoodRoadType::None;
    }

    void generateResult(char cards[52*8], int cut, std::vector<Baccarat::Result>& results){
        int skip = getCardPoint(cards[cut]);
        if(skip == 0){ skip = 10; }
        int index = cut + skip+1;
        int curWin = 0;
        std::vector<int> daluList;
        int len = 52*8;
        while(true){
            if(index > len){
                break;
            }

            int cardsCount = 4;
            char la = cards[index < len ? index : (index-len)];
            char lb = cards[(index+2) < len ? (index+2) : (index+2-len)];
            char ra = cards[(index+1) < len ? (index+1) : (index+1-len)];
            char rb = cards[(index+3) < len ? (index+3) : (index+3-len)];
            char lc = 0;
            char rc = 0;
            char lr = getCardsPoint(la, lb);
            char rr = getCardsPoint(ra, rb);
            if(lr < 8 && rr < 8){
            if(lr<=5){
                lc = cards[(index + cardsCount) < len ? (index + cardsCount) : (index + cardsCount-len)];
                lr = getCardsPoint(la, lb, lc);
                ++cardsCount;
            }
            int lcp = getCardPoint(lc);
            if(rr<=2
                || (rr==3 && (lc==0 || lcp!=8))
                || (rr==4 && (lc==0 || (lcp!=0 && lcp!=1 && lcp!=8 && lcp!=9)))
                || (rr==5 && (lc==0 || (lcp!=0 && lcp!=1 && lcp!=2 && lcp!=3 && lcp!=8 && lcp!=9)))
                || (rr==6 && (lcp==6 || lcp == 7))){
                    rc = cards[(index + cardsCount) < len ? (index + cardsCount) : (index + cardsCount-len)];
                    rr = getCardsPoint(ra, rb, rc);
                    ++cardsCount;
                }
            }

            Baccarat::GoodRoadType goodRoadType = Baccarat::GoodRoadType::None;
            int goodRoadTypeParam1 = 0;
            int goodRoadTypeParam2 = 0;
            if(lr==rr){
                if(results.size()>0 && results.back().goodRoadType != Baccarat::GoodRoadType::None){
                    goodRoadType = results.back().goodRoadType;
                    goodRoadTypeParam1 = results.back().goodRoadTypeParam1;
                    goodRoadTypeParam2 = results.back().goodRoadTypeParam2;
                }
            }else if(lr>rr && curWin==1 || lr<rr && curWin==2){
                daluList[daluList.size()-1]++;
            }else{
                daluList.emplace_back(1);
                curWin = lr>rr ? 1 : 2;
            }

            goodRoadType = detectGoodRoad(daluList, goodRoadTypeParam1, goodRoadTypeParam2);

            results.push_back(Baccarat::Result{
                index, cardsCount,
                {la,ra,lb,rb,lc,rc,lr,rr},
                goodRoadType, goodRoadTypeParam1, goodRoadTypeParam2
            });

            index += cardsCount;
        }
    }

    void Baccarat::newRound(Baccarat::TableInfo& table){
        srand(time(0));

        for(int i=0; i<8; ++i){
            for(int j=1; j<=52; ++j){
                table.cards[i*52+j-1] = j;
            }
        }

        for(int i=0; i<52*8-1; ++i){
            int j = i+rand()%(52*8-i);
            std::swap(table.cards[i], table.cards[j]);
        }
        for(int i=0; i<52*8; ++i){
            table.btcs[i] = getCardStr(table.cards[i], i+1);
            table.btcTails[i] = table.btcs[i].substr(table.btcs[i].size()-2);
            table.btcChecks[i] = Utils::sha512(table.btcs[i]);
        }
        table.cut = Utils::randomInt(50, 101);
        int64_t big_tableId = google::protobuf::BigEndian::ToHost64(table.tableId);
        int32_t big_round = google::protobuf::BigEndian::ToHost32(table.round);
        int32_t bit_cut = google::protobuf::BigEndian::ToHost32(table.cut);
        std::string pack = std::string((char*)(&big_tableId), 8)
                        + std::string((char*)(&big_round), 4)
                        + std::string((char*)(&bit_cut), 4)
                        + std::string(table.cards);
        for(int i=0; i<52*8; ++i){
            pack += table.btcs[i];
        }

        table.encripted = Utils::aes256_encode(pack, table.roundUUID);


        generateResult(table.cards, table.cut, table.results);
    }

    std::string Baccarat::cardsString(Baccarat::TableInfo& table){
        std::string str;
        int no = 0;
        for(auto& result : table.results){
            str += std::to_string(++no) + ": " + getCardStr(result.cards[0]) + " " + getCardStr(result.cards[2]) + " " + getCardStr(result.cards[4]) + " vs "
                + getCardStr(result.cards[1]) + " " + getCardStr(result.cards[3]) + " " + getCardStr(result.cards[5]) + " | ";
        }

        str += "/n";

        for(auto& result : table.results){
            str += result.cards[6] == result.cards[7] ? "和" : (result.cards[6] > result.cards[7] ? "闲" : "庄");
        }

        return str;
    }

    std::map<int, Baccarat::TableInfo> Baccarat::_tables;
    ERRORCODE Baccarat::startNewRound(long tableId, int round, const std::string& roundUUID){
        auto table_it = _tables.find(tableId);

        if(table_it==_tables.end()){
            _tables.emplace(std::make_pair(tableId, TableInfo()));
            table_it = _tables.find(tableId);
        }

        TableInfo newTableInfo;
        table_it->second = std::move(newTableInfo);
        auto& table = table_it->second;
        table.round = round;
        table.roundUUID = roundUUID;
        table.tableId = tableId;
        newRound(table);

        return ERRORCODE::OK;
    }

    ERRORCODE Baccarat::decode(long tableId, const std::string& roundUUID, int turn, const std::string& encripted, TableInfo& table){
        std::string decripted = Utils::aes256_decode(encripted, roundUUID);
        if(decripted.size() == 0){
            spdlog::warn("解密失败，数据为空");
            return ERRORCODE::DECODE_ERROR;
        }
        if(decripted.size() != 16+8*52+8*52*16+1){
            spdlog::warn("解密失败，数据位数不对:{},{}", decripted.size(),16+8*52+8*52*16);
            return ERRORCODE::DECODE_ERROR;
        }
        table.roundUUID = roundUUID;
        table.tableId = google::protobuf::BigEndian::FromHost64(*((int64_t*)decripted.c_str()));
        if(table.tableId != tableId){
            return ERRORCODE::CHECK_PARAM_ERROR;
        }
        table.round = google::protobuf::BigEndian::FromHost32(*((int32_t*)(decripted.c_str()+8)));
        table.cut = google::protobuf::BigEndian::FromHost32(*((int32_t*)(decripted.c_str()+12)));
        memcpy(table.cards, decripted.c_str()+16, 8*52);
        for(int i=0; i<8*52; ++i){
            table.btcs[i] = std::string(decripted.c_str()+16+8*52+i*(16)+1, 16);
        }
        if(turn != 0){
            generateResult(table.cards, table.cut, table.results);
        }
        return ERRORCODE::OK;
    }
};