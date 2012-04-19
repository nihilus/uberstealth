#pragma once

// Provides serialization of arbitrary objects over boost asio sockets

#include <iostream>
#include <sstream>
#pragma warning(disable : 4512)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(default : 4512)
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace uberstealth {

	class RemoteStealthConnection {
	public:
		RemoteStealthConnection(boost::asio::io_service& ioService) :
			socket_(ioService) {}

		boost::asio::ip::tcp::socket& socket() { return socket_; }

		template <typename structT>
		void syncRead(structT& item) {
			char inboundHeader[HeaderLength];
			boost::asio::read(socket_, boost::asio::buffer(inboundHeader));
			std::istringstream is(std::string(inboundHeader, HeaderLength));
			size_t inboundDataSize = 0;
			if (!(is >> std::hex >> inboundDataSize)) {
				throw std::runtime_error("Error while processing inbound data: received invalid header.");
			}

			std::vector<char> inboundData(inboundDataSize);
			boost::asio::read(socket_, boost::asio::buffer(inboundData));
			try	{
				std::string archiveString(&inboundData[0], inboundData.size());
				std::istringstream archiveStream(archiveString);
				boost::archive::text_iarchive archive(archiveStream);
				archive >> item;
			}
			catch (const std::exception&) {
				throw std::runtime_error("Error while processing inbound data: unable to deserialize protocol buffer.");
			}
		}
		
		template <typename structT>
		void syncWrite(const structT& item)	{
			std::ostringstream archiveStream;
			boost::archive::text_oarchive archive(archiveStream);
			archive << item;
			std::string outboundData = archiveStream.str();

			std::ostringstream headerStream;
			headerStream << std::setw(HeaderLength) << std::hex << outboundData.size();
			if (!headerStream || headerStream.str().size() != HeaderLength) {
				throw std::runtime_error("Error while processing outbound data: unable to serialize protocol buffer.");
			}

			// Gather buffers and write at once.
			std::vector<boost::asio::const_buffer> buffers;
			std::string headerString = headerStream.str();
			buffers.push_back(boost::asio::buffer(headerString));
			buffers.push_back(boost::asio::buffer(outboundData));
			boost::asio::write(socket_, buffers);
		}

	private:
		boost::asio::ip::tcp::socket socket_;
		static const int HeaderLength = 8;
	};
}