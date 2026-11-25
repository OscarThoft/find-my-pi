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
std::string GameHandler::to_string(gameStatus::Status status)
{
    switch (status) {
        case gameStatus::Status::INACTIVE: return "Inactive";
        case gameStatus::Status::RUNNING:  return "Running";
        case gameStatus::Status::FOUND:    return "Found";
        default:                          return "unknown";
    }
}

// using traits (bruges af run() og websocket (on live update))
using traits_t = restinio::traits_t<
        restinio::asio_timer_manager_t,
        restinio::single_threaded_ostream_logger_t,
        router_t>;


void GameHandler::run()
{
    restinio::run(
        m_ioContext,
        restinio::on_this_thread<traits_t>()
            .address("0.0.0.0")
            .port(9000)
            .request_handler(make_router(*this))
    );
}

// ON GAME START FUNCTION
restinio::request_handling_status_t GameHandler::on_gamestart( const restinio::request_handle_t& req,
        rr::route_params_t params)
{
    // kalder startgame function i gemmercontroller.cpp 
    m_controller.startGame();

    // tekst via websocket til søgere forbundet til /chat
    sendMessage("Spillet starter nu");

    // HTTP svar 
    auto resp = req->create_response(restinio::status_no_content());

    return resp.done();
}


// ON GAME STATUS FUNCTION
// den her sendes når client beder om det og er IKKE websocket pt. 
// er udkommenteret fordi /chat gør det automatisk 
/*
restinio::request_handling_status_t GameHandler::on_gamestatus(
        const restinio::request_handle_t& req, rr::route_params_t params) const
{
    auto resp = init_resp(req->create_response()); 

    auto Resp = m_controller.requestData();

    resp.set_body(json_dto::to_json(Resp));
    return resp.done();
}
*/


// Router
std::unique_ptr<router_t> make_router(GameHandler& handler)
{
    auto router = std::make_unique<router_t>();

    auto by = [&](auto method) 
    {
        using namespace std::placeholders;
        return std::bind(method, &handler, _1, _2);
    };

    // midlertidigt udkommenteret:
    // router->http_get(
       // "/gamestatus", by(&GameHandler::on_gamestatus));

    router->http_post(
        "/gamestart", by(&GameHandler::on_gamestart));

    // web socket 
    router->http_get(
        "/chat", by(&GameHandler::on_live_update));

    // Hvis ingen route matcher
    router->non_matched_request_handler([](auto req) 
    {
        return req->create_response(restinio::status_not_found())
            .connection_close()
            .done();
    });

    return router;
}

// ON LIVE UPDATE FUNCTION
// kan sættes til at sende afstand hvert 2. sek eller lignende. 
restinio::request_handling_status_t GameHandler::on_live_update(
    const restinio::request_handle_t& req, rr::route_params_t params)
{
    if (restinio::http_connection_header_t::upgrade == req->header().connection()) 
	    {
			// wsh står for WebSocket handle, og repræsentere den enklete forbindelse
			auto wsh = rws::upgrade<traits_t> (*req, rws::activation_t::immediate,[ this ](auto wsh, auto m) 
			{
				if (rws::opcode_t::text_frame == m->opcode() 
					|| rws::opcode_t::binary_frame == m->opcode() 
					|| rws::opcode_t::continuation_frame == m->opcode()) 
				{
					wsh->send_message(*m);  // det her "ekkoer"
				} 
				else if (rws::opcode_t::ping_frame == m->opcode())
				{
					auto resp = *m;
					resp.set_opcode(rws::opcode_t::pong_frame); // check wether connection is alive, when a ping fram is reveiced 
					wsh->send_message(resp);
				}
				else if (rws::opcode_t::connection_close_frame == m->opcode())
				{
					m_registry.erase(wsh->connection_id());
				}
			});

			m_registry.emplace(wsh->connection_id(), wsh); // gemmer  forbindelse til den nye søger


            // henter nyeste data og status
            // auto resp = m_controller.requestData();

            // // sender STRAX afstand og status når søger forbinder
            // vi kan evt lave en if status = inactiv, kun send status, if status runing send status og afstand
            // wsh->send_message(rws::final_frame, rws::opcode_t::text_frame, json_dto::to_json(resp));
            
			return restinio::request_accepted();
	    }

	return restinio::request_rejected();
}

// send ny distance 
void GameHandler::sendData(double data) {
    restinio::asio_ns::post(m_ioContext, [this, data]() {
        sendMessage(json_dto::to_json(distance));
    });
}

// send string beskeder 
void GameHandler::sendData(const std::string& message) {
    restinio::asio_ns::post(m_ioContext, [this, message]() {
        sendMessage(json_dto::to_json(message));
    });
}

void GameHandler::sendMessage(const std::string& message)
{ 
	//Loop through all connected clients and send the message to each one
        //for (auto [k, v] : m_registry)

    for(const auto& [id, wsh] : m_registry)
    {
		wsh->send_message(rws::final_frame, rws::opcode_t::text_frame, message);
	}
}