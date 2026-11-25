#pragma once

#include <string>
#include <memory>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>
#include <cstdint>
#include <map>
#include <restinio/websocket/websocket.hpp>

#include "../Hider/inc/gameStatus.h"
#include "../Hider/inc/gameData.h"
#include "../Hider/inc/gemmerController.h"
// chat siger erstant med class gemmerController for at undgå cirkulær includes?
// og istedet include det i serveres.cpp??

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

//websocket 
namespace rws = restinio::websocket::basic;
using ws_registry_t = std::map< std::uint64_t, rws::ws_handle_t >;


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

    // websocket 
   //  auto on_live_update();
   restinio::request_handling_status_t on_live_update(
    const restinio::request_handle_t& req,
    rr::route_params_t params);

    // sendMessage(); - KAN EVT GØRES PRIVATE!!
    void sendMessage(const std::string& message);



    // skal være public HVIS send sendData skal bruge det
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

     // send ny data UDEN struct 
    // void sendData(const std::string& state, double distance);

    // eller MED  struct 
    void sendData(GameStateAndData data);

private:
        // web socket
    ws_registry_t m_registry;

    // til sendData på event loop??
    restinio::asio_ns::io_context m_ioContext;

    gameStatus& m_status;
    gameData& m_data;
    gemmerController& m_controller;

    static std::string to_string(gameStatus::Status status);

   

    template<typename RESP>
    static RESP init_resp(RESP resp)
    {
        resp.append_header(
            "Content-Type", "application/json; charset=utf-8");
        return resp;
    }
};

std::unique_ptr<router_t> make_router(GameHandler& handler);