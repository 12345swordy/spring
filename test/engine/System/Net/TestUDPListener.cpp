
#include "System/Net/UDPListener.h"
#include "System/Log/ILog.h"

#include <catch_amalgamated.hpp>

namespace streflop {
	template<typename T> inline void streflop_init() {
		// Do nothing by default, or for unknown types
	}
}

class SocketTest {
public:
	SocketTest(){
	}
	void TestPort(int port, bool result = true)
	{
#ifndef NO_IPV6
		const std::string& err = netcode::UDPListener::TryBindSocket(port, socket, "::");
#else
		const std::string& err = netcode::UDPListener::TryBindSocket(port, socket, "127.0.0.1");
#endif
		INFO(err);
		CHECK(err.empty() == result);

	}
	void TestHost(const char* address, bool result = true)
	{
		const std::string& err = netcode::UDPListener::TryBindSocket(11111, socket, address);
		INFO(err);
		CHECK(err.empty() == result);
	}
private:
	std::shared_ptr<asio::ip::udp::socket> socket;
};

TEST_CASE("TryBindSocket")
{
	SocketTest t;

	// IP v4 & v6 addresses
	LOG("\nIP v4 & v6 addresses");
	t.TestHost("127.0.0.1");
	t.TestHost("0.0.0.0");
#ifndef NO_IPV6
	t.TestHost("::");
	t.TestHost("::1");

	// badly named invalid IP v6 addresses
	LOG("\nbadly named invalid IP v6 addresses");
	t.TestHost("::2", false);
	t.TestHost("fe80::224:1dff:fecf:df44/64", false);
	t.TestHost("fe80::224:1dff:fecf:df44", false);
	t.TestHost("::224:1dff:fecf:df44/64", false);
	t.TestHost("fe80::", false);
	t.TestHost("fe80", false);
	/*
		FIXME: for some reason this test works on windows (binds to ipv4 0.0.0.224) and fails on linux/osx

		:8 is reserved. :96 was IPv4 compatible IPv6 addresses, so ::2 would be 0.0.0.2.
		No implementation is required to support this scheme any longer. That's why ::2 doesn't work
		::ffff:x:y/96 is new IPv4-Mapped IPv6 Address
		http://tools.ietf.org/html/rfc4291 sections 2.5.5.1 and 2.5.5.2
	*/
	//	TestHost("224:1dff:fecf:df44/64", false);
#endif

	// host-names
	LOG("\nhost-names");
	t.TestHost("localhost");
	t.TestHost("local.lan", false);
	t.TestHost("google.com", false);

	// normal ports
	LOG("\nnormal ports");
	t.TestPort(1024);
	t.TestPort(11111);
	t.TestPort(32000);
	t.TestPort(65535);

	// special ports (reserved for core services)
/*
	t.TestPort(0, false); //port 0 binds to a random port
	t.TestPort(1, false); // <1024 usually requires root permissions
	t.TestPort(128, false);
	t.TestPort(1023, false);
*/
	// out-of-range ports
	LOG("\nout-of-range ports");
	t.TestPort(65536, false);
	t.TestPort(65537, false);
	t.TestPort(-1, false);
}

