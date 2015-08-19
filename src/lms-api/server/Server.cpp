/*
 * Copyright (C) 2013 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <utility>

#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>

#include "logger/Logger.hpp"

#include "Server.hpp"

namespace LmsAPI {
namespace Server {

Server::Server(const endpoint_type& bindEndpoint,
		boost::filesystem::path certPath,
		boost::filesystem::path privKeyPath,
		boost::filesystem::path dhPath,
		Wt::Dbo::SqlConnectionPool& connectionPool)
:
_acceptor(_ioService, bindEndpoint, true /*SO_REUSEADDR*/),
_connectionManager(),
_context(boost::asio::ssl::context::tlsv1_server),
_connectionPool(connectionPool)
{
	_ioService.setThreadCount(1); // TODO parametrize

	_context.set_options( boost::asio::ssl::context::default_workarounds // TODO check this thing
			| boost::asio::ssl::context::single_dh_use
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::no_sslv3
			);
//	context_.set_password_callback(boost::bind(&server::get_password, this));
	_context.use_certificate_chain_file(certPath.string());
	_context.use_private_key_file(privKeyPath.string(), boost::asio::ssl::context::pem);
	_context.use_tmp_dh_file(dhPath.string());
}

void
Server::start()
{
	// While the server is running, there is always at least one
	// asynchronous operation outstanding: the asynchronous accept call waiting
	// for new incoming connections.
	asyncAccept();

	_ioService.start();
}

void
Server::asyncAccept()
{
	std::shared_ptr<Connection> newConnection = std::make_shared<Connection>(_ioService, _context, _connectionManager, _connectionPool);

	_acceptor.async_accept(newConnection->getSocket(),
			boost::bind(&Server::handleAccept, this, newConnection, boost::asio::placeholders::error));
}

void
Server::handleAccept(std::shared_ptr<Connection> newConnection, boost::system::error_code ec)
{
	// Check whether the server was stopped before this
	// completion handler had a chance to run.
	if (!_acceptor.is_open())
	{
		return;
	}

	if (!ec)
	{
		_connectionManager.start(newConnection);

		// Accept another connection
		// TODO: add some limit?
		asyncAccept();
	}
	else
		LMS_LOG(MOD_REMOTE, SEV_ERROR) << "handleAccept: " << ec.message();
}


void
Server::stop()
{
	// The server is stopped by cancelling all outstanding asynchronous
	// operations.
	_acceptor.close();
	_connectionManager.stopAll();

	_ioService.stop();
}

} // namespace Server
} // namespace LmsAPI
