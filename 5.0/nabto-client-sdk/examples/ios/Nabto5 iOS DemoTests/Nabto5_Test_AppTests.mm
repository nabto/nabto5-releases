//
//  Nabto5_Test_AppTests.m
//  Nabto5 iOS DemoTests
//
//  Created by Ulrik Gammelby on 08/05/2019.
//  Copyright © 2019 Nabto ApS. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "nabto/nabto_client.h"



@interface Nabto5_Test_AppTests : XCTestCase

@end

@implementation Nabto5_Test_AppTests

const char clientPrivateKey_[] =
"(-----BEGIN EC PARAMETERS-----\r\n"
"BggqhkjOPQMBBw==\r\n"
"-----END EC PARAMETERS-----\r\n"
"-----BEGIN EC PRIVATE KEY-----\r\n"
"MHcCAQEEIDDisQBQEJ+xjPiJakkIjoBxddnMvu29UiLcHEn0gy89oAoGCCqGSM49\r\n"
"AwEHoUQDQgAEZo/RZ0fxqZoqP4cc6jsKcs60NwoE7mP6FZEMw0aaJB5IgVeSYV7j\r\n"
"cbVaAwNUBYXiDoq0MaZFM0hud1V5uQ054Q==\r\n"
"-----END EC PRIVATE KEY-----\r\n";


const char clientPublicKey_[] =
"(-----BEGIN CERTIFICATE-----\r\n"
"MIIBajCCARCgAwIBAgIJAKLte4uCfxFGMAoGCCqGSM49BAMCMBAxDjAMBgNVBAMM\r\n"
"BW5hYnRvMB4XDTE4MDgxNDA3MjgwOVoXDTQ4MDgwNjA3MjgwOVowEDEOMAwGA1UE\r\n"
"AwwFbmFidG8wWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARmj9FnR/Gpmio/hxzq\r\n"
"OwpyzrQ3CgTuY/oVkQzDRpokHkiBV5JhXuNxtVoDA1QFheIOirQxpkUzSG53VXm5\r\n"
"DTnho1MwUTAdBgNVHQ4EFgQUITzaBNogAsQ8TYH8ggg2SO2YPR8wHwYDVR0jBBgw\r\n"
"FoAUITzaBNogAsQ8TYH8ggg2SO2YPR8wDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjO\r\n"
"PQQDAgNIADBFAiEAjp8BTB1b3myh7GippV/0VrqghIhHuLzYXSoCdtSglPECIH0S\r\n"
"tCx50TsEh2rqC1wUAj1tc4AK8GnWh+LHaSRrTVIl\r\n"
"-----END CERTIFICATE-----\r\n";

const char* server_ = "http://localhost";
const char* deviceId_ = "testdevice";
const char* productId_ = "test";


- (void)setUp {
    const char* envServer = getenv("CLIENT_LB_HOST");
    if (envServer) {
        server_ = envServer;
    }
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    XCTAssert(true);
}

- (void)testNabtoVersion {
    NSString* version = [NSString stringWithUTF8String:nabto_client_version()];
    XCTAssertNotNil(version);
    XCTAssertTrue([version containsString:@"5"]);
}

- (void)testPrepareNabtoConnection {
    NabtoClientContext* context = nabto_client_context_new();
    NabtoClientConnection* connection = nabto_client_connection_new(context);
    NabtoClientError ec;
    ec = nabto_client_connection_set_server_url(connection, server_);
    
    ec = nabto_client_connection_set_product_id(connection, productId_);
    ec = nabto_client_connection_set_device_id(connection, deviceId_);
    
    ec = nabto_client_connection_set_public_key(connection, clientPublicKey_);
    
    ec = nabto_client_connection_set_private_key(connection, clientPrivateKey_);
    
    XCTAssertTrue(ec == NABTO_CLIENT_OK);
}

- (void)testSetBadPrivateKey {
    NabtoClientContext* context = nabto_client_context_new();
    NabtoClientConnection* connection = nabto_client_connection_new(context);
    NabtoClientError ec;
    ec = nabto_client_connection_set_private_key(connection, "bad key");    
    XCTAssertTrue(ec == NABTO_CLIENT_OK);
}


@end
