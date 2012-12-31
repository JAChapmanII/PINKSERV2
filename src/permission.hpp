#ifndef PERMISSION_HPP
#define PERMISSION_HPP

#include <string>
#include <map>

namespace Permission {
	enum Permission { Execute = 0x1, Append = 0x2, Write = 0x4, Modify = 0x8 };
	static uint8_t xPermissions = Permission::Execute;
	static uint8_t xawPermissions = xPermissions |
		Permission::Append | Permission::Write;
	static uint8_t xawmPermissions = xawPermissions | Permission::Modify;
}
namespace PermissionType {
	enum PermissionType { Admin, Owner, User, SUser };
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

	Permissions();

	static Permissions parse(std::string perms);

	void apply(PermissionFragment pfrag);
	bool allowed(Permission::Permission p, std::string nick, int level = 5);
};

bool hasPermission(Permission::Permission p, std::string nick, std::string variable, int level = 5);
// TODO: ensurePermission. Does above, throws appropriate exception on failure

#endif // PERMISSION_HPP
