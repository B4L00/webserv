#include "RegisteredUsers.hpp"

RegisteredUsers::RegisteredUsers()
{}

RegisteredUsers::~RegisteredUsers()
{}

const std::map<std::string, std::string> &RegisteredUsers::getDb() const { return _db; }

void RegisteredUsers::addDb(std::string &str)
{
	int pos;
	int end_pos;
	std::string nom;
	std::string password;

	if (str.find("nom=") != std::string::npos && str.find("password=") != std::string::npos)
	{
		pos = 4;
		end_pos = str.find("&");
		nom = str.substr(pos, end_pos - pos);

		pos = str.find("password=");
		pos += 9;
		password = str.substr(pos);

		_db.insert(std::pair<std::string, std::string>(nom, password));
		std::cout << "size: " << _db.size() << std::endl;
	}
}

bool RegisteredUsers::authenticate(std::string str)
{
	int pos;
	int end_pos;
	std::string nom;
	std::string password;

	if (str.find("nom=") != std::string::npos && str.find("password=") != std::string::npos)
	{
		pos = 4;
		end_pos = str.find("&");
		nom = str.substr(pos, end_pos - pos);

		pos = str.find("password=");
		pos += 9;
		password = str.substr(pos);

		std::map<std::string, std::string>::const_iterator it = _db.find(nom);

		if (it != _db.end() && it->second == password)
			return (true);
	}
	return (false);
}