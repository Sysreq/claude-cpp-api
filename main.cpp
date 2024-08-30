import Claude;

int main() {
    Claude::API client(YOUR API KEY);

    auto transaction1 = client.Create<Objects::Transaction>();
    auto msg1 = transaction1->Create<Objects::Message>();
    msg1->Set<Fields::Role>("user");
    msg1->Create<Objects::TextContent>( Fields::Text { "Hey Claude!"} );
    msg1->Create<Objects::TextContent>(Fields::Text{ "How are you today?" });

    auto msg2 = transaction1->Create<Objects::Message>();
    msg2->Set<Fields::Role>("assistant");
    msg2->Create<Objects::TextContent>(Fields::Text{ "What up bro, just plotting to" });

    client.Send(transaction1);

    std::cout << "Done\n";
	
    return 0;
}