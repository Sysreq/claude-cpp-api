import Claude;

int main() {
    Claude::API client(YOUR API KEY);

    auto transaction1 = client.Create<Transaction>();
    auto msg1 = transaction1->Create<SimpleMessage>();
    msg1->Set<Message>("Hey Claude! How is it going?");
    client.Send(transaction1);

    auto transaction2 = client.Create<Transaction>();
    auto msg_list= transaction2->Create<MessageList>();
    auto new_line = msg_list->Create<Line>(Text { "How many r's in the word strawberry?"});
    client.Send(transaction2);
	
    return 0;
}