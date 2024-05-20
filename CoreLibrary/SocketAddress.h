#pragma once

class SocketAddress
{
public:
	SocketAddress() = default;
	SocketAddress(SOCKADDR_IN socketAddress);
	SocketAddress(std::wstring ip, uint16_t port);

	SOCKADDR_IN&	GetSocketAddress() { return _socketAddress; }
	std::wstring	GetIpAddress();
	uint16_t		GetPort() { return ::ntohs(_socketAddress.sin_port); }

public:
	static IN_ADDR	IpToAddress(const WCHAR* ip);

private:
	SOCKADDR_IN		_socketAddress = {};
};
