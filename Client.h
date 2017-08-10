struct Client
{
	int identifier;
	pthread_t thread;
	int socket;
	struct sockaddr_in clientAddress;
	string name;
};

string getStringIdentifier(Client client) {
	char buffer[80] = {};
	sprintf(buffer, "%d", client.identifier);
	return string(buffer);
}

