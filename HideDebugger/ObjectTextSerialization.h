// Provides text serialization of arbitrary objects to be sent across the network, process boundaries, etc.

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#pragma warning(disable : 4512)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(default : 4512)
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace uberstealth {
	const int HeaderLength = 8;

	template <typename structT, typename recvHandlerT>
	void deserialize(structT& item, recvHandlerT recv) {
		// First read header.
		char inboundHeader[HeaderLength];
		recv(boost::asio::buffer(inboundHeader));

		istringstream is(std::string(inboundHeader, HeaderLength));
		size_t inboundDataSize = 0;
		if (!(is >> hex >> inboundDataSize))
			throw std::runtime_error("Error while processing inbound data: invalid header received");

		// Header is correct, so read data.
		std::vector<char> inboundData(inboundDataSize);
		recv(boost::asio::buffer(inboundData));
		try	{
			std::string archiveString(&inboundData[0], inboundData.size());
			std::istringstream archiveStream(archiveString);
			boost::archive::text_iarchive archive(archiveStream);
			archive >> item;
		}
		catch (const std::exception&) {
			throw std::runtime_error("Error while processing inbound data: unable to deserialize protocol buffer");
		}
	}

	template <typename structT, typename transportT>
	void serialize(const structT& item, typename transportT) {
		ostringstream archiveStream;
		boost::archive::text_oarchive archive(archiveStream);
		archive << item;
		std::string outboundData = archiveStream.str();

		std::ostringstream headerStream;
		headerStream << setw(HeaderLength) << hex << outboundData.size();
		if (!headerStream || headerStream.str().size() != HeaderLength)
			throw std::runtime_error("Error while processing outbound data: unable to serialize protocol buffer");

		// Gather buffers and write at once.
		std::vector<boost::asio::const_buffer> buffers;
		std::string headerString = headerStream.str();
		buffers.push_back(boost::asio::buffer(headerString));
		buffers.push_back(boost::asio::buffer(outboundData));
		// write data to socket/whatever
		//boost::asio::write(socket_, buffers);
		//send(buffers);
	}
}