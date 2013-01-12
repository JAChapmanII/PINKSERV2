#ifndef PERMISSION_HPP
#define PERMISSION_HPP

#include <string>
#include <map>

namespace Permission {
	enum Permission { Read = 0x1, Write = 0x2, Execute = 0x4, Modify = 0x8 };
	std::string asString(Permission p);

	extern uint8_t all;
	extern uint8_t rx;

	bool hasPermission(Permission p, std::string nick, std::string variable);
	void ensurePermission(Permission p, std::string nick, std::string variable);
}
namespace PermissionType {
	enum PermissionType { Owner = 0x1, Admin = 0x2, User = 0x4, SUser = 0x8 };
}

// TODO struct -> class?
struct PermissionFragment {
	PermissionType::PermissionType type;
	std::string nick;
	uint8_t perms;

	static PermissionFragment parse(std::string pstr);
};

// TODO struct -> class?
struct Permissions {
	uint8_t admin;
	std::string owner_nick;
	uint8_t owner;
	std::map<std::string, uint8_t> suser;
	uint8_t user;
	uint8_t sticky;

	Permissions();
	Permissions(std::string iowner);
	Permissions(Permission::Permission p);

	static Permissions parse(std::string perms);

	void apply(PermissionFragment pfrag);
	// TODO: what the hell is level?
	bool allowed(Permission::Permission p, std::string nick, int level = 5);
};


#endif // PERMISSION_HPP
