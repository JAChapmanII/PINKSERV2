#include "permission.hpp"
using std::string;
using std::map;

#include <vector>
using std::vector;

#include <algorithm>
using std::transform;

#include "util.hpp"
using util::split;
using util::trimWhitespace;
using util::contains;
using util::startsWith;
using util::toOrdinal;

#include "global.hpp"
#include "variable.hpp"
#include "varstore.hpp"

uint8_t Permission::all = Read | Write | Execute | Modify;
uint8_t Permission::rx = Read | Execute;

string Permission::asString(Permission p) {
	switch(p) {
		case Read: return "read from";
		case Write: return "write to";
		case Execute: return "execute";
		case Modify: return "modify";
		default: return "(undefined-permission)";
	}
	return "(undefined-permission)";
}

PermissionFragment PermissionFragment::parse(string pstr) {
	PermissionFragment pf = { PermissionType::User, "", 0x0 };

	// split this part on "=",
	vector<string> frags = split(pstr, "=");
	// there need to be two terms
	if(frags.size() != 2)
		throw pstr + " is malformed";
	// trim extra whitespace from all tokens
	transform(frags.begin(), frags.end(), frags.begin(), trimWhitespace);

	// make sure there aren't wonky letters in the perms part
	static string validPermissionCharacters = "rwxm0";
	for(auto c : frags[1])
		if(!contains(validPermissionCharacters, (c)))
			throw string(1, c) + " is not a valid permission";

	// determine the actual permissions value
	if(contains(frags[1], 'r'))
		pf.perms |= Permission::Read;
	if(contains(frags[1], 'w'))
		pf.perms |= Permission::Write;
	if(contains(frags[1], 'x'))
		pf.perms |= Permission::Execute;
	if(contains(frags[1], 'm'))
		pf.perms |= Permission::Modify;

	// apply it to the proper thing
	if(frags[0].length() == 1) {
		switch(frags[0][0]) {
			case 'o': pf.type = PermissionType::Owner; break;
			case 'a': pf.type = PermissionType::Admin; break;
			case 'u': pf.type = PermissionType::User; break;
			default:
				throw string(1, frags[0][0]) + " is not valid identifier";
		}
	} else {
		if(!startsWith(frags[0], "n:"))
			throw frags[0] + " is a bad identifier";
		string nick = frags[0].substr(2);
		pf.type = PermissionType::SUser;
		pf.nick = nick;
	}

	// return the built fragment
	return pf;
}

Permissions::Permissions() : admin(Permission::all),
		owner_nick(""), owner(Permission::all),
		suser(), user(Permission::all), sticky(0x0) {
}
Permissions::Permissions(string iowner) : admin(Permission::all),
		owner_nick(iowner), owner(Permission::all),
		suser(), user(Permission::rx), sticky(0x0) {
}
Permissions::Permissions(Permission::Permission p) :
		admin(Permission::all),
		owner_nick(), owner(p),
		suser(), user(p), sticky(0x0) {
}

Permissions Permissions::parse(string perms) {
	Permissions p;

	// split perms into parts by ",", trim extra whitespace
	vector<string> parts = makeList(perms);
	uint8_t maxUserp = 0x0;
	for(unsigned i = 0; i < parts.size(); ++i) {
		string fragOrdinal = toOrdinal(i + 1) + " fragment";
		PermissionFragment pf = PermissionFragment::parse(parts[i]);
		p.apply(pf);
		if(pf.type == PermissionType::SUser)
			maxUserp |= pf.perms;
	}

	// check for heirarchy conflicts
	if((p.admin | p.owner) != p.admin)
		throw (string)"owner cannot have more powers than admins";
	if((p.owner | p.user) != p.owner)
		throw (string)"users cannot have more powers than the owner";
	if((p.owner | maxUserp) != p.owner)
		throw (string)"no specific user can have more powers than the owner";
	if((p.admin | maxUserp) != p.admin)
		throw (string)"no specific user can have more powers than an admin";

	// return the built permissions object
	return p;
}

void Permissions::apply(PermissionFragment pfrag) {
	switch(pfrag.type) {
		case PermissionType::Admin: this->admin = pfrag.perms; break;
		case PermissionType::Owner: this->owner = pfrag.perms; break;
		case PermissionType::User: this->user = pfrag.perms; break;
		case PermissionType::SUser:
			this->suser[pfrag.nick] = pfrag.perms;
			break;
		default:
			throw (string)"invalid permission type";
	}
}
bool Permissions::allowed(Permission::Permission p, string nick, int level) {
	uint8_t fperms = 0x0;
	int mlevel = 0;

	// either the user is specified directly, or they're just a user
	if(contains(this->suser, nick)) {
		fperms |= this->suser[nick];
	} else {
		fperms |= this->user;
		mlevel = 1;
	}

	// if the user is the owner, add in their permisssions
	if(this->owner_nick == nick) {
		fperms |= this->owner;
		mlevel = 2;
	}

	// if the user is an admin, add in admin permissions
	if(contains(global::vars.getList("bot.admins"), nick)) {
		fperms |= this->admin;
		mlevel = 3;
	}

	// if it's not a permission issue, don't worry about level
	if(p != Permission::Modify)
		return p & fperms;

	// normal users are not allowed to have modify permissions
	if(level < 2)
		return false;
	// make sure the user has permissions and is at a higher level
	return (p & fperms) && (mlevel > level);
}

bool Permission::hasPermission(Permission p, string nick, string variable) {
	// the bot owner can do whatever they want
	if(global::vars.getString("bot.owner") == nick)
		return true;

	Permissions perms = global::vars.get(variable).permissions;
	return perms.allowed(p, nick);
}
void Permission::ensurePermission(Permission p, std::string nick, std::string variable) {
	if(hasPermission(p, nick, variable))
		return;
	throw nick + " does not have permission to " + asString(p) + " " + variable;
}

