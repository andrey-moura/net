#include <web_client.hpp>

#include <networking.hpp>
#include <json.hpp>

using namespace uva;
using namespace networking;

uva::networking::basic_web_client::basic_web_client(std::string __host)
{
    if(!uva::networking::is_initialized()) {
        uva::networking::init(run_mode::async);
    }

    bool https = false;

    if(__host.starts_with("http://"))
    {
        m_protocol = "http";
    } else if(__host.starts_with("https://"))
    {
        m_protocol = "https";
    } else {
        throw std::runtime_error(std::format("invalid protocol for '{}'", __host));
    }


    m_host = __host.substr(m_protocol.size()+3); //+ "://"
}

uva::networking::basic_web_client::~basic_web_client()
{
    if(m_socket.is_open()) {
        m_socket.close();
    }
}

void uva::networking::basic_web_client::connect_if_is_not_open()
{
    if(!m_socket.is_open())
    {
        error_code ec = m_socket.connect(m_protocol, m_host);

        if(!ec) {
            if(m_socket.needs_handshake()) {
                ec = m_socket.client_handshake();
            }
        }

        if(ec)
        {
            on_connection_error(ec);
        }
    }
}

void uva::networking::basic_web_client::get(const std::string &route, std::map<std::string, var> params, std::map<std::string, var> headers, const std::string &body, std::function<void(http_message)> on_success, std::function<void(error_code&)> on_error)
{
    connect_if_is_not_open();

    if(m_socket.is_open()) {
        headers["Content-type"] = "text/plain; charset=utf-8";
        write_http_request(m_socket, m_host, route, params, headers, body, [on_success,this]() {
            on_success(read_http_response(m_socket));
        }, on_error);
    }
}

void uva::networking::basic_web_client::get(const std::string &route, std::map<std::string, var> params, std::map<std::string, var> headers, const var &body, std::function<void(http_message)> on_success, std::function<void(error_code &)> on_error)
{
    connect_if_is_not_open();

    if(m_socket.is_open()) {
        headers["Content-type"] = "application/json; charset=utf-8";
        write_http_request(m_socket, m_host, route, params, headers, json::enconde(body), [on_success,this]() {
            on_success(read_http_response(m_socket));
        }, on_error);
    }
}

void uva::networking::basic_web_client::on_connection_error(const uva::networking::error_code &ec)
{
    throw std::runtime_error(std::format("An error occurred while trying to establish a connection: {}", ec.message()));
}