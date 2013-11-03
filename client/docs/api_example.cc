case GAME_CLIENT_CONNECT: // return functions!
	{ // need scope because variable decleration!
		ClientInfoRequest *itemReq = new ClientInfoRequest();
		NullResponse *itemRes = new NullResponse();
		itemReq->set_playernum(arg0);
		auto ttt = std::bind(&Recordsystem::test, this, _1, _2);

		EXECUTE_ASYNC(&Q3dfApi_Stub::ClientConnected, Q3dfApi_, itemReq, itemRes, ttt);
	}
		
		
void Recordsystem::test(Message *res, rpc::Error *err) {
	if(!err->IsNil()) {
		gRecordsystem->GetSyscalls()->Printf(va("echoStub.Echo.Error: %s\n", err->String().c_str()));
	}
}
