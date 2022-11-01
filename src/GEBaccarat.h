#pragma once
#include <map>
#include <vector>
#include <string>
#include "GEConfig.h"

namespace GE{
    class Baccarat{
    private:
        Baccarat() = delete;
        ~Baccarat() = delete;

    public:
        enum class GoodRoadType{
            None,
            Dragon,
            Jump,
            Room,
            Sit,
            Triangle
        };

        typedef struct Result{
            int index;
            int count;
            char cards[8];
            GoodRoadType goodRoadType;
            int goodRoadTypeParam1;
            int goodRoadTypeParam2;
        }Result;

        typedef struct TableInfo{
            int64_t tableId{};
            int32_t round{};
            std::string roundUUID;
            char cards[52*8] = {};
            int32_t cut{};
            std::string btcs[52*8];
            std::string btcTails[52*8];
            std::string btcChecks[52*8];
            std::string encripted;
            int turn{};
            std::vector<Result> results;
        }TableInfo;

    public:
        static ERRORCODE startNewRound(long tableId, int round, const std::string& roundUUID);
        static TableInfo* getTableInfo(long tableId);
        static Result* getResult(long tableId, int round, int turn, ERRORCODE& errorCode);
        static bool autoCheckEnd(long tableId);
        static ERRORCODE decode(long tableId, const std::string& roundUUID, int turn, const std::string& encripted, TableInfo& table);

    private:
        static std::string cardsString(Baccarat::TableInfo& table);
        static void newRound(Baccarat::TableInfo& table);
        static bool checkRisk(Baccarat::TableInfo& table);

    private:
        static std::map<int, TableInfo> _tables;
    };
}