printHook1_ = new Q3Hook(G_PRINT, EXECUTE_TYPE_BEFORE, [](Q3Hook *hook) {
	EchoRequest *echoArgs = new EchoRequest();
	NullResponse *echoReply = new NullResponse();

	std::string msg((const char *)hook->getParamPtr(0));
	fix_utf8_string(msg);
	echoArgs->set_msg(msg);

	EXECUTE_ASYNC(&EchoService_Stub::Echo, gRecordsystem->apiEchoService, echoArgs, echoReply, [](Message *msg, rpc::Error *err) {
		if(!err->IsNil()) {
			printf("echoStub.Echo.Error: %s\n", err->String().c_str());
		}else if(!((NullResponse *)msg)->msg().empty())
			printf("echoStub.Echo.Return: %s\n", ((NullResponse *)msg)->msg().c_str());
	});
});
vm_syscall_->addHook(printHook1_);



	testHook1_ = new Q3Hook(GAME_CLIENT_CONNECT, EXECUTE_TYPE_BEFORE, [](Q3Hook *hook) {
		gRecordsystem->GetSyscalls()->Printf(va("CL: %i | firstTime: %i | isBot: %i\r\n", hook->getParam(0), hook->getParam(1), hook->getParam(2)));
		hook->setReturnVMA("blabal bad hook user");
	});

	this->addHook(testHook1_);



