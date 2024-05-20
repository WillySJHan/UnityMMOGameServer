#include "pch.h"
#include "SocketAddress.h"

SocketAddress::SocketAddress(SOCKADDR_IN socketAddress) : _socketAddress(socketAddress)
{
}

SocketAddress::SocketAddress(std::wstring ip, uint16_t port)
{
	::memset(&_socketAddress, 0, sizeof(_socketAddress));
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_addr = IpToAddress(ip.c_str());
	_socketAddress.sin_port = ::htons(port);
}

std::wstring SocketAddress::GetIpAddress()
{
	wchar_t buffer[100];
	::InetNtopW(AF_INET, &_socketAddress.sin_addr, buffer, sizeof(buffer) / sizeof(wchar_t));
	return std::wstring(buffer);
}

IN_ADDR SocketAddress::IpToAddress(const WCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
