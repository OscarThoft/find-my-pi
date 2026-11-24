#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>

// GEMMER: HTTP-server paa TCP-port 9000.
namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

enum class Status { INACTIVE, RUNNING, FOUND };

std::string to_string(Status status)
{
    switch (status) {
        case Status::INACTIVE: return "Inactive";
        case Status::RUNNING: return "Running";
        case Status::FOUND: return "Found";
        default: return "unknown";
    }
}

class GameHandler {
public:
    auto on_gamestart(const restinio::request_handle_t& req, rr::route_params_t)
    {
        auto resp = req->create_response(restinio::status_no_content());
        m_status = Status::RUNNING; // Kun test: ingen getter eller anden klasse.
        std::cout << "Spillet er startet!\n";
        return resp.done();
    }

    auto on_gamestatus(const restinio::request_handle_t& req, rr::route_params_t) const
    {
        auto resp = init_resp(req->create_response());
        GameStateOgData Resp;
        Resp.state = to_string(m_status);
        Resp.distance = m_distance;
        resp.set_body(json_dto::to_json(Resp));
        return resp.done();
    }

private:
    Status m_status = Status::INACTIVE;
    double m_distance = 12.5; // Hardkodet testafstand.

    struct GameStateOgData {
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
        resp.append_header("Content-Type", "application/json; charset=utf-8");
        return resp;
    }
};

auto make_router(GameHandler& handler)
{
    auto router = std::make_unique<router_t>();
    auto by = [&](auto method) {
        using namespace std::placeholders;
        return std::bind(method, &handler, _1, _2);
    };
    router->http_get("/gamestatus", by(&GameHandler::on_gamestatus));
    router->http_post("/gamestart", by(&GameHandler::on_gamestart));
    router->non_matched_request_handler([](auto req) {
        return req->create_response(restinio::status_not_found())
            .connection_close().done();
    });
    return router;
}

int main()
{
    GameHandler handler;
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
