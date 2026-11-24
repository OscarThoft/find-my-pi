#pragma once

#include <string>
#include <memory>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>

#include "../../Hider/inc/gameStatus.h"
#include "../../Hider/inc/gameData.h"
#include "../../Hider/inc/gemmerController.h"

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

class GameHandler {
public:
    explicit GameHandler(gameStatus& status, gameData& data, gemmerController& controller);

    restinio::request_handling_status_t on_gamestart(
        const restinio::request_handle_t& req,
        rr::route_params_t params);

    restinio::request_handling_status_t on_gamestatus(
        const restinio::request_handle_t& req,
        rr::route_params_t params) const;

    void run();

private:
    gameStatus& m_status;
    gameData& m_data;
    gemmerController& m_controller;

    static std::string to_string(gameStatus::Status status);

    struct GameStateAndData {
        std::string state;
        double distance;

        template<typename JSON_IO>
        void json_io(JSON_IO& io)
        {
            io & json_dto::mandatory("state", state);
            io & json_dto::mandatory("distance", distance);
        }
    };

    template<typename RESP>
    static RESP init_resp(RESP resp)
    {
        resp.append_header(
            "Content-Type", "application/json; charset=utf-8");
        return resp;
    }
};

std::unique_ptr<router_t> make_router(GameHandler& handler);