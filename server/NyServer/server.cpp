#include "server.h"

#include <iostream>
#include <functional>
#include <memory>
#include <string>


// Gemmer
// Med HTTP


// construktor gem referencer til status og data
GameHandler::GameHandler(gameStatus& status, gameData& data, gemmerController& controller)
        : m_status{status}, m_data{data}, m_controller{controller}
    {}

// Konverter enum til string
std::string to_string(gameStatus::Status status)
{
    switch (status) {
        case gameStatus::Status::INACTIVE: return "Inactive";
        case gameStatus::Status::RUNNING:  return "Running";
        case gameStatus::Status::FOUND:    return "Found";
        default:                          return "unknown";
    }
}


restinio::request_handling_status_t GameHandler::on_gamestart( const restinio::request_handle_t& req,
        rr::route_params_t params)
    {
        auto resp = req->create_response(restinio::status_no_content());

        m_controller.startGame();

        return resp.done();
    }

restinio::request_handling_status_t GameHandler::on_gamestatus(
        const restinio::request_handle_t& req, rr::route_params_t params) const
    {
        auto resp = init_resp(req->create_response()); 

        auto Resp = m_controller.requestData();

        resp.set_body(json_dto::to_json(Resp));
        return resp.done();
    }


// Router
std::unique_ptr<router_t> make_router(GameHandler& handler)
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
void GameHandler::run()
{
    using traits_t = restinio::traits_t<
        restinio::asio_timer_manager_t,
        restinio::single_threaded_ostream_logger_t,
        router_t>;

    restinio::run(
        restinio::on_this_thread<traits_t>()
            .address("0.0.0.0")
            .port(9000)
            .request_handler(make_router(*this))
    );
}