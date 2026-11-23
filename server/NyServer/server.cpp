#include "server.h"

#include <iostream>
#include <functional>
#include <memory>
#include <string>

// Gemmer
// Med HTTP


// construktor gem refernecer til status og data
GameHandler::GameHandler(GameStatus& status, GameData& data)
        : m_status{status}, m_data{data}
    {}

// Konverter enum til string
std::string to_string(GameStatus::Status status)
{
    switch (status) {
        case GameStatus::Status::INACTIVE: return "Inactive";
        case GameStatus::Status::RUNNING:  return "Running";
        case GameStatus::Status::FOUND:    return "Found";
        default:                          return "unknown";
    }
}

 auto on_gamestart(const restinio::request_handle_t& req,
                      rr::route_params_t)
    {
        auto resp = req->create_response(restinio::status_no_content());

        // Kald funktionen, der sætter spilstatus til RUNNING

        return resp.done();
    }

    auto on_gamestatus(const restinio::request_handle_t& req,
                       rr::route_params_t) const
    {
        auto resp = init_resp(req->create_response());

        GameStateOgData Resp;
        Resp.state = to_string(m_status.getGameStatus());
        Resp.distance = m_data.getDistance();

        resp.set_body(json_dto::to_json(Resp));
        return resp.done();
    }


// Router
auto make_router(GameHandler& handler)
{
    auto router = std::make_unique<router_t>();

    auto by = [&](auto method) {
        using namespace std::placeholders;
        return std::bind(method, &handler, _1, _2);
    };

    router->http_get(
        "/gamestatus", by(&GameHandler::on_gamestatus));

    router->http_post(
        "/gamestart", by(&GameHandler::on_gamestart));

    // Hvis ingen route matcher
    router->non_matched_request_handler([](auto req) {
        return req->create_response(restinio::status_not_found())
            .connection_close()
            .done();
    });

    return router;
}

// Main til test
int main()
{
    // Opret objekter med klassernes default-constructors
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