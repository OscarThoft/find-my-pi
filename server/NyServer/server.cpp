#include <iostream>
#include <string>

// Gemmer!!!! 

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

// to string function 
    std::string to_string(gameStatus::Status status)
{
    switch (status) {
        case Status::INACTIVE:    return "Inactive";
        case Status::RUNNING: return "Running";
        case Status::FOUND: return "Found";
        default: return "unknown";
    }
}

// handler game 
class GameHandler {
public:
explicit GameHandler(GameStatus& status, GameData& data)
    : m_status{status}, m_data{data}
    {}

 // GameHandler(const GameHandler&) = delete;
 // GameHandler (GameHandler &&) = delete;

auto on_gamestart(const restinio::request_handle_t& req, rr::route_params_t) {
    auto resp = init_resp(req->create_response());

    // kald funktion der sætter game status = running;

    resp.set_body("\"Game started\"");
    return resp.done();
 }

 auto on_gamestatus(const restinio::request_handle_t& req, rr::route_params_t) const 
    {
        auto resp = init_resp(req->create_response());

        GameStateOgData Resp;
            Resp.state = to_string(m_status.getGameStatus()); 
            Resp.distance = m_data.getDistance();

        resp.set_body(json_dto::to_json(Resp));
        return resp.done();
    };

 private:
    // inkludere instancer af typen GameStatus og GameData 
    GameStatus& m_status;
    GameData& m_data;

    // definere en struct der indeholder status og afstand
    
    struct GameStateOgData 
        {
            std::string state; 
            double distance; 

            template<typename JSON_IO>
            void json_io(JSON_IO & io) 
            {
                io & json_dto::mandatory("state", state);
                io & json_dto::mandatory("distance", distance);
            }    
        };

    template<typename RESP> static RESP init_resp(RESP resp) 
    {
        resp.append_header("Content-Type", "application/json; charset=utf-8");
        return resp;
    }
};


// router 

auto make_router(GameHandler & handler) {
    auto router = std::make_unique<router_t>();

    auto by = [&](auto method) {
        using namespace std::placeholders;
        return std::bind(method, &handler, _1, _2);
    };

    router->http_get("/gamestatus", by(&GameHandler::on_gamestatus));
    router->http_post("/gamestart", by(&GameHandler::on_gamestart));


    // hvis ingen router matcher
    router->non_matched_request_handler([](auto req) {
        return req->create_response(restinio::status_not_found())
            .connection_close()
            .done();
    });

    return router;
}

// main

int main()
{
    // oprettere tomme/default GameStatus objekter og GameData objecter 
    GameStatus status;
    GameData data;
    
    GameHandler handler{status, data};

   using traits_t = restinio::traits_t<
   restinio::asio_timer_manager_t, 
   restinio::single_threaded_ostream_logger_t, 
   router_t>;

   restinio::run(
      restinio::on_this_thread<traits_t>()
         .address("0.0.0.0")
         .port(9000)
         .request_handler(make_router(handler))
   );
}
