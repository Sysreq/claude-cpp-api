import Claude;

int main() {
    Claude::API client(YOUR API KEY);

    auto transaction1 = client.Create<Transaction>();
    auto msg1 = transaction1->Create<Message>();
    msg1->Create<Fields::TextContent>( Fields::Text { "Hey Claude!"} );

    auto msg2 = transaction1->Create<Message>();
    msg2->Set<Fields::Role>("assistant");
    msg2->Create<Fields::TextContent>(Fields::Text{ "What up bro, just plotting to" });

    client.Send(transaction1);

    std::cout << "Done\n";
	
    return 0;
}