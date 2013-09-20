package service;

option cc_generic_services = true;

message EchoRequest {
        optional string msg = 1;
}

message EchoResponse {
        optional string msg = 1;
}

service EchoService {
        rpc Echo (EchoRequest) returns (EchoResponse);
}

